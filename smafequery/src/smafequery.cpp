///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// smafequery.cpp
//
// SpectralMind Audio Feature Extraction Query Tool
// Main file
// ------------------------------------------------------------------------
//
//
// Version $Id$
//
//
//
//
//
//
///////////////////////////////////////////////////////////////////////////


// ------------------------------------------------------------------------
// includes

#include "config.h"

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include "argtable2.h"
#include <string>
#include <ctime>

#include "smafeutil.h"
#include "smafeFeatureVectorClasses.h"
#include "smafestoredb.h"
#include "smafeTextfileOutput.h"
#include "smafefile.h"
#include "smafeLogger.h"
#include "smafeopt.h"

#include "smafestore_specific_include_no_text.h"



// ------------------------------------------------------------------------
// constants
/** program name */
const char PROGNAME[] = "smafequery";
/** command line argument for feature vector query */
const char ARGUMENT_QUERY_FV[] = "--fv";
/** command line argument for nn query */
const char ARGUMENT_QUERY_NN[] = "--nn";
/** constant for fv query */
const int SQ_QUERY_FV = 1;
const int SQ_QUERY_NN = 2;
/** constant for file/track query mode */
const int SQ_FINGERPRINT = 1;
const int SQ_FILENAME = 2;


// ------------------------------------------------------------------------
// global vars
/** which query? */
int sq_cmd;
/** which mode (filename or fingerprint) */
int sq_querymode;
/** stream of filenames to query, separated by line break */
std::stringstream ssFiles(std::stringstream::in | std::stringstream::out);
/** Vector with track_ids or filesmafequery --nn -k 5 -d 2 -f 8 --dbname=exp_tunesbag --dbuser=smurf --dbpwd=papa_ids (depending on sq_	querymode) */
std::vector<long> vecRecordIds;
// -- to be moved to "database options class" at some time
/** collection name */
std::string sCollectionName("");
/** options */
Smafeopt* so;
/** use segment fvs?, used in live mode  */
bool bUseSegmFvs = false;

// ------------------------------------------------------------------------
// query functions (will probably be exported to be called by GUI or stuff)


/** Performs a query for nearest neighbours
 *
 * @param filename name of input file
 * @param bUseFP use fingerprint query or verbatim filename
 * @param k_nn how many nearest ns?
 * @param fvt_id featurevectortype_id
 * @param dist_id distancetype_id
 */
/*
 void query_nn_filename(std::string filename, bool bUseFP, int k_nn, long fvt_id, long dist_id) {
 }
 */

/** Performs a query for feature vector given a filename
 *
 * @param filename name of input file
 * @param bUseFP use fingerprint query or verbatim filename
 * @param fvt_id featurevectortype_id
 */
/*
 void query_fv_filename(std::string filename, bool bUseFP, long fvt_id) {
 }
 */

/** Performs a query for feature vectors given a vector of track_ids
 *
 * @param record_ids vector of track or file ids
 * @param bTrackList true if record_ids contains track_ids; false if it contains file_ids
 * @param fvt_id featurevectortype_id
 * @param outfileprefix The begin of the output file. Suffix like ".SSD.vec" is appended
 */
void query_fv_track_or_file_ids(std::vector<long> record_ids, bool bTrackList,
		long fvt_id, const char* outfileprefix, bool bUseSegmFvs) {
	SMAFE_STORE_DB_CLASS* db;

	std::vector<SmafeAbstractFeatureVector_Ptr> fvs;
	SmafeAbstractFeatureVector_Ptr fv;
	SmafeAbstractFeatureVector* safv;

	db = new SMAFE_STORE_DB_CLASS();
	try {
		db->openConnection(so->strDbhost, so->strDbport, so->strDbuser, so->strDbpwd, so->strDbname);
	} catch (std::string& s) {
		SMAFELOG_FUNC(SMAFELOG_FATAL, s);
		exit(1);
	}

	// iterate through ids
	for (std::vector<long>::iterator iter = record_ids.begin(); iter
	< record_ids.end(); iter++) {
		try {
			SMAFELOG_FUNC(SMAFELOG_DEBUG, "Reading vector " + stringify(iter - record_ids.begin() + 1) + " of " + stringify(record_ids.size()));

			long theTrack_id, file_id;
			if (bTrackList) {
				// iterator contains track id
				theTrack_id = *iter;

				if (!bUseSegmFvs) {
					// Track FVS

					// read fv: get also fvtype info, take the first-best file,
					//		and let us here do the error checking (ie if fv was found or not)
					// segmentnr is not used
					// load file_id
					safv = db->readFeatureVector(fvt_id, theTrack_id, true, -1, -1,
							true, true);
					if (safv != 0) {
						// wrap nicely in smart pointer and add to vector
						fv.reset(safv);
						fvs.push_back(fv);
					} else {
						// This usually happens if we do not have a complete set of fvs for all fvts and tracks
						// or it will happen for _each_ track if the user specified an fvtype that
						// does not exist at all.
						SMAFELOG_FUNC(SMAFELOG_DEBUG, "Feature vector not found for track_id = "+stringify(theTrack_id)+", featurevectortype_id = "+stringify(fvt_id));
					}
				} else {
					// Segment FVS

					long theSegment_id = 0;
					do {
						// read fv: get also fvtype info, take the first-best file,
						//		and let us here do the error checking (ie if fv was found or not)
						// load file_id
						safv = db->readFeatureVector(fvt_id, theTrack_id, true, -1, theSegment_id,
								true, true);
						if (safv != 0) {
							// wrap nicely in smart pointer and add to vector
							fv.reset(safv);
							fvs.push_back(fv);

							theSegment_id++;
						} else {
							// This happens if ther are not more segmetns for this track
							SMAFELOG_FUNC(SMAFELOG_DEBUG2, "Feature vector not found for track_id = "+stringify(theTrack_id)+", segmentnr = "+stringify(theSegment_id)+", featurevectortype_id = "+stringify(fvt_id));
						}
					} while (safv != 0);
				}
			} else {
				// iterator contains file_id
				file_id = *iter;
				theTrack_id = db->getTrackIDForFile(file_id);
				SMAFELOG_FUNC(SMAFELOG_DEBUG, "Found track_id=" + stringify(theTrack_id) + " for file_id=" + stringify(file_id));
				if (theTrack_id >= 0) {
					fv.reset(db->readFeatureVector(fvt_id, theTrack_id, true,
							file_id, -1, false, true));
					fvs.push_back(fv);
				} else {
					// track id < -1, that means filename query was done and no track found
					SMAFELOG_FUNC(SMAFELOG_ERROR, "No track found for file_id=" + stringify(file_id) + ". Skipping.");
				}
			}
		} // try block
		catch (std::string& s) {
			SMAFELOG_FUNC(SMAFELOG_ERROR, s);
		} // catch block
	}
	try {
		// write to file
		SmafeTextfileOutput sto;

		// take fvtype info of first fv to get our suffix
		std::string outfile;
		if (fvs.size() > 0)
			outfile = std::string(outfileprefix) + "." + fvs[0]->fvtype->name
			+ ".vec";
		else
			outfile = std::string(outfileprefix) + ".UNKNOWN.vec";

		// store passphrase for later
		std::string pp = std::string(verysecretpassphrase);
		// set to "" for output
		strcpy(verysecretpassphrase, "");
		sto.output(fvs, outfile);
		// restore passphrsae
		strcpy(verysecretpassphrase, pp.c_str());
		//		SMAFELOG_FUNC(SMAFELOG_INFO, "Requested feature vectors have been written in Somlib format to " + outfile);
	} // try block
	catch (std::string& s) {
		SMAFELOG_FUNC(SMAFELOG_FATAL, s);
	} // catch block
}

/** prints nearest neighbours (result of nn query) */
void print_nns(std::vector<Nn_result_rs_Ptr> nns, long query_track_id) {
	std::cout << "\n\nNearest neighbours for track_id = " << query_track_id
			<< ":" << std::endl;
	int place = 0;
	if (nns.size() > 0) {
		// iterate through vector
		for (std::vector<Nn_result_rs_Ptr>::iterator iter = nns.begin(); iter
		< nns.end(); iter++) {
			std::cout << "(" << (++place) << ") " << iter->get()->uri << ": d="
					<< iter->get()->dist << " (Track_id: "
					<< (*iter).get()->track_id << ")" << std::endl;
		}
	} else {
		std::cout
		<< "\n\nNo nearest neighbours found with specified parameters"
		<< std::endl;
	}
	std::cout << std::endl;
}

/** Performs a query for nearest neighbour given a track_id
 *
 * @param db open database connection
 * @param track_id the id of track record
 * @param fvt_id featurevectortype_id
 * @param dist_id distancetype_id
 * @param k number of nn to print
 * @param sCollectionName name of collection if results should be filterted according to collection.
 * 		Empty string otherise (no restriction)
 */
std::vector<Nn_result_rs_Ptr> query_nn_track_id(SMAFE_STORE_DB_CLASS* db,
		long track_id, long fvt_id, long dist_id, int k,
		std::string sCollectionName) {
	std::vector<Nn_result_rs_Ptr> nns;

	// See http://www.velocityreviews.com/forums/t288572-virtual-function-and-overloading-.html
	// or http://www.parashift.com/c++-faq-lite/strange-inheritance.html#faq-23.6
	// for explanation why we have to specifiy the base class here
	db->SmafeStoreDB::query_nn(nns, track_id, fvt_id, dist_id, sCollectionName,
			k);

	return nns;
}

/** Performs queries for nearest neighbour given a vector of track ids
 *
 * @param record_ids vector of track or file ids
 * @param bTrackList true if record_ids contains track_ids; false if it contains file_ids
 * @param fvt_id featurevectortype_id
 * @param dist_id distancetype_id
 * @param k number of nn to print
 * @param bPrint if true, results are printed to cout.
 */
void query_nn_track_or_file_ids(std::vector<long> record_ids, bool bTrackList,
		long fvt_id, long dist_id, std::string sCollectionName, int k,
		bool bPrint) {
	SMAFE_STORE_DB_CLASS* db;

	db = new SMAFE_STORE_DB_CLASS();
	try {
		db->openConnection(so->strDbhost, so->strDbport, so->strDbuser, so->strDbpwd, so->strDbname);
	} catch (std::string& s) {
		SMAFELOG_FUNC(SMAFELOG_FATAL, s);
		exit(1);
	}

	// iterate through track_ids
	for (std::vector<long>::iterator iter = record_ids.begin(); iter
	< record_ids.end(); iter++) {
		try {
			long theTrack_id, file_id;
			if (bTrackList) {
				// iterator contains track id
				theTrack_id = *iter;
			} else {
				file_id = *iter;
				theTrack_id = db->getTrackIDForFile(file_id);
				SMAFELOG_FUNC(SMAFELOG_DEBUG, "Found track_id=" + stringify(theTrack_id) + " for file_id=" + stringify(file_id));
			}
			if (theTrack_id >= 0) {
				std::vector<Nn_result_rs_Ptr> nns = query_nn_track_id(db,
						theTrack_id, fvt_id, dist_id, k, sCollectionName);
				if (bPrint)
					print_nns(nns, theTrack_id);
			} else {
				// track id < -1, that means filename query was done and no track found
				SMAFELOG_FUNC(SMAFELOG_ERROR, "No track found for file_id=" + stringify(file_id) + ". Skipping.");
			}
		} // try block
		catch (std::string& s) {
			SMAFELOG_FUNC(SMAFELOG_ERROR, s);
		} // catch block
	}

	delete db;
}

// ------------------------------------------------------------------------
// internal functions for smafequery backend


/** Iterateres thru list of filenames and creates list of track_ids
 * depending on method to use (fingerprint or verbatim)
 * <p>Side effects / global vars: vecRecordIds is being emptied and then filled
 */
void prepareListOfTrackIds() {
	std::string sFile;
	long track_id;
	long collection_id = -1;
	SMAFE_STORE_DB_CLASS* db;

	db = new SMAFE_STORE_DB_CLASS();
	try {
		db->openConnection(so->strDbhost, so->strDbport, so->strDbuser, so->strDbpwd, so->strDbname);
	} catch (std::string& s) {
		SMAFELOG_FUNC(SMAFELOG_FATAL, s);
		exit(1);
	}

	if (sCollectionName != "") {
		// get collectoin_id
		collection_id = db->getCollectionId(sCollectionName);
		SMAFELOG_FUNC(SMAFELOG_INFO, "Will use those tracks only that are contained in the collection '" + sCollectionName + "' (id=" + stringify(collection_id) + ")");
	}

	vecRecordIds.clear();
	do {
		getline(ssFiles, sFile);
		if (!ssFiles.eof()) {
			trimWhitespace(sFile);
			if (sFile != "") {
				// use fingerprint of file
				try {
					//t_fingerprint fingerprint = new char[33];
					t_fingerprint fingerprint;
					// this loads the file and this might fail, thus the try catch block
					SmafeFile* f = SmafeFile::getInstance(sFile.c_str());
					getFingerprint(f->getAudiobuffer(),
							f->getAudioformat().iDatasize, fingerprint);
					delete f;
					//getFingerprint(sFile, fingerprint);

					// this fingerprint contained in DB?
					if ((track_id = db->isTrackInDatabase(fingerprint)) >= 0) {
						SMAFELOG_FUNC(SMAFELOG_DEBUG, "Found track " + stringify(track_id));
						if (collection_id == -1 || db->isTrackInCollection(
								track_id, collection_id))
							vecRecordIds.push_back(track_id);
						else
							SMAFELOG_FUNC(SMAFELOG_INFO, "Found track " + stringify(track_id) + " which is not in the required collection .");
					} else
						SMAFELOG_FUNC(SMAFELOG_ERROR, std::string("Fingerprint of ") + sFile + std::string(" not found in database. Skipping."));
				} // try block
				catch (std::string& s) {
					SMAFELOG_FUNC(SMAFELOG_FATAL, s);
				} // catch block
			} // if (sFile != "")
		} // if (!ssFiles.eof())
	} while (!ssFiles.eof());

	delete db;
}

/** Iterateres thru list of filenames and creates list of file_ids
 * <p>Side effects / global vars: vecRecordIds is being emptied and then filled
 */
void prepareListOfFileIds() {
	std::string sFile;
	std::vector<long> file_ids;
	long collection_id = -1;
	SMAFE_STORE_DB_CLASS* db;

	db = new SMAFE_STORE_DB_CLASS();
	try {
		db->openConnection(so->strDbhost, so->strDbport, so->strDbuser, so->strDbpwd, so->strDbname);

		if (sCollectionName != "") {
			// get collectoin_id
			collection_id = db->getCollectionId(sCollectionName);
			SMAFELOG_FUNC(SMAFELOG_INFO, "Will use those tracks only that are contained in the collection '" + sCollectionName + "' (id=" + stringify(collection_id) + ")");
		}

		vecRecordIds.clear();
		do {
			getline(ssFiles, sFile);
			if (!ssFiles.eof()) {
				trimWhitespace(sFile);
				if (sFile != "") {
					// do use filename "as is"

					// contained in db?
					file_ids = db->isFileInDatabase(sFile);
					for (std::vector<long>::iterator iter = file_ids.begin(); iter
					< file_ids.end(); iter++) {
						long file_id = *iter;
						long track_id = db->getTrackIDForFile(file_id);
						if (collection_id == -1 || db->isTrackInCollection(
								track_id, collection_id))
							vecRecordIds.push_back(file_id);
						//							vecRecordIds.insert(vecRecordIds.end(),
						//									file_id.begin(), file_id.end()); // a deque would in principal be better because it avoids moving of the data if reallocation has to be done. Still in this toy example we do not worry

						else
							SMAFELOG_FUNC(SMAFELOG_INFO, "Found file " + stringify(file_id) + " which is not in the required collection .");
					}
					if (file_ids.size() == 0) SMAFELOG_FUNC(SMAFELOG_ERROR, sFile + std::string( " not found in database. Skipping."));
				} // if (sFile != "")
			} // if (!ssFiles.eof())
		} while (!ssFiles.eof());

		delete db;
	} catch (std::string& s) {
		SMAFELOG_FUNC(SMAFELOG_FATAL, s);
		exit(1);
	}
}

/** Processes command line arguments using argtable library
 * <p>Note: this function sets variable ssFiles
 * @param argc number of command line arguments (from main())
 * @param argv array of c-strings (from main())
 */
void processCommandLineArguments(int argc, char* argv[]) {

	/* Define the allowable command line options, collecting them in argtable[] */
	// command 1
	char fv_glossar[200];
	sprintf(fv_glossar,
			"query feature vectors from database and write them to specified output file");
	struct arg_lit *query_fv = arg_lit1(NULL, "fv", fv_glossar);
	struct arg_lit *querymode = arg_lit0("n", "filenamequery",
			"use filename to query database (not fingerprint)");
	struct arg_int *fvtype = arg_int1("f", "featurevectortype", "<n>",
			"ID of feature vector type to use");
	struct arg_str
	*arg_sCollectionName =
			arg_str0(NULL, "collection", "NAME",
					"Collection name (for database storage) (currently only in --nn query mode)");
	struct arg_lit *help = arg_lit0(NULL, "help", "print this help and exit");
	struct arg_str *arg_dbconf = arg_str1(NULL, "dbconf", "DATABASE-CONFIGURATION-FILE",
			"Specify file that contains database connection details");

	struct arg_int
	*arg_verbose =
			arg_int0(
					"v",
					"verbosity",
					"0-6",
					"Set verbosity level (=log level). The lower the value the more verbose the program behaves. Default is 3");
	struct arg_lit
	*filelist =
			arg_lit0("l", "isfilelist",
					"treat specified file as textfile containing list of files to query");
	struct arg_str
	*arg_outputfile =
			arg_str1(
					"o",
					"output",
					"OUTPUTFILEPREFIX",
					"Use OUTPUTFILEPREFIX.XX.vec to write the feature vectors to. XX indicates the type of feature vector. OUTPUTFILEPREFIX may contain a path.\n\t**\n\tNB: If files with the same names exist they will be *OVERWRITTEN* without notice!\n\t**");
	struct arg_file
	*infile =
			arg_file0(
					NULL,
					NULL,
					"<file>",
					"file to query /or/ filelist (see -l). If this parameter is not specified, --all must be used");
	struct arg_lit
	*arg_all =
			arg_lit0(
					NULL,
					"all",
					"query all feature vectors of specified type. If this parameter is not specified, <file> must be used");
	struct arg_lit *arg_segmentfvs = arg_lit0("s", "segmentfvs", "Use feature vectors of segments instead of those of tracks.");

	struct arg_end *end = arg_end(20);
	void* argtable1[] = { query_fv, querymode, fvtype, arg_sCollectionName,
			arg_outputfile, arg_dbconf,
			arg_verbose, help, filelist, infile, arg_all, arg_segmentfvs, end };

	struct arg_lit *query_nn = arg_lit1(NULL, "nn",
			"query k nearest neighbours from database");
	struct arg_int *k_nn = arg_int1("k", NULL, "<k>",
			"number of neighbours to return");
	struct arg_int *disttype = arg_int1("d", "disttype", "<n>",
			"ID of distance type to use");
	void* argtable2[] = { query_nn, k_nn, querymode, fvtype, disttype,
			arg_sCollectionName, arg_dbconf, help, arg_verbose, filelist, infile, end };

	int nerrors;

	/* verify the argtable[] entries were allocated sucessfully */
	if (arg_nullcheck(argtable1) != 0 || arg_nullcheck(argtable2) != 0) {
		/* NULL entries were detected, some allocations must have failed */
		std::cerr << PROGNAME << ": insufficient memory" << std::endl;
		exit(2);
	}

	// check first argument to decide which argtable we use for parsing
	if (argc >= 2) {
		if (0 == strcmp(argv[1], ARGUMENT_QUERY_FV)) {
			sq_cmd = SQ_QUERY_FV;
			nerrors = arg_parse(argc, argv, argtable1);
		} else {
			sq_cmd = SQ_QUERY_NN;
			nerrors = arg_parse(argc, argv, argtable2);
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
		//		std::cout << "**\nNB: If file with the same name exists it will be *OVERWRITTEN* without notice!\n**" <<std::endl;
		std::cout << std::endl << "-OR-" << std::endl << std::endl;
		std::cout << PROGNAME;
		arg_print_syntax(stdout, argtable2, "\n");
		arg_print_glossary(stdout, argtable2, "  %-22s %s\n");
		exit(0);
	}

	if (nerrors == 0) {
		// ---- db stuff
		// db options file
		if (arg_dbconf->count > 0) {
			SMAFELOG_FUNC(SMAFELOG_DEBUG, "Parsing db configuration file");
			so->parseDbOpts(std::string(arg_dbconf->sval[0]));
		}

		// ----- gneeral ooptoins
		if (arg_verbose->count > 0) {
			// change loglevel
			SmafeLogger::smlog->setLoglevel(arg_verbose->ival[0]);
		}

		if (arg_sCollectionName->count > 0) {
			sCollectionName = std::string(arg_sCollectionName->sval[0]);
			SMAFELOG_FUNC(SMAFELOG_DEBUG, "Using collection name " + sCollectionName);
		}

		// in nn query mode: <file> must be used
		if (sq_cmd == SQ_QUERY_NN && infile->count == 0) {
			SMAFELOG_FUNC(SMAFELOG_FATAL, "--nn: <file> not specified.");
			exit(1);
		}

		// in fv query mode: either <file> or --all must be used
		if (sq_cmd == SQ_QUERY_FV && infile->count == 0 && arg_all->count == 0) {
			SMAFELOG_FUNC(SMAFELOG_FATAL, "--fv: Neither <file> not --all has been specified. Please use one of them.");
			exit(1);
		}

		// in fv query mode: not both!
		if (sq_cmd == SQ_QUERY_FV && infile->count > 0 && arg_all->count > 0) {
			SMAFELOG_FUNC(SMAFELOG_FATAL, "--fv: Only one of the parameters <file> and --all can been specified.");
			exit(1);
		}

		// if --all is used: -l can not be used
		if (arg_all->count > 0 && filelist->count > 0) {
			SMAFELOG_FUNC(SMAFELOG_FATAL, "--all cannot be combined with --isfilelist.");
			exit(1);
		}

		// if --all is used: -n cannot be used
		if (arg_all->count > 0 && querymode->count > 0) {
			SMAFELOG_FUNC(SMAFELOG_FATAL, "--all cannot be combined with --filenamequery.");
			exit(1);
		}

		if (arg_segmentfvs->count > 0) {
			bUseSegmFvs = true;
		} else {
			bUseSegmFvs = false;
		}
		SMAFELOG_FUNC(SMAFELOG_INFO, "bUseSegments=" + stringify(bUseSegmFvs));

		// load config (passphrase!)
		so->loadConfigFromDb();


		// ---- get list of files to query
		if (filelist->count > 0) {
			// query for filename
			sq_querymode = SQ_FILENAME;

			// treat specified file as textfile with filenames
			std::ifstream ifsFileList(infile->filename[0]);
			std::string line;
			if (ifsFileList.is_open()) {
				while (!ifsFileList.eof()) {
					// read filenames from textfile and add to stream
					getline(ifsFileList, line);
					ssFiles << line << std::endl;
				}
				ifsFileList.close();
			} else {
				SMAFELOG_FUNC(SMAFELOG_FATAL, "Cannot find filelist " + std::string(infile->filename[0]));
				exit(2);
			}
		} else {
			// we only have one file
			ssFiles << infile->filename[0] << std::endl;
		}

		// if --all is used: get "all"
		if (arg_all->count > 0) {
			SMAFELOG_FUNC(SMAFELOG_INFO, "Query for all tracks.");

			// query mode is track id
			sq_querymode = SQ_FINGERPRINT;

			// get all track ids
			try {
				SMAFE_STORE_DB_CLASS* db = new SMAFE_STORE_DB_CLASS();
				// open connection
				db->openConnection(so->strDbhost, so->strDbport, so->strDbuser, so->strDbpwd, so->strDbname);

				vecRecordIds = db->getTrack_ids(sCollectionName);
			} catch (std::string& s) {
				SMAFELOG_FUNC(SMAFELOG_FATAL, s);
				exit(1);
			}
		} else {

			// querymode
			if (querymode->count > 0) {
				sq_querymode = SQ_FILENAME;
				SMAFELOG_FUNC(SMAFELOG_INFO, "Query for approximate file names.");
				SMAFELOG_FUNC(SMAFELOG_INFO, "Note that this query mode may lead to duplicate feature vectors if more than one file references the same track / feature vector.");
			} else {
				sq_querymode = SQ_FINGERPRINT;
				SMAFELOG_FUNC(SMAFELOG_INFO, "Query for fingerprint.");
			}

			// ---- get either list of file_ids or of track_ids
			if (sq_querymode == SQ_FILENAME) {
				// get file ids
				prepareListOfFileIds();
			} else {
				// track_ids
				prepareListOfTrackIds();
			}
		}

		// ---- execute queries
		if (sq_cmd == SQ_QUERY_FV)
			query_fv_track_or_file_ids(vecRecordIds, sq_querymode
					== SQ_FINGERPRINT, fvtype->ival[0], arg_outputfile->sval[0], bUseSegmFvs);
		else
			query_nn_track_or_file_ids(vecRecordIds, sq_querymode
					== SQ_FINGERPRINT, fvtype->ival[0], disttype->ival[0],
					sCollectionName, k_nn->ival[0], true);

	} else {
		arg_print_errors(stdout, end, PROGNAME);
		std::cout << "--help gives usage information" << std::endl;
		exit(1);
	}
	arg_freetable(argtable1, sizeof(argtable1) / sizeof(argtable1[0]));
	// argtable 2 shares some instances with argtable1. These have been freed already.
	//arg_freetable(argtable2,sizeof(argtable2)/sizeof(argtable2[0]));
}

// ------------------------------------------------------------------------
// main

/** well that's the entry point of this cute application */
int main(int argc, char* argv[]) {
	try {

		// for "benchmark"
		clock_t begin = clock();

		splashScreen("Query frontend");

		// Process command line options
		so = new Smafeopt();
		try {
			processCommandLineArguments(argc, argv);

		} catch (std::string& s) {
			SMAFELOG_FUNC(SMAFELOG_FATAL, s);
			exit(2);
		}

		clock_t end = clock();
		char strings_in_c_suck[100];
		sprintf(strings_in_c_suck, "Execution time: %.0f ms.\n", diffclock(end,
				begin));
		SMAFELOG_FUNC(SMAFELOG_INFO, std::string(strings_in_c_suck));
	} catch (std::string& s) {
		SMAFELOG_FUNC(SMAFELOG_FATAL, "Error caught in main(..)");
		SMAFELOG_FUNC(SMAFELOG_FATAL, s);
		exit(1);
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
	delete so;

}
