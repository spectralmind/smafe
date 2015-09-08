///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// smafewrapd.cpp
//
// SpectralMind Audio Feature Extraction Wrapper
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

#include "smafeopt.h"
#include "smafeutil.h"
#include "smafeRPExtractor.h"
#include "SmafeTimbralExtractor.h"
#include "SmafeSomlibfileExtractor.h"
#include "tLiveNNMessage.h"

#include "smafeparamhelper.h"

#include "smafestoredb.h"
#include "smafefile.h"
#include "smafeTextfileOutput.h"

#include "smafestore_specific_include.h"

#include "boost/filesystem.hpp"
#include <boost/asio.hpp>
#include <boost/algorithm/string/replace.hpp>


#include "smafeLogger.h"
#include "smafeProgress.h"


using boost::asio::ip::tcp;
namespace fs = boost::filesystem;



// ------------------------------------------------------------------------
// constants
/** program name */
const char PROGNAME[] = "smafewrapd";
/** command line argument for options file */
//const char ARGUMENT_OPTIONS_FILE[] = "usefile";
/** command line argument "file" if db-daemon mode */
//const char ARGUMENT_OPTIONS_JOBS[] = "usejobs";

// ------------------------------------------------------------------------
// global vars and constants (daemon related)

/** if daemon is to be terminated after next finished job
 * <p>This flag is set to true if term signal is caught */
bool b_should_terminate = false;
/** options  */
Smafeopt* so;
/** admin options */
tAdminparams* ap;




#include "smafedaemon2.h"


/** Logs all options that we use  and taht are specific to this program */
void logOptions() {
	// do not log db stuff if live mode
	// TODO
	SMAFELOG_FUNC(SMAFELOG_INFO, "dbhost =\t\t\t" + so->strDbhost);
	SMAFELOG_FUNC(SMAFELOG_INFO, "dbport =\t\t\t" + so->strDbport);
	SMAFELOG_FUNC(SMAFELOG_INFO, "dbname =\t\t\t" + so->strDbname);
	SMAFELOG_FUNC(SMAFELOG_INFO, "dbuser =\t\t\t" + so->strDbuser);
	SMAFELOG_FUNC(SMAFELOG_INFO, "dbpwd =\t\t\t**************"); // ;-)

	SMAFELOG_FUNC(SMAFELOG_DEBUG, so->toString());

	// repeat those with INFO level that are safe for customer
	SMAFELOG_FUNC(SMAFELOG_INFO, "File destination =\t\t" + so->sFileDest);
	SMAFELOG_FUNC(SMAFELOG_INFO, "Limit of files =\t\t" + stringify(so->limittrackscount));  //was: SMAFELOG_FUNC(SMAFELOG_INFO, "Limit of files =\t\t" + so->limittrackscount);
	// why wasn't there any compile error??



	/*
	SMAFELOG_FUNC(SMAFELOG_DEBUG, "bReturnSegmentFeatures =\t" + stringify(so->bReturnSegmentFeatures));
	SMAFELOG_FUNC(SMAFELOG_DEBUG, "bNormalizeFFTEnergy =\t" + stringify(so->bNormalizeFFTEnergy));
	SMAFELOG_FUNC(SMAFELOG_DEBUG, "bTransformDecibel =\t" + stringify(so->bTransformDecibel));
	SMAFELOG_FUNC(SMAFELOG_DEBUG, "uiSkipin =\t" + stringify(so->uiSkipin));
	SMAFELOG_FUNC(SMAFELOG_DEBUG, "distancetype_ids.size =\t" + stringify(so->distancetype_ids.size()));

	std::stringstream ss(std::stringstream::in | std::stringstream::out);
	std::copy(so->distancetype_ids.begin(), so->distancetype_ids.end(), std::ostream_iterator<long>(ss, ", "));
	SMAFELOG_FUNC(SMAFELOG_DEBUG, "distancetype_ids =\t" + ss.str());

	SMAFELOG_FUNC(SMAFELOG_DEBUG, "limittrackscount =\t" + stringify(so->limittrackscount));
	 */
	/*
	SMAFELOG_FUNC(SMAFELOG_DEBUG, " =\t" + stringify(so->));
	SMAFELOG_FUNC(SMAFELOG_DEBUG, " =\t" + stringify(so->));
	SMAFELOG_FUNC(SMAFELOG_DEBUG, " =\t" + stringify(so->));
	SMAFELOG_FUNC(SMAFELOG_DEBUG, " =\t" + stringify(so->));
	SMAFELOG_FUNC(SMAFELOG_DEBUG, " =\t" + stringify(so->));
	SMAFELOG_FUNC(SMAFELOG_DEBUG, " =\t" + stringify(so->));
	 */
}



// ------------------------------------------------------------------------
// Subs for that used to be in old main


/** Prepare feature vector type variables
 * @param
 */
void prepareFeatureVectorType(Smafeopt* so,
		SmafeFVType_Ptr_map &featureVectorTypes,
		SmafeExtractor_Ptr_vector &vExtractors,
		SmafeSomlibfileExtractor* &smafeSomlibExt, SMAFE_STORE_DB_CLASS* db) {

	// have additional properties of fv types set
	// must be done before ensureRPFeatVecTypeRecord(...)
	try {
		for (SmafeExtractor_Ptr_vector::iterator iter = vExtractors.begin(); iter
		< vExtractors.end(); iter++) {
			SmafeExtractor* theEx = iter->get();

			theEx->setFVTProperties(featureVectorTypes, *so);

			if (theEx->getName() == SmafeSomlibfileExtractor::EXTRACTORNAME) {
				// hold reference for later
				smafeSomlibExt = (SmafeSomlibfileExtractor*) theEx;
			}
		} // end of iterator
		assert(smafeSomlibExt != NULL);
	} catch (std::string& s) {
		SMAFELOG_FUNC(SMAFELOG_FATAL, s);
		exit(1);
	}


	// Store fvtypes in db if not text file output and if no live mode
#if !defined(SMAFE_NODB)
	if (!ap->bToTextfile && !ap->bLivemode) {
		// -------------- database version
		try {
			db->startTransaction();

			SMAFELOG_FUNC(SMAFELOG_DEBUG, "Starting to insert feature vector types if necessary.");

			// ensure that fv type records are in db
			// loop over fvt instances
			for (SmafeFVType_Ptr_map::iterator iter =
					featureVectorTypes.begin(); iter
					!= featureVectorTypes.end(); iter++) {
				// if to be calculated
				if (so->mapExtractGenerally[iter->first]) {
					db->ensureRPFeatVecTypeRecord(iter->second.get());
				}
			} // end of iterator

			// finish transaction
			db->finishTransaction(true);

		} catch (std::string& s) {
			SMAFELOG_FUNC(SMAFELOG_FATAL, s);
			exit(1);
		}

	} //if (!bToTextfile)
#endif

}


/** Switch logging to logfile */
void switchToLogfile(tAdminparams* ap) {

	// new log
	// new log must be init before fork(). Why? Don't know yet
	SMAFELOG_FUNC(SMAFELOG_INFO, "Logging to '" + ap->sLogfilename + "'");

	delete SmafeLogger::smlog;
	try {
		SmafeLogger::smlog = new SmafeLogger(ap->sLogfilename,
				SmafeLogger::DEFAULT_LOGLEVEL);
		SmafeLogger::smlog->setLoglevel(ap->loglevel_requested);
	} catch (std::string& s) {
		std::cerr << s << std::endl;
		std::cerr << "Please provide a valid filename." << std::endl;
		std::cerr << "Exitting" << std::endl;
		exit (3);
	}
}

/** Opens TCP socket connection to running daemon, and receive the options as serialized string, and
 * create Smafeopt object from it.
 * <p>The given instance so is deleted, and the reference to the newly created is given back.
 */
Smafeopt* receiveOptionsFromDistd(Smafeopt* so, tAdminparams* ap) {

	Smafeopt* so_;
	try {
		tcp::iostream s(ap->sLivehost, stringify(ap->iLiveport));

		if (!s) {
			throw std::string("Cannot connect to " + ap->sLivehost + ":" + stringify(ap->iLiveport));
		}

		// send command
		s << "GETOPTS" << std::endl;

		// receive
		std::stringstream ssIn(std::stringstream::in | std::stringstream::out);
		std::string line;
		std::string line_plain;
		std::getline(s, line);
		SMAFELOG_FUNC(SMAFELOG_DEBUG, "Read line: " + line);

		// decrypt
		try {
			line_plain = decryptString(line.c_str(), stdpassphrase);
		} catch (CryptoPP::Exception& e) {
			throw "Error decrypting options: " + e.GetWhat();

		}

		SMAFELOG_FUNC(SMAFELOG_DEBUG, "After decrypt: " + line_plain);
		ssIn << boost_s11n_workaround_135(line_plain);

		{
			boost::archive::text_iarchive ia(ssIn);

			// restore  from the archive
			ia >> BOOST_SERIALIZATION_NVP(so_);
		}
		SMAFELOG_FUNC(SMAFELOG_DEBUG, "Smafeopt object created");
		return so_;
	} catch (std::exception& e) {
		SMAFELOG_FUNC(SMAFELOG_FATAL, e.what());
		exit(1);
	} catch (std::string& s) {
		SMAFELOG_FUNC(SMAFELOG_FATAL, s);
		exit(1);
	}
}

/** Checks if options are ok, ie:
 * - exactly one feature vector enabled
 * Throws exception of type std::string if anyhting is not ok.
 */
void optionsValid4Livemode(Smafeopt* so, SmafeFVType_Ptr_map* featureVectorTypes) {
	int iNumEnabled;

	iNumEnabled = 0;
	for (SmafeFVType_Ptr_map::iterator iter =
			featureVectorTypes->begin(); iter
			!= featureVectorTypes->end(); iter++) {
		if (so->mapExtractGenerally[iter->first]) iNumEnabled++;
	} // end of iterator
	if (iNumEnabled > 1) {
		throw std::string("At most one feature vector should be enabled. The options received from the smafedistd indicate " + stringify(iNumEnabled));
		//throw std::string("Exactly one feature vector should be enabled. The options received from the smafedistd indicate " + stringify(iNumEnabled));
	}
	if (iNumEnabled == 0) {
		SMAFELOG_FUNC(SMAFELOG_WARNING, "Server process indicates 0 feature vectors."); // see also #277
	}


	if (so->distancetype_ids.size() != 1) {
		throw std::string("Exactly one distance type should be enabled. The options received from the smafedistd indicate " + stringify(so->distancetype_ids.size()));
	}
}

/** Sends feature vector to running smafedistd daemon and receives result (a string, no
 * more assumptions made at this point).
 * @param fvs pointer to vector of feature vectors. Assumed exactly one member in this vector
 * @param so pointer to options
 * @param ap pointer to program specific options
 */
std::string receiveDistcalcResult(std::vector<SmafeAbstractFeatureVector_Ptr>* fvs, Smafeopt* so, tAdminparams* ap) {

	try {
		tcp::iostream s(ap->sLivehost, stringify(ap->iLiveport));

		if (!s) {
			throw std::string("Cannot connect to " + ap->sLivehost + ":" + stringify(ap->iLiveport));
		}

		std::string ss_enc, ss_final;
		std::stringstream ss(std::stringstream::in | std::stringstream::out);

		tLiveNNMessage msg1(fvs->at(0).get(), ap->iLivetopk, ap->lCollectionId);

		//SmafeAbstractFeatureVector* safv = fvs->at(0).get();
		{
			boost::archive::text_oarchive oa(ss);
			oa << BOOST_SERIALIZATION_NVP(msg1);
		}
		// encryption
		ss_enc = encryptString(ss.str().c_str(), stdpassphrase);
		//		ss_enc = ss.str();
		// replace line breaks with spaces (line break is an EOT sign
		ss_final = boost::algorithm::replace_all_copy(ss_enc, "\n", " ") ;

		// Log in human readable format
		SMAFELOG_FUNC(SMAFELOG_DEBUG, "Sending: " + ss_final);

		// ab die Post
		s << ss_final << std::endl;


		/*
		// receive
		std::string sout;
		std::stringstream ssIn(std::stringstream::in | std::stringstream::out);
		while (std::getline(s, sout)) {
			SMAFELOG_FUNC(SMAFELOG_DEBUG, "Read line: " + sout);
			ssIn << sout << "\n";
		}
		return ssIn.str();
		 */

		// receive
		std::string sout;
		std::getline(s, sout);
		SMAFELOG_FUNC(SMAFELOG_DEBUG, "Read line: " + sout);
		return  boost::algorithm::replace_all_copy(sout, "||", "\n");


	} catch (std::exception& e) {
		SMAFELOG_FUNC(SMAFELOG_FATAL, e.what());
		exit(1);
	}
}


/** Performs one feature extraction job
 * This sub is called both if in daemon mode and in "normal" mode, ie, with command line parameters
 * Note that this sub also does the database communication, AND returns the feature vectors if textfile output mode is selected.
 * @param bManageTransactions if true, transaction management is done within the sub
 * @throws Exception in case of error. Exception is of type std::string
 */
std::vector<SmafeAbstractFeatureVector_Ptr> featureExtractionForFile(
		bool bManageTransactions, Smafeopt* so,
		SmafeFVType_Ptr_map &featureVectorTypes,
		SmafeExtractor_Ptr_vector &vExtractors, SMAFE_STORE_DB_CLASS* db,
		bool bSomlibmode, std::string filename, bool bDoNotUseDB,
		std::string sCollectionName, clock_t beginFile,
		SmafeSomlibfileExtractor* smafeSomlibExt, std::string guid,
		std::string external_key, bool &bStoredNewFV, long addfilejob_id) {
	// ------- Vars ----------
	SmafeFile* f = NULL;
	SmafeFile_Ptr f_ptr;
	/** Is a record with the current collection name in the Collection table? */
	bool bCollectionRecordInDB = false;
	/** Did the current file add the collection record ? */
	bool bCurrentFileLoopAddedCollectionRecord = false;
	/** id of collection record */
	long lCollection_id;
	/** array of feature vectors returned */
	std::vector<SmafeAbstractFeatureVector_Ptr> fvs;
	/** array of feature vectors returned (for segments) */
	std::vector<SmafeAbstractFeatureVector_Ptr> segmentfvs;
	/** to track if all extractors for a song have been skipped */
	bool bAllExtractorsSkipped;
	/** to track if an exception occured at at least one extractor */
	bool bExceptionOccuredInCurrentFile;

	bAllExtractorsSkipped = true;
	bExceptionOccuredInCurrentFile = false;
	bStoredNewFV = false;

	SMAFELOG_FUNC(SMAFELOG_DEBUG, "filename: " + filename);


	f = SmafeFile::getInstance(filename.c_str(), bSomlibmode);
	f_ptr.reset(f);
	so->setSongDependentOpts(f_ptr->getAudioformat().iSamplerate);

	// first lap time
	SMAFELOG_TIME_FUNC(beginFile, "file loaded");

#if ! defined(SMAFE_NODB)

	if (!bDoNotUseDB) {
		// -------------- database version

		long track_id;
		long file_id;
		/** has a track been added ?*/
		bool bTrackAdded = false;

		int db_ret = 0;

		t_fingerprint fingerprint;
		t_filehash hash;

		// Get fingerprint
		//fingerprint = new char[33];
		getFingerprint(f_ptr->getAudiobuffer(),
				f_ptr->getAudioformat().iDatasize, fingerprint);
		// Get hash value
		//hash = new char[33]; // md5 hash has 32 characters, plus \0
		getHashvalue(f_ptr->getAudiobuffer(),
				f_ptr->getAudioformat().iDatasize, hash);

		// This file already in Db? (query for hash)
		bool bFileInDB;
		file_id = db->isFileInDatabase(hash, filename, external_key, guid);
		bFileInDB = file_id >= 0;

		bool bTrackInDB, bFv;

		// Get info about track
		if (bFileInDB) {
			// This file is in DB
			SMAFELOG_FUNC(SMAFELOG_DEBUG, "File is already in database as file_id " + stringify(file_id));
			// get track_id
			track_id = db->getTrackIDForFile(file_id);
		} else {
			// This file is not in DB
			SMAFELOG_FUNC(SMAFELOG_DEBUG, "File is not in database");
			// Is this track already in Db? (query for fingerprint)
			track_id = db->isTrackInDatabase(fingerprint);
		}
		bTrackInDB = track_id >= 0;

		if (bTrackInDB) {
			// Track is in DB
			SMAFELOG_FUNC(SMAFELOG_DEBUG, "Track is already in database as track_id " + stringify(track_id));

			//bCalcAtLeastOne = false;
			// check for each sub feature vector part (RP etc)
			// whether the feature vector is in db
			// and change options accordingly
			// (no need to calc features twice)
			for (SmafeFVType_Ptr_map::iterator iter =
					featureVectorTypes.begin(); iter
					!= featureVectorTypes.end(); iter++) {
				if (so->mapExtractGenerally[iter->first]) {
					// exists Feature Vector?
					bFv = db->isFeatVecInDatabase(track_id, iter->second->id);
					// if no: calc at least one
					//bCalcAtLeastOne = bCalcAtLeastOne || !bFv;
					// if yes: dont calc this feature vector again
					so->mapExtractForSong[iter->first] = !bFv;
				} else {
					so->mapExtractForSong[iter->first] = false;
				}
			} // end of iterator

		} else { // if (bTrackInDB)
			// take all sub features to calculate according to options
			// ie make a copy
			so->mapExtractForSong = string_bool_map(so->mapExtractGenerally);
			SMAFELOG_FUNC(SMAFELOG_DEBUG, "Track not in DB, so copy mapExtractGenerally");
		}

		if (bManageTransactions) {
			// Start transaction per song
			db->startTransaction();
		}

		// Store track record?
		if (!bFileInDB && !bTrackInDB) {
			SMAFELOG_FUNC(SMAFELOG_INFO, "Writing track record to DB");
			track_id = db->storeTrackRecord(fingerprint);
			bTrackAdded = true;
		}

		// Store file record?
		if (!bFileInDB) {
			SMAFELOG_FUNC(SMAFELOG_INFO, "Writing file record to DB");
			tAudioformat ad = f_ptr->getAudioformat();
			file_id = db->storeFileRecord(track_id, hash, &ad, guid,
					external_key);
		}

		// Collection handling? if name is not empty: store this collection reference
		if (sCollectionName != "") {
			// Store collection record?
			if (!bCollectionRecordInDB && (lCollection_id
					= db->isCollectionInDatabase(sCollectionName)) < 0) {
				SMAFELOG_FUNC(SMAFELOG_INFO, "Writing collection record to DB");
				bCurrentFileLoopAddedCollectionRecord = true;
				lCollection_id = db->storeCollectionRecord(sCollectionName);
			}

			// Collection file record
			SMAFELOG_FUNC(SMAFELOG_DEBUG, "Writing collection-file record to DB");
			(void) db->ensureCollectionFileInDatabase(lCollection_id, file_id);
		}
		// insert default collection
		(void) db->ensureCollectionFileInDatabase_reservedname(
				SmafeStoreDB::RESERVEDCOLLECTIONS_DEFAULT, false, file_id);

		// remove file from _removed collection in case
		long lTuplesAff = db->removeFromRemovedCollectionIfNecessary(file_id);
		// we expect 0 or 1
		SMAFELOG_FUNC(SMAFELOG_DEBUG, stringify(lTuplesAff) + " collection-file mappings deleted when trying to remove the obsolete _removed mappings for this file.");
		if (lTuplesAff!= 0 and lTuplesAff != 1) {
			SMAFELOG_FUNC(SMAFELOG_WARNING, "Number of collection-file mappings deleted when trying to remove the obsolete _removed mappings for this file is "+stringify(lTuplesAff)+"  but the number should be 0 or 1!");
		}

		SMAFELOG_TIME_FUNC(beginFile, "fingerprints calculated and track, file & collection records inserted if necessary");

		// remove all elements from vector (feature vectors from previous songs)
		fvs.clear();
		segmentfvs.clear();

		SMAFELOG_FUNC(SMAFELOG_DEBUG, "Starting feature calculation");

		tAudioformat af = f_ptr->getAudioformat();

		for (SmafeExtractor_Ptr_vector::iterator iter = vExtractors.begin(); iter
		< vExtractors.end(); iter++) {
			SmafeExtractor* theEx = iter->get();

			try {
				if (theEx->getName() != SmafeSomlibfileExtractor::EXTRACTORNAME) {

					// not somlib extractor
					bAllExtractorsSkipped = !theEx->getFeatures(
							f_ptr->getAudiobuffer_normalized(), &af, so,
							&featureVectorTypes, fvs, segmentfvs, filename) && bAllExtractorsSkipped;
				} else {
					if (smafeSomlibExt != NULL) {
						// somlib file extractor
						bAllExtractorsSkipped = !smafeSomlibExt->getFeatures(
								NULL, NULL, so, &featureVectorTypes, fvs, segmentfvs,  filename)
								&& bAllExtractorsSkipped;
					} else {
						SMAFELOG_FUNC(SMAFELOG_DEBUG, "smafeSomlibExt is NULL, not calling somlib extractor");
					}
				}
			} // try block
			catch (std::string& s) {
				SMAFELOG_FUNC(SMAFELOG_ERROR, s);
				SMAFELOG_FUNC(SMAFELOG_WARNING, "... skipping " + theEx->getName());
				bExceptionOccuredInCurrentFile = true;
			}

		} // end of iterator SmafeExtractor_Ptr_vector

		SMAFELOG_TIME_FUNC(beginFile, "features calculated");

		// if features has been extracted
		if (fvs.size() > 0) {

			// Database output
			SMAFELOG_FUNC(SMAFELOG_INFO, "Writing feature vector record(s) to DB");

			db_ret = db->store(fvs, segmentfvs, file_id, track_id, addfilejob_id);

			SMAFELOG_TIME_FUNC(beginFile, "feature vectors stored in DB");

			if (db_ret != 0)
				throw("Problem with database storage.");

			if (ap->bPropagateJobsToSmafedistd) {
				// iterate
				for (std::vector<SmafeAbstractFeatureVector_Ptr>::iterator
						iter = fvs.begin(); iter < fvs.end(); iter++) {

					SmafeAbstractFeatureVector* theFv = iter->get();

					tDistancejobRecord djr;
					djr.priority = 0;
					djr.featurevectortype_id = theFv->fvtype->id;
					djr.track_id = track_id;
					djr.smafejob_addfile_id = addfilejob_id;
					db->storeDistancejobRecords(&djr);
				} // end of iterator

				SMAFELOG_TIME_FUNC(beginFile, "Propagation for distance daemon done");
			}
			// add smui add track if track has been newly added (do not propagate track if track has already been there and only
			// a new fv has been added.
			if (bTrackAdded && ap->bPropagateJobsToSmui) {
				// do not iterate, make only one record, also if more feature vectors are extracted
				if (fvs.size() > 0) {
					tSmuijob_addtrackRecord rec;
					rec.priority = 0;
					rec.track_id = track_id;
					db->storeSmuijob_addtrackRecord(&rec);
				}
				SMAFELOG_TIME_FUNC(beginFile, "Propagation for Smui done");
			}

			if (bManageTransactions) {
				// Commit!
				db->finishTransaction(true);
			}

			if (bCurrentFileLoopAddedCollectionRecord)
				bCollectionRecordInDB = true;

			bStoredNewFV = true;
		} else {
			// no features extracted.
			// Can mean:
			// - all features already in DB
			// or
			// - exceptions / error occured
			if (bExceptionOccuredInCurrentFile) {
				throw std::string(
						"No new features extracted for this file and at least one extractor raised an error.");
			} else {
				SMAFELOG_FUNC(SMAFELOG_INFO, "No new features extracted for this file.");
				if (bManageTransactions) {
					db->finishTransaction(true);
				}
			}
		}

		//					delete[] hash;
		//delete[] fingerprint;

		SMAFELOG_TIME_FUNC(beginFile, "Song finished (pending db changes committed)");
	} //if (!bToTextfile)

#endif

	if (bDoNotUseDB) {
		// -------------- textfile version

		SmafeNumericFeatureVector_Ptr fvp;

		SMAFELOG_FUNC(SMAFELOG_DEBUG, "Not using database.");

		// take all sub features to calculate according to options
		so->mapExtractForSong = string_bool_map(so->mapExtractGenerally);

		SMAFELOG_FUNC(SMAFELOG_DEBUG, "Starting feature calculation");

		tAudioformat af = f_ptr->getAudioformat();

		for (SmafeExtractor_Ptr_vector::iterator iter = vExtractors.begin(); iter
		< vExtractors.end(); iter++) {
			SmafeExtractor* theEx = iter->get();

			try {
				if (theEx->getName() != SmafeSomlibfileExtractor::EXTRACTORNAME) {

					// not somlib extractor
					bAllExtractorsSkipped = !theEx->getFeatures(
							f_ptr->getAudiobuffer_normalized(), &af, so,
							&featureVectorTypes, fvs, segmentfvs, filename);
				} else {
					// od not call the somlibfile extractor
				}
			} // try block
			catch (std::string& s) {
				SMAFELOG_FUNC(SMAFELOG_ERROR, s);
				SMAFELOG_FUNC(SMAFELOG_WARNING, "... skipping " + theEx->getName());
				bExceptionOccuredInCurrentFile = true;
			}

		} // end of iterator SmafeExtractor_Ptr_vector

		SMAFELOG_TIME_FUNC(beginFile, "features calculated");

	} else { // if bToTextfile
#if defined(SMAFE_NODB)
		// This exception is thrown iff:
		// - SMAFE_NODB is defined, AND
		// - bToTextfile is false
		// This should not happen, because bToTextfile should have
		// been set to true automatically
		throw std::string("Mismatch of preprocessor constant SMAFE_NODB and variable bToTextfile");
#endif
	} // else of (bToTextfile)

	// clean up file
	// is now smart pointer


	// return feature vectors
	return fvs;
}



#if !defined(DEPLOY)
// ------------------------------------------------------------------------
// old main

/** well that USED TO BE the entry point of this cute application */
// NOTE: parameters are not passed by reference (&)
void oldmain(Smafeopt* so, SmafeFVType_Ptr_map featureVectorTypes,
		SmafeExtractor_Ptr_vector vExtractors, std::string sFilename, std::string sCollectionName) {

	// for "benchmark"
	clock_t begin = clock();

	/** number of files to process */
	unsigned long ulFilesTotal = 0;

	// Stream that stores the filename(s) to be processed
	std::stringstream ssFiles(std::stringstream::in | std::stringstream::out);

	// file list?
	if (ap->bIsFileList) {
		// treat specified file as textfile with filenames
		std::ifstream ifsFileList(sFilename.c_str());
		std::string line;
		if (ifsFileList.is_open()) {
			while (!ifsFileList.eof()) {
				// read filenames from textfile and add to stream
				getline(ifsFileList, line);
				trimWhitespace(line);
				// if not empty line
				if (line != "") {
					ssFiles << line << std::endl;
					ulFilesTotal++;
				}
			}
			ifsFileList.close();
		} else {
			SMAFELOG_FUNC(SMAFELOG_FATAL, "Cannot find filelist " + sFilename);
			exit(2);
		}
	} else { // bIsFileList
		// is it a file?
		fs::path full_path(fs::initial_path<fs::path>());
		full_path = fs::complete(fs::path(sFilename, fs::native));
		full_path = fs::system_complete(fs::path(sFilename, fs::native));
		if (fs::is_directory(full_path)) {
			// argument is path name
			SMAFELOG_FUNC(SMAFELOG_DEBUG, std::string("Command line argument is directory name"));
			fs::directory_iterator end_itr; // default construction yields past-the-end
			for (fs::directory_iterator p(full_path), end_itr; p != end_itr; ++p) {
				// check for being file and wav ending
				// (is preliminary, should be more generic)
				if (!fs::is_directory(*p) && (fs::extension(*p) == std::string(
						".wav") || fs::extension(*p) == std::string(".mp3"))) {
					ssFiles << fs::system_complete(p->string()) << std::endl;
					SMAFELOG_FUNC(SMAFELOG_INFO, std::string("Adding to list: ") + p->string());
					ulFilesTotal++;
				} else
					SMAFELOG_FUNC(SMAFELOG_INFO, std::string("Skipping ") + p->string());
			}
		} else {
			// Argument is filename
			SMAFELOG_FUNC(SMAFELOG_DEBUG, std::string("Command line argument is NOT a directory, assuming file name"));
			ssFiles << full_path << std::endl;
			ulFilesTotal++;
		}
	} // else of bIsFileList

	if (ulFilesTotal == 0) {
		SMAFELOG_FUNC(SMAFELOG_FATAL, "No files to process. Exiting");
		exit(1);
	}

	clock_t beginFile;
	/** progress object */
	SmafeProgress sprogress;
	std::string sFile;
	/** Did the current file add the collection record ? */
	bool bCurrentFileLoopAddedCollectionRecord = false;
	/** array of feature vectors returned  - all*/
	std::vector<SmafeAbstractFeatureVector_Ptr> fvs_all; //
	/** array of feature vectors returned  - one iteration */
	std::vector<SmafeAbstractFeatureVector_Ptr> fvs;
	/** counter for successful files */
	int countOK = 0;
	/** counter for exceptions */
	int countExc = 0;
	/** Hold reference to somlibfile extractor */
	SmafeSomlibfileExtractor* smafeSomlibExt = NULL;
	/** just dummy, not used */
	bool bDummy;

	SMAFE_STORE_DB_CLASS* db = NULL; // NB: the actual type could be void*, if SMAFE_NODB is used

#if !defined(SMAFE_NODB)
	if (!ap->bToTextfile) {
		try {
			db = new SMAFE_STORE_DB_CLASS();
			db->openConnection(so->strDbhost, so->strDbport, so->strDbuser, so->strDbpwd, so->strDbname);

			// check for smui (copied also to main (for daemon mode)
			if (db->tableExists("dbinfo")) {
				ap->bPropagateJobsToSmui = true;
				SMAFELOG_FUNC(SMAFELOG_INFO, "Smui database schema found. Will propagate jobs to Smui subsystem.");
			} else {
				ap->bPropagateJobsToSmui = false;
				SMAFELOG_FUNC(SMAFELOG_INFO, "Smui database schema NOT found. Will NOT propagate jobs to Smui subsystem.");
				SMAFELOG_FUNC(SMAFELOG_DEBUG, "Remember: Existance of table 'dbinfo' is checked.");
			}
		} // try block
		catch (std::string& s) {
			SMAFELOG_FUNC(SMAFELOG_FATAL, s);
			//std::cout << s << "\n";
		} // catch block
	}
#endif

	// somlib file setting from adminparameters to smafeopt (latter is used by extractors)
	so->sSomlibfile = ap->sSomlibfile;

	// call sub fvts
	prepareFeatureVectorType(so, featureVectorTypes, vExtractors,
			smafeSomlibExt, db);
	assert(smafeSomlibExt != NULL);

	// check for special mode: somlib file only
	//	if (smafeRPExt.getNumberOfFeaturesToBeExtracted(featureVectorTypes, so)
	//			== 1
	//			&& so->mapExtractGenerally[SmafeSomlibfileExtractor::FVTYPE_NAME_SOMLIB]) {
	if (so->mapExtractGenerally[SmafeSomlibfileExtractor::FVTYPE_NAME_SOMLIB]) {
		SMAFELOG_FUNC(SMAFELOG_INFO, "** Special mode enabled: somlib only **");

		smafeSomlibExt->generateFilelistFromSomlibfile(ssFiles);

		ap->bSomlibmode = true;
	}

	sprogress = SmafeProgress(ulFilesTotal);

	// main loop
	do {
		// start stop watch for this file
		beginFile = clock();

		getline(ssFiles, sFile);
		// only if we got a new string (this avoids processing the last file twice)
		if (!ssFiles.eof()) {
			SMAFELOG_FUNC(SMAFELOG_INFO, std::string("Next file: ") + sFile);
			sFilename = sFile;

			try {
				// call sub
				// do neither use external_key nor guid
				// -1 as addfile job id: no addfile job!
				fvs = featureExtractionForFile(true, so, featureVectorTypes,
						vExtractors, db, ap->bSomlibmode, sFilename,
						ap->bToTextfile, ap->sCollectionName, beginFile,
						smafeSomlibExt, stringify((double)rand()), "", bDummy, -1);

				// insert into other
				fvs_all.insert(fvs_all.end(), fvs.begin(), fvs.end());

				SMAFELOG_FUNC(SMAFELOG_DEBUG, "currently we have " + stringify(fvs_all.size()) + " fvs.");

				countOK++;
			} // try block
			catch (std::string& s) {
				SMAFELOG_FUNC(SMAFELOG_ERROR, s);
				SMAFELOG_FUNC(SMAFELOG_ERROR, "... skipping this file.");
				countExc++;

#if ! defined(SMAFE_NODB)
				// rollback only if program has been compiled with db support
				// and db output is enabled for this run
				if (!ap->bToTextfile && db != NULL) {
					try {
						db->finishTransaction(false);
					} catch (std::string& s) {
						SMAFELOG_FUNC(SMAFELOG_ERROR, s);
						SMAFELOG_FUNC(SMAFELOG_ERROR, "Rollback failed.");
					}
					bCurrentFileLoopAddedCollectionRecord = false;
				}
#endif

				//std::cout << s << "\n";
			} // catch block

			// log estimation
			sprogress.setProcessed(countOK + countExc);
			SmafeLogger::smlog->log_esttimeleft(SmafeLogger::SMAFELOG_INFO,
					&sprogress);

		} // (!ssFiles.eof()
	} while (!ssFiles.eof());

	// print summary
	char strings_in_c_suck[100];
	sprintf(strings_in_c_suck,
			"Processed %i files successfully, %i exceptions occured.", countOK,
			countExc);
	SMAFELOG_FUNC(SMAFELOG_INFO, std::string(strings_in_c_suck));

	// output as textfile if requested and at least one member in vectors
	if (ap->bToTextfile) {
		SmafeTextfileOutput sto;

		SMAFELOG_FUNC(SMAFELOG_INFO, "Writing feature vectors in Somlib format to file(s)");

		// iterate through all types
		for (SmafeFVType_Ptr_map::iterator iter = featureVectorTypes.begin(); iter
		!= featureVectorTypes.end(); iter++) {
			try {
				SMAFELOG_FUNC(SMAFELOG_DEBUG, "Writing feature vectors in Somlib format to file: " + iter->first);
				SMAFELOG_FUNC(SMAFELOG_DEBUG2, "currently we have " + stringify(fvs_all.size()) + " fvs.");

				sto.output(fvs_all, iter->first,
						std::string(ap->fv_output_files_prefix), SmafeTextfileOutput::FV_OUTPUT_FILES_SUFFIX);
			} // try block
			catch (std::string& s) {
				SMAFELOG_FUNC(SMAFELOG_FATAL, s);
			} // catch block

		} // end of iterator

	}// (bToTextfile)

#if ! defined(SMAFE_NODB)
	delete db;
#endif

	// execution time

	clock_t end = clock();
	sprintf(strings_in_c_suck, "Execution time: %.0f ms.\n", diffclock(end,
			begin));
	SMAFELOG_FUNC(SMAFELOG_INFO, std::string(strings_in_c_suck));

} // void oldmain(int argc, char* argv[])
#endif // only if not deploy

// ------------------------------------------------------------------------
/** one round of the main loop
 * @param so pointer to options instance
 * @param featureVectorTypes Smart pointer to map of feature vector types
 * @return false if caller should exit, true otherwise
 */
void  main_loop_round(Smafeopt* so, SmafeFVType_Ptr_map featureVectorTypes,
		SmafeExtractor_Ptr_vector vExtractors) {
	/** open task for one job category? */
	bool bVacancy = true;
	/** open task for any job category? */
	bool bAnyvacancy = true;
	long lNumVacancies, lNumCurrentVac;
	SMAFE_STORE_DB_CLASS* db = NULL;
	/** sql command */
	//	char sqlcmd[1500];
	std::string sqlcmd;
	/** will be set to true if error has occured */
	bool bErrorOccured = false;
	/** tSmafejob_addfileRecord */
	tSmafejob_addfileRecord af_rec;
	/** delete file job */
	tSmafejob_deletefileRecord df_rec;
	/** delete collection job */
	tSmafejob_deletecollectionRecord dc_rec;
	/** number of rows affected */
	long nTups;
	/** flag: new fv inserted? */
	bool bStoredNewFV;
	/** track count */
	size_t stTracks;
	/** progress object */
	SmafeProgress* sprogress = NULL;
	/** counter for successful files */
	int countOK = 0;
	/** counter for exceptions */
	int countExc = 0;

	try {
		// create db connection
		db = new SMAFE_STORE_DB_CLASS();
		db->openConnection(so->strDbhost, so->strDbport, so->strDbuser, so->strDbpwd, so->strDbname);

		while (bAnyvacancy && !b_should_terminate) {

			bAnyvacancy = false;

			// START ------- add file jobs -------------

			// get info on job market situation
			bVacancy = db->getOpenTaskInfo_addfile(af_rec, lNumVacancies,
					lNumCurrentVac);

			bAnyvacancy = bAnyvacancy || bVacancy;

			if (sprogress == NULL) {
				sprogress = new SmafeProgress(lNumCurrentVac);
			}


			// Announce open tasks
			// Note: "open" is a grep keyword used in the init script: /etc/init.d/smafed status
			SMAFELOG_FUNC(SMAFELOG_INFO, stringify(lNumVacancies) + " open addfile task(s) found.");

			// check for licence restrictions
			SmafeParamHelper::licenseCheck(so);

			if (bVacancy && !ap->bPrintStatsOnly) {



				// Begin local log
				/** stringstream for local log */
				std::stringstream llog;

				SmafeLogger::smlog->setDesttmp(&llog, af_rec.log, std::string(
						PROGNAME) + ", pid=" + stringify(my_getpid()));

				// Mark task as being processed in db
				db->startTransaction();
				// stricter concurrency control
				db->executeStatement(
						"SET TRANSACTION ISOLATION LEVEL SERIALIZABLE");

				sqlcmd = "UPDATE smafejob_addfile SET status='" + ap->daemonId
						+ "', started=CURRENT_TIMESTAMP WHERE id=" + stringify(af_rec.id);

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

					SMAFELOG_FUNC(SMAFELOG_INFO, std::string("Starting addfile task: id=") + stringify(af_rec.id) +
							std::string(", file_uri=") + af_rec.file_uri + std::string(", external_key=") + af_rec.external_key);

					// for "benchmark"
					clock_t begin_clock = clock();
					time_t begin_time = time(NULL);

					try {
						// start transaction
						// that can happen here because in the loop we did not insert anything
						// Remember that insert.. only collects the data in a vector and only
						// at commit time the corresponding sql COPY command is generated and executed
						db->startTransaction();

						// check for limit
						if (so->limittrackscount > 0) {
							stTracks = db->getFileCount();
							SMAFELOG_FUNC(SMAFELOG_DEBUG, "File record count: " + stringify(stTracks));
							if (stTracks >= so->limittrackscount) {
								// limit reached
								SMAFELOG_FUNC(SMAFELOG_DEBUG, "Limit reached: " + stringify(so->limittrackscount));
								throw std::string("File limit of " + stringify(so->limittrackscount) +  " reached. No new files will be analyzed. Please contact office@spectralmind.com");
							} else {
								// limit not yet reached
								SMAFELOG_FUNC(SMAFELOG_DEBUG, "Limit NOT reached: " + stringify(so->limittrackscount));
								// check for warning
								if (stTracks >= double(so->limittrackscount) * 0.9) {
									SMAFELOG_FUNC(SMAFELOG_WARNING, "File limit of " + stringify(so->limittrackscount) + " almost reached, only "+stringify(so->limittrackscount-stTracks)+" files left. Please contact office@spectralmind.com");
								}
							}
						}


						// do feature extraction


						// call sub
						// feature vectors are saved to db in the sub, no need to welcome them here
						// anymore
						(void) featureExtractionForFile(false, so,
								featureVectorTypes, vExtractors, db,
								ap->bSomlibmode, af_rec.file_uri, false,
								af_rec.collection_name, clock(), NULL,
								af_rec.guid, af_rec.external_key, bStoredNewFV,
								af_rec.id);

						try {
							// move or delete file (or do nothing)
							if (so->sFileDest == "") {
								// do nothing
								SMAFELOG_FUNC(SMAFELOG_DEBUG, "Processed file is not deleted or moved.");
							} else if (so->sFileDest == "-") {
								// delete
								SMAFELOG_FUNC(SMAFELOG_DEBUG, "Processed file is to be deleted.");
								if (remove(af_rec.file_uri.c_str()) != 0)
									throw std::string("Error deleting file "
											+ af_rec.file_uri + " - "
											+ stringify(errno));
								else
									SMAFELOG_FUNC(SMAFELOG_INFO, "File deleted: " + af_rec.file_uri);
							} else {
								// move
								fs::path pTo = fs::path(so->sFileDest) / fs::path(
										af_rec.file_uri).leaf();
								try {
									fs::rename(fs::path(af_rec.file_uri), pTo);
								} catch (fs::filesystem_error err) {
									// rename failed, try again using copy and delete
									fs::copy_file(fs::path(af_rec.file_uri), pTo);
									fs::remove(fs::path(af_rec.file_uri));
								}
								SMAFELOG_FUNC(SMAFELOG_INFO, "File moved: " + af_rec.file_uri + " to " + pTo.string());
							}
						} catch (fs::filesystem_error err) {
							// rethrow the error as a string
							throw std::string(err.what());
						}

						//						std::string sCode = SmafeStoreDB::STATUSFEDONE;
						std::string sCode = SmafeStoreDB::STATUSOK;

						sqlcmd
						= "UPDATE smafejob_addfile SET status='"
								+ sCode
								+ "', finished1=CURRENT_TIMESTAMP, finished=CURRENT_TIMESTAMP, log='"
								+ db->escapeString(llog.str())
								+ "' WHERE id=" + stringify(af_rec.id);
						db->executeStatement(sqlcmd.c_str());

						SMAFELOG_FUNC(SMAFELOG_DEBUG, "Committing");
						db->finishTransaction(true);

						// log time
						clock_t end_clock = clock();
						time_t end_time = time(NULL);
						SMAFELOG_FUNC(SMAFELOG_INFO, "Finished addfile task: id=" + stringify(af_rec.id) +
								", file_uri=" + af_rec.file_uri + ", external_key=" + af_rec.external_key +
								+ " in " +
								stringify(difftime (end_time,begin_time)) + " s (usr time " +
								stringify(diffclock(end_clock,begin_clock)) + " ms)");

						countOK++;
					} catch (std::string& s) {
						SMAFELOG_FUNC(SMAFELOG_ERROR, "Exception occured during add file job (feature extraction).");
						SMAFELOG_FUNC(SMAFELOG_ERROR, s);
						SMAFELOG_FUNC(SMAFELOG_WARNING, "Rolling back and marking job record as FAILED");
						db->finishTransaction(false);

						// Marking as ERROR
						db->startTransaction();
						sqlcmd = "UPDATE smafejob_addfile SET status='"
								+ SmafeStoreDB::STATUSFAILED + " " + db->escapeString(s)
								+ "', finished=CURRENT_TIMESTAMP, log='"
								+ db->escapeString(llog.str()) + "' WHERE id="
								+ stringify(af_rec.id);
						db->executeStatement(sqlcmd.c_str());

						db->finishTransaction(true);

						countExc++;

					} // end of catch block

				} else { // if (db->finishTransaction_serializationcheck())
					SMAFELOG_FUNC(SMAFELOG_INFO, "Ooops. Another daemon took my task just when I was about to start...");
					db->finishTransaction(false);
				} // if (db->finishTransaction_serializationcheck())

				// end local log
				SmafeLogger::smlog->resetDesttmp();

				// log estimation
				sprogress->setProcessed(countOK + countExc);
				SmafeLogger::smlog->log_esttimeleft(SmafeLogger::SMAFELOG_INFO,
						sprogress);

			} else {
				if (!ap->bPrintStatsOnly)
					SMAFELOG_FUNC(SMAFELOG_INFO, "No open addfile tasks found");
			} // if (bVacancy && !bPrintStatsOnly)

			// END ------- add file jobs -------------


			// START ------- delete file jobs -------------
			// get info on job market situation
			bVacancy = db->getOpenTaskInfo_deletefile(df_rec, lNumVacancies,
					lNumCurrentVac);
			bAnyvacancy = bAnyvacancy || bVacancy;

			// Announce open tasks
			// Note: "open" is a grep keyword used in the init script: /etc/init.d/smafed status
			SMAFELOG_FUNC(SMAFELOG_INFO, stringify(lNumVacancies) + " open deletefile task(s) found.");
			//SMAFELOG_FUNC(SMAFELOG_INFO, stringify(lNumCurrentVac) + " open task(s) matching current parameters.");

			if (bVacancy && !ap->bPrintStatsOnly) {
				// Begin local log
				/** stringstream for local log */
				std::stringstream llog;

				SmafeLogger::smlog->setDesttmp(&llog, df_rec.log, std::string(
						PROGNAME) + ", pid=" + stringify(my_getpid()));

				// Mark task as being processed in db
				db->startTransaction();
				// stricter concurrency control
				db->executeStatement(
						"SET TRANSACTION ISOLATION LEVEL SERIALIZABLE");

				sqlcmd = "UPDATE smafejob_deletefile SET status='" + ap->daemonId
						+ "', started=CURRENT_TIMESTAMP WHERE id=" + stringify(
								df_rec.id);

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

					SMAFELOG_FUNC(SMAFELOG_INFO, std::string("Starting deletefile task: id=") + stringify(df_rec.id) +
							std::string(", file_id=") + stringify(df_rec.file_id) + ", collection_name=" + df_rec.collection_name);

					// for "benchmark"
					clock_t begin_clock = clock();
					time_t begin_time = time(NULL);

					try {
						// start transaction
						db->startTransaction();

						// start the real work

						// remove from all collections or just from one?
						if (df_rec.collection_name == "") {
							// remove file from all collections
							SMAFELOG_FUNC(SMAFELOG_DEBUG, "remove file from all collections");
							sqlcmd
							= "DELETE FROM collection_file WHERE file_id="
									+ stringify(df_rec.file_id) + ";";
							SMAFELOG_FUNC(SMAFELOG_DEBUG, "sql: " + sqlcmd);
							nTups = db->executeStatement(sqlcmd.c_str());
							// now, there is no collection_file record for this file
						} else {
							// remove file from one collections
							SMAFELOG_FUNC(SMAFELOG_DEBUG, "remove file from collection " + df_rec.collection_name);
							sqlcmd
							= "DELETE FROM collection_file WHERE file_id="
									+ stringify(df_rec.file_id)
									+ " AND collection_id="
									+ "(SELECT c.id from collection c WHERE c.collection_name = '"
									+ db->escapeString(
											df_rec.collection_name)
											+ "');";
							SMAFELOG_FUNC(SMAFELOG_DEBUG, "sql: " + sqlcmd);
							nTups = db->executeStatement(sqlcmd.c_str());
						}

						// add file to _removed only if no other collection is connected
						db->ensureCollectionFileInDatabase_reservedname(
								SmafeStoreDB::RESERVEDCOLLECTIONS_REMOVED,
								true, df_rec.file_id);

						// log numjber of tups
						SMAFELOG_FUNC(SMAFELOG_INFO, "Deleted " + stringify(nTups) + " rows.");
						if (nTups == 0)
							SMAFELOG_FUNC(SMAFELOG_WARNING, "No rows affected from command: " + sqlcmd + ". Still this job will be marked as OK");

						// possibly propagate to smui
						if (ap->bPropagateJobsToSmui) {
							// if this was the last file that has been deleted for this track
							// we generate an appropriate smui task

							// get track id
							long track_id = db->getTrackIDForFile(
									df_rec.file_id);

							// get file ids but not removed ones
							std::vector<long> vTmp = db->getFilesForTrack(
									track_id, false);

							// if not at least one
							if (vTmp.size() == 0) {
								tSmuijob_deletetrackRecord rec;
								rec.priority = df_rec.priority;
								rec.track_id = track_id;
								db->storeSmuijob_deletetrackRecord(&rec);
								SMAFELOG_FUNC(SMAFELOG_INFO, "Propagation for Smui done (delete track job created for track_id=" + stringify(track_id) + ")");
							}

						} // if (bPropagateJobsToSmui)

						// successful finish
						sqlcmd = "UPDATE smafejob_deletefile SET status='"
								+ SmafeStoreDB::STATUSOK
								+ "', finished=CURRENT_TIMESTAMP, log='"
								+ db->escapeString(llog.str()) + "' WHERE id="
								+ stringify(df_rec.id);
						db->executeStatement(sqlcmd.c_str());

						SMAFELOG_FUNC(SMAFELOG_DEBUG, "Committing");
						db->finishTransaction(true);

						// log time
						clock_t end_clock = clock();
						time_t end_time = time(NULL);
						SMAFELOG_FUNC(SMAFELOG_INFO, "Finished deletefile task: id=" + stringify(df_rec.id) +
								", file_id=" + stringify(df_rec.file_id) + ", collection_name=" + df_rec.collection_name
								+ " in " +
								stringify(difftime (end_time,begin_time)) + " s (usr time " +
								stringify(diffclock(end_clock,begin_clock)) + " ms)");
					} catch (std::string& s) {
						SMAFELOG_FUNC(SMAFELOG_ERROR, "Exception occured during delete file job.");
						SMAFELOG_FUNC(SMAFELOG_ERROR, s);
						SMAFELOG_FUNC(SMAFELOG_WARNING, "Rolling back and marking job record as FAILED");
						db->finishTransaction(false);

						// Marking as ERROR
						db->startTransaction();
						sqlcmd = "UPDATE smafejob_deletefile SET status='"
								+ SmafeStoreDB::STATUSFAILED  + " " + db->escapeString(s)
								+ "', finished=CURRENT_TIMESTAMP, log='"
								+ db->escapeString(llog.str()) + "' WHERE id="
								+ stringify(df_rec.id);
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
				if (!ap->bPrintStatsOnly)
					SMAFELOG_FUNC(SMAFELOG_INFO, "No open deletefile tasks found");
			} // if (bVacancy && !bPrintStatsOnly)
			// END ------- delete file jobs -------------


			// START ------- delete collection jobs -------------
			// get info on job market situation
			bVacancy = db->getOpenTaskInfo_deletecollection(dc_rec,
					lNumVacancies, lNumCurrentVac);
			bAnyvacancy = bAnyvacancy || bVacancy;

			// Announce open tasks
			// Note: "open" is a grep keyword used in the init script: /etc/init.d/smafed status
			SMAFELOG_FUNC(SMAFELOG_INFO, stringify(lNumVacancies) + " open delete collection task(s) found.");
			//SMAFELOG_FUNC(SMAFELOG_INFO, stringify(lNumCurrentVac) + " open task(s) matching current parameters.");

			if (bVacancy && !ap->bPrintStatsOnly) {

				SMAFELOG_FUNC(SMAFELOG_WARNING, "Delete collection not yet implemented!");

			} else {
				if (!ap->bPrintStatsOnly)
					SMAFELOG_FUNC(SMAFELOG_INFO, "No open deletecollection tasks found");
			} // if (bVacancy && !bPrintStatsOnly)
			// END ------- delete collection jobs -------------

			if (ap->bPrintStatsOnly)
				b_should_terminate = true;

		} // while


	} catch (std::string& s) {
		SMAFELOG_FUNC(SMAFELOG_FATAL, s);
		bErrorOccured = true;
	}

	if (sprogress != NULL) delete sprogress;
	delete db;

	// exit in case of error if not daemon mode
	if (bNoDaemon && bErrorOccured)
		b_should_terminate = true;

}

/** well that's the entry point of this cute application */
int main(int argc, char* argv[]) {

/*
 // Test for 1.35 de-s11n problem
 // http://stackoverflow.com/questions/8314202/backwards-compatibility-of-vector-deserialization-with-boost-serialization
	std::vector<long> testvector;
	std::string val = "22 serialization::archive 4 2 1 2"; // v1.35 archive
	{
	std::stringstream ss(std::stringstream::in | std::stringstream::out);
	ss << val << std::endl;
	boost::archive::text_iarchive ia(ss);
	ia >> BOOST_SERIALIZATION_NVP(testvector);
	}
	// print vector
	for(int i=0; i<testvector.size(); ++i)
		std::cout << testvector[i] << " ";
	std::cout << std::endl;
	exit(0);
*/

	try {

		/** Options instance */
		so = new Smafeopt();
		ap = new tAdminparams;
		ap->bPropagateJobsToSmafedistd = true;
		ap->bPropagateJobsToSmui = false;

#if defined(DEPLOY)
		splashScreen("Smafewrap daemon");
#else
		splashScreen("Smafewrap daemon - admin version");
#endif

		// call init function if static linking is used
#if defined(STATICLINKED)
		ippStaticInit();
#endif

		// get Capabilities of available extractors
		// add a line here if you make a new one!
		SmafeFVType_Ptr_map featureVectorTypes;

		SmafeExtractor_Ptr_vector vExtractors;
		SmafeExtractor_Ptr ex_ptr;

		smafeRPExtractor* smafeRPExt = new smafeRPExtractor();
		smafeRPExt->getCapabilities(featureVectorTypes);
		ex_ptr.reset(smafeRPExt);
		vExtractors.push_back(ex_ptr);

		SmafeTimbralExtractor* smafeTimbralExt = new SmafeTimbralExtractor();
		smafeTimbralExt->getCapabilities(featureVectorTypes);
		ex_ptr.reset(smafeTimbralExt);
		vExtractors.push_back(ex_ptr);

		SmafeSomlibfileExtractor* smafeSomlibExt = new SmafeSomlibfileExtractor();
		smafeSomlibExt->getCapabilities(featureVectorTypes);
		ex_ptr.reset(smafeSomlibExt);
		vExtractors.push_back(ex_ptr);

		// Process command line options
		try {
			bool ret;
			// this also loads options from db IF not admin mode and not live mode
			ret = SmafeParamHelper::processCommandLineArguments(argc, argv, featureVectorTypes, so, ap, bNoDaemon);
			// if false, we should quit
			if (!ret) {
				SMAFELOG_FUNC(SMAFELOG_INFO, "Running in config mode, finished, quitting.");
				exit(0);
			}

			so->setDependentOpts(); // calculate derived options

			if (ap->bJobsMode) {
				// only in jobs mode. in live mode the real options will be received later by the server
				logOptions();
			}
		} catch (std::string& s) {
			SMAFELOG_FUNC(SMAFELOG_FATAL, s);
			exit(1);
		}

		if (!bNoDaemon) {
#if defined(SMAFEDISTD_REAL_DAEMON)
			// -------- switch to logfile

			switchToLogfile(ap);

			splashScreen("Smafewrap daemon. ID=" + ap->daemonId);

			// mirror all params
			logOptions();
			SMAFELOG_FUNC(SMAFELOG_INFO, "polling interval=" + stringify(ap->pollInterval));
			SMAFELOG_FUNC(SMAFELOG_INFO, "processed file destination=" + so->sFileDest);


			daemonize();
#else
			SMAFELOG_FUNC(SMAFELOG_FATAL, "Trying to daemonize but is not possible on this platform (see HAVE_WORKING_FORK from configure)");
			exit(1);
#endif
		}

		SMAFELOG_FUNC(SMAFELOG_DEBUG, "Number of supported feature vector types is " + stringify(featureVectorTypes.size()));

		// some assertions as a last chance to bail out
		assert(!(ap->bLivemode && ap->bJobsMode));
		assert(!(ap->bLivemode && !bNoDaemon));

		//---------------------- live mode
		if (ap->bLivemode) {
			try {
				SmafeSomlibfileExtractor* smafeSomlibExt = NULL;
				clock_t beginFile;
				/** value not used */
				bool bDummy;
				/** array of feature vectors returned  - one iteration */
				std::vector<SmafeAbstractFeatureVector_Ptr> fvs;

				// switch to log file
				//switchToLogfile(ap);

				// call sub fvts
				prepareFeatureVectorType(so, featureVectorTypes, vExtractors,
						smafeSomlibExt, NULL);

				// get options
				SMAFELOG_FUNC(SMAFELOG_INFO, "Start requesting options from daemon...");
				so = receiveOptionsFromDistd(so, ap);
				SMAFELOG_FUNC(SMAFELOG_INFO, "...Options received.");
				so->setDependentOpts(); // calculate derived options
				logOptions();
				try {
					optionsValid4Livemode(so, &featureVectorTypes);
				} catch (std::string& s) {
					SMAFELOG_FUNC(SMAFELOG_FATAL, "Options (received from smafedistd) are not valid for live mode query. This can, e.g., happen if more than one feature vector are configured.");
					SMAFELOG_FUNC(SMAFELOG_FATAL, s);
					exit(1);
				}

				// start stop watch for this file
				beginFile = clock();

				// Start feature extraction, no guid, no external key,
				SMAFELOG_FUNC(SMAFELOG_DEBUG, "Next file: " + ap->sLivefile);
				fvs = featureExtractionForFile(false, so, featureVectorTypes, vExtractors, NULL, false, ap->sLivefile, true, "", beginFile, smafeSomlibExt, "", "", bDummy, -1);

				if (! (fvs.size() == 1)) {
					SMAFELOG_FUNC(SMAFELOG_FATAL, "Unexpected: no or more than one feature vector returned. Size() of vector: " + stringify(fvs.size()));
					exit(1);
				}

				// send fv and receive result
				SMAFELOG_FUNC(SMAFELOG_INFO, "Start requesting nearest neighbour from daemon...");
				std::string sResult = receiveDistcalcResult(&fvs, so, ap);
				SMAFELOG_FUNC(SMAFELOG_INFO, "...Received result from daemon");

				// output to std out
				std::cout << sResult << std::endl;

			} // try block
			catch (std::string& s) {
				SMAFELOG_FUNC(SMAFELOG_ERROR, s);
			}
		} else

			//--------------------- jobs mode
			if (ap->bJobsMode) {
				// daemon init procedure START
				/** Hold reference to somlibfile extractor */
				SmafeSomlibfileExtractor* smafeSomlibExt = NULL;

				SMAFE_STORE_DB_CLASS* db = NULL; // NB: the actual type could be void*, if SMAFE_NODB is used


#if !defined(SMAFE_NODB)
				try {
					// check for licence restrictions
					SmafeParamHelper::licenseCheck(so);

					db = new SMAFE_STORE_DB_CLASS();
					db->openConnection(so->strDbhost, so->strDbport, so->strDbuser, so->strDbpwd, so->strDbname);
				} catch (std::string& s) {
					SMAFELOG_FUNC(SMAFELOG_FATAL, s);
					exit(1);
				}
#else
				SMAFELOG_FUNC(SMAFELOG_FATAL, "Jobs mode with SMAFE_NODB not possible.");
				exit(1);
#endif

				// call sub fvts
				prepareFeatureVectorType(so, featureVectorTypes, vExtractors,
						smafeSomlibExt, db);

				// check for smui (also copied to oldmain!)
				if (db->tableExists("dbinfo")) {
					ap->bPropagateJobsToSmui = true;
					SMAFELOG_FUNC(SMAFELOG_INFO, "Smui database schema found. Will propagate jobs to Smui subsystem.");
				} else {
					ap->bPropagateJobsToSmui = false;
					SMAFELOG_FUNC(SMAFELOG_INFO, "Smui database schema NOT found. Will NOT propagate jobs to Smui subsystem.");
					SMAFELOG_FUNC(SMAFELOG_DEBUG, "Remember: Existance of table 'dbinfo' is checked.");
				}

				delete db;
				db = NULL;

				// daemon init procedure END


				// main loop
				while (!b_should_terminate) {
					main_loop_round(so, featureVectorTypes, vExtractors);

					if (!b_should_terminate && !ap->bPrintStatsOnly) {
						SMAFELOG_FUNC(SMAFELOG_INFO, "Going to sleep for " + stringify(ap->pollInterval) + " minutes...");
						// check if interval is < 0: then exit
						if (ap->pollInterval >= 0) {
							// check for licence restrictions
							SmafeParamHelper::licenseCheck(so);

							sleep(ap->pollInterval * 60); // pollInterval is in minutes
						} else
							b_should_terminate = true;
					}
					if (ap->bPrintStatsOnly) {
						SMAFELOG_FUNC(SMAFELOG_INFO, "No action (stats parameter given).");
						b_should_terminate = true;
					}
				}

				SMAFELOG_FUNC(SMAFELOG_INFO, "Daemon says Good Bye.");
			} else {
				//--------------------- admin mode / file mode

#if defined(DEPLOY)
				SMAFELOG_FUNC(SMAFELOG_FATAL, "Compiled as deploy version. So, only jobs mode is possible.");
				exit(1);
#else
				oldmain(so, featureVectorTypes, vExtractors, ap->sFilename, ap->sCollectionName);
#endif
			}

		delete so;
		delete ap;

	}catch( std::exception const & e ) {
		SMAFELOG_FUNC(SMAFELOG_FATAL, e.what());
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

