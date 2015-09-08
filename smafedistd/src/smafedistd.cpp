///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// smafedistd.cpp
//
// SpectralMind Audio Feature Extraction Distance Calculation Daemon
// Main file
// ------------------------------------------------------------------------
//
//
// Version $Id: smafequery.cpp 166 2009-03-24 21:25:13Z ewald $
//
//
//
//
//
//
///////////////////////////////////////////////////////////////////////////


// ------------------------------------------------------------------------
// includes

/** created by configure script */
#include "config.h"

#include "smafedaemon1.h"

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <ctime>
#include <vector>
//#include <unistd.h>
#include <algorithm>
#include <limits>
#include <string>

// for OpenMP
#include <omp.h>

#include "argtable2.h"

#include "smafeutil.h"
#include "smafeFeatureVectorClasses.h"
#include "smafestore_specific_include.h"
#include "smafeDistancesCalc.h"
#include "smafeopt.h"
#include "tLiveNNMessage.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>

using boost::asio::ip::tcp;



// ------------------------------------------------------------------------
// constants

// ------------------ debug / development constants (you do not want to change them)
/** DEBUG-mode:
 * - do not delete task record
 */
const bool SMAFEDISTD_DEBUG = false;
/** defensive mode: check if distance record exists before inserting
 * usually not necessary
 */
const bool SMAFEDISTD_DEFENSIVE_MODE = false;
/** Boost mode: store fvs in Hashmap */
const bool SMAFEDISTD_OPT = true;
/** Use topk or not
 !topk implies SMAFEDISTD_OPT being also enabled!*/
const bool SMAFEDISTD_USETOPK = true;




// ----------------------------------- program behaviour constants

/** Parameter p for generic L norm */
const double NORM_P = 2;

/** Memory limit for hash map */
//const size_t SMAFEDISTD_MEM_LIMIT = 1*1024*1024; // 1 GB
const size_t SMAFEDISTD_DEFAULT_MEM_LIMIT = 5 * 1024; // 1 GB
/** epsilon for available memory detection: if used memory (estimated) is more than
 * epsilon % below the limit we say that memory is available
 */
const float SMAFEDISTD_MEM_EPSILON = 0.01;

/** defualt polling interval in min */
const int DEFAULT_POLLING_INTERVAL = 10;
/** maximal value for distance */
const double DISTANCE_MORETHANMAX_VALUE = std::numeric_limits<double>::max();
/** minimal  value for distance
 * Assumption: all distances are >= 0 */
const double DISTANCE_LESSTHANMIN_VALUE = -1;
// mode constants
/** constant for jobs mode */
const int MODE_JOBSMODE = 1;
/** constant for live mode */
const int MODE_LIVEMODE = 2;
/** argument for jobs mode */
const std::string ARGUMENT_JOBSMODE = "--jobs";
/** argument for live mode*/
const std::string ARGUMENT_LIVEMODE = "--live";

/** program name */
const char PROGNAME[] = "smafedistd";

// ------------------------------------------------------------------------
// typedefs
/** Stores top k distances of all fvs */
typedef std::map<long, t_double_deque> tTop_ds;
/** smart pionter to socket */
typedef boost::shared_ptr<tcp::socket> socket_ptr;


// ------------------------------------------------------------------------
// global vars
/** mode */
int iMode = 0;
/** options */
Smafeopt* so;
/** identifier for this daemon process */
std::string daemonId;
/** poll interval in minutes */
int pollInterval = DEFAULT_POLLING_INTERVAL;
/** vector of fvtype_ids, if this parameter is given at command line.
 * <p>If this vector remains empty all fv types are calculated
 */
std::vector<long> fvtype_ids;
/** hashmap for storing feature vectors */
tFVMap fvbuffer;
/** vector of vectors to store top k distances of all fvs */
tTop_ds top_ds;
/** last featurevectortype id */
long lLastFVT_id = -1;
/** last distance type id */
long lLastDT_id = -1;
/** if daemon is to be terminated after next finished job
 * <p>This flag is set to true if term signal is caught */
bool b_should_terminate = false;
/** name of logfile */
std::string sLogfilename;
/** loglevel requested */
int loglevel_requested;
/** only print open tasks? */
bool bPrintStatsOnly = false;
/** limit for tasks (if no limit, the variable is < 0) */
int iRecLimit = -1;
/** counter for tasks */
int iRecCounter = 0;
/** true if bulk operation is enabled (ie, if specific index (-es) should be dropped and recreated */
bool bBulkOp = false;
/** true if initial run assumption is used */
bool bInitialRun = false;
/** This instance has dropped the index already? */
bool bIndexDropped = false;
/** current top k (initially top_k, might by reduced if memory is full) */
size_t top_k_cur;
/** offset for top distance values that are stored in top_ds.
 * <p>Example: if the worst 100 of the 500 top d values are stored (ie, values #400 to #499
 * of sorted d values), top_k_offset = 400, top_k_cur = 100 and top_k = 500
 */
size_t top_k_offset = 0;
/** Indictes how many last "top k" distance values can safely be removed from top_ds
 * without risking to lose information.
 * <p>Example: if only one distance value d1 (per track) is stored we cannot remove this one
 * in case that we get a new value d2 that is lower than the limit as is is highly probable
 * that there are many distances in the db between d1 and d2
 */
size_t top_k_safe_removable;
/** purge mode */
bool bPurgemode = false;
/** fvtype to use (if multiple) */
long lFvTypeLiveMode = -1;
/** use segment fvs?, used in live mode  */
bool bUseSegmFvs = false;
/** port */
int iPort;
/** Portion of fv to load at once. 0... all fVs at once */
long lFVPortion = 0;
/** mem limit in Mbyte */
size_t smafedistd_mem_limit = SMAFEDISTD_DEFAULT_MEM_LIMIT;
/** minimum distance for fvt/dt combination */
double dMinDist = DISTANCE_MORETHANMAX_VALUE;
/** maximum distance for fvt/dt combination
 * Assumption: all distances are >= 0 */
double dMaxDist = DISTANCE_LESSTHANMIN_VALUE;

#include "smafedaemon2.h"

/** Performs --list operation (query all fv and dist types and print it to stdout
 * Precondition: db-strings like strDbhost must have been set by cmd line arg processing function */
void doList() {
	try {
		SMAFE_STORE_DB_CLASS* db = NULL;
		// create db connection
		db = new SMAFE_STORE_DB_CLASS();
		db->openConnection(so->strDbhost, so->strDbport, so->strDbuser, so->strDbpwd, so->strDbname);

		SMAFELOG_FUNC(SMAFELOG_INFO, "");
		SMAFELOG_FUNC(SMAFELOG_INFO, "");

		SMAFELOG_FUNC(SMAFELOG_INFO, "FEATUREVECTORTYPEs in '" + stringify(so->strDbname) + "':");
		SMAFELOG_FUNC(SMAFELOG_INFO, "");
		SMAFELOG_FUNC(SMAFELOG_INFO, "id | name | version | dimx | dimy | parameters");
		SMAFELOG_FUNC(SMAFELOG_INFO, "--------------------------------------------------------------------------------------");
		std::vector<SmafeFVType_Ptr> fvtypes;
		db->getFeaturevectortypes(fvtypes);
		for (std::vector<SmafeFVType_Ptr>::iterator iter = fvtypes.begin(); iter
		< fvtypes.end(); iter++) {
			SmafeFVType dt = *(iter->get());
			SMAFELOG_FUNC(SMAFELOG_INFO, " " + stringify(dt.id) + " | " + dt.name + "\t| " + stringify(dt.version)+ "\t| " + stringify(dt.dimension_x)+ "\t| " + stringify(dt.dimension_y) + "\t| " + dt.parameters);
		}
		SMAFELOG_FUNC(SMAFELOG_INFO, "");

		SMAFELOG_FUNC(SMAFELOG_INFO, "DISTANCETYPEs in '" + stringify(so->strDbname) + "':");
		SMAFELOG_FUNC(SMAFELOG_INFO, "");
		SMAFELOG_FUNC(SMAFELOG_INFO, "id | name");
		SMAFELOG_FUNC(SMAFELOG_INFO, "---+----------------------------------------------------------------------------------");
		std::vector<tDistancetype> disttypes;
		db->getDistancetypes(disttypes);
		for (std::vector<tDistancetype>::iterator iter = disttypes.begin(); iter
		< disttypes.end(); iter++) {
			tDistancetype dt = *iter;
			SMAFELOG_FUNC(SMAFELOG_INFO, " " + stringify(dt.id) + " | " + dt.name);
		}
		SMAFELOG_FUNC(SMAFELOG_INFO, "");

		// close connection
		delete db;
	} catch (std::string& s) {
		SMAFELOG_FUNC(SMAFELOG_FATAL, s);
	}
}


/** Checks if a port is free or not
 * @return true if port is available, false otherwise*/
bool isPortAvailable(unsigned short port) {
	try {
		boost::asio::io_service io_service;
		tcp::iostream stream;
		tcp::endpoint endpoint(tcp::v4(), port);
		tcp::acceptor acceptor(io_service, endpoint);
	} catch (...) {
		return false;
	}
	return true;
}




/** Processes command line arguments using argtable library
 * **Note that in this sub, the config from the db is not yet set!**
 * <p>Postconditions: These vars are set</p>
 * <ul>
 * <li>daemonId
 * <li>pollInterval
 * <li>all 4 db related options
 * </ul>
 * @param argc number of command line arguments (from main())
 * @param argv array of c-strings (from main())
 */
void processCommandLineArguments(int argc, char* argv[]) {

	/* Define the allowable command line options, collecting them in argtable[] */
	struct arg_lit *mode_jobs = arg_lit1(NULL, "jobs", "Run in jobs mode");
	struct arg_str
	*arg_daemonID =
			arg_str0(
					NULL,
					"id",
					"IDENTIFIER",
					"Identifier for daemon instance. IDENTIFIER must not contain whitespace or special chars.");
	struct arg_int *arg_interval = arg_int0("i", "interval", "MINUTES",
			"Polling interval in minutes. Default is 10");
	struct arg_int
	*arg_fvtype_ids =
			arg_intn(
					"f",
					"fvtypes",
					"ID",
					0,
					99,
					"Feature vector type ids to be calculated, default is no restriction. Parameter may occur more than once.");
	struct arg_lit
	*arg_bulk =
			arg_lit0(
					NULL,
					"bulk",
					"Enable high performant bulk operation. Precondition: Empty distance model. NOTE: You must provide a db user with advanced privileges that may delete an index!");
	struct arg_lit
	*arg_initial =
			arg_lit0(
					NULL,
					"initial-run",
					"Assumes that this is the first run, ie., that the distance table is empty. Basically, the daemon will not query for existing distances, speeding up processing a lot. Do not use if you start multiple instances.");


	struct arg_str *arg_log = arg_str0("l", "log", "FILENAME",
			"Name of logfile. If not specified, a random name is chosen.");
	struct arg_int *arg_memlimit = arg_int0("m", "mem", "N",
			"Memory limit for this process in MB. Default is 5120 (5 GB)");
	struct arg_int *arg_tasklimit = arg_int0(NULL, "limit", "N",
			"Sets task limit. Default is no limit");
	struct arg_int *arg_verbose =
			arg_int0(
					"v",
					"verbosity",
					"0-6",
					"Set verbosity level (=log level). The lower the value the more verbose. Default is 3");
	struct arg_lit
	*arg_no_daemon =
			arg_lit0(NULL, "no-daemon",
					"Runs program as normal forground process (no forking). No logfile allowed");
	struct arg_lit *arg_stats = arg_lit0(NULL, "stats",
			"Print number of open tasks and exit");
	struct arg_lit *arg_purge = arg_lit0(NULL, "purge",
			"Delete superfluous distance records and exit");
	struct arg_str *arg_dbconf = arg_str0(NULL, "dbconf", "DATABASE-CONFIGURATION-FILE",
			"Specify file that contains database connection details");
	struct arg_lit *help = arg_lit0(NULL, "help", "Print this help and exit");
	struct arg_lit
	*list =
			arg_lit0(
					NULL,
					"list",
					"List all feature vector and distance types available in the specified database and exit");
	struct arg_end *end = arg_end(20);

	// argument list for jopbs mode
	void* argtable1[] = { mode_jobs, arg_daemonID, arg_no_daemon, arg_interval,
			arg_fvtype_ids, arg_bulk, arg_initial,
			arg_verbose, arg_dbconf,
			arg_log, arg_stats, arg_purge, arg_memlimit, arg_tasklimit, help,
			list, end };


	// additional options for live mode
	struct arg_lit *mode_live = arg_lit1(NULL, "live", "Run in live mode. NOTE: must be 1st argument!");
	struct arg_int *arg_port =
			arg_int0(
					"p",
					"liveport",
					"PORT",
					(std::string("The port the daemon should listen on. Default is ") + stringify(SMAFELIVEMODE_STDPORT)).c_str());
	struct arg_lit *arg_segmentfvs = arg_lit0("s", "segmentfvs", "Use feature vectors of segments instead of those of tracks.");
	struct arg_int *arg_fvtype = arg_int0("f", "featurevectortype", "<n>",
			"ID of feature vector type to use");
	struct arg_int *arg_FVPortion = arg_int0(NULL, "portion", "N",
				"Number of feature vectors to load as one portion. If memory is limited try a smaller number here. 0 ... Load all at once. Default = 0");


	// argument list for live mode
	void* argtable2[] = {mode_live, arg_fvtype, arg_port, arg_segmentfvs,
			arg_verbose, arg_dbconf, arg_log, arg_memlimit, arg_FVPortion, help, end };

	int nerrors = 1;

	/* verify the argtable[] entries were allocated sucessfully */
	if (arg_nullcheck(argtable1) != 0 || arg_nullcheck(argtable2) != 0) {
		/* NULL entries were detected, some allocations must have failed */
		std::cerr << PROGNAME << ": insufficient memory" << std::endl;
		exit(2);
	}

	// check first argument to decide which argtable we use for parsing
	if (argc >= 2) {
		if (std::string(argv[1]) ==  ARGUMENT_JOBSMODE) {
			iMode = MODE_JOBSMODE;
			nerrors = arg_parse(argc, argv, argtable1);
		} else if (std::string(argv[1]) ==  ARGUMENT_LIVEMODE) {
			iMode = MODE_LIVEMODE;
			nerrors = arg_parse(argc, argv, argtable2);
		} else {
			std::cerr << "Unexpected mode argument: " << argv[1] << std::endl;
			help->count = 1;
		}
	} else {
		// no argument given
		help->count = 1;
	}

	/* special case: '--help' takes precedence over error reporting */
	if (help->count > 0) {



		std::cout << "USAGE " << std::endl << std::endl << PROGNAME;
		arg_print_syntax(stdout, argtable1, "\n");
		arg_print_glossary(stdout, argtable1, "  %-22s %s\n");
		std::cout << std::endl << "-OR-" << std::endl << std::endl;
		std::cout << PROGNAME;
		arg_print_syntax(stdout, argtable2, "\n");
		arg_print_glossary(stdout, argtable2, "  %-22s %s\n");
		exit(1);






		std::cout << "USAGE " << std::endl << std::endl << PROGNAME;
		arg_print_syntax(stdout, argtable1, "\n");
		arg_print_glossary(stdout, argtable1, "  %-22s %s\n");
		std::cout << std::endl << std::endl;

#if defined(SMAFEDISTD_REAL_DAEMON)
		std::cout
		<< "This program works as a daemon, i.e., it is executed in the background and not attached to a shell."
		<< std::endl;
		std::cout << std::endl;
		std::cout
		<< "Currently, the preferred way to stop a running daemon is sending a SIGTERM signal to it. Then, the program will exit after finishing the current job (if any)."
		<< std::endl;
		std::cout << std::endl;
		std::cout << "How to send a SIGTERM signal to a running instance:"
				<< std::endl;
		std::cout << "  1) ps aux | grep " << PROGNAME
				<< "          (this gives you the <PID>)" << std::endl;
		std::cout << "  2) kill <PID>" << std::endl;
#else
		std::cout << "This program works NOT as a daemon because this platform does not support forking (or, there has been a problem at compiling the application)." << std::endl;
#endif

		exit(0);
	}

	if (nerrors == 0) {

		// options common to both modes-------------------

		// verbosity level
		// must be on top
		if (arg_verbose->count > 0) {
			loglevel_requested = arg_verbose->ival[0];
			// change loglevel
			SmafeLogger::smlog->setLoglevel(loglevel_requested);
		} else
			loglevel_requested = SmafeLogger::DEFAULT_LOGLEVEL;

		// logfile
		if (arg_log->count > 0) {
			sLogfilename = std::string(arg_log->sval[0]);
		} else {
			sLogfilename = std::string(PROGNAME) + "." + stringify(my_getpid()) + ".log";
		}

		// db options file
		if (arg_dbconf->count > 0) {
			SMAFELOG_FUNC(SMAFELOG_DEBUG, "Parsing db configuration file");
			so->parseDbOpts(std::string(arg_dbconf->sval[0]));
		}

		// mem limit
		if (arg_memlimit->count > 0) {
			if (arg_memlimit->ival[0] > 0) {
				smafedistd_mem_limit = arg_memlimit->ival[0];
			} else {
				SMAFELOG_FUNC(SMAFELOG_FATAL, "Mem parameter must be greater 0. Exiting.");
				exit(1);
			}
		}
		SMAFELOG_FUNC(SMAFELOG_INFO, "mem limit=" + stringify(smafedistd_mem_limit) + " MBs");

		// only in jobs mode -------------------------------
		if (iMode == MODE_JOBSMODE) {

			// identifier
			if (arg_daemonID->count > 0) {
				daemonId = std::string(arg_daemonID->sval[0]);
				if (daemonId == SmafeStoreDB::STATUSOK || (0 == daemonId.find(SmafeStoreDB::STATUSFAILED))) {
					SMAFELOG_FUNC(SMAFELOG_FATAL, "Identifier " + daemonId + " is illegal. It must not be '"+SmafeStoreDB::STATUSOK+"' and must not start with '"+SmafeStoreDB::STATUSFAILED+"'");
					exit(1);
				}
			} else {
				if (arg_stats->count == 0 && list->count == 0) {
					// is actually mandatory, only for --stats and --list it is not.
					SMAFELOG_FUNC(SMAFELOG_FATAL, "Please specify an identifier for this daemon (--id).");
					exit(1);
				}
			}


			// no-daemon
			if (arg_no_daemon->count > 0) {
#if defined(SMAFEDISTD_REAL_DAEMON)
				if (arg_log->count > 0) {
					SMAFELOG_FUNC(SMAFELOG_FATAL, "--no-daemon and --log cannot be combined. If program runs as forground process output is written to stdout and stderr.");
					exit(1);
				} else {
					// no daemon
					SMAFELOG_FUNC(SMAFELOG_INFO, "Running in 'normal mode' (ie, not as daemon)");
					bNoDaemon = true;
				}
#else
				SMAFELOG_FUNC(SMAFELOG_INFO, "Parameter --no-daemon is implied since this executable is compiled without daemon mode support.");
#endif
			}



			// ----- list distance types?
			if (list->count > 0) {
				doList();
				exit(0);
			}

			// ----- other options


			// polling interval
			if (arg_interval->count > 0)
				pollInterval = arg_interval->ival[0];
			if (pollInterval == 0) {
				SMAFELOG_FUNC(SMAFELOG_WARNING, "Polling interval set to 0 which means that the daemon will perform busy waiting.");
			}
			if (pollInterval < 0) {
				SMAFELOG_FUNC(SMAFELOG_INFO, "Daemon will stop after last finished task (since pollInterval < 0)");
			} else {
				SMAFELOG_FUNC(SMAFELOG_INFO, "polling interval=" + stringify(pollInterval));
			}



			// task limit
			if (arg_tasklimit->count > 0) {
				if (arg_tasklimit->ival[0] >= 1) {
					iRecLimit = arg_tasklimit->ival[0];
					SMAFELOG_FUNC(SMAFELOG_INFO, "Task limit set to " + stringify(iRecLimit));
				} else {
					SMAFELOG_FUNC(SMAFELOG_FATAL, "Task limit must be > 0");
					exit(1);
				}
			}


			// stats
			if (arg_stats->count > 0) {
				bPrintStatsOnly = true;
#if defined(SMAFEDISTD_REAL_DAEMON)
				SMAFELOG_FUNC(SMAFELOG_INFO, "Stats mode, so running in 'normal mode' (ie, not as daemon)");
				bNoDaemon = true;
#endif
			} else
				bPrintStatsOnly = false;

			// purge
			if (arg_purge->count > 0)
				bPurgemode = true;
			else
				bPurgemode = false;

			// initial run
			if (arg_initial->count > 0) {
				bInitialRun = true;
				SMAFELOG_FUNC(SMAFELOG_INFO, "Initial-run mode enabled. Assuming that distance table is empty.");
			} else
				bInitialRun = false;

			/*
			// inital run -> bulk.
			if (bInitialRun && !bBulkOp) {
				SMAFELOG_FUNC(SMAFELOG_FATAL, "Initial run assumption can only be used in conjunction with --bulk.");
				exit(2);
			}
			 */

			// bulk
			if (arg_bulk->count > 0) {
				bBulkOp = true;
				SMAFELOG_FUNC(SMAFELOG_INFO, "Bulk mode enabled.");
				if (!bInitialRun) {
					SMAFELOG_FUNC(SMAFELOG_INFO, "Bulk mode implies --initial-run mode. Enabling mode.");
					bInitialRun = true;
				}

				/*
			 if (pollInterval >= 0) {
			 SMAFELOG_FUNC(SMAFELOG_INFO, "Bulk mode: Daemon will quit after last task.");
			 pollInterval = -1; // set to negativ value so it will temriante after last task and not stay in mem
			 }
				 */
			} else
				bBulkOp = false;

			// fv types
			if (arg_fvtype_ids->count > 0) {
				// parameter given
				for (int i = 0; i < arg_fvtype_ids->count; i++) {
					fvtype_ids.push_back(arg_fvtype_ids->ival[i]);
					SMAFELOG_FUNC(SMAFELOG_INFO, "Will also calculate feature vector type_id " + stringify(fvtype_ids[i]));
				}
			} else {
				// param not given, default values
				// vector remains empty
				SMAFELOG_FUNC(SMAFELOG_INFO, "Will calculate all feature vector type ids");
			}

			// ----- check for argument errors that cannot be caught by argtable

			// daemon id
			/* no check for now - would need another boost lib
		 if (!daemonId.matches(RXidentifier)) {
		 SMAFELOG_FUNC(SMAFELOG_FATAL, "Invalid daemon identifier: " + daemonId);
		 exit(2);
		 }
			 */

		} // only jobs mode

		// live mode -----------------------------------------
		if (iMode == MODE_LIVEMODE) {
			// fvtype
			if (arg_fvtype->count > 0) {
				lFvTypeLiveMode = arg_fvtype->ival[0];
			}
			SMAFELOG_FUNC(SMAFELOG_INFO, "fvtype=" + stringify(lFvTypeLiveMode));

			// portion
			if (arg_FVPortion->count > 0) {
				if (arg_FVPortion->ival[0] >= 0) {
					lFVPortion = arg_FVPortion->ival[0];
				} else {
					SMAFELOG_FUNC(SMAFELOG_FATAL, "Portion parameter must be greater or equal 0. Exiting.");
					exit(1);
				}
			}
			SMAFELOG_FUNC(SMAFELOG_INFO, "FV portion=" + stringify(lFVPortion));



			// port
			if (arg_port->count > 0) {
				iPort = arg_port->ival[0];
			} else
				iPort = SMAFELIVEMODE_STDPORT;
			SMAFELOG_FUNC(SMAFELOG_INFO, "port=" + stringify(iPort));
			// check if port is available
			if (!isPortAvailable(iPort)) throw std::string("Port " + stringify(iPort) + " is already in use.");

			if (arg_segmentfvs->count > 0) {
				bUseSegmFvs = true;
			} else {
				bUseSegmFvs = false;
			}
			SMAFELOG_FUNC(SMAFELOG_INFO, "bUseSegments=" + stringify(bUseSegmFvs));

			//			bNoDaemon = true; // this can be commented in for debug reasons (valgrind)

		} // live mode

		// -------- switch to logfile
		// iff daemon mode is to come
		if (!bNoDaemon) {
			// new log
			// new log must be init before fork(). Why? Don't know yet
			SMAFELOG_FUNC(SMAFELOG_INFO, "Logging to '" + sLogfilename + "'");

			delete SmafeLogger::smlog;
			try {
				SmafeLogger::smlog = new SmafeLogger(sLogfilename,
						SmafeLogger::DEFAULT_LOGLEVEL);
				SmafeLogger::smlog->setLoglevel(loglevel_requested);
				splashScreen("Distance calculation daemon. ID=" + daemonId);
			} catch (std::string& s) {
				std::cerr << s << std::endl;
				std::cerr << "Please provide a valid filename." << std::endl;
				std::cerr << "Exitting" << std::endl;
				exit (3);
			}

			// mirror all params
			SMAFELOG_FUNC(SMAFELOG_INFO, "dbhost=" + stringify(so->strDbhost));
			SMAFELOG_FUNC(SMAFELOG_INFO, "dbname=" + stringify(so->strDbname));
			SMAFELOG_FUNC(SMAFELOG_INFO, "dbuser=" + stringify(so->strDbuser));
			SMAFELOG_FUNC(SMAFELOG_INFO, "dbpwd=**************"); // ;-)
			if (strlen(verysecretpassphrase) > 0)
				SMAFELOG_FUNC(SMAFELOG_INFO, "Encryption/decryption enabled"); // ;-)
			else
				SMAFELOG_FUNC(SMAFELOG_INFO, "No encryption/decryption."); // ;-)
			SMAFELOG_FUNC(SMAFELOG_INFO, "polling interval=" + stringify(pollInterval));
			SMAFELOG_FUNC(SMAFELOG_INFO, "mem limit=" + stringify(smafedistd_mem_limit) + " MBs");
			SMAFELOG_FUNC(SMAFELOG_INFO, "Task limit set to " + stringify(iRecLimit));
			SMAFELOG_FUNC(SMAFELOG_INFO, "FV portion=" + stringify(lFVPortion));
			SMAFELOG_FUNC(SMAFELOG_INFO, "bPurgemode=" + stringify(bPurgemode));
			SMAFELOG_FUNC(SMAFELOG_INFO, "bBulkOp=" + stringify(bBulkOp));
			SMAFELOG_FUNC(SMAFELOG_INFO, "bbInitialRun=" + stringify(bInitialRun));
			SMAFELOG_FUNC(SMAFELOG_INFO, "SMAFEDISTD_USETOPK=" + stringify(SMAFEDISTD_USETOPK));

			// fv types
			if (arg_fvtype_ids->count > 0) {
				// parameter given
				for (int i = 0; i < arg_fvtype_ids->count; i++) {
					fvtype_ids.push_back(arg_fvtype_ids->ival[i]);
					SMAFELOG_FUNC(SMAFELOG_INFO, "Will also calculate feature vector type_id " + stringify(fvtype_ids[i]));
				}
			} else {
				// param not given, default values
				// vector remains empty
				SMAFELOG_FUNC(SMAFELOG_INFO, "Will calculate all feature vector type ids");
			}
		}

	} else {
		arg_print_errors(stdout, end, PROGNAME);
		std::cout << "--help gives usage information" << std::endl;
		exit(1);
	}
	arg_freetable(argtable1, sizeof(argtable1) / sizeof(argtable1[0]));
	// argtable 2 shares some instances with argtable1. These have been freed already.
	//arg_freetable(argtable2,sizeof(argtable2)/sizeof(argtable2[0]));
}


/** Initializes variables that are dependent on the configuration loaded from db
 * Note: DB-Vars must be loaded */
void initDependentVars() {
	if (iMode == MODE_JOBSMODE) {
		// top k
		top_k_cur = so->top_k;
		top_k_safe_removable = so->top_k - 1;
	}
	if (iMode == MODE_LIVEMODE) {

	}
}


/** Logs all options that we use  and taht are specific to this program */
void logOptions() {
	if (iMode == MODE_JOBSMODE) {
		SMAFELOG_FUNC(SMAFELOG_INFO, "topk=" + stringify(so->top_k));
	}
	SMAFELOG_FUNC(SMAFELOG_DEBUG, stringify(so->toString()));
	/*
	 *	if (iMode == MODE_LIVEMODE) {
		SMAFELOG_FUNC(SMAFELOG_INFO, "livetopk=" + stringify(iLivetopk));
	}
	 */
}



/** Performs purge operation for selected distance- and fv types.
 * <p>This sub uses the variables topk, distancetype_ids, fvtype_ids
 */

void purge() {
	try {
		SMAFELOG_FUNC(SMAFELOG_INFO, "Performing purging operation...");
		SMAFE_STORE_DB_CLASS* db = NULL;
		// create db connection
		db = new SMAFE_STORE_DB_CLASS();
		db->openConnection(so->strDbhost, so->strDbport, so->strDbuser, so->strDbpwd, so->strDbname);

		unsigned long ulDeleted = 0;
		unsigned long ulDelOne = 0;

		std::vector<long> vTracks = db->getTrack_ids("");
		std::vector<long> vDists, vFvts;
		std::vector<long> *pvDists, *pvFvts;

		// check if distancetypes and fv types are given
		// if yes, take these
		// if not, get all available from db
		if (so->distancetype_ids.size() > 0)
			pvDists = &(so->distancetype_ids);
		else {
			vDists = db->getDistance_ids();
			pvDists = &(vDists);
		}
		if (fvtype_ids.size() > 0)
			pvFvts = &(fvtype_ids);
		else {
			vFvts = db->getFeaturevectortype_ids();
			pvFvts = &(vFvts);
		}

		// iterate through distance types
		for (std::vector<long>::iterator iter_d = pvDists->begin(); iter_d
		< pvDists->end(); iter_d++) {
			// ... fvtypes
			for (std::vector<long>::iterator iter_fv = pvFvts->begin(); iter_fv
			< pvFvts->end(); iter_fv++) {
				// ... tracks
				for (std::vector<long>::iterator iter_tr = vTracks.begin(); iter_tr
				< vTracks.end(); iter_tr++) {
					SMAFELOG_FUNC(SMAFELOG_DEBUG, "Purging:  fvt_id = " + stringify(*iter_fv) + ", dist_id = " + stringify(*iter_d) + ", track_a_id = " + stringify(*iter_tr));
					ulDelOne = db->purgeDistanceTable(*iter_tr, *iter_fv,
							*iter_d, so->top_k);
					SMAFELOG_FUNC(SMAFELOG_DEBUG, "Number of deleted records: " + stringify(ulDelOne));
					ulDeleted += ulDelOne;
				}
			}
		}
		SMAFELOG_FUNC(SMAFELOG_INFO, "");

		// close connection
		delete db;

		SMAFELOG_FUNC(SMAFELOG_INFO, "Purging operation finished. Deleted " + stringify(ulDeleted) + " records.");
	} catch (std::string& s) {
		SMAFELOG_FUNC(SMAFELOG_FATAL, s);
		exit(1);
	}
}

/** helper function that returns (estimated) memory usage of this process
 * in MEGAByte
 */
double getEstMemoryUsed() {
	long size1, size2;
	// fv buffer
	if (fvbuffer.size() > 0)
		size1 = fvbuffer.size() * fvbuffer.begin()->second.get()->sizeOf();
	else
		size1 = 0;
	// distance structure
	size2 = top_ds.size() * (top_k_cur * sizeof(double) + sizeof(long));

	return (double(size1) + double(size2)) / 1024.0 / 1024.0;
}

/** returns true, if the estimated current memory used is MEM_EPSILON below the limit
 *
 */
bool memAvailable() {
	return getEstMemoryUsed() < smafedistd_mem_limit * (1
			- SMAFEDISTD_MEM_EPSILON);
}

/** logs min and max distance values
 *
 */
void logMinmax() {
	if (dMinDist != DISTANCE_MORETHANMAX_VALUE) {
		SMAFELOG_FUNC(SMAFELOG_INFO, "Minimal _calculated_ distance value for this featurevectortype / distancetype combination: " + stringify(dMinDist));
		SMAFELOG_FUNC(SMAFELOG_INFO, "Please bear in mind that the database may contain lower or greater values if these have been inserted by another smafedistd process.");
	}
	if (dMaxDist != DISTANCE_LESSTHANMIN_VALUE) {
		SMAFELOG_FUNC(SMAFELOG_INFO, "Maximal _calculated_ distance value for this featurevectortype / distancetype combination: " + stringify(dMaxDist));
		SMAFELOG_FUNC(SMAFELOG_INFO, "Please bear in mind that the database may contain lower or greater values if these have been inserted by another smafedistd process.");
	}
}

/** Inserts distance record (if none yet exists for the specified keys)
 * @param db Open database connection
 * @param currentJob_fvt_id db key
 * @param currentJob_dist_id db key
 * @param currentJob_track_id db key (=track_a_id in distance table)
 * @param track_b_id db key
 * @param d value for distance. <b>If negative, NULL will be inserted</b>
 * */
void insert_distance_record(SMAFE_STORE_DB_CLASS* db, long currentJob_fvt_id,
		long currentJob_dist_id, long currentJob_track_id, long track_b_id,
		double d) {

	// if defensive mode, check for existing record
	if (!SMAFEDISTD_DEFENSIVE_MODE || (SMAFEDISTD_DEFENSIVE_MODE
			&& !db->isDistanceRecordInDatabase(currentJob_track_id, track_b_id,
					currentJob_fvt_id, currentJob_dist_id))) {

		/*
		 char distvalue[20];
		 char sqlcmd[1500];


		 // check if d is negative
		 if (d < 0) {
		 // insert NULL
		 sprintf(distvalue, "NULL");
		 SMAFELOG_FUNC(SMAFELOG_WARNING, "Inserting NULL distance value for vector pair (" + stringify(currentJob_track_id) + ", " + stringify(track_b_id) +
		 "), featurevectortype_id=" + stringify(currentJob_fvt_id) + ", distancetype_id=" + stringify(currentJob_dist_id));
		 } else {
		 // insert number
		 sprintf(distvalue, "%f", d);
		 }


		 sprintf(sqlcmd, "INSERT INTO distance \
				(track_a_id, track_b_id, featurevectortype_id, distancetype_id, value) \
				VALUES (%li, %li, %li, %li, %s)",
		 currentJob_track_id, track_b_id, currentJob_fvt_id, currentJob_dist_id, distvalue);

		 db->executeStatement(sqlcmd);

		 */
		db->insertDistanceRecord(currentJob_track_id, track_b_id,
				currentJob_fvt_id, currentJob_dist_id, d);

		SMAFELOG_FUNC(SMAFELOG_DEBUG, "Distance "+stringify(d)+" from track_id=" + stringify(currentJob_track_id) +
				" to track_id=" + stringify(track_b_id) + " inserted.");
	} else {
		SMAFELOG_FUNC(SMAFELOG_WARNING, "Distance record already in database: track_id=" +
				stringify(currentJob_track_id) + " to track_id=" + stringify(track_b_id));
	}
}

/** Returns requested feature vector.
 * Takes our fvbuffer into account, ie, if the fv is available in the buffer, we do not fetch it from the
 * database.
 * <p>NOTE: dist_id is used if SMAFEDISTD_USETOPK is true and we have jobs mode
 * <p>Global vars: uses bUseSegmFvs, fvbuffer
 * @param db open db connection
 * @param fvt_id, track_id .. primary key
 * @param to_be_deleted OUT parameter: true if returned feature vector should be deleted (freed)
 * @return instance of subclass of SAFV
 *
 */
// with boost::shared_ptr
SmafeAbstractFeatureVector* getFeatureVector(SMAFE_STORE_DB_CLASS* db,
		long fvt_id, long track_id, long segmentnr, long dist_id, bool &to_be_deleted) {
	if (SMAFEDISTD_OPT) {
		// optimized version
		tFVMap::iterator iter;
		iter = fvbuffer.find(longlongpair(track_id, segmentnr));
		if (iter != fvbuffer.end()) {
			// found in hashmap
			SMAFELOG_FUNC(SMAFELOG_DEBUG, "Track_id " + stringify(track_id) + ", segmentnr "+stringify(segmentnr)+" found in map. The top k distances might have changed in the mean time but we ignore it.");
			to_be_deleted = false; // do not delete a retrieved fv

			// get top k distances if top k is used AND initual run is _not_ assumed AND top_ds is empty (latter means, that it has probably been cleared since the fv itself has been loaded)
			// if the latter is assumed, then: do not query but just "assume" that there are not distances in table
			// only do that if we have jobs mode
			if (top_ds[track_id].size() == 0) {
				if (SMAFEDISTD_USETOPK && iMode == MODE_JOBSMODE) {
					if (!bInitialRun) {
						// get top k distances
						// See http://www.velocityreviews.com/forums/t288572-virtual-function-and-overloading-.html
						// or http://www.parashift.com/c++-faq-lite/strange-inheritance.html#faq-23.6
						// for explanation why we have to specifiy the base class here
						top_ds[track_id] = db->SmafeStoreDB::query_nn(track_id,
								fvt_id, dist_id, std::string(""), top_k_offset,
								so->top_k);
					} else
						SMAFELOG_FUNC(SMAFELOG_DEBUG, "Assumption, that no distance records exist for this track.");
				}
			}

			return iter->second.get();
		} else {
			// not ofund in map
			SmafeAbstractFeatureVector_Ptr fv_ptr;
			SmafeAbstractFeatureVector* fv;

			// determine from which table we read
			if (!bUseSegmFvs) { // fvt_id, track_id, false, -1, -1, false, true);
				fv = db->readFeatureVector(fvt_id, track_id, false, -1, -1, false, false);
			} else {
				//				fv = db->readSegmentFeatureVector(fvt_id, track_id, segmentnr);
				fv = db->readFeatureVector(fvt_id, track_id, false, -1, segmentnr, false, false);
			}

			// should this fv bie stored in map?
			// only if buffer does not exceed memory limit
			size_t currentSize = fvbuffer.size() * fv->sizeOf();
			if (memAvailable()) {
				fv_ptr.reset(fv);
				fvbuffer[longlongpair(track_id, segmentnr)] = fv_ptr;
				SMAFELOG_FUNC(SMAFELOG_DEBUG, "Track_id " + stringify(track_id) + " added to map. Contains now " + stringify(fvbuffer.size()) + " elements.");
				to_be_deleted = false; // do not delete fv if stored in map !!
			} else {
				to_be_deleted = true;
				SMAFELOG_FUNC(SMAFELOG_DEBUG, "Track_id " + stringify(track_id) + " NOT added to map. Contains now " + stringify(fvbuffer.size()) + " elements.");
				SMAFELOG_FUNC(SMAFELOG_DEBUG2, "Memory Limit:" + stringify(smafedistd_mem_limit) + " MB");
			};
			SMAFELOG_FUNC(SMAFELOG_DEBUG, "size of map in MB: " + stringify(double(currentSize) / 1024.0 / 1024.0));

			// get top k distances if top k is used AND initual run is _not_ assumed
			// if the latter is assumed, then: do not query but just "assume" that there are not distances in table
			// only do that if we have jobs mode
			if (SMAFEDISTD_USETOPK && iMode == MODE_JOBSMODE) {
				if (!bInitialRun) {
					// get top k distances
					// See http://www.velocityreviews.com/forums/t288572-virtual-function-and-overloading-.html
					// or http://www.parashift.com/c++-faq-lite/strange-inheritance.html#faq-23.6
					// for explanation why we have to specifiy the base class here
					top_ds[track_id] = db->SmafeStoreDB::query_nn(track_id,
							fvt_id, dist_id, std::string(""), top_k_offset,
							so->top_k);
				} else
					SMAFELOG_FUNC(SMAFELOG_DEBUG, "Assumption, that no distance records exist for this track.");
			}
			return fv;
		}
	} else {
		// normal version
		to_be_deleted = true; // always delete
		return db->readFeatureVector(fvt_id, track_id);
	}
}



/** Calculates distances from given feature vector to all fvs of tracks and/or segments in given vector (except for given currentJob_track_id) and returns sorted list of distances.
 * This function also loads the feature vector if necessary
 * @param db db object with open connection
 * @param dists_cur OUTPUT reference to vector where the distance values and corresponding (part) primary keys are stored
 * @param fvCurrent reference to featurevector from which all distances are calculated
 * @param vTracks reference to vector of part primary key of tracks to which the distances should be calculcated
 * @param currentJob_track_id current track_id: omit for distance calculation
 * @param currentJob_fvt_id Feature vector type ID
 * @param currentJob_dist_id Distance type id
 */
void calcDistsToAllFvs(SMAFE_STORE_DB_CLASS* db, std::vector<doublelonglongpairpair>* dists_cur,  SmafeAbstractFeatureVector* fvCurrent, std::vector<longlongpair>* vTracks, long currentJob_track_id, long currentJob_fvt_id, long currentJob_dist_id) {

	/** flag if other fv should be deleted */
	bool bDeleteOtherFV;
	/** Other feature vector*/
	SmafeAbstractFeatureVector* fvOther;

	SmafeNumericFeatureVector *snfv1;
	snfv1 = dynamic_cast<SmafeNumericFeatureVector*> (fvCurrent);

	for (std::vector<longlongpair>::iterator iter = vTracks->begin(); iter
	!= vTracks->end(); iter++) {
		double d; // the distance
		double rev_d;  // reverse distance!
		long track_b_id = iter->first;
		long track_b_segmentnr = iter->second; // not used in db mode

		// if (currentJob_track_id != track_b_id) { // do not calc distances from track X to track X
		if (iMode == MODE_LIVEMODE || currentJob_track_id > track_b_id) {    // do calc distances for tracks with lower id OR if in live mode

			SMAFELOG_FUNC(SMAFELOG_DEBUG, "Calculating distance to track_id=" + stringify(track_b_id) + ", segmentnr=" + stringify(track_b_segmentnr));

			// get our other fv
			fvOther = getFeatureVector(db, currentJob_fvt_id,
					track_b_id, track_b_segmentnr, currentJob_dist_id,
					bDeleteOtherFV);

			SMAFELOG_FUNC(SMAFELOG_DEBUG2, "currentJob_dist_id=" + stringify(currentJob_dist_id));

			// calc distance
			// check distance type first
			if (currentJob_dist_id >= 1
					&& currentJob_dist_id
					<= SmafeDistancesCalc::DISTFUNC_MAPPING_ARRAY_ELEMENTS) {

				SmafeNumericFeatureVector *snfv2;
				// Casting
				snfv2 = dynamic_cast<SmafeNumericFeatureVector*> (fvOther);
				if (snfv1 && snfv2) {
					// use NORM_P as the parameter for p
					// it is acutally only used in the LN distance function
					// in all other ones it is just ignored
					// (necessary, though, to have a common function pointer)
					d = SmafeDistancesCalc::distfunc_mapping[currentJob_dist_id](
							NORM_P, snfv1->buffer,
							snfv1->buflen,
							snfv2->buffer,
							snfv2->buflen);

					SMAFELOG_FUNC(SMAFELOG_DEBUG2, "d=" + stringify(d));
				} else {
					SMAFELOG_FUNC(SMAFELOG_WARNING, "Casting has not been successful.");
					d = -2;
				}
				// reverse
				rev_d = d;
			} else { // (currentJob_dist_id <= SmafeDistancesCalc::DISTFUNC_MAPPING_ARRAY_ELEMENTS)
				throw std::string("Distancetype_id "
						+ stringify(currentJob_dist_id)
						+ " is not within valid range!");
			} // (currentJob_dist_id <= SmafeDistancesCalc::DISTFUNC_MAPPING_ARRAY_ELEMENTS)


			// copy distance value to distance container
			dists_cur->push_back(doublelonglongpairpair(d, longlongpair(track_b_id, track_b_segmentnr)));


			/* (also read end of comment)
			 * As of 2010-03-22
			 * we do not insert the reverse distance record.
			 * For each job, distances to _all_ other tracks are calculated
			 * (not just to the lower ids).
			 * Thus, the track can be immediately used for active querying
			 * but does only occur in query results of other tracks if those
			 * jobs are finished.
			 *
			 * As of 2011-02
			 * we do insert reverse distance records iff
			 * 		- Jobs mode, and
			 * 		- NOT bulk mode
			 *
			 * As of 2011-03-09
			 * we do insert reverse distances iff
			 * 		- in jobs mode
			 */

			// If Regular Jobs mode
			//if (iMode == MODE_JOBSMODE && !bBulkOp) { // only if jobs mode and NOT bulk mode
			if (iMode == MODE_JOBSMODE ) { // only if jobs mode

				// check, if this distance is smaller than best k so far
				// at this moment, top_ds contains sorted arrays of distances (up to k_top_cur)
				double d_limit;
				bool bPresentInTop_ds = false;
				if (SMAFEDISTD_USETOPK) {
					// check if the other track is already in top_ds
					bPresentInTop_ds = top_ds.find(track_b_id) != top_ds.end() && top_ds[track_b_id].size() > 0;
					// if it is and if the container contains at least top_k distances...
					if (bPresentInTop_ds
							&& top_ds[track_b_id].size()
							+ top_k_offset >= top_k_cur) {
						// ... we get the largest one (last one) as limit...
						d_limit = top_ds[track_b_id].back();
						SMAFELOG_FUNC(SMAFELOG_DEBUG2, "Limit: " + stringify(d_limit));
					} else {
						// ... else, we take this very distance in all cases.
						SMAFELOG_FUNC(SMAFELOG_DEBUG2, "bPresentInTop_ds=" + stringify(bPresentInTop_ds) + ", we set limits to highest double available.");
						d_limit
						= std::numeric_limits<double>::max();
					}
				} else {
					// topk method not used
					// ... else, we take this very distance in all cases.
					d_limit = std::numeric_limits<double>::max();
				}
				if (rev_d < d_limit) {
					// insert reverse distance record
					insert_distance_record(db, currentJob_fvt_id,
							currentJob_dist_id, track_b_id,
							currentJob_track_id, rev_d);

					// copy distance value to distance container
					//					dists_cur->push_back(doublelonglongpairpair(rev_d, longlongpair(track_b_id, track_b_segmentnr)));

					if (SMAFEDISTD_USETOPK) {
						// update top_ds
						if (!bPresentInTop_ds) {
							// just insert
							top_ds[track_b_id].push_back(rev_d);
							SMAFELOG_FUNC(SMAFELOG_DEBUG, "Adding first member to top_ds["+stringify(track_b_id)+"] container");
							// since top_k_cur must be greater or equal 1, we do not have to remove a member here
						} else {
							// iterate through contianer and insert at proper position
							for (t_double_deque::iterator iter =
									top_ds[track_b_id].begin(); iter
									< top_ds[track_b_id].end(); iter++) {
								// if new distance vlaue lower than current position, insert it before.
								if (rev_d < (*iter)) {
									top_ds[track_b_id].insert(iter,
											rev_d);
									// if we do not insert at the beginning
									// we can safely remove on more
									if (iter
											!= top_ds[track_b_id].begin())
										top_k_safe_removable++;
									break;
								}
								// at this position we can never be at the end of the iterator (new value must have been inserted
								// at some time and thus the loop must be "broken"
								assert(iter != top_ds[track_b_id].end());
							} // for loop
							// if too many members remove largest member
							// unless we cannot do that safely
							if (top_ds[track_b_id].size()
									> top_k_cur) {
								if (top_k_safe_removable > 0) {
									top_ds[track_b_id].pop_back();
									top_k_safe_removable--;
								} else
									SMAFELOG_FUNC(SMAFELOG_DEBUG, "Limit distance value (top_k) cannot safely be removed.");
							}

						} // else branch of !bPresentInTop_ds

						// maintain info about min and max distance
						// take minimum of current minimum and lowest distance in this distancejob
						dMinDist = std::min(dMinDist, rev_d);

					} // if (SMAFEDISTD_USETOPK)

				} else {
					// distance is not persisted
					SMAFELOG_FUNC(SMAFELOG_DEBUG, "Skipping distance " + stringify(rev_d) + ": is greater than top " + stringify(top_k_cur) + " distance (" + stringify(d_limit) + ")");

					// maintain info about min and max distance
					// take maximum of curent maximum and greatest distance in this distancejob
					dMaxDist = std::max(dMaxDist, rev_d);
				}

			} // Jobs mode and  not bulk mode


			// delete other fv
			if (bDeleteOtherFV)
				delete fvOther;
		} else { // currentJob_track_id != track_b_id
			SMAFELOG_FUNC(SMAFELOG_DEBUG, "Omitting track_id=" + stringify(track_b_id) + ", segmentnr=" + stringify(track_b_segmentnr) + " (because is >= current track_id)");
		}

	} // for loop


	// distance records from current fv to top k fvs

	// sort by first element of the pair automatically (ie distance)
	std::sort(dists_cur->begin(), dists_cur->end());
}



/** Calculates distances from given feature vector to all fvs of tracks and/or segments in given collection
 * and returns sorted list of distances.
 * <p>This function is for live mode and uses parallel execution.
 *
 * TODO obsolte: Calculates distances from given feature vector to all fvs of tracks and/or segments in given vector (except for given currentJob_track_id) and returns sorted list of distances.
 * This function also loads the feature vector if necessary
 * @param db db object with open connection
 * @param dists_cur OUTPUT reference to vector where the distance values and corresponding (part) primary keys are stored
 * @param fvCurrent reference to featurevector from which all distances are calculated
 * @param vTracks reference to vector of part primary key of tracks to which the distances should be calculcated
 * @param currentJob_track_id current track_id: omit for distance calculation
 * @param currentJob_fvt_id Feature vector type ID
 * @param currentJob_dist_id Distance type id
 */
void calcDistsToAllFvs(std::vector<doublelonglongpairpair>* dists_cur,  SmafeAbstractFeatureVector* fvCurrent, const tlonglongsafvStructVector &vOtherFvs /*, long currentJob_track_id, long currentJob_fvt_id    */ , long currentJob_dist_id, long topk) {

	/** Other feature vector*/
	SmafeAbstractFeatureVector* fvOther;

	// Cast one featurevector, we will need it quite often
	SmafeNumericFeatureVector *snfv1;
	snfv1 = dynamic_cast<SmafeNumericFeatureVector*> (fvCurrent);

#pragma omp parallel
{
	std::vector<doublelonglongpairpair> vPrivateDists;

	SMAFELOG_FUNC(SMAFELOG_DEBUG, "Parallel execution started with " + stringify(omp_get_num_threads()) + " threads");

#pragma omp for
	for (long l = 0; l < vOtherFvs.size(); l++) {
		double d; // the distance

		// check distance type first
		if (currentJob_dist_id >= 1
				&& currentJob_dist_id
				<= SmafeDistancesCalc::DISTFUNC_MAPPING_ARRAY_ELEMENTS) {

			SMAFELOG_FUNC(SMAFELOG_DEBUG2, "(" + stringify(omp_get_thread_num()) + ") l=" + stringify(l));

			SmafeNumericFeatureVector *snfv2;
			// Casting
			snfv2 = dynamic_cast<SmafeNumericFeatureVector*> (vOtherFvs.at(l).fv.get());

			if (!snfv1) SMAFELOG_FUNC(SMAFELOG_WARNING, "Casting 1 has not been successful.");
			if (!snfv2) SMAFELOG_FUNC(SMAFELOG_WARNING, "Casting 2 has not been successful.");

			if (snfv1 && snfv2) {
				// use NORM_P as the parameter for p
				// it is acutally only used in the LN distance function
				// in all other ones it is just ignored
				// (necessary, though, to have a common function pointer)
				d = SmafeDistancesCalc::distfunc_mapping[currentJob_dist_id](
						NORM_P, snfv1->buffer,
						snfv1->buflen,
						snfv2->buffer,
						snfv2->buflen);

				SMAFELOG_FUNC(SMAFELOG_DEBUG2, "(" + stringify(omp_get_thread_num()) + ") d=" + stringify(d));
			} else {
				SMAFELOG_FUNC(SMAFELOG_WARNING, "(" + stringify(omp_get_thread_num()) + ") Casting has not been successful.");
				d = -2;
			}
		} else { // (currentJob_dist_id <= SmafeDistancesCalc::DISTFUNC_MAPPING_ARRAY_ELEMENTS)
			throw std::string("Distancetype_id "
					+ stringify(currentJob_dist_id)
					+ " is not within valid range!");
		} // (currentJob_dist_id <= SmafeDistancesCalc::DISTFUNC_MAPPING_ARRAY_ELEMENTS)

		// d is not set (private var)

		// Add to private vector of distances
		vPrivateDists.push_back(doublelonglongpairpair(d, longlongpair(vOtherFvs.at(l).track_id, vOtherFvs.at(l).segmentnr )));

	} // for

	// sort private list of distances
	std::sort(vPrivateDists.begin(), vPrivateDists.end());


	// critical section: to avoid race conditions, this section must be executed sequentially be each thread
#pragma omp critical
{

	// collect private distances (sorted), and combine the top k of each vector into one shared vector
	dists_cur->insert(dists_cur->end(), vPrivateDists.begin(), std::min(vPrivateDists.begin() + topk, vPrivateDists.end()));
	SMAFELOG_FUNC(SMAFELOG_DEBUG, "Shared vector contains now " + stringify(dists_cur->size()) + " elements, says thread " + stringify(omp_get_thread_num()));

} // end pragma omp critical


} // end pragma omp parallel for

	// sort final vector
	// sort by first element of the pair automatically (ie distance)
	std::sort(dists_cur->begin(), dists_cur->end());
}



/** Loads all feature vectors into memory
 * TODO DOC
 * <b>Side effects on global vars</b>:
 * <ul>
 * <li>fvbuffer is emptied and filled with feature vectors
 * <li>lllsafvMap is emptied and filled
 * @param vTracks OUTPUT vector to contain all track/segmentnr pairs currently in the db */
void initLiveMode(long &currentJob_fvt_id, /* longlong_set_deque &tracks_collections, */  tFVMap &fvbuffer, tlonglonglongsafvStructVectorMap &lllsafvMap) {

	SMAFE_STORE_DB_CLASS* db = NULL;

	// Open db connection
	db = new SMAFE_STORE_DB_CLASS();
	db->openConnection(so->strDbhost, so->strDbport, so->strDbuser, so->strDbpwd, so->strDbname);

	// read fvtype ids: currently, only one is supported
	//std::vector<long> vFvtypeIds = db->getFeaturevectortype_ids();
	std::vector< SmafeFVType_Ptr > vFvTypes;
	db->getFeaturevectortypes(vFvTypes);

	delete db;

	// check error conditions
	if (vFvTypes.size() > 1 && lFvTypeLiveMode == -1) {
		SMAFELOG_FUNC(SMAFELOG_FATAL, "The database contains more than one feature vector type. Please specify which one to use (option -f).");
		exit(1);
	}
	if (vFvTypes.size() == 0 ) {
		SMAFELOG_FUNC(SMAFELOG_FATAL, "No feature vector type found in database. Is database empty?");
		exit(1);
	}
	if (so->distancetype_ids.size() != 1) {
		SMAFELOG_FUNC(SMAFELOG_FATAL, "Currently the live calculation works with one distance type enabled in the database only. (Change with smafewrapd --config)");
		exit(1);
	}
	if (vFvTypes.size() == 1)
		currentJob_fvt_id = vFvTypes[0]->id;
	else {
		currentJob_fvt_id = lFvTypeLiveMode;
		// adapt also the options field
		for (std::vector< SmafeFVType_Ptr >::iterator iter =
				vFvTypes.begin(); iter
				!= vFvTypes.end(); iter++) {
			// if the id of this fvt is the chosen one, mark the extract flag is true, otherwise false
			if (lFvTypeLiveMode == iter->get()->id)
				so->mapExtractGenerally[iter->get()->name] = true;
			else
				so->mapExtractGenerally[iter->get()->name] = false;
		} // end of iterator
	}


	// read track info and fvs

	db = new SMAFE_STORE_DB_CLASS();
	db->openConnection(so->strDbhost, so->strDbport, so->strDbuser, so->strDbpwd, so->strDbname);


	SMAFELOG_FUNC(SMAFELOG_INFO, "Reading feature vectors from db...");
	if (bUseSegmFvs) {


		db->readAllFeatureVectors(currentJob_fvt_id, true, false, fvbuffer, lFVPortion);

		/*
		// Segments version OLD

		SMAFELOG_FUNC(SMAFELOG_INFO, "Reading track information from db");
		vTracks = db->getSegmentTracksForDistanceCalc(-1, currentJob_fvt_id, -1); // -1: these arguments are not used in sub
		SMAFELOG_FUNC(SMAFELOG_INFO, "Starting to read feature vectors from db into memory.");

		fvbuffer.clear();

		for (std::vector<longlongpair>::iterator iter = vTracks.begin(); iter
		!= vTracks.end(); iter++) {
			long track_b_id = iter->first;
			long track_b_segmentnr = iter->second;

			SMAFELOG_FUNC(SMAFELOG_DEBUG, "Loading track_id " + stringify(track_b_id) + ", segment " + stringify(track_b_segmentnr));

			// get our other fv
			// we do not need to specify dist_id because we are in live mode.
			(void) getFeatureVector(db, currentJob_fvt_id,
					track_b_id, track_b_segmentnr,  -1,
					bDeleteOtherFV);

			if (bDeleteOtherFV == true) {
				// seems like we've run out of memory
				throw std::string("Not enough memory to add more feature vectors. Please use parameter --mem to reserve more memory.");
			}

			// Refresh db connection every 17000 tracks because of the SSL renegotiation problem
			// http://archives.postgresql.org/message-id/41EE6009-A3E0-4C3A-8A83-BB39D934B461@mac.com
			// http://www.postgresql.org/docs/8.3/interactive/runtime-config-connection.html
			// Postgres default value is 512 MB, this limit seems to be reached after 17700 RP feature vectors
			// As a workaround (we do not expect the postgres server to be configured) we make another connection
			if (fvbuffer.size() % 17000 == 0) {
				delete db;
				db = new SMAFE_STORE_DB_CLASS();
				db->openConnection(so->strDbhost, so->strDbport, so->strDbuser, so->strDbpwd, so->strDbname);
				SMAFELOG_FUNC(SMAFELOG_DEBUG, "Database connection renewed at " + stringify(fvbuffer.size()) + " feature vectors.");
			}


			// Assertion should hold because fvs that are stored in the buffer (should be every one!) are not to e deleted.
			//assert(bDeleteOtherFV == false);
		}
		 */

	} else {
		// Tracks version (no segments)
		db->readAllFeatureVectors(currentJob_fvt_id, false, false, fvbuffer, lFVPortion);
	}
	SMAFELOG_FUNC(SMAFELOG_INFO, "...done (Reading feature vectors from db)");
	SMAFELOG_FUNC(SMAFELOG_INFO, stringify(fvbuffer.size()) + " feature vectors are loaded. ");


	SMAFELOG_FUNC(SMAFELOG_INFO, "Reading track and collection information from db...");
	db->getCollectionReferences(currentJob_fvt_id, bUseSegmFvs, fvbuffer, lllsafvMap);

	SMAFELOG_FUNC(SMAFELOG_INFO, "...done (Reading track and collection information from db)");



	// monitor memory
	double curMemUsed = getEstMemoryUsed();
	SMAFELOG_FUNC(SMAFELOG_INFO, "Estimated memory usage: " + stringify(curMemUsed) + " MB");


	delete db;

}



/** Sends serialized options to given stream
 * @param stream reference to open TCP/IP stream
 * @param so reference to options
 * */
void sendOptions(tcp::iostream &stream, Smafeopt* so) {
	std::stringstream ss(std::stringstream::in | std::stringstream::out);
	std::string ss_enc;

	{
		boost::archive::text_oarchive oa(ss);
		//boost::archive::xml_oarchive oa(ss);
		oa << BOOST_SERIALIZATION_NVP(so);
	}

	SMAFELOG_FUNC(SMAFELOG_DEBUG, "Serialized options: " + ss.str() );

	ss_enc = encryptString(ss.str().c_str(), stdpassphrase);

	std::string sOut = boost::algorithm::replace_all_copy(ss_enc, "\n", " ");
	SMAFELOG_FUNC(SMAFELOG_DEBUG, "Sending: " +  sOut );
	stream << sOut;
}


/** Calculates nearest neighbour for this feature vector and sends the answer back into the stream
 * @param stream reference to open TCP/IP stream
 * @param sFv encrypted, serialized feature vector
 *  */
void sendNearestNeighbour(tcp::iostream &stream, SmafeAbstractFeatureVector *fvCurrent, long currentJob_fvt_id, long currentJob_dist_id, const tlonglongsafvStructVector &vOtherFvs, int iLivetopk) {


	try {
		std::vector<doublelonglongpairpair> dists_cur;

		// Calc all dists
		// we do not have an open db connection here but assume that all fvs are loaded already
		// 0: current job track_id
		SMAFELOG_FUNC(SMAFELOG_DEBUG, "Starting distance calculation...");
		// (std::vector<doublelonglongpairpair>* dists_cur,  SmafeAbstractFeatureVector* fvCurrent, tlonglongsafvStructVector* vOtherFvs /*, long currentJob_track_id, long currentJob_fvt_id    */ , long currentJob_dist_id, long topk)
		calcDistsToAllFvs(&dists_cur,  fvCurrent, vOtherFvs, currentJob_dist_id, iLivetopk);
		SMAFELOG_FUNC(SMAFELOG_DEBUG, "...finished distance calculation. Received " + stringify(dists_cur.size()) + " entries.");

		// output the structure as csv
		// header
		stream << "track_id" << "," << "segment_id"<< ","  << "distance" << "||";

		// data
		int c = 0;
		for (std::vector<doublelonglongpairpair>::iterator iter = dists_cur.begin(); iter < dists_cur.end() && c < iLivetopk; iter++) {
			//				track_id					segment_id					distance
			stream << iter->second.first << "," << iter->second.second << ","  << iter->first << "||";
			c++;
		}
		// flush
		//		stream << std::endl;

	} catch (std::string& s) {
		SMAFELOG_FUNC(SMAFELOG_ERROR, s);
		stream << s;
	}
	catch (boost::archive::archive_exception& e) {
		SMAFELOG_FUNC(SMAFELOG_ERROR, e.what());
		stream << e.what();
	} catch (CryptoPP::Exception& e) {
		SMAFELOG_FUNC(SMAFELOG_ERROR, "Error decrypting options: " + e.GetWhat());
		stream << e.GetWhat();
	}
}

/** Finishes bulk mode:
 * - recreates indexes if they have been dropped
 * - switch bulk mode flag
 */
void finishBulkMode(SMAFE_STORE_DB_CLASS* db) {
	if (bBulkOp && bIndexDropped) {
		if (!db->distanceTableIndexExists()) {
			SMAFELOG_FUNC(SMAFELOG_INFO, "Bulk mode: Index(es) for distance table is/are being re-created. This can take a while. ");
			db->startTransaction();
			db->createDistanceTableIndex();
			db->finishTransaction(true);
			SMAFELOG_FUNC(SMAFELOG_INFO, "Bulk mode: Index(es) for distance table has/ve been re-created.");
		} else {
			SMAFELOG_FUNC(SMAFELOG_WARNING, "Bulk mode: Index(es) for distance table are to be re-created but it/they exist/s already.");
		}
		/*
		 // currently disabled because consumes significant time
		 // purging after index re-creation
		 if (!bErrorOccured) {
		 purge();
		 } else {
		 SMAFELOG_FUNC(SMAFELOG_INFO, "No purging because error occured.");
		 }
		 */

		// run in normal mode from now on
		bBulkOp = false;
		SMAFELOG_FUNC(SMAFELOG_INFO, "Bulk mode disabled, running in regular mode from now on.");
	}
}

// ------------------------------------------------------------------------
// one round of the main loop in jobs mode
void main_loop_round_jobsmode() {
	bool bVacancy;
	long lNumVacancies, lNumCurrentVac, currentJob_fvt_id, currentJob_track_id,
	currentJob_dist_id;
	SMAFE_STORE_DB_CLASS* db = NULL;
	/** feature vector of current job */
	SmafeAbstractFeatureVector* fvCurrent = NULL;
	/** flag if current fv should be deleted */
	bool bDeleteCurrentFV;
	/** Collection of tracks to calculate distance to (may include current track_id, will be omitted in sub */
	std::vector<longlongpair> vTracks;
	/** container for distance values from current fv to all others */
	std::vector<doublelonglongpairpair> dists_cur;
	/** sql command */
	char sqlcmd[1500];
	/* will be set to true if error has occured */
	bool bErrorOccured = false;

	try {

		// create db connection
		db = new SMAFE_STORE_DB_CLASS();
		db->openConnection(so->strDbhost, so->strDbport, so->strDbuser, so->strDbpwd, so->strDbname);

		// if bulk mode: check for empty distance table
		if (bBulkOp && !db->tableIsEmpty("distance")) {
			throw std::string("Bulk modes expects distance table to be empty.");
		}

		do {
			// get info on job market situation
			bVacancy = db->getOpenTaskInfo(&so->distancetype_ids, &fvtype_ids,
					lNumVacancies, lNumCurrentVac, currentJob_track_id,
					currentJob_fvt_id, currentJob_dist_id);

			// Announce open tasks
			// Note: "open task(s) matching current parameters" is a grep keyword used in the init script: /etc/init.d/smafed status
			SMAFELOG_FUNC(SMAFELOG_INFO, stringify(lNumVacancies) + " open task(s) found.");
			SMAFELOG_FUNC(SMAFELOG_INFO, stringify(lNumCurrentVac) + " open task(s) matching current parameters.");

			if (bVacancy && !bPrintStatsOnly) {

				//  bulk mode index deletion was here

				// if in bulk mode and the current job is about a track that we do not have in the track_id list
				// that we read at the very beginning of our work,
				// we must assume that other tracks have been added since then. As IN BULK MODE we cannot
				// garantuee that these newly added tracks appear as NNs for older tracks,
				// we must switch to regular mode.
				// one exception: if this is the first job at all, vTracks will be empty and this does not apply
				if (bBulkOp && vTracks.size() != 0) {
					std::vector<longlongpair>::iterator iter;
					iter = std::find(vTracks.begin(), vTracks.end(), longlongpair(currentJob_track_id, -1));
					if (iter == vTracks.end()) {
						// NOT found in vector
						SMAFELOG_FUNC(SMAFELOG_DEBUG, "Bulk mode, but current track_id "+ stringify(currentJob_track_id) + " not found in vTracks.");
						SMAFELOG_FUNC(SMAFELOG_INFO, "Newly added tracks encountered. Stopping bulk operation and switching to regular mode.");
						finishBulkMode(db);
					}
				}

				// Mark task as being processed in db
				db->startTransaction();
				// stricter concurrency control
				db->executeStatement(
						"SET TRANSACTION ISOLATION LEVEL SERIALIZABLE");

				sprintf(
						sqlcmd,
						"UPDATE distancejob SET status='%s', started=CURRENT_TIMESTAMP \
						WHERE \
						featurevectortype_id=%li AND \
						track_id=%li AND \
						distancetype_id=%li",
						daemonId.c_str(), currentJob_fvt_id,
						currentJob_track_id, currentJob_dist_id);

				// NB: Since we have strict isolation level the next statement will block if
				// another process P has already performed an (uncommitted) update (or deletion) against
				// the same row
				// After P commits its changes our statement may fail
				// with an "SERIALIZATION FAILURE". In this case the function
				// will return false and this process will _not_ work on the task
				// In case that P rolls back its changes we can continue
				// normally and work at this task as expected.
				if (db->executeStatement_serializationcheck(sqlcmd)) {
					db->finishTransaction(true);

					try {

						SMAFELOG_FUNC(SMAFELOG_INFO, std::string("Starting task: ") +
								std::string("track_id=") + stringify(currentJob_track_id) +
								", featurevectortype_id=" + stringify(currentJob_fvt_id) +
								", distancetype_id=" + stringify(currentJob_dist_id));

						// for "benchmark"
						clock_t begin_clock = clock();
						time_t begin_time = time(NULL);

						// if we have another fvt_id as last time, empty the fvbuffer
						if (SMAFEDISTD_OPT) {
							if (lLastFVT_id != currentJob_fvt_id) {
								fvbuffer.clear();
								SMAFELOG_FUNC(SMAFELOG_DEBUG, "New feature vector type " + stringify(currentJob_fvt_id) + ": map cleared.");
							}
						}

						// if we have another fvt_id OR another dist_id: empty top_ds
						if (lLastDT_id != currentJob_dist_id || lLastFVT_id
								!= currentJob_fvt_id) {
							// empty top_ds;
							top_ds.clear();
							SMAFELOG_FUNC(SMAFELOG_DEBUG, "New feature vector type " + stringify(currentJob_fvt_id) +
									" and/or new distance type " + stringify(currentJob_dist_id) +
									": top_ds cleared.");

							// Also print min/max values and reset them.
							logMinmax();
							dMinDist = DISTANCE_MORETHANMAX_VALUE;
							dMaxDist = DISTANCE_LESSTHANMIN_VALUE;
						}

						// in case that any of the values changed, update them
						lLastFVT_id = currentJob_fvt_id;
						lLastDT_id = currentJob_dist_id;

						// get our current fv
						// -1 as segment nr because we are in jobs mode and here we do not have segment fvs
						SMAFELOG_FUNC(SMAFELOG_DEBUG, "Reading current feature vector");
						fvCurrent = getFeatureVector(db, currentJob_fvt_id,
								currentJob_track_id, -1, currentJob_dist_id,
								bDeleteCurrentFV);

						// if regular mode:
						// get list of tracks that we have to calculate the distance to
						// (=all other)
						if (!bBulkOp) {
							SMAFELOG_FUNC(SMAFELOG_DEBUG, "No bulk mode, query for other tracks in each loop.");
							//vTracks = db->getOtherTracksForDistanceCalc(currentJob_track_id, currentJob_fvt_id, currentJob_dist_id);
							// only lower track ids
							vTracks = db->getLowerTracksForDistanceCalc(
									currentJob_track_id, currentJob_fvt_id,
									currentJob_dist_id);
						} else {
							// bulk mode: if not already done, get track ids now and "forever"
							if (vTracks.size() == 0) {
								SMAFELOG_FUNC(SMAFELOG_DEBUG, "Bulk mode, vTracks is empty. Filling vTracks for the first time.");
								// get all, also include current track id (-1: no track_id constraint
								vTracks = db->getOtherTracksForDistanceCalc(-1, currentJob_fvt_id, currentJob_dist_id);
							}  else {
								; // NOOP: bulk mode and vTracks is filled
							}
						}


						// clear vector and set capacity to expected number of tracks
						dists_cur.clear();
						dists_cur.reserve(vTracks.size() + 3);


						// do the calc
						calcDistsToAllFvs(db, &dists_cur, fvCurrent, &vTracks, currentJob_track_id,  currentJob_fvt_id, currentJob_dist_id);


						// iterate through top k distances
						for (size_t i = 0; i < std::min((size_t) so->top_k,
								dists_cur.size()); i++) {
							// insert distance record
							insert_distance_record(db, currentJob_fvt_id,
									currentJob_dist_id, currentJob_track_id,
									dists_cur[i].second.first, dists_cur[i].first); // .second.first is the track_b_id
							if (SMAFEDISTD_USETOPK) {
								// udpate top_ds (copy top values)
								if (i >= top_k_offset) {
									top_ds[currentJob_track_id].push_back(
											dists_cur[i].first);
								}
							}
						}

						// maintain info about min and max distance
						// take minimum of current minimum and lowest distance in this distancejob
						dMinDist = std::min(dMinDist, dists_cur.front().first);
						// take maximum of curent maximum and greatest distance in this distancejob
						dMaxDist = std::max(dMaxDist, dists_cur.back().first);

						// info about top_ds structure
						SMAFELOG_FUNC(SMAFELOG_DEBUG, "top_ds contains now " + stringify(top_ds.size()) + " vectors of approx " + stringify(top_k_cur) + " distance values.");

						// If bulk mode: drop index
						// This happens here before the start of a transaction and before anyhting is inserted,
						// but AFTER all the fvs are read from db.
						if (bBulkOp && !bIndexDropped) {
							// check if index is present
							if (db->distanceTableIndexExists()) {
								db->dropDistanceTableIndex();
								bIndexDropped = true;
								SMAFELOG_FUNC(SMAFELOG_INFO, "Bulk mode: Index(es) for distance table has/ve been dropped.");
							} else {
								bBulkOp = false;
								SMAFELOG_FUNC(SMAFELOG_INFO, "Bulk mode: Index(es) for distance table do not exist. Another daemon probably works in bulk mode. \nBulk mode DISABLED for this daemon instance.");
							}
						}

						// start transaction
						// that can happen here because in the loop we did not insert anything
						// Remember that insert.. only collects the data in a vector and only
						// at commit time the corresponding sql COPY command is generated and executed
						db->startTransaction();

						// mark task as done
						if (!SMAFEDISTD_DEBUG) {

							sprintf(
									sqlcmd,
									"UPDATE distancejob SET status='%s', finished=CURRENT_TIMESTAMP \
									WHERE \
									featurevectortype_id=%li AND \
									track_id=%li AND \
									distancetype_id=%li",
									SmafeStoreDB::STATUSOK.c_str() , currentJob_fvt_id,
									currentJob_track_id, currentJob_dist_id);
							db->executeStatement(sqlcmd);
						}

						SMAFELOG_FUNC(SMAFELOG_DEBUG, "Committing Distance table records.");
						db->finishTransaction(true);

						// log time
						clock_t end_clock = clock();
						time_t end_time = time(NULL);
						SMAFELOG_FUNC(SMAFELOG_INFO, std::string("Finished task: ") +
								std::string("track_id=") + stringify(currentJob_track_id) +
								", featurevectortype_id=" + stringify(currentJob_fvt_id) +
								", distancetype_id=" + stringify(currentJob_dist_id) + " in " +
								stringify(difftime (end_time,begin_time)) + " s (usr time " +
								stringify(diffclock(end_clock,begin_clock)) + " ms)");

						iRecCounter++;

						// monitor memory
						double curMemUsed = getEstMemoryUsed();
						SMAFELOG_FUNC(SMAFELOG_INFO, "Estimated memory usage: " + stringify(curMemUsed) + " MB");
						if (!memAvailable()) {
							SMAFELOG_FUNC(SMAFELOG_WARNING, "No more 'memory available': Shrinking of datastructure is currently not supported. Daemon will stop. Plesae restart it with a higher value for --mem parameter.");
							SMAFELOG_FUNC(SMAFELOG_INFO, "Details: Estimated current memory load: "  + stringify(curMemUsed) + " MB.");
							bVacancy = false;
							b_should_terminate = true;
							/*
							SMAFELOG_FUNC(SMAFELOG_INFO, "No more 'memory available' so we shrink top_ds data structure.");
							if (top_k_cur == 1) {
								SMAFELOG_FUNC(SMAFELOG_FATAL, "Shrinking of top_ds not possible, top_k_cur == 1. ");
								throw std::string(
										"Not enough memory, shrinking of top_ds not possible, top_k_cur == 1. ");
							}
							// reduce top_k_cur by half, rounding up
							size_t old_top_k_cur = top_k_cur;
							top_k_cur = int(ceil(double(top_k_cur) / 2.0));
							// adjust k_top_offset
							top_k_offset = so->top_k - top_k_cur;
							// adjust safe removable:
							// at most the value that we had AND ALSO
							// at most the new number of stored values minus 1
							top_k_safe_removable = std::min(
									top_k_safe_removable, top_k_cur - 1);
							// iterate through top_ds and remove elemens from beginning of each deque
							for (tTop_ds::iterator iter = top_ds.begin(); iter
							!= top_ds.end(); iter++) {
								for (size_t i = 0; i < old_top_k_cur - top_k_cur; i++) {
									iter->second.pop_front();
								}
							} // for loop
							SMAFELOG_FUNC(SMAFELOG_INFO, "Top_ds successfully shrinked from " + stringify(old_top_k_cur) + " to " + stringify(top_k_cur) + " distance values per track");
							 */
						}

					} catch (std::string& s) {
						SMAFELOG_FUNC(SMAFELOG_WARNING, "Exception occured during distance calculation.");
						SMAFELOG_FUNC(SMAFELOG_WARNING, s);
						SMAFELOG_FUNC(SMAFELOG_WARNING, "Rolling back distance table records and marking distancejob record as failed");
						db->finishTransaction(false);

						// rolling back distancejob record (status field to NULL again)
						db->startTransaction();
						sprintf(
								sqlcmd,
								"UPDATE distancejob SET status='%s', started=NULL \
								WHERE \
								featurevectortype_id=%li AND \
								track_id=%li AND \
								distancetype_id=%li",
								SmafeStoreDB::STATUSFAILED.c_str(),
								currentJob_fvt_id, currentJob_track_id,
								currentJob_dist_id);
						db->executeStatement(sqlcmd);

						db->finishTransaction(true);

					} // end of catch block

					// delete current fv
					if (bDeleteCurrentFV)
						delete fvCurrent;

				} else { // if (db->finishTransaction_serializationcheck())
					SMAFELOG_FUNC(SMAFELOG_INFO, "Ooops. Another daemon took my task just when I was about to start...");
					db->finishTransaction(false);
				} // if (db->finishTransaction_serializationcheck())
			} else {
				if (!bPrintStatsOnly)
					SMAFELOG_FUNC(SMAFELOG_INFO, "No appropriate open tasks found");
			} // if (bVacancy && !bPrintStatsOnly)

		} while (bVacancy && !b_should_terminate && !bPrintStatsOnly
				&& (iRecCounter < iRecLimit || iRecLimit < 0));

		if (iRecCounter >= iRecLimit && iRecLimit >= 0) {
			b_should_terminate = true;
			SMAFELOG_FUNC(SMAFELOG_INFO, "Stopping calculation after " + stringify(iRecLimit) + " tasks.");
		}

		logMinmax();

		// Finish bulk mode
		finishBulkMode(db);

	} catch (std::string& s) {
		SMAFELOG_FUNC(SMAFELOG_FATAL, s);
		bErrorOccured = true;
	}


	delete db;

	// exit in case of error if not daemon mode
	if (bNoDaemon && bErrorOccured)
		exit(1);

}


/*
//void main_loop_round_livemode(tcp::iostream* stream, std::vector<longlongpair>* vTracks, long currentJob_fvt_id, boost::asio::io_service *io_service, const boost::system::error_code& error);

void start_accept(boost::asio::io_service *io_service, std::vector<longlongpair>* vTracks, long currentJob_fvt_id) {
	//				alarm(20);
	tcp::iostream stream;
	tcp::endpoint endpoint(tcp::v4(), iPort);
	tcp::acceptor acceptor(*io_service, endpoint);

	tcp::endpoint remote_ep;

	SMAFELOG_FUNC(SMAFELOG_INFO, "Waiting for incoming connection on port " + stringify(iPort));
	//acceptor.accept(*stream.rdbuf(), remote_ep);
	//				acceptor.async_accept(*stream.rdbuf(), main_loop_round_livemode);
 *acceptor.async_accept(*stream.rdbuf(), boost::bind(&main_loop_round_livemode, &stream, vTracks, currentJob_fvt_id, io_service, boost::asio::placeholders::error()));
	//				acceptor.async_accept(*stream.rdbuf(), boost::bind(&main_loop_round_livemode, stream, boost::asio::placeholders::error()));

}
 */


// ------------------------------------------------------------------------
// one round of the main loop in live mode
void main_loop_round_livemode(tcp::iostream* stream, const tlonglonglongsafvStructVectorMap &lllsafvMap, long currentJob_fvt_id) {
	/** top k, as received by wrapd */
	int iLivetopk = 50;
	long collection_id = SmafeStoreDB::RESERVEDCOLLECTIONS_DEFAULT;

	//	if (!error)  {


	// receive
	std::string line, fv, line_plain;
	std::getline(*stream, line);
	SMAFELOG_FUNC(SMAFELOG_DEBUG, "Read line: " + line);

	// check command
	if (trimWhitespace(line) == SMAFELIVEMODE_COMMAND_GETOPTS) {
		SMAFELOG_FUNC(SMAFELOG_DEBUG, "About to send options");
		sendOptions(*stream, so);
	} else {
		tLiveNNMessage msg1;
		tlonglonglongsafvStructVectorMap::const_iterator iter_collection;

		// decrypt
		SMAFELOG_FUNC(SMAFELOG_DEBUG, "Decrypting...");
		line_plain = decryptString(line.c_str(), stdpassphrase);
		//		line_plain = line;

		SMAFELOG_FUNC(SMAFELOG_DEBUG, "Trying to parse as tLiveNNMessage...");

		// de-s11n
		{
			SMAFELOG_FUNC(SMAFELOG_DEBUG2, line_plain);

			std::stringstream ss(std::stringstream::in
					| std::stringstream::out);
			//			ss << boost_s11n_workaround_135(line_plain) << std::endl;
			ss << (line_plain) << std::endl;

			boost::archive::text_iarchive ia(ss);

			// restore  from the archive
			ia >> BOOST_SERIALIZATION_NVP(msg1);
		}

		// Check validity of options
		iLivetopk = msg1.iLivetopk;
		if ( ! (iLivetopk > 0 && iLivetopk < 10000)) throw std::string("Invalid value for topk. Must be between 0 and 10000. Value received: " + stringify(iLivetopk));
		SMAFELOG_FUNC(SMAFELOG_DEBUG, "iLivetopk=" + stringify(iLivetopk));
		collection_id = msg1.lCollectionId;
		if ( ! (collection_id > 0 )) throw std::string("Invalid value for collection_id. Must be greater than 0. Value received: " + stringify(collection_id));
		SMAFELOG_FUNC(SMAFELOG_DEBUG, "collection_id=" + stringify(collection_id));


		iter_collection = lllsafvMap.find(collection_id);
		if (iter_collection == lllsafvMap.end()) {
			throw std::string("collection_id " + stringify(collection_id) + " not found.");
		}


		/*
		Json::Value jsRoot;   // will contain the root value after parsing.
		Json::Reader jsReader;
		bool parsingSuccessful = jsReader.parse( line, jsRoot );
		if ( !parsingSuccessful ) 		{
		    // report to the user the failure and their locations in the document.
			throw std::string("Failed to parse JSON document. More info: ") + jsReader.getFormatedErrorMessages();
		}
		 */

		/*
		// Check validity of options
		if (jsRoot["cmd"] != "nn") throw std::string("Unknown command: ") + stringify(jsRoot["cmd"]);
		fv = jsRoot["fv"].asString();
		iLivetopk = jsRoot["options"]["topk"].asInt();
		if ( ! (iLivetopk > 0 && iLivetopk < 10000)) throw std::string("Invalid value for topk. Must be between 0 and 10000. Value received: " + stringify(iLivetopk));
		collection_id = jsRoot["options"]["collection_id"].asInt();
		if ( ! (collection_id > 0 )) throw std::string("Invalid value for collection_id. Must be greater than 0. Value received: " + stringify(collection_id));
		 */


		sendNearestNeighbour(*stream, msg1.safv, currentJob_fvt_id, so->distancetype_ids[0], iter_collection->second, iLivetopk);
	}
	/*
		// next connection
		if (!b_should_terminate) { //start_accept(io_service, vTracks, currentJob_fvt_id);

			SMAFELOG_FUNC(SMAFELOG_INFO, "Waiting for incoming connection on port " + stringify(iPort));
			//acceptor.accept(*stream.rdbuf(), remote_ep);
			//				acceptor.async_accept(*stream.rdbuf(), main_loop_round_livemode);
			acceptor->async_accept(*stream->rdbuf(), boost::bind(&main_loop_round_livemode, stream, vTracks, currentJob_fvt_id, io_service, acceptor, endpoint, boost::asio::placeholders::error()));
			//				acceptor.async_accept(*stream.rdbuf(), boost::bind(&main_loop_round_livemode, stream, boost::asio::placeholders::error()));

		}
	 */
	//	} else {
	//		throw std::string(error.message());
	//	}
}


// ------------------------------------------------------------------------
// main

/** well that's the entry point of this cute application */
int main(int argc, char* argv[]) {

	try {

		splashScreen("Distance calculation daemon");

		so = new Smafeopt();
		try {
			processCommandLineArguments(argc, argv);
			so->loadConfigFromDb();

			// check log file min req
			if (loglevel_requested < so->min_loglevel) {
				SMAFELOG_FUNC(SMAFELOG_FATAL, "Log level requested is more verbose than allowed by database config. Allowed log level = " + stringify(so->min_loglevel));
				exit(2);
			}

			initDependentVars();
			logOptions();

		} catch (std::string& s) {
			SMAFELOG_FUNC(SMAFELOG_FATAL, s);
			exit(1);
		}

#if defined(SMAFEDISTD_REAL_DAEMON)
		/* Daemonize */
		if (!bNoDaemon)
			daemonize();
#endif


		//----------------- jobs mode ----------------------
		if (iMode == MODE_JOBSMODE) {

			if (bPurgemode) {
				// purging
				purge();
			} else {
				// main loop
				while (!b_should_terminate) {
					main_loop_round_jobsmode();

					if (!b_should_terminate && !bPrintStatsOnly) {
						SMAFELOG_FUNC(SMAFELOG_INFO, "Going to sleep for " + stringify(pollInterval) + " minutes...");
						// check if interval is < 0: then exit
						if (pollInterval >= 0)
							sleep(pollInterval * 60); // pollInterval is in minutes
						else
							b_should_terminate = true;
					}
					if (bPrintStatsOnly) {
						SMAFELOG_FUNC(SMAFELOG_INFO, "No action (stats parameter given).");
						b_should_terminate = true;
					}
				}
			}
		} // jobs mode

		//----------------- live mode ----------------------
		if (iMode == MODE_LIVEMODE) {
			// main loop

			/** Collection of tracks to calculate distance to */
			//std::vector<longlongpair> vTracks;
			long currentJob_fvt_id;
			//longlong_set_deque tracks_collections;
			/** hashmap for storing references to feature vectores in vectors, indexed by collection id */
			tlonglonglongsafvStructVectorMap lllsafvMap;




			try {
				// check if port is available
				if (!isPortAvailable(iPort)) throw std::string("Port " + stringify(iPort) + " is already in use.");

				// initialisation
				initLiveMode(currentJob_fvt_id, fvbuffer, lllsafvMap);

				boost::asio::io_service io_service;

				while (!b_should_terminate) {

					tcp::iostream stream;
					tcp::endpoint endpoint(tcp::v4(), iPort);
					tcp::acceptor acceptor(io_service, endpoint);

					tcp::endpoint remote_ep;

					SMAFELOG_FUNC(SMAFELOG_INFO, "Waiting for incoming connection on port " + stringify(iPort));
					acceptor.accept(*stream.rdbuf(), remote_ep);

					//				if (stream) {
					SMAFELOG_FUNC(SMAFELOG_INFO, "Accepted connection from " + stringify(remote_ep));
					main_loop_round_livemode(&stream, lllsafvMap, currentJob_fvt_id);
					//				} else {
					//					b_should_terminate = true;
					//					SMAFELOG_FUNC(SMAFELOG_INFO, "Returned from accept(..) without a connection?!");
					//				}
				}


				//				acceptor.async_accept(*stream.rdbuf(), main_loop_round_livemode);
				//			acceptor.async_accept(*stream.rdbuf(), boost::bind(&main_loop_round_livemode, &stream, &vTracks, currentJob_fvt_id, &io_service, &acceptor, &endpoint, boost::asio::placeholders::error()));
				//				acceptor.async_accept(*stream.rdbuf(), boost::bind(&main_loop_round_livemode, stream, boost::asio::placeholders::error()));


				//			start_accept(&io_service, &vTracks, currentJob_fvt_id);
				//			io_service.run();

			} catch (const std::exception& e) {
				SMAFELOG_FUNC(SMAFELOG_FATAL, e.what());
			}
			catch (const std::string& s) {
				SMAFELOG_FUNC(SMAFELOG_FATAL, s);
			}
			/*
			 * None of these catch clauses in fact catches a c string literal (char*)!
			catch (const char* ch) {
				SMAFELOG_FUNC(SMAFELOG_FATAL, std::string(ch));
			}
			catch ( char const *ch) {
				SMAFELOG_FUNC(SMAFELOG_FATAL, "2" + std::string(ch));
			}
			catch ( char  *ch) {
				SMAFELOG_FUNC(SMAFELOG_FATAL, "3" + std::string(ch));
			}
			 */
		}

		SMAFELOG_FUNC(SMAFELOG_INFO, "Daemon says Good Bye.");

		delete so;

	} catch (...) {
		// Catch all
		// Try to log, if that does not work, write to stderr
		try {
			SMAFELOG_FUNC(SMAFELOG_FATAL, "Uncaught exception :-(");
		} catch (...) {
			std::cerr << "Uncaught exception :-(" << std::endl;
		}
		exit(1);
	}

}
