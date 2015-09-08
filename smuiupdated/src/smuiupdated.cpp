///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2010 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// smuiupdated.cpp
//
// SpectralMind Audio Feature Extraction Wrapper
// Main file
// ------------------------------------------------------------------------
//
//
// Version $Id: smafewrapd.cpp 393 2010-03-23 18:30:50Z ewald $
//
//
//
//
//
//
///////////////////////////////////////////////////////////////////////////



// Boost 1.44 introduces a Version 3 of the Filesystem library.
// Since our code works only with Version 2 we define this constant.
// According to docs, deprecated Version 2 is supported up to Boost 1.47
#define BOOST_FILESYSTEM_VERSION 2

// ------------------------------------------------------------------------
// includes
#include "config.h"

#include "smafedaemon1.h"

#include <string>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include "argtable2.h"
#include <ctime>
#include <algorithm>

#include "smafeutil.h"
#include "smafestoredb.h"
#include "smafeDistancesCalc.h"

#include "smafestore_specific_include.h"

#include "boost/filesystem.hpp"
#include "boost/tokenizer.hpp"
#include <boost/lexical_cast.hpp>

#include "smafeLogger.h"
#include "smafeopt.h"

// ------------------------------------------------------------------------
// namespace defs
/** alias for boost::filesystem */
namespace fs = boost::filesystem;

// ------------------------------------------------------------------------
// typedefs
/** vector of feature vectors wrapped in smart pointers to store codebook vectors in it
 * (it is assumed that all fvs are of the same fv type!) */
typedef std::vector<SmafeNumericFeatureVector_Ptr> tNumericalFvVector;

// ------------------------------------------------------------------------
// constants
/** program name */
const char PROGNAME[] = "smuiupdated";


// ------------------------------------------------------------------------
// global vars
/** options */
Smafeopt* so;
// start daemon related
/** defualt polling interval in min */
const int DEFAULT_POLLING_INTERVAL = 10;
/** identifier for this daemon process */
std::string daemonId;
/** poll interval in minutes */
int pollInterval = DEFAULT_POLLING_INTERVAL;
/** name of logfile */
std::string sLogfilename;
/** loglevel requested */
int loglevel_requested;
/** if daemon is to be terminated after next finished job
 * <p>This flag is set to true if term signal is caught */
bool b_should_terminate = false;
// end daemon related
// start work releated
/** vector of feature vectors, filled from codebook */
tNumericalFvVector fvbuffer;
// end work releated
// strat properties of codebook (set at runtime, wehn codebook is loaded)
/** Dimensionality of the vectors */
int iCodebookDims;
/** Map dimension in x and y -direction*/
int iCodebookDimx, iCodebookDimy;
/** type of topology. true=hexa, false=rect */
bool bCodebookTopoltype;
/** record for the dbinfo information */
tDbinfoRecord dbinfo_rec;
/** flag: db info loaded? */
bool bDbinfoLoaded = false;
// end vars set during runtime


// ------------------------------------------------------------------------
// global options
// start program behaviour options
/** only print open tasks? */
bool bPrintStatsOnly = false;
// end program behaviour options
// start program options (to be read from database later on)
/** featurevectortype id */
long lFvtId;
// end program options


// leave that include here
#include "smafedaemon2.h"

/** Processes command line arguments using argtable library
 <p>To add a smafeopt class backed command line argument,
 perform the following steps:
 <ol>
 <li>Add public member in smafeopt.h			(smafe)
 <li>Assign default value in smafeopt.cpp	(smafe)
 <li>Define a new struct (arg_lit, arg_int etc) in this method
 (processCommandLineArguments()) and add details  (shortopts, longopts)
 according to appropriate constructor
 <li>Add this struct to void* argtable[] array
 <li>Transfer the value from the struct to the smafeopt instance
 (<i>if (nerrors == 0)</i> block)
 </ol>
 * @param argc number of command line arguments (from main())
 * @param argv array of c-strings (from main())
 * @param so initialized, empty  smafeopt instance that is to be filled
 */
void processCommandLineArguments(int argc, char* argv[]) {

	/* Define the allowable command line options, collecting them in argtable[] */
	/* Syntax 1: command line arguments and file or dir */
	// daemon options
	struct arg_str
	*arg_daemonID =
			arg_str0(
					NULL,
					"id",
					"IDENTIFIER",
					"Identifier for daemon instance. IDENTIFIER must not contain whitespace or special chars.");
	struct arg_int *arg_interval = arg_int0(NULL, "interval", "MINUTES",
			"Polling interval in minutes. Default is 10");
	struct arg_lit
	*arg_no_daemon =
			arg_lit0(NULL, "no-daemon",
					"Runs program as normal forground process (no forking). No logfile allowed");
	struct arg_str *arg_log = arg_str0(NULL, "log", "FILENAME",
			"Name of logfile. If not specified, a random name is chosen.");
	struct arg_lit *arg_stats = arg_lit0(NULL, "stats",
			"Print number of open tasks and exit");


	//struct arg_str *arg_passphrase = arg_str0("p", "passphrase", "PASSPHRASE", "Passphrase for database encryption (max 63 characters)");
	struct arg_int
	*arg_verbose =
			arg_int0(
					"v",
					"verbosity",
					"0-6",
					"Set verbosity level (=log level). The lower the value the more verbose the program behaves. Default is 3");

	struct arg_int *arg_lFvtId = arg_int1("f", "featurevectorype_id",
			"FEATUREVECTORTYPE_ID",
			"Featurevectortype_id to use. Must be contained in the database.");
	struct arg_str *arg_dbconf = arg_str0(NULL, "dbconf", "DATABASE-CONFIGURATION-FILE",
			"Specify file that contains database connection details");
	struct arg_lit *help = arg_lit0(NULL, "help", "print this help and exit");
	struct arg_end *end = arg_end(20);

	void* argtable[] = { arg_daemonID, arg_lFvtId,
			arg_no_daemon, arg_interval, arg_dbconf, arg_verbose, arg_log,
			arg_stats, help, end };

	int nerrors;

	/* verify the argtable[] entries were allocated sucessfully */
	if (arg_nullcheck(argtable) != 0) {
		/* NULL entries were detected, some allocations must have failed */
		std::cerr << PROGNAME << ": insufficient memory" << std::endl;
		exit(2);
	}

	// if no parameter is given: show help
	if (argc > 1) {
		// Parse the command line as defined by argtable[]
		nerrors = arg_parse(argc, argv, argtable);
	} else {
		// no argument given
		help->count = 1;
	}

	/* special case: '--help' takes precedence over error reporting */
	if (help->count > 0) {
		std::cout << "Usage: " << PROGNAME;
		arg_print_syntax(stdout, argtable, "\n");
		std::cout << std::endl;
		arg_print_glossary(stdout, argtable, "  %-27s %s\n");
		std::cout << "" << std::endl;

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
		exit(1);
	}

	if (nerrors == 0) {
		// verbosity level
		// must be on top
		if (arg_verbose->count > 0) {
			loglevel_requested = arg_verbose->ival[0];
			// change loglevel
			SmafeLogger::smlog->setLoglevel(loglevel_requested);
		} else
			loglevel_requested = SmafeLogger::DEFAULT_LOGLEVEL;

		// identifier
		if (arg_daemonID->count > 0) {
			daemonId = std::string(arg_daemonID->sval[0]);
			if (daemonId == SmafeStoreDB::STATUSOK || (0 == daemonId.find(SmafeStoreDB::STATUSFAILED))) {
				SMAFELOG_FUNC(SMAFELOG_FATAL, "Identifier " + daemonId + " is illegal. It must not be '"+SmafeStoreDB::STATUSOK+"' and must not start with '"+SmafeStoreDB::STATUSFAILED+"'");
				exit(2);
			}
		} else {
			if (arg_stats->count == 0) {
				// is actually mandatory, only for --stats and --list it is not.
				SMAFELOG_FUNC(SMAFELOG_FATAL, "Please specify an identifier for this daemon (--id).");
				exit(2);
			}
		}

		// logfile
		// uses daemonId
		if (arg_log->count > 0) {
			sLogfilename = std::string(arg_log->sval[0]);
		} else {
			sLogfilename = std::string(PROGNAME) + "." + stringify(my_getpid())
																			+ ".log";
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

		// stats
		if (arg_stats->count > 0) {
			bPrintStatsOnly = true;
#if defined(SMAFEDISTD_REAL_DAEMON)
			SMAFELOG_FUNC(SMAFELOG_INFO, "Stats mode, so running in 'normal mode' (ie, not as daemon)");
			bNoDaemon = true;
#endif
		} else
			bPrintStatsOnly = false;



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

		// fvtid
		if (arg_lFvtId->count > 0)
			lFvtId = arg_lFvtId->ival[0];
		else {
			SMAFELOG_FUNC(SMAFELOG_FATAL, "Specify fvtype_id");
			exit(2);
		}
		if (lFvtId < 0) {
			SMAFELOG_FUNC(SMAFELOG_FATAL, "Featurevectortype_id cannot be < 0");
			exit(2);
		}

		// ---- db stuff
		// db options file
		if (arg_dbconf->count > 0) {
			SMAFELOG_FUNC(SMAFELOG_DEBUG, "Parsing db configuration file");
			so->parseDbOpts(std::string(arg_dbconf->sval[0]));
		}
		/*
		// Passphrase
		if (arg_passphrase->count > 0) {
			if (strlen(arg_passphrase->sval[0]) <= 63) {
				strcpy(verysecretpassphrase, arg_passphrase->sval[0]);
				SMAFELOG_FUNC(SMAFELOG_INFO, "Data encryption / decryption is enabled.");
			} else {
				SMAFELOG_FUNC(SMAFELOG_FATAL, "Passphrase too long. Max 63 characters.");
				exit(2);
			}
		} else {
			SMAFELOG_FUNC(SMAFELOG_INFO, "Data encryption / decryption is DISABLED!");
		}
		 */

		// switch to logfile was here

	} else {
		arg_print_errors(stdout, end, PROGNAME);
		std::cout << "--help gives usage information" << std::endl;
		exit(1);
	}
	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
}



/** Initializes variables that are dependent on the configuration loaded from db
 * Note: DB-Vars must be loaded */
void initDependentVars() {

}


/** Logs all options that we use  and taht are specific to this program */
void logOptions() {
	// mirror all params
	SMAFELOG_FUNC(SMAFELOG_INFO, "dbhost=" + stringify(so->strDbhost));
	SMAFELOG_FUNC(SMAFELOG_INFO, "dbname=" + stringify(so->strDbname));
	SMAFELOG_FUNC(SMAFELOG_INFO, "dbuser=" + stringify(so->strDbuser));
	SMAFELOG_FUNC(SMAFELOG_INFO, "dbpwd=**************"); // ;-)
	SMAFELOG_FUNC(SMAFELOG_INFO, "polling interval=" + stringify(pollInterval));
	SMAFELOG_FUNC(SMAFELOG_INFO, "code book=" + so->sCodebookfile);
	SMAFELOG_FUNC(SMAFELOG_INFO, "dMaxBubbleVincinityFactor=" + stringify(so->dMaxBubbleVincinityFactor));
	SMAFELOG_FUNC(SMAFELOG_INFO, "feature vector type id =" + lFvtId);
}



/** load and store codebook
 * Uses the following global vars:
 * -fvbuffer (writing)
 * -sCodebookfile (reading)
 * throws exception in case of error
 */
void loadCodebook() {

	// construct file input stream
	std::ifstream ifsCodebook(so->sCodebookfile.c_str());
	std::string line, token, devnull;
	std::stringstream ssLines(std::stringstream::in | std::stringstream::out);
	/** counts vectors */
	int c = 0;
	/** ocunt lines */
	int l = 0;
	/** counts element of vector */
	int c1 = 0;
	/** the fvtype */
	SmafeFVType fvt;
	/** buffer for storing one vector */
	double* dbuf;
	/** pointer to fv */
	SmafeNumericFeatureVector_Ptr snfv_ptr;
	/** separators */
	boost::char_separator<char> mysep(" ");

	if (ifsCodebook.is_open()) {

		SMAFELOG_FUNC(SMAFELOG_INFO, "Reading codebook file " + so->sCodebookfile);

		ifsCodebook >> iCodebookDims;
		SMAFELOG_FUNC(SMAFELOG_DEBUG, "iCodebookDims=" + stringify(iCodebookDims));
		ifsCodebook >> token;
		bCodebookTopoltype = token == "hexa";
		SMAFELOG_FUNC(SMAFELOG_DEBUG, "bCodebookTopoltype=" + stringify(bCodebookTopoltype) + " (true=hexa, false=rect)");
		ifsCodebook >> iCodebookDimx;
		SMAFELOG_FUNC(SMAFELOG_DEBUG, "x width=" + stringify(iCodebookDimx));
		ifsCodebook >> iCodebookDimy;
		SMAFELOG_FUNC(SMAFELOG_DEBUG, "y height=" + stringify(iCodebookDimy));
		ifsCodebook >> devnull; // discarded

		// create feature vector type
		fvt.name = "loaded_from_file";
		fvt.version = 1;
		fvt.setProperties(iCodebookDims, 1, "Loaded from som_pak codebook "
				+ so->sCodebookfile);

		// pre-allocation of vector and buffer
		fvbuffer.reserve(iCodebookDimx * iCodebookDimy);
		dbuf = new double[iCodebookDims];
		// from here to the delete[] there must not be an exit from the program flow!

		// read lines until end
		while (!ifsCodebook.fail()) {
			getline(ifsCodebook, line);
			l++;

			if (line != "") {

				//			std::cout << line << std::endl;

				typedef boost::tokenizer<boost::char_separator<char> >
				tMyTokenizer;
				tMyTokenizer tok(line, mysep);
				c1 = 0;
				// iterate throw vector eleemnts
				for (tMyTokenizer::iterator beg = tok.begin(); beg != tok.end(); ++beg) {
					//				std::cout << c1 << std::endl;
					//				std::cout << *beg << std::endl;

					if (c1 >= iCodebookDims) {
						throw std::string("Unexpected vector length in line "
								+ stringify(l) + ". Expected " + stringify(
										iCodebookDims) + " but found  " + stringify(c1
												+ 1) + " or more.");
					}
					// store in buffer

					try {
						dbuf[c1] = boost::lexical_cast<double>(*beg);
					} catch (const boost::bad_lexical_cast &) {
						throw std::string("Conversion error in line "
								+ stringify(l) + ", token '" + *beg + "'");
					}

					++c1;
				}

				// check if number match
				if (c1 != iCodebookDims) {
					SMAFELOG_FUNC(SMAFELOG_WARNING,
							"Unexpected vector length in line "+stringify(l)+". Expected "
							+ stringify(iCodebookDims) + " but found "
							+ stringify(c1));
				}

				SmafeNumericFeatureVector *snfv =
						new SmafeNumericFeatureVector(dbuf, &fvt, true);
				snfv_ptr.reset(snfv);
				fvbuffer.push_back(snfv_ptr);

				//counter
				c++;

				// progress
				if (c % 1000 == 0) {
					SMAFELOG_FUNC(SMAFELOG_INFO, "...read " + stringify(c) + " lines...");
				}
			}
		}

		// close file
		ifsCodebook.close();

		// de-allocate buffer
		delete[] dbuf;

		// check if number match
		if (c != iCodebookDimx * iCodebookDimy) {
			throw std::string(
					"Unexpected numbers of prototype vectors. Expected "
					+ stringify(iCodebookDimx * iCodebookDimy)
					+ " vectors but read " + stringify(c));
		}
	} else {
		throw std::string("Could not open codebookfile: ") + so->sCodebookfile;
	}
	SMAFELOG_FUNC(SMAFELOG_INFO, "Loaded " + stringify(c) + " prototype vectors");
}

/** Performs one smui add track job.
 * @param at_rec IN/OUT the job record (pointer to)
 * Throws exception of type std::string wiht error message in case of a problem
 */
void performSmuiaddtrack(SMAFE_STORE_DB_CLASS* db,
		tSmuijob_addtrackRecord* at_rec) {
	// local vars			------------------------
	SmafeAbstractFeatureVector *safv;
	SmafeNumericFeatureVector *snfv;
	double dLowestDistance = std::numeric_limits<double>::max();
	/** the index of the best matching fv */
	long lFvIndex;
	/** x and y positions in som (translated from lFvIndex) */
	long lXPos, lYPos;
	double d;
	/** sql command */
	std::string sqlcmd;
	/** matching bubbles in each layer
	 * Note: are indexed starting from 1
	 * allocate enough memory */
	std::vector<long> vBubbleIds(dbinfo_rec.numberoflayers + 1, 0);
	/** rows affected */
	long lRowsaffected;
	/** string for sql part where FK are changed */
	std::string str_bubbles_fk = "";
	/** range in Som unites */
	long lRange;
	/** tmp string */
	std::string sTmp;

	// load candidate fv 	------------------------
	SMAFELOG_FUNC(SMAFELOG_DEBUG, "Reading:  fvt_id = " + stringify(lFvtId) + ", track_id = " + stringify(at_rec->track_id));

	// read feature vector, but no meta data. Error handling  in smafestore
	safv = db->readFeatureVector(lFvtId, at_rec->track_id);
	/*
	 // check for error
	 if (safv == NULL) {
	 throw "could not load feature vector: fvt_id = " + stringify(lFvtId)
	 + ", track_id = " + stringify(at_rec->track_id);
	 }
	 */
	// cast to numeric
	snfv = dynamic_cast<SmafeNumericFeatureVector*> (safv);
	if (!snfv) {
		throw "Casting has not been successful.";
	}
	// now, safv contains the loaded candidate feature vector

	//writeArrayAsCode(std::cout, "candidate", snfv->buffer, snfv->buflen);


	// iterate and calc all distances------------------------
	long i = -1;
	lFvIndex = -1;
	for (tNumericalFvVector::iterator iter = fvbuffer.begin(); iter
	!= fvbuffer.end(); iter++) {
		i++;
		// calc dist
		// currently, we use always euclidean!
		// -1: this param is not used for Euclidean dist.
		d = SmafeDistancesCalc::getDistance_L2(-1, snfv->buffer, snfv->buflen,
				iter->get()->buffer, iter->get()->buflen);

		//writeArrayAsCode(std::cout, "iter", iter->get()->buffer, iter->get()->buflen);


		// store best distance
		if (d < dLowestDistance) {
			dLowestDistance = d;
			lFvIndex = i;
		}
	}
	if (lFvIndex < 0) {
		throw "Unexpected: lFvIndex is < 0.";
	}
	// translate to coordinates
	lYPos = lFvIndex / iCodebookDimx; // integer division, supposed to returned the biggest integer that is smaller than the float result
	lXPos = lFvIndex % iCodebookDimx;
	SMAFELOG_FUNC(SMAFELOG_INFO, "Mapping track to (" + stringify(lXPos) + "/" + stringify(lYPos) + ")");

	// db updates 	------------------------
	std::string sThisPoint = "ST_SetSRID(ST_MakePoint(" + stringify(lXPos)
																	+ "," + stringify(lYPos) + "),4326)";

	// iterate through layers and adapt bubbles
	for (int l = 1; l < dbinfo_rec.numberoflayers; l++) {
		// get nearest bubble
		lRange = floor(std::min(iCodebookDimx, iCodebookDimy)
		* so->dMaxBubbleVincinityFactor);
		vBubbleIds[l] = db->getNearestBubble(lXPos, lYPos, l, lRange);

		// if -1 returned (no buble found), make a second query with a bigger range
		if (vBubbleIds[l] == -1) {
			SMAFELOG_FUNC(SMAFELOG_INFO, "No bubble found for range " + stringify(lRange) + ". Trying double sized range.");
			vBubbleIds[l] = db->getNearestBubble(lXPos, lYPos, l, lRange * 2);
			if (vBubbleIds[l] == -1) {
				SMAFELOG_FUNC(SMAFELOG_WARNING, "Still no bubble found.");
				sTmp = "NULL";
			} else
				sTmp = stringify(vBubbleIds[l]);
		} else
			sTmp = stringify(vBubbleIds[l]);

		// update count and size
		sqlcmd
		= "UPDATE bubbles" + stringify(l)
		+ " SET count=count+1, size=size*sqrt((CAST (count AS float)+1)/CAST (count AS float)) WHERE id="
		+ sTmp + ";";
		lRowsaffected = db->executeStatement(sqlcmd.c_str());
		if (lRowsaffected != 1) {
			throw "Expected exactly one affected row, but found " + stringify(
					lRowsaffected);
		}
		// build fk change string (for track record)
		str_bubbles_fk += ",bubbles" + stringify(l) + "_id=" + sTmp;
	}

	// update geom and foreign keys in trac
	sqlcmd = "UPDATE track SET geom=" + sThisPoint + str_bubbles_fk
			+ " WHERE id=" + stringify(at_rec->track_id) + ";";
	lRowsaffected = db->executeStatement(sqlcmd.c_str());
	if (lRowsaffected != 1) {
		throw "Expected exactly one affected row, but found " + stringify(
				lRowsaffected);
	}
}

/** Performs one smui delete track job.
 * @param at_rec IN/OUT the job record (pointer to)
 * Throws exception of type std::string wiht error message in case of a problem
 */
void performSmuideletetrack(SMAFE_STORE_DB_CLASS* db,
		tSmuijob_deletetrackRecord* dt_rec) {
	// local vars			------------------------
	/** sql command */
	std::string sqlcmd;
	/** matching bubbles in each layer
	 * Note: are indexed starting from 1
	 * allocate enough memory */
	std::vector<long> vBubbleIds(dbinfo_rec.numberoflayers + 1, 0);
	/** rows affected */
	long lRowsaffected;
	/** string for sql part where FK are changed */
	std::string str_bubbles_fk = "";
	/** tmp string */
	std::string sTmp;
	/** bubble count */
	long lCount;
	/** bubble id */
	long lBubbleId;

	// update geom  in track
	sqlcmd = "UPDATE track SET geom=NULL WHERE id="
			+ stringify(dt_rec->track_id) + ";";
	lRowsaffected = db->executeStatement(sqlcmd.c_str());
	if (lRowsaffected != 1) {
		throw "Expected exactly one affected row, but found " + stringify(
				lRowsaffected);
	}

	// iterate through layers and adapt bubbles
	for (int l = 1; l < dbinfo_rec.numberoflayers; l++) {
		// get id and count
		db->getBubbleInfo(l, dt_rec->track_id, lBubbleId, lCount);

		if (lBubbleId < 0) {
			// some error
			throw "Bubble in layer " + stringify(l) + " for track "
					+ stringify(dt_rec->track_id) + " not found. Maybe this job has already been executed?";
		}

		// set the fk for this bubble layer to NULL
		sqlcmd = "UPDATE track SET bubbles" + stringify(l)
																		+ "_id=NULL WHERE id=" + stringify(dt_rec->track_id) + ";";
		lRowsaffected = db->executeStatement(sqlcmd.c_str());
		if (lRowsaffected != 1) {
			throw "Expected exactly one affected row, but found " + stringify(
					lRowsaffected);
		}

		// count > 1 ?
		if (lCount > 1) {
			// decrease count by one
			// adapt size: multiply with factor f where
			//		f = sqrt(new count / count)
			//		new count ... count - 1
			sqlcmd
			= "UPDATE bubbles" + stringify(l)
			+ " SET count=count-1, size=size*sqrt((CAST (count AS float)-1)/CAST (count AS float)) WHERE id="
			+ stringify(lBubbleId) + ";";
			lRowsaffected = db->executeStatement(sqlcmd.c_str());
			if (lRowsaffected != 1) {
				throw "Expected exactly one affected row, but found "
				+ stringify(lRowsaffected);
			}
		} else {
			// remove bubble
			sqlcmd = "DELETE FROM bubbles" + stringify(l) + " WHERE id="
					+ stringify(lBubbleId) + ";";
			lRowsaffected = db->executeStatement(sqlcmd.c_str());
			if (lRowsaffected != 1) {
				throw "Expected exactly one affected row, but found "
				+ stringify(lRowsaffected);
			}
		}

	}

}

// ------------------------------------------------------------------------
/** one round of the main loop
 * dispatcher function
 */
void main_loop_round() {
	/** open task for one job category? */
	bool bVacancy = true;
	/** open task for any job category? */
	bool bAnyvacancy = true;
	long lNumVacancies, lNumCurrentVac;
	SMAFE_STORE_DB_CLASS* db = NULL;
	/** sql command */
	std::string sqlcmd;
	/** will be set to true if error has occured */
	bool bErrorOccured = false;
	/** job record */
	tSmuijob_addtrackRecord at_rec;
	/** job record */
	tSmuijob_deletetrackRecord dt_rec;

	try {
		// create db connection
		db = new SMAFE_STORE_DB_CLASS();
		db->openConnection(so->strDbhost, so->strDbport, so->strDbuser, so->strDbpwd, so->strDbname);

		// load db info if not yet done
		if (!bDbinfoLoaded) {
			try {
				db->getDbinfo(dbinfo_rec);
				SMAFELOG_FUNC(SMAFELOG_DEBUG, "dbinfo loaded.");

				// compare dimensions with the ones from codebook
				if (dbinfo_rec.dimx != iCodebookDimx) {
					throw std::string("Mismatch of SOM width: " + stringify(
							dbinfo_rec.dimx) + " vs "
							+ stringify(iCodebookDimx));
				}
				if (dbinfo_rec.dimy != iCodebookDimy) {
					throw std::string("Mismatch of SOM height: " + stringify(
							dbinfo_rec.dimy) + " vs "
							+ stringify(iCodebookDimy));
				}
				bDbinfoLoaded = true;
			} catch (std::string& s) {
				SMAFELOG_FUNC(SMAFELOG_FATAL, s);
				bErrorOccured = true;
			}
		}

		while (bAnyvacancy && !b_should_terminate && !bErrorOccured) {

			bAnyvacancy = false;

			// START ------- add track jobs -------------

			// get info on job market situation
			bVacancy = db->getOpenTaskInfo_smuiaddtrack(at_rec, lNumVacancies,
					lNumCurrentVac);
			bAnyvacancy = bAnyvacancy || bVacancy;

			// Announce open tasks
			SMAFELOG_FUNC(SMAFELOG_INFO, stringify(lNumVacancies) + " open add track task(s) found.");

			if (bVacancy && !bPrintStatsOnly) {
				// Begin local log
				/** stringstream for local log */
				std::stringstream llog;

				SmafeLogger::smlog->setDesttmp(&llog, at_rec.log, std::string(
						PROGNAME) + ", pid=" + stringify(my_getpid()));

				// Mark task as being processed in db
				db->startTransaction();
				// stricter concurrency control
				db->executeStatement(
						"SET TRANSACTION ISOLATION LEVEL SERIALIZABLE");

				sqlcmd = "UPDATE smuijob_addtrack SET status='" + daemonId
						+ "', started=CURRENT_TIMESTAMP WHERE id=" + stringify(
								at_rec.id);

				// NB: Since we have strict isolation level the next statement will block if
				// another process P has already performed an (uncommitted) update (or deletion) against
				// the same row
				// After P commits its changes our statement may fail
				// with an "SERIALIZATION FAILURE". In this case the function
				// will return false and this process will _not_ work on the task
				// In case that P rolls back its changes we can continue
				// normally and work at this task as expected.
				if (db->executeStatement_serializationcheck(sqlcmd.c_str())) {
					db->finishTransaction(true);

					SMAFELOG_FUNC(SMAFELOG_INFO, "Starting smuiaddtrack task: id=" + stringify(at_rec.id) +
							std::string(", track_id=") + stringify(at_rec.track_id));

					// for "benchmark"
					clock_t begin_clock = clock();
					time_t begin_time = time(NULL);

					try {
						// start transaction
						// that can happen here because in the loop we did not insert anything
						// Remember that insert.. only collects the data in a vector and only
						// at commit time the corresponding sql COPY command is generated and executed
						db->startTransaction();

						// do something
						performSmuiaddtrack(db, &at_rec);

						// write OK status code
						std::string sCode = SmafeStoreDB::STATUSOK;
						sqlcmd = "UPDATE smuijob_addtrack SET status='" + sCode
								+ "', finished=CURRENT_TIMESTAMP, log='"
								+ db->escapeString(llog.str()) + "' WHERE id="
								+ stringify(at_rec.id);
						db->executeStatement(sqlcmd.c_str());

						SMAFELOG_FUNC(SMAFELOG_DEBUG, "Committing");
						db->finishTransaction(true);

						// log time
						clock_t end_clock = clock();
						time_t end_time = time(NULL);
						SMAFELOG_FUNC(SMAFELOG_INFO, "Finished smuiaddtrack task: id=" + stringify(at_rec.id) +
								std::string(", track_id=") + stringify(at_rec.track_id)
								+ " in " +
								stringify(difftime (end_time,begin_time)) + " s (usr time " +
								stringify(diffclock(end_clock,begin_clock)) + " ms)");
					} catch (std::string& s) {
						SMAFELOG_FUNC(SMAFELOG_WARNING, "Exception occured during smuiaddtrack job.");
						SMAFELOG_FUNC(SMAFELOG_WARNING, s);
						SMAFELOG_FUNC(SMAFELOG_ERROR, "Rolling back and marking job record as FAILED");
						db->finishTransaction(false);

						// Marking as ERROR
						db->startTransaction();
						sqlcmd = "UPDATE smuijob_addtrack SET status='"
								+ SmafeStoreDB::STATUSFAILED
								+ "', finished=CURRENT_TIMESTAMP, log='"
								+ db->escapeString(llog.str()) + "' WHERE id="
								+ stringify(at_rec.id);
						db->executeStatement(sqlcmd.c_str());

						db->finishTransaction(true);

					} // end of catch block

				} else { // if (db->finishTransaction_serializationcheck())
					SMAFELOG_FUNC(SMAFELOG_INFO, "Ooops. Another daemon took my task just when I was about to start...");
					db->finishTransaction(false);
				} // if (db->finishTransaction_serializationcheck())

				// end local log
				SmafeLogger::smlog->resetDesttmp();

			} else {
				if (!bPrintStatsOnly)
					SMAFELOG_FUNC(SMAFELOG_INFO, "No open addtrack tasks found");
			} // if (bVacancy && !bPrintStatsOnly)

			// END ------- add track jobs -------------


			// START ------- delete track jobs -------------

			// get info on job market situation
			bVacancy = db->getOpenTaskInfo_smuideletetrack(dt_rec,
					lNumVacancies, lNumCurrentVac);
			bAnyvacancy = bAnyvacancy || bVacancy;

			// Announce open tasks
			SMAFELOG_FUNC(SMAFELOG_INFO, stringify(lNumVacancies) + " open delete track task(s) found.");

			if (bVacancy && !bPrintStatsOnly) {
				// Begin local log
				/** stringstream for local log */
				std::stringstream llog;

				SmafeLogger::smlog->setDesttmp(&llog, at_rec.log, std::string(
						PROGNAME) + ", pid=" + stringify(my_getpid()));

				// Mark task as being processed in db
				db->startTransaction();
				// stricter concurrency control
				db->executeStatement(
						"SET TRANSACTION ISOLATION LEVEL SERIALIZABLE");

				sqlcmd = "UPDATE smuijob_deletetrack SET status='" + daemonId
						+ "', started=CURRENT_TIMESTAMP WHERE id=" + stringify(
								dt_rec.id);

				// NB: Since we have strict isolation level the next statement will block if
				// another process P has already performed an (uncommitted) update (or deletion) against
				// the same row
				// After P commits its changes our statement may fail
				// with an "SERIALIZATION FAILURE". In this case the function
				// will return false and this process will _not_ work on the task
				// In case that P rolls back its changes we can continue
				// normally and work at this task as expected.
				if (db->executeStatement_serializationcheck(sqlcmd.c_str())) {
					db->finishTransaction(true);

					SMAFELOG_FUNC(SMAFELOG_INFO, "Starting smuideletetrack task: id=" + stringify(dt_rec.id) +
							std::string(", track_id=") + stringify(dt_rec.track_id));

					// for "benchmark"
					clock_t begin_clock = clock();
					time_t begin_time = time(NULL);

					try {
						// start transaction
						// that can happen here because in the loop we did not insert anything
						// Remember that insert.. only collects the data in a vector and only
						// at commit time the corresponding sql COPY command is generated and executed
						db->startTransaction();

						// do something
						performSmuideletetrack(db, &dt_rec);

						// write OK status code
						std::string sCode = SmafeStoreDB::STATUSOK;
						sqlcmd = "UPDATE smuijob_deletetrack SET status='"
								+ sCode
								+ "', finished=CURRENT_TIMESTAMP, log='"
								+ db->escapeString(llog.str()) + "' WHERE id="
								+ stringify(dt_rec.id);
						db->executeStatement(sqlcmd.c_str());

						SMAFELOG_FUNC(SMAFELOG_DEBUG, "Committing");
						db->finishTransaction(true);

						// log time
						clock_t end_clock = clock();
						time_t end_time = time(NULL);
						SMAFELOG_FUNC(SMAFELOG_INFO, "Finished smuideletetrack task: id=" + stringify(dt_rec.id) +
								std::string(", track_id=") + stringify(dt_rec.track_id)
								+ " in " +
								stringify(difftime (end_time,begin_time)) + " s (usr time " +
								stringify(diffclock(end_clock,begin_clock)) + " ms)");
					} catch (std::string& s) {
						SMAFELOG_FUNC(SMAFELOG_WARNING, "Exception occured during smuideletetrack job.");
						SMAFELOG_FUNC(SMAFELOG_WARNING, s);
						SMAFELOG_FUNC(SMAFELOG_ERROR, "Rolling back and marking job record as FAILED");
						db->finishTransaction(false);

						// Marking as ERROR
						db->startTransaction();
						sqlcmd = "UPDATE smuijob_deletetrack SET status='"
								+ SmafeStoreDB::STATUSFAILED
								+ "', finished=CURRENT_TIMESTAMP, log='"
								+ db->escapeString(llog.str()) + "' WHERE id="
								+ stringify(dt_rec.id);
						db->executeStatement(sqlcmd.c_str());

						db->finishTransaction(true);

					} // end of catch block

				} else { // if (db->finishTransaction_serializationcheck())
					SMAFELOG_FUNC(SMAFELOG_INFO, "Ooops. Another daemon took my task just when I was about to start...");
					db->finishTransaction(false);
				} // if (db->finishTransaction_serializationcheck())

				// end local log
				SmafeLogger::smlog->resetDesttmp();

			} else {
				if (!bPrintStatsOnly)
					SMAFELOG_FUNC(SMAFELOG_INFO, "No open deletetrack tasks found");
			} // if (bVacancy && !bPrintStatsOnly)

			// END ------- delete track jobs -------------


			if (bPrintStatsOnly)
				b_should_terminate = true;

		} // while


	} catch (std::string& s) {
		SMAFELOG_FUNC(SMAFELOG_FATAL, s);
		bErrorOccured = true;
	}

	delete db;

	// exit in case of error if not daemon mode
	if (bNoDaemon && bErrorOccured)
		exit(1);

}

/** well that's the entry point of this cute application */
int main(int argc, char* argv[]) {

	try {

		splashScreen("Smuiupdate daemon");

		// Process command line options
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
			exit(2);
		}

		// Load code book
		try {
			loadCodebook();

		} catch (std::string& s) {
			SMAFELOG_FUNC(SMAFELOG_FATAL, "Error reading codebook: " + s);
			exit(1);
		}

		if (!bNoDaemon) {
#if defined(SMAFEDISTD_REAL_DAEMON)

			// -------- switch to logfile

			// new log
			// new log must be init before fork(). Why? Don't know yet
			SMAFELOG_FUNC(SMAFELOG_INFO, "Logging to '" + sLogfilename + "'");

			delete SmafeLogger::smlog;
			try {
				SmafeLogger::smlog = new SmafeLogger(sLogfilename,
						SmafeLogger::DEFAULT_LOGLEVEL);
				SmafeLogger::smlog->setLoglevel(loglevel_requested);
				splashScreen("Smafewrap daemon. ID=" + daemonId);
			} catch (std::string& s) {
				std::cerr << s << std::endl;
				std::cerr << "Please provide a valid filename." << std::endl;
				std::cerr << "Exitting" << std::endl;
				exit (3);
			}
			logOptions();

			daemonize();
#endif
		}

		// main loop
		while (!b_should_terminate) {
			main_loop_round();

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

