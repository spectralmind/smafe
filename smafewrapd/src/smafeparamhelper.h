///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// smafeparamhelper.h
//
// SpectralMind Audio Feature Extraction Wrapper
// Static class for param processing
// ------------------------------------------------------------------------
//
//
// Version $Id: smafewrapd.cpp 453 2010-06-24 08:41:07Z ewald $
//
//
//
///////////////////////////////////////////////////////////////////////////


#pragma once

// Boost 1.44 introduces a Version 3 of the Filesystem library.
// Since our code works only with Version 2 we define this constant.
// According to docs, deprecated Version 2 is supported up to Boost 1.47
#define BOOST_FILESYSTEM_VERSION 2

// ------------------------------------------------------------------------
// includes

#include "config.h"

#include <string>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include "argtable2.h"
#include "boost/filesystem.hpp"

#include "smafeopt.h"
#include "smafeutil.h"

#include "smafeLogger.h"


namespace fs = boost::filesystem;
// namespace alias




/** defualt polling interval in min */
const int DEFAULT_POLLING_INTERVAL = 10;
/** Default value for livetopk: how many values are returned in a live query */
const int DEFAULTLIVETOPK = 100;


class SmafeParamHelper {
public:

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
	 * @param ap pointer to admin option struct. May be NULL if not-deploy version
	 * @return false if caller should quit the program (no error, but eg because we only wanted config mode), true otherwise
	 */
	static bool processCommandLineArguments(int argc, char* argv[],
			SmafeFVType_Ptr_map fvts, Smafeopt* so, tAdminparams* ap, bool& bNoDaemon) {


		// extract progname
		fs::path pProgname = fs::path(std::string(argv[0]), fs::native);
		std::string sProgname = stringify(pProgname.leaf()); // this is valid for 1.35, but deprected in latest version. There, use .filename() instead



		// -------------- check for config generation (or admin mode)
		// to be set to true if we have admin mode (not deploy version and admin param given)
		bool bAdmin=false;
#if !defined(DEPLOY)
		// assert struct instance is not NULL
		assert(ap != NULL);
		if (argc >=2 && std::string(argv[1]) == "--admin") bAdmin = true; // NOTE: this corresponds to the argtable struct below! Search for "admin"
#endif

		if (argc >=2 && (std::string(argv[1]) == "--config" || bAdmin)) { // todo make constant
			if (argc >= 3) {
				SmafeParamHelper::processCommandLineArguments_dbset(std::string(argv[2]), fvts, so, bAdmin, sProgname);
			} else {
				std::cerr << "Invalid use of --config or --admin option" << std::endl;
				exit(2);
			}
		}


		// if config mode
		if (argc >=2 && std::string(argv[1]) == "--config") { // todo make constant
			// generate sql file with name = <optionsfile>.sql

			try {
				SmafeParamHelper::generateSqlFile(so, std::string(argv[2]) + ".sql");
			} catch (std::string& s) {
				SMAFELOG_FUNC(SMAFELOG_FATAL, s);
				exit(1);
			}
			return false;

		}
		// now we do not have config mode anymore!

		// let's take care of the rest of the arguments (--no-daemon etc, "light set", but also admin params)
		SmafeParamHelper::processCommandLineArguments_lightset(argc, argv, so, bAdmin, ap, bNoDaemon, sProgname);

		// if not admin mode and not live mode, load options from db
		if (!bAdmin && !ap->bLivemode) so->loadConfigFromDb();

		// check log file min req
		if (ap->loglevel_requested < so->min_loglevel) {
			SMAFELOG_FUNC(SMAFELOG_FATAL, "Log level requested is more verbose than allowed by database config. Allowed log level = " + stringify(so->min_loglevel));
			exit(2);
		}

		// by now, so options and ap options are set (either from command line params or loaded from db)

		return true;
	} // static void processCommandLineArguments


	/** performs the check for license restrictions */
	static void licenseCheck(Smafeopt* so) {

		SMAFE_STORE_DB_CLASS* db;

		// Open db connection and store config in db
		db = new SMAFE_STORE_DB_CLASS();
		db->openConnection(so->strDbhost, so->strDbport, so->strDbuser, so->strDbpwd, so->strDbname);

		// check if soft date is set
		if (so->limitsoftdate.is_not_a_date()) {
			if (so->lastuseddate.is_not_a_date()) {
				// First time run
				SMAFELOG_FUNC(SMAFELOG_DEBUG, "First time run for this database.");
				// set soft limit date if trial is enabled (days > 0)
				// otherwise use the hard limit also for the soft limit
				if (so->limittrialdays > 0)
					so->limitsoftdate = bg::day_clock::local_day() + bg::days(so->limittrialdays);
				else
					so->limitsoftdate = so->limitharddate;
				// set last used date
				so->lastuseddate = bg::day_clock::local_day();

				// store in database
				SMAFELOG_FUNC(SMAFELOG_DEBUG, "Loading configuration from database " + so->strDbname + " on " + so->strDbhost + " with user " + so->strDbuser);


				std::string dummy = db->storeConfigRecord("limitsoftdate", bg::to_iso_extended_string(so->limitsoftdate));
				so->sLastuseddateKey = db->storeConfigRecord("lastuseddate", bg::to_iso_extended_string(so->lastuseddate));


			} else {
				// soft date not there but lastuseddate
				throw std::string("ERROR 101. Please contact support@spectralmind.com");
			}
		} else {
			// soft date is set, do limits check

			// get current date
			bg::date now = bg::day_clock::local_day();

			if (now < so->lastuseddate) {
				SMAFELOG_FUNC(SMAFELOG_DEBUG, "now: " + stringify(now));
				SMAFELOG_FUNC(SMAFELOG_DEBUG, "lastuseddate: " + stringify(so->lastuseddate));
				throw std::string("ERROR 102. Please contact support@spectralmind.com");
			}
			// save last used date
			so->lastuseddate = now;
			// update in db
			db->storeConfigRecord("lastuseddate", bg::to_iso_extended_string(so->lastuseddate), so->sLastuseddateKey);
			// check for violences
			if (so->limitsoftdate < now) {
				SMAFELOG_FUNC(SMAFELOG_DEBUG, "limitsoftdate: " + stringify(so->limitsoftdate));
				SMAFELOG_FUNC(SMAFELOG_DEBUG, "now: " + stringify(now));
				throw std::string("ERROR 103. Please contact support@spectralmind.com");
			}
			if (so->limitharddate < now) {
				SMAFELOG_FUNC(SMAFELOG_DEBUG, "limitharddate: " + stringify(so->limitharddate));
				SMAFELOG_FUNC(SMAFELOG_DEBUG, "now: " + stringify(now));
				throw std::string("ERROR 104. Please contact support@spectralmind.com");
			}
		}

		delete db;

	}



private:
	/** loads given options from file and parses them.
	 * The options correspond to those that would be saved into and loaded from db
	 * This method writes the extracted options to the provided smafeopt instance and may exit
	 * the program in case of error
	 */
	static void processCommandLineArguments_dbset(std::string optfilename, SmafeFVType_Ptr_map fvts, Smafeopt* so, bool bWithHelp, std::string sProgname) {

		// ---------- load opt file
		int argc = 1;
		std::string line;
		std::stringstream ssLines(std::stringstream::in
				| std::stringstream::out);
		std::ifstream myfile(optfilename.c_str());
		if (myfile.is_open()) {
			while (!myfile.eof()) {
				getline(myfile, line);
				ssLines << line << std::endl;
				argc++;
			}
			myfile.close();
		} else {
			std::cerr << "Options file not found: " << optfilename << std::endl;
			exit(1);
		}

		char** argv = new char*[argc];
		argc = 1;
		argv[0] = new char[2]; // no program name
		strcpy(argv[0], "a\0");
		// move arguments from options file to argv array
		// unless it is an empty string or the line starts with an # (=comment)
		while (!ssLines.eof()) {
			getline(ssLines, line);
			trimWhitespace(line);
			if (line != "" && line[0] != '#') {
				argv[argc] = new char[line.length() + 1];
				strcpy(argv[argc], line.c_str());
				argc++;
			}
		}
		// now, argc and argv are set as if those would have been the command line params

		// ------------------------------ start of argument handling

		const int sizeFvts = fvts.size();

		/* Define the allowable command line options, collecting them in argtable[] */
		/* Syntax 1: command line arguments and file or dir */

		struct arg_int *uiSkipin = arg_int0("i", "skipin", "<n>",
				"number of samples to skip at beginning of song");
		struct arg_int *uiSkipout = arg_int0("o", "skipout", "<n>",
				"number of samples to skip at end of song");
		struct arg_int *uiStepwidth = arg_int0("w", "stepwidth", "<n>",
				"stepwith for segment iteration (1 = take each segment)");
		struct arg_int *bReturnSegmentFeatures = arg_int0(NULL,
				"returnSegmentFeatures", "0|1", "store all segments' features in db?");
		struct arg_int *bNormalizeFFTEnergy = arg_int0(NULL, "normalizeFFTEnergy",
				"0|1", "normalize FFT energy?");
		struct arg_int *bTransformDecibel = arg_int0("d", "transformDecibel",
				"0|1", "transform to decibel?");
		struct arg_int *bTransformSone = arg_int0("n", "transformSone", "0|1",
				"transform to sone?");
		struct arg_int *uiModAmplLimit = arg_int0("a", "modAmplLimit", "<n>",
				"which parts to take for rp (~ Matlab's mod_ampl_limit)");
		struct arg_int *bIncludeDC = arg_int0(NULL, "bIncludeDC", "0|1",
				"also take DC component for rp?");
		struct arg_int *shNumBarkBands = arg_int0(NULL, "shNumBarkBands", "<n>",
				"number of bark bands (44kHz audio fills 24)");
		struct arg_int *bFluctuationStrengthWeighting = arg_int0(NULL, "bFlucSW",
				"0|1", "Use fluctuation strength weighting for RP?");
		struct arg_int *bBlurring1 = arg_int0("b", "bBlurring1", "0|1",
				"Blurring, matrix 1?");
		struct arg_int *bBlurring2 = arg_int0("B", "bBlurring2", "0|1",
				"Blurring, matrix 2?");

		//		struct arg_str
		//		*arg_bToTextfile =
		//				arg_str0(
		//						NULL,
		//						"text",
		//						"PREFIX",
		//						"Write feature vectors to textfiles rather than to database, using PREFIX.XX.vec for the filenames where XX indicates the type of feature vector. PREFIX may contain a path.\n\t**\n\tNB: If files with the same names exist they will be *OVERWRITTEN* without notice!\n\t**");
		struct arg_str
		*arg_sFileDest =
				arg_str0(
						NULL,
						"filedest",
						"PATH",
						"Destination for processed files to move to. Empty string means that file is not moved, '-' means that file is deleted.");
		// from smafedistd
		struct arg_int
		*arg_disttype_ids =
				arg_intn(
						"d",
						"disttypes",
						"ID",
						0,
						20,
						"Distance type ids to be calculated, default is no restriction. Parameter may occur more than once.");
		struct arg_int
		*arg_topk =
				arg_int1(
						NULL,
						"topk",
						"N",
						"Sets maximum number of distance records per feature vector/distance pair to persist.");

		// from smuiupdated
		struct arg_str
		*arg_sCodebookfile =
				arg_str1(
						"b",
						"codebook",
						"<som_pak CODEBOOK FILE>",
						"Codebook file that contains the prototype vectors of the som. Must be in som_pak / somtoolbox format.");
		struct arg_dbl
		*arg_rangefactor =
				arg_dbl0(
						"r",
						"bubblerangefactor",
						"FACTOR",
						"Determines max range for nearest bubble. Range = minimal SOM side length * FACTOR. Must be 0 <= FACTOR <= 1. Default is 0.5");


		struct arg_int
		*arg_verbose =
				arg_int0(
						"v",
						"verbosity",
						"0-6",
						"Set minimal verbosity level (=log level). The lower the value the more verbose the program behaves. Default is 3");

		struct arg_str *arg_passphrase = arg_str0("p", "passphrase", "PASSPHRASE", "Passphrase for database encryption");
		struct arg_int
		*arg_limittrackscount =
				arg_int0(
						NULL,
						"limittrackscount",
						"N",
						"Sets maximum number of tracks that the database can hold. 0 ... unlimited. Default is 0");
		struct arg_int
		*arg_limittrialdays =
				arg_int0(
						NULL,
						"limittrialdays",
						"N",
						"Sets number of trial period. Trial period starts at first run of smafewrapd at a new database. 0 ... no restriction. Default is 0");
		struct arg_str
		*arg_limitharddate =
				arg_str0(
						NULL,
						"limitharddate",
						"YYYY-MM-DD",
						"Sets date after which software cannot be used anymore.  Default is some date in the distant future");


		struct arg_lit *help = arg_lit0(NULL, "help", "print this help and exit");
		struct arg_end *end = arg_end(20);

		// dynamic arguments, derived from possible feature vector types
		arg_int** dynamic_args = new arg_int*[sizeFvts];
		// darn arrays of char* strings because strings in C suck big time
		char** strings_in_c_suck = new char*[sizeFvts];
		char** strings_in_c_suck2 = new char*[sizeFvts];
		// iterate through vector of feature vector types
		{
			int i = 0;
			for (SmafeFVType_Ptr_map::iterator iter = fvts.begin(); iter
			!= fvts.end(); iter++, i++) {
				strings_in_c_suck[i] = new char[50];
				strings_in_c_suck2[i] = new char[50];
				sprintf(strings_in_c_suck[i], "bExtract%s", iter->first.c_str());
				sprintf(strings_in_c_suck2[i],
						"enable extraction of %s feature vector type",
						iter->first.c_str());
				dynamic_args[i] = arg_int0(NULL, strings_in_c_suck[i], "0|1",
						strings_in_c_suck2[i]);
			} // end of iterator
		}

		int numNonDynamicArgs = 25; // CHANGE number here!
		// one less if no help
		if (!bWithHelp) numNonDynamicArgs--;

		void** argtable = new void*[numNonDynamicArgs + sizeFvts];
		for (int i = 0; i < sizeFvts; i++)
			argtable[i] = dynamic_args[i];
		argtable[sizeFvts + 0] = uiSkipin;
		argtable[sizeFvts + 1] = uiSkipout;
		argtable[sizeFvts + 2] = uiStepwidth;
		argtable[sizeFvts + 3] = bReturnSegmentFeatures;
		argtable[sizeFvts + 4] = bNormalizeFFTEnergy;
		argtable[sizeFvts + 5] = bTransformDecibel;
		argtable[sizeFvts + 6] = bTransformSone;
		argtable[sizeFvts + 7] = uiModAmplLimit;
		argtable[sizeFvts + 8] = bIncludeDC;
		argtable[sizeFvts + 9] = shNumBarkBands;
		argtable[sizeFvts + 10] = bFluctuationStrengthWeighting;
		argtable[sizeFvts + 11] = bBlurring1;
		argtable[sizeFvts + 12] = bBlurring2;
		//		argtable[sizeFvts + 12] = arg_bToTextfile;
		argtable[sizeFvts + 13] = arg_sFileDest;
		argtable[sizeFvts + 14] = arg_disttype_ids;
		argtable[sizeFvts + 15] = arg_topk;
		argtable[sizeFvts + 16] = arg_sCodebookfile;
		argtable[sizeFvts + 17] = arg_rangefactor;
		argtable[sizeFvts + 18] = arg_verbose;
		argtable[sizeFvts + 19] = arg_passphrase;
		argtable[sizeFvts + 20] = arg_limittrackscount;
		argtable[sizeFvts + 21] = arg_limittrialdays;
		argtable[sizeFvts + 22] = arg_limitharddate;
		if (bWithHelp) {
			argtable[sizeFvts + 23] = help;
			argtable[sizeFvts + 24] = end;
		} else {
			argtable[sizeFvts + 23] = end;
		}
		int nerrors;

		/* verify the argtable[] entries were allocated sucessfully */
		if (arg_nullcheck(argtable) != 0) {
			/* NULL entries were detected, some allocations must have failed */
			std::cerr << sProgname << ": insufficient memory" << std::endl;
			exit(2);
		}


		// Parse the command line as defined by argtable[]
		nerrors = arg_parse(argc, argv, argtable);


		/* special case: '--help' takes precedence over error reporting */
		if (bWithHelp) {
			if (help->count > 0) {
				std::cout << "Usage: " << sProgname << " --config <config file>\n";
#if !defined(DEPLOY)
				std::cout << "Usage: " << sProgname << " --admin <config file>  [other options, list with --help] ... ";
#endif
				std::cout << std::endl << std::endl;

				std::cout << "In --config mode a possibly encrypted SQL script <config file>.sql is generated that is to be exected against the database.  \n";
				std::cout << "In --admin mode the daemon uses the options directly.  \n\n";

				std::cout << "These are the options for config files.\nA config file is a textfile containing command line arguments, each in one line.\n\n";

				arg_print_glossary(stdout, argtable, "  %-27s %s\n");


				std::cout << "\n\n**Somlib special mode**" << std::endl;
				std::cout
				<< "\tIn this mode a somlib file is parsed and its contents are written to db. The <file> parameter or filelist will not be used."
				<< std::endl;
				std::cout
				<< "\tTo enable somlib special mode: set bExtractSomlib to 1 (setting all other extractors to 0)  and specify a somlib file."
				<< std::endl;
				std::cout << "" << std::endl;

				exit(1);
			}
		} // with help

		if (nerrors == 0) {
			// verbosity level
			// must be on top
			if (arg_verbose->count > 0) {
				so->min_loglevel = arg_verbose->ival[0];
				// change loglevel
				SmafeLogger::smlog->setLoglevel(so->min_loglevel);
			} else
				so->min_loglevel = SmafeLogger::DEFAULT_LOGLEVEL;

			// dynamic arguments (uses bJobsMode)
			{
				int i = 0;
				bool bAtLeastOneFv = false;
				for (SmafeFVType_Ptr_map::iterator iter = fvts.begin(); iter
				!= fvts.end(); iter++, i++) {
					if (dynamic_args[i]->count > 0) {
						so->mapExtractGenerally[iter->first]
						                        = dynamic_args[i]->ival[0] == 1;
						bAtLeastOneFv = bAtLeastOneFv
								|| so->mapExtractGenerally[iter->first];
					} else {
						so->mapExtractGenerally[iter->first] = false; // if option is not given, do not extract the feature vector
					}
				} // end of iterator
				// check if anything should be done
				if (!bAtLeastOneFv) {
					SMAFELOG_FUNC(SMAFELOG_FATAL, "Please enable at least one feature vector command line parameter (--bExtractXXX=1).");
					exit(2);
				}
			}

			// fixed arguments
			so->uiSkipin = uiSkipin->count > 0 ? uiSkipin->ival[0] : so->uiSkipin;
			so->uiSkipout = uiSkipout->count > 0 ? uiSkipout->ival[0]
			                                                       : so->uiSkipout;
			so->uiStepwidth = uiStepwidth->count > 0 ? uiStepwidth->ival[0]
			                                                             : so->uiStepwidth;
			so->bReturnSegmentFeatures
			= bReturnSegmentFeatures->count > 0 ? bReturnSegmentFeatures->ival[0]
			                                                                   == 1
			                                                                   : so->bReturnSegmentFeatures;

			so->bNormalizeFFTEnergy
			= bNormalizeFFTEnergy->count > 0 ? bNormalizeFFTEnergy->ival[0]
			                                                             == 1 : so->bNormalizeFFTEnergy;
			so->bTransformDecibel
			= bTransformDecibel->count > 0 ? bTransformDecibel->ival[0]
			                                                         == 1 : so->bTransformDecibel;
			so->bTransformSone
			= bTransformSone->count > 0 ? bTransformSone->ival[0] == 1
					: so->bTransformSone;
			so->uiModAmplLimit
			= uiModAmplLimit->count > 0 ? uiModAmplLimit->ival[0]
			                                                   : so->uiModAmplLimit;
			so->bIncludeDC = bIncludeDC->count > 0 ? bIncludeDC->ival[0] == 1
					: so->bIncludeDC;
			so->shNumBarkBands
			= shNumBarkBands->count > 0 ? shNumBarkBands->ival[0]
			                                                   : so->shNumBarkBands;
			so->bFluctuationStrengthWeighting
			= bFluctuationStrengthWeighting->count > 0 ? bFluctuationStrengthWeighting->ival[0]
			                                                                                 == 1
			                                                                                 : so->bFluctuationStrengthWeighting;
			so->bBlurring1 = bBlurring1->count > 0 ? bBlurring1->ival[0] == 1
					: so->bBlurring1;
			so->bBlurring2 = bBlurring2->count > 0 ? bBlurring2->ival[0] == 1
					: so->bBlurring2;

			if (arg_sFileDest->count > 0) {
				so->sFileDest = std::string(arg_sFileDest->sval[0]);
				// check for tilde:
				if (so->sFileDest.find('~') != so->sFileDest.npos) {
					// tilde found: abort
					SMAFELOG_FUNC(SMAFELOG_FATAL, so->sFileDest + ": Tilde expansion not supported. Please choose a path without ~.");
					exit(2);
				}
				SMAFELOG_FUNC(SMAFELOG_INFO, "processed file destination=" + so->sFileDest);
			}


			// top k
			if (arg_topk->count > 0) {
				if (arg_topk->ival[0] > 0) {
					so->top_k = arg_topk->ival[0];
					//top_k_safe_removable = top_k - 1; should be done in smafedistd, after the config is loaded
				} else {
					SMAFELOG_FUNC(SMAFELOG_FATAL, "Top k parameter must be greater 0. Exiting.");
					exit(1);
				}
			}
			SMAFELOG_FUNC(SMAFELOG_INFO, "topk=" + stringify(so->top_k));


			// distance types
			if (arg_disttype_ids->count > 0) {
				// parameter given
				for (int i = 0; i < arg_disttype_ids->count; i++) {
					so->distancetype_ids.push_back(arg_disttype_ids->ival[i]);
					SMAFELOG_FUNC(SMAFELOG_INFO, "Will also calculate distancetype_id " + stringify(so->distancetype_ids[i]));
				}
			} else {
				// param not given, default values
				// vector remains empty
				SMAFELOG_FUNC(SMAFELOG_INFO, "Will calculate all distance type ids");
			}

			// range bubble
			if (arg_rangefactor->count > 0)
				so->dMaxBubbleVincinityFactor = arg_rangefactor->dval[0];
			if (so->dMaxBubbleVincinityFactor < 0 || so->dMaxBubbleVincinityFactor > 1) {
				SMAFELOG_FUNC(SMAFELOG_WARNING, "Bubble range factor parameter is out of range.");
				exit(2);
			}
			SMAFELOG_FUNC(SMAFELOG_INFO, "bubble range factor=" + stringify(so->dMaxBubbleVincinityFactor));

			// codebook
			so->sCodebookfile = std::string(arg_sCodebookfile->sval[0]);


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

			// limit tracks
			if (arg_limittrackscount->count > 0) {
				if (arg_limittrackscount->ival[0] >= 0) {
					so->limittrackscount = arg_limittrackscount->ival[0];
				} else {
					SMAFELOG_FUNC(SMAFELOG_FATAL, "Track limit parameter must be greater or equal 0. Exiting.");
					exit(1);
				}
			}
			SMAFELOG_FUNC(SMAFELOG_INFO, "tracks limit=" + stringify(so->limittrackscount));
			// limit days
			if (arg_limittrialdays->count > 0) {
				if (arg_limittrialdays->ival[0] >= 0) {
					so->limittrialdays = arg_limittrialdays->ival[0];
				} else {
					SMAFELOG_FUNC(SMAFELOG_FATAL, "Trial period parameter must be greater or equal 0. Exiting.");
					exit(1);
				}
			}
			SMAFELOG_FUNC(SMAFELOG_INFO, "trial period=" + stringify(so->limittrialdays));
			// date limit
			if (arg_limitharddate->count > 0) {
				try {
					so->limitharddate = bg::from_simple_string(std::string(arg_limitharddate->sval[0]));
				} catch(std::exception& e) {
					std::cout << "  Exception: " <<  e.what() << std::endl;
					exit(1);
				}
			}
			SMAFELOG_FUNC(SMAFELOG_INFO, "expiration date=" + bg::to_simple_string(so->limitharddate));


		} else {
			arg_print_errors(stdout, end, sProgname.c_str());
			std::cout << "--help gives usage information" << std::endl;
			exit(1);
		}
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		for (int i = 0; i < sizeFvts; i++) {
			delete[] strings_in_c_suck[i];
			delete[] strings_in_c_suck2[i];
		}
		delete[] strings_in_c_suck;
		delete[] strings_in_c_suck2;
	}


	/**
	 * @param so pointer to SmafeOpt instance where those options have been set that are to be stored in db
	 * @param sSQLFile filename for sql file
	 * @throws std::string if error occurs (file cannot be created etc)
	 */
	static void generateSqlFile(Smafeopt* so, std::string sSQLFile) {
		// Process command line options
		try {

			SMAFE_STORE_DB_CLASS* db = NULL;

			// Open db connection and store config in db
			// although we do not issue commands against the db, we still need the connection
			// for escaping
			// UPDATE: for now, we do not use escaping
			db = new SMAFE_STORE_DB_CLASS();
			//				db->openConnection(so->strDbhost, so->strDbport, so->strDbuser, so->strDbpwd, so->strDbname);

			SMAFELOG_FUNC(SMAFELOG_INFO, "Writing SQL commands to " + sSQLFile + " ...");
			// open file
			std::ofstream outfile (sSQLFile.c_str());
			// check if good
			if (!outfile.good())
				throw "Error writing to file " + sSQLFile;

			// empty the table
			outfile << db->clearConfigRecords();

			so->generateSQLCommandsForConfigStoreInDb(db, outfile);

			outfile.close();
			SMAFELOG_FUNC(SMAFELOG_INFO, "...finished");
			SMAFELOG_FUNC(SMAFELOG_INFO, "SQL file successfully created - next step would be executing it against the database.");
			delete db;
		} catch (std::string& s) {
			SMAFELOG_FUNC(SMAFELOG_FATAL, s);
			exit(1);
		}
	}


	static void processCommandLineArguments_lightset(int argc, char* argv[], Smafeopt* so,
			bool bAdmin, tAdminparams* ap, bool& bNoDaemon, std::string sProgname)  {

		/* Define the allowable command line options, collecting them in argtable[] */
		/* Syntax 1: command line arguments and file or dir */

		const int numGenerlOpts = 15;

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
		struct arg_str *arg_dbconf = arg_str0(NULL, "dbconf", "DATABASE-CONFIGURATION-FILE",
				"Specify file that contains database connection details");
		struct arg_str *arg_livehost = arg_str0(NULL, "livehost", "HOSTNAME-LIVEDAEMON",
				"Hostname for connection with live daemon. Default is localhost");
		std::string arg_liveport_desc = std::string("Port number for connection with live daemon. Default is ") + stringify(SMAFELIVEMODE_STDPORT); // var decl and def is necesary. Must not happen on the fly
		struct arg_int *arg_liveport = arg_int0("p", "liveport", "PORT-LIVEDAEMON",
				arg_liveport_desc.c_str());
		struct arg_lit *arg_livelive = arg_lit0(NULL, "live",
				"Run in live mode, ie, use a running live daemon for distance calculation");
		struct arg_str *arg_livefile = arg_str0(NULL, "livefile",
				"FILENAME",
				"input file for live mode");
		std::string arg_livetopk_desc =std::string("Determines how many results are returned in a live query. Default is ") + stringify(DEFAULTLIVETOPK);
		struct arg_int *arg_livetopk = arg_int0(NULL, "livetopk", "N",
				arg_livetopk_desc.c_str() );
		std::string arg_livecid_desc =std::string("Restrict result to specific collection. Default: default collection (all tracks)");
		struct arg_int *arg_livecid = arg_int0(NULL, "livecollectionid", "COLLECTION ID",
				arg_livecid_desc.c_str());
		struct arg_int
		*arg_verbose =
				arg_int0(
						"v",
						"verbosity",
						"0-6",
						"Set verbosity level (=log level). The lower the value the more verbose the program behaves. Default is 3");
		struct arg_lit *help = arg_lit0(NULL, "help", "print this help and exit");
		struct arg_end *end = arg_end(20);

		int numAdminOpts = 0;

		// admin only options
		numAdminOpts = 6;

		// dummy admin param that must be matched
		struct arg_str
		*arg_admindummy =
				arg_str1(
						NULL,
						"admin",
						"OPTIONSFILE",
						"Optionsfile to load instead of loading from database. Must be FIRST parameter, and you must NOT use a = (but a whitespace) !");


		struct arg_str *arg_sCollectionName = arg_str0(NULL, "collection", "NAME",
				"Collection name (for database storage)");

		struct arg_str
		*arg_bToTextfile =
				arg_str0(
						NULL,
						"text",
						"PREFIX",
						"Write feature vectors to textfiles rather than to database, using PREFIX.XX.vec for the filenames where XX indicates the type of feature vector. PREFIX may contain a path.\n\t**\n\tNB: If files with the same names exist they will be *OVERWRITTEN* without notice!\n\t**");

		struct arg_str *arg_somlib = arg_str0(NULL, "somlib", "SOMLIBVECTORFILE",
				"Somlib vector file if somlib file extractor is used.");

		struct arg_lit
		*filelist =
				arg_lit0("l", "isfilelist",
						"treat specified file as textfile containing list of files to query");

		struct arg_file *infile = arg_file1(NULL, NULL,
				"[<input file>|<input dir>|<filelist>]",
				"input file, directory or filelist (see parameter -l)");



		void** argtable = new void*[numGenerlOpts + numAdminOpts];

		int c=0;

		if (bAdmin) {
			argtable[c++] = arg_admindummy;
		}
		argtable[c++] = arg_dbconf;
		argtable[c++] = arg_daemonID;
		argtable[c++] = arg_no_daemon;
		argtable[c++] = arg_interval;
		argtable[c++] = arg_verbose;
		argtable[c++] = arg_log;
		argtable[c++] = arg_stats;
		argtable[c++] = arg_livelive;
		argtable[c++] = arg_livehost;
		argtable[c++] = arg_liveport;
		argtable[c++] = arg_livetopk;
		argtable[c++] = arg_livecid;
		argtable[c++] = arg_livefile;
		argtable[c++] = help;
		if (!bAdmin) argtable[c++] = end;

		if (bAdmin) {
			argtable[c++] = arg_sCollectionName;
			argtable[c++] = arg_bToTextfile;
			argtable[c++] = arg_somlib;
			argtable[c++] = filelist;
			argtable[c++] = infile;
			argtable[c++] = end;
		}


		int nerrors;

		/* verify the argtable[] entries were allocated sucessfully */
		if (arg_nullcheck(argtable) != 0) {
			/* NULL entries were detected, some allocations must have failed */
			std::cerr << sProgname << ": insufficient memory" << std::endl;
			exit(2);
		}

		// if no parameter is given: show help
		if (argc > 1) {
			// Parse the command line as defined by argtable[]
			nerrors = arg_parse(argc, argv, argtable);
		} else {
			help->count = 1;
		}

		if (help->count > 0) {
			// todo help
			/* special case: '--help' takes precedence over error reporting */

			std::cout << "Usage: " << sProgname;
			arg_print_syntax(stdout, argtable, "\n");
			std::cout << std::endl;
			arg_print_glossary(stdout, argtable, "  %-27s %s\n");

			if (bAdmin) {
				std::cout << "\n\twhere <options file> is a textfile containing command line arguments, each in one line" << std::endl;

				std::cout << "\n\n**Somlib special mode**" << std::endl;
				std::cout
				<< "\tIn this mode a somlib file is parsed and its contents are written to db. The <file> parameter or filelist will not be used."
				<< std::endl;
				std::cout
				<< "\tTo enable somlib special mode: set bExtractSomlib to 1 (setting all other extractors to 0)  and specify a somlib file."
				<< std::endl;
				std::cout << "" << std::endl;
			}

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
			std::cout << "  1) ps aux | grep " << sProgname
					<< "          (this gives you the <PID>)" << std::endl;
			std::cout << "  2) kill <PID>" << std::endl;
#else
			std::cout << "This program works NOT as a daemon because this platform does not support forking (or, there has been a problem at compiling the application)." << std::endl;
#endif

			exit(1);
		}


		if (nerrors == 0) {
			// verbosity level //todo check for min level
			// must be on top
			if (arg_verbose->count > 0) {
				ap->loglevel_requested = arg_verbose->ival[0];
				// change loglevel
				SmafeLogger::smlog->setLoglevel(ap->loglevel_requested);
			} else
				ap->loglevel_requested = SmafeLogger::DEFAULT_LOGLEVEL;

			// check args for live mode
			if (arg_livelive->count > 0) {
				ap->bLivemode = true;
				SMAFELOG_FUNC(SMAFELOG_INFO, "Live distance query mode");
			} else {
				ap->bLivemode = false;
			}

			// identifier
			if (arg_daemonID->count > 0) {
				ap->daemonId = std::string(arg_daemonID->sval[0]);
				if (ap->daemonId == SmafeStoreDB::STATUSOK || (0 == ap->daemonId.find(SmafeStoreDB::STATUSFAILED))) {
					SMAFELOG_FUNC(SMAFELOG_FATAL, "Identifier " + ap->daemonId + " is illegal. It must not be '"+SmafeStoreDB::STATUSOK+"' and must not start with '"+SmafeStoreDB::STATUSFAILED+"'");
					exit(2);
				}
			} else {
				if (arg_stats->count == 0 && arg_no_daemon->count == 0 && arg_livelive->count == 0 &&  !(bAdmin &&  arg_bToTextfile->count > 0)) {
					// is actually mandatory, only for --stats and --no-daemon is not, and for live also not
					SMAFELOG_FUNC(SMAFELOG_FATAL, "Please specify an identifier for this daemon (--id).");
					exit(2);
				}
			}

			// logfile
			// uses daemonId
			if (arg_log->count > 0) {
				ap->sLogfilename = std::string(arg_log->sval[0]);
			} else {
				ap->sLogfilename = std::string(sProgname) + "." + stringify(my_getpid()) + ".log";
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
				ap->bPrintStatsOnly = true;
#if defined(SMAFEDISTD_REAL_DAEMON)
				SMAFELOG_FUNC(SMAFELOG_INFO, "Stats mode, so running in 'normal mode' (ie, not as daemon)");
				bNoDaemon = true;
#endif
			} else
				ap->bPrintStatsOnly = false;



			// polling interval
			if (arg_interval->count > 0)
				ap->pollInterval = arg_interval->ival[0];
			else
				ap->pollInterval = DEFAULT_POLLING_INTERVAL;
			if (ap->pollInterval == 0) {
				SMAFELOG_FUNC(SMAFELOG_WARNING, "Polling interval set to 0 which means that the daemon will perform busy waiting.");
			}
			if (ap->pollInterval < 0) {
				SMAFELOG_FUNC(SMAFELOG_INFO, "Daemon will stop after last finished task (since pollInterval < 0)");
			} else {
				SMAFELOG_FUNC(SMAFELOG_INFO, "polling interval=" + stringify(ap->pollInterval));
			}

			// ---- db stuff
			// db options file
			if (arg_dbconf->count > 0) {
				SMAFELOG_FUNC(SMAFELOG_DEBUG, "Parsing db configuration file");
				so->parseDbOpts(std::string(arg_dbconf->sval[0]));
			}


			// live mode stuff
			if (ap->bLivemode) {
				if (!bNoDaemon) {
					SMAFELOG_FUNC(SMAFELOG_FATAL, "Live mode implies usage of --no-daemon.");
					exit(2);
				}
				if (arg_dbconf->count > 0) {
					SMAFELOG_FUNC(SMAFELOG_FATAL, "In live mode --dbconf is not allowed.");
					exit(2);
				}
				if (arg_daemonID->count > 0) {
					SMAFELOG_FUNC(SMAFELOG_FATAL, "In live mode --id cannot be used.");
					exit(2);
				}
				if (arg_log->count > 0) {
					SMAFELOG_FUNC(SMAFELOG_FATAL, "In live mode --log cannot be used.");
					exit(2);
				}
				if (arg_interval->count > 0) {
					SMAFELOG_FUNC(SMAFELOG_FATAL, "In live mode --interval cannot be used.");
					exit(2);
				}
				if (arg_stats->count > 0) {
					SMAFELOG_FUNC(SMAFELOG_FATAL, "In live mode --stats cannot be used.");
					exit(2);
				}

				if (arg_livefile->count > 0) {
					ap->sLivefile = std::string(arg_livefile->sval[0]);
					SMAFELOG_FUNC(SMAFELOG_INFO, "File for live query: " + ap->sLivefile);
				} else {
					SMAFELOG_FUNC(SMAFELOG_FATAL, "Please specify a file to be used for live query.");
					exit(2);
				}
				if (arg_livehost->count > 0) {
					ap->sLivehost = std::string(arg_livehost->sval[0]);
				} else
					ap->sLivehost = "localhost";
				SMAFELOG_FUNC(SMAFELOG_INFO, "Hostname: " + ap->sLivehost);

				if (arg_liveport->count > 0) {
					ap->iLiveport = arg_liveport->ival[0];
				} else {
					ap->iLiveport = SMAFELIVEMODE_STDPORT;
				}
				SMAFELOG_FUNC(SMAFELOG_INFO, "Port number: " + stringify(ap->iLiveport));

				// live topk
				if (arg_livetopk->count > 0) {
					ap->iLivetopk = arg_livetopk->ival[0];
				} else
					ap->iLivetopk = DEFAULTLIVETOPK;
				SMAFELOG_FUNC(SMAFELOG_INFO, "livetopk: " + stringify(ap->iLivetopk));

				// live collection id
				if (arg_livecid->count > 0) {
					ap->lCollectionId = arg_livecid->ival[0];
				} else
					ap->lCollectionId = SmafeStoreDB::RESERVEDCOLLECTIONS_DEFAULT;
				SMAFELOG_FUNC(SMAFELOG_INFO, "Collection id: " + stringify(ap->lCollectionId));

			}


			// ----admin params
			if (bAdmin) {
				ap->sCollectionName = arg_sCollectionName->count > 0 ? std::string(
						arg_sCollectionName->sval[0]) : "";


				if (arg_bToTextfile->count > 0) {
					ap->bToTextfile = true;
					ap->fv_output_files_prefix = std::string(arg_bToTextfile->sval[0]);
					// check for tilde:
					if (ap->fv_output_files_prefix.find('~') != ap->fv_output_files_prefix.npos) {
						// tilde found: abort
						SMAFELOG_FUNC(SMAFELOG_FATAL,ap->fv_output_files_prefix + ": Tilde expansion not supported. Please choose a textfile output path without ~.");
						exit(2);
					}
					SMAFELOG_FUNC(SMAFELOG_INFO, "Textfile output enabled.");
				} else {
					ap->bToTextfile = false;
				}


				if (arg_somlib->count > 0) {
					ap->sSomlibfile = std::string(arg_somlib->sval[0]);
					ap->bSomlibmode = true;
					SMAFELOG_FUNC(SMAFELOG_INFO, "Setting somlib file: " + ap->sSomlibfile);
				} else {
					ap->sSomlibfile = "";
					ap->bSomlibmode = false;
				}

				ap->bIsFileList = filelist->count > 0;




				ap->sFilename = std::string(infile->filename[0]);

				// no jobs mode for admin (why shuld you use that?)
				ap->bJobsMode = false;


				if (arg_dbconf->count == 0 && arg_bToTextfile->count == 0) {
					SMAFELOG_FUNC(SMAFELOG_FATAL, "Please specify either db configuration file (--dbconf) or text file output mode.");
					exit(2);
				}

				// no NO-jobs mode and daemon mode at the same time
				if (! ap->bJobsMode && ! bNoDaemon ) {
					SMAFELOG_FUNC(SMAFELOG_FATAL, "Daemon mode not possible if not processing jobs from database");
					exit(2);
				}


				// not text output and return segment features at the same time
				if ( ap->bToTextfile && so->bReturnSegmentFeatures  ) {
					SMAFELOG_FUNC(SMAFELOG_FATAL, "Returning segment features when using text file output is not possible.");
					exit(2);
				}


			} else { // bAdmin
				// no admin
				ap->bToTextfile = false;
				ap->bSomlibmode = false;
				if (ap->bLivemode)
					ap->bJobsMode = false;
				else
					ap->bJobsMode = true;
				ap->bPrintStatsOnly = false;

				if (arg_dbconf->count == 0 && !arg_livelive->count > 0) {
					SMAFELOG_FUNC(SMAFELOG_FATAL, "Please specify db configuration file (--dbconf)");
					exit(2);
				}
			}



		} else {
			arg_print_errors(stdout, end, sProgname.c_str());
			std::cout << "--help gives usage information" << std::endl;
			exit(1);
		}

		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));

	}



};
