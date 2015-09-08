///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// smafeopt.h
//
// Class representing options
// ------------------------------------------------------------------------
//
// $Id$
//
//
//
//
//
//
//
///////////////////////////////////////////////////////////////////////////

#pragma once

#include "config.h"
#include "argtable2.h"

#include "smafeExportDefs.h"
#include "smafeLogger.h"

#include "smafestore_specific_include_no_text.h"

#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
// order of includes:
// http://lists.boost.org/boost-users/2007/08/30333.php
//#include <boost/serialization/export.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
//#include <boost/serialization/base_object.hpp>


#include <boost/lexical_cast.hpp>

//#include <boost/date_time/posix_time/posix_time.hpp> //include all types plus i/o
//#include <boost/date_time/posix_time/posix_time_types.hpp> //no i/o just types


#include <boost/date_time/gregorian/gregorian.hpp> //include all types plus i/o
//#include <boost/date_time/gregorian/gregorian_types.hpp> //no i/o just types

#include <boost/date_time/gregorian/greg_serialize.hpp>

namespace bg = boost::gregorian;

// Strange: without including string here there are compile errors. To be resolved... EPEI
#include <string>


#include <map>




/** Class representing options for RP feature extraction. Is also container for
 * precalculated stuff like filter windows etc.
 */
class DLLEXPORT Smafeopt {
public:

	// ----------- to be loaded from db

	/** Number of samples to skip at beginning (Skipin) and end of song (skipout) */
	unsigned int uiSkipin;
	/** Number of samples to skip at beginning (Skipin) and end of song (skipout) */
	unsigned int uiSkipout;
	/** stepwith for segment iteration (1 = take each segment) */
	unsigned int uiStepwidth;
	/** return all segments' features? */
	bool bReturnSegmentFeatures;
	/** normalize to FFT energy (Gabriel's method from project) */
	bool bNormalizeFFTEnergy;
	/** transform to decibel? */
	bool bTransformDecibel;
	/** transform to sone? */
	bool bTransformSone;
	/** which parts to take for rp (~ Matlab's mod_ampl_limit) */
	unsigned int uiModAmplLimit;
	/** also take DC component for rp? */
	bool bIncludeDC;
	/** number of bark bands  */
	short shNumBarkBands;
	/** Use fluctuation strength weighting for RP? */
	bool bFluctuationStrengthWeighting;
	/** Blurring with first matrix ? */
	bool bBlurring1;
	/** Blurring with second matrix? */
	bool bBlurring2;
	/** fv output files prefix, may include path */
	//	std::string fv_output_files_prefix;
	/** path where file is moved to after processing (jobs mode) */
	std::string sFileDest;
	/** minimum verbosity (to exclude debug messages) */
	int min_loglevel;
	/** vector of distancetype_ids */
	std::vector<long> distancetype_ids;
	/** top k */
	size_t top_k;
	/** codebook file */
	std::string sCodebookfile;
	/** defines factor for max vincinity for bubbles to change.
	 * Calculation to som units:
	 * min(som-width, som-height) * factor
	 * sql nearest neighbour queries need that constant to be efficient.
	 * the nearest bubble should be within that area from the new file.
	 * If too low, no bubble gets updated.
	 * If too high, sql queries takes unnecessarily too long to complete
	 */
	double dMaxBubbleVincinityFactor;


	// note: for dynamic options
	/** Map with <String, boolean> pairs that specifies whether
	 * a specific feature vector type should _generally_ be extracted
	 * <p>This is specified via command line arguments.</p>
	 */
	string_bool_map mapExtractGenerally;

	/** # of max tracks in db
	 * 0 .. unrestricted */
	size_t limittrackscount;

	/** trial limits in days */
	size_t limittrialdays;

	/** hard expiration date */
	bg::date limitharddate;

	/** "soft" expiration date
	 * this date is defined and stored in the db at the first run of smafewrapd.
	 * Formula: current date + limittrialdays = limitsoftdate
	 * The absence of this field in config table indicates the frist run of the software */
	bg::date limitsoftdate;

	/** date of last run of smafewrapd
	 * This date is stored in db by smafewrapd during runtime*/
	bg::date lastuseddate;
	/** encryped key for lastuseddate to use for UPDATE later */
	std::string sLastuseddateKey;




	// ------------ set via command line args or config file
	/** db host */
	std::string strDbhost;
	/** db port */
	std::string strDbport;
	/** db name  */
	std::string strDbname;
	/** db user  */
	std::string  strDbuser;
	/** db pwd   */
	std::string  strDbpwd;

	/** name of somlib file
	 * for SmafeSomlibfileExtractor only */
	std::string sSomlibfile;




	// ------------ derived (set at runtime, maybe for each song)
	// the following options are set at runtime for each song individually
	/** FFT-Size for feature extraction */
	int iFFTSize;
	/** FFT-Size for 2nd FFT (to get RP form BarkSpec) */
	int iFFTSize_rp, iFFTOrder_rp;
	/** length of rh vectors */
	int iRHLen;
	/** we take only a subset of the feature set rows after 2nd FFT */
	unsigned int uiFeatFrom, uiFeatTo, uiFeatFrom_rh, rh_len;
	int iBarkBandsUsed;

	/** Map with <String, boolean> pairs that specifies whether
	 * a specific feature vector type should  be extracted for
	 * the _next_ song.
	 * <p>That depends on both command line arguments and
	 * the existance of feature vector type records in the db
	 * ( unless output is textfile)</p>
	 */
	string_bool_map mapExtractForSong;



	Smafeopt(){
		// default values
		// interim: options marked with # are not used
		//sFilename = std::string("(not set)");
		uiSkipin = 1;      // as of 2010-03-22: default params as discussed with TL
		uiSkipout = 1;
		uiStepwidth = 3;
		bReturnSegmentFeatures = false;
		bNormalizeFFTEnergy = false;
		bTransformDecibel = true;
		bTransformSone = true;
		uiModAmplLimit = 60;
		bIncludeDC = false;
		shNumBarkBands = 24;
		bFluctuationStrengthWeighting = true;
		bBlurring1 = false; // as of 2010-02-10: discussed with TL/EP
		bBlurring2 = false;

		//		fv_output_files_prefix = "";

		sFileDest = "";

		// almost no restriction
		min_loglevel = 1;

		top_k = 500;

		sCodebookfile = "";

		dMaxBubbleVincinityFactor = 0.5;

		limittrackscount = 0;
		limittrialdays = 0;
		limitharddate = bg::from_simple_string("2081-03-16");

		strDbhost = "localhost";
		strDbport = "5432";
		strDbname = "smafestore";
		strDbuser = "smurf";
		strDbpwd = "papa";
	}

	~Smafeopt() {}




	/** Sets derived class members (uiFeatFrom, uiFeatTo, rh_len)
	 * <p><b>Preconditions</b>: bIncludeDC, uiModAmplLimit must be set!</p> */
	void setDependentOpts(){
		// Determine which part of features to take
		// MATLAB-REFIMP:1014-1016
		// uiFeatFrom_rh is the offset used at feat->rp array
		// for determining which features we use for rh.if (opt->bIncludeDC) {
		if (bIncludeDC) {
			uiFeatFrom = 0;
			uiFeatTo = uiModAmplLimit-1;
			//uiFeatFrom_rh = 1; // feat->rp includes DC, never use DC for rh features
			rh_len = uiFeatTo-uiFeatFrom;
		} else {
			uiFeatFrom = 1;
			uiFeatTo = uiModAmplLimit;
			//uiFeatFrom_rh = 0; // feat->rp does NOT include DC, so start from 0
			rh_len = uiFeatTo-uiFeatFrom+1;
		}
	}

	/** Sets class members that depend on audio specs of each song
	 * <p>This function is necessary because for database storage
	 * we have to know these details before feature extraction
	 * actually takes place</p> */
	void setSongDependentOpts(int iSamplerate) {
		// segment_size should always be around 6 secs, fft_window_size should
		// always be around 23ms
		// MATLAB-REFIMP:848
		if (iSamplerate == 0) {
			// 0 is dummy value
			iFFTSize = 0;
			SMAFELOG_FUNC(SMAFELOG_DEBUG, "Setting iFFTSize to 0 (dummy value)");
		} else if (iSamplerate <= 11025) {
			//		iSegmentSize = 65536; // 2^16
			iFFTSize = 256;
			//shNumBarkBands = 19;
		} else if (iSamplerate <= 22050) {
			//		iSegmentSize = 131072; // 2^17;
			iFFTSize = 512;
			//shNumBarkBands = 22;
		} else { // assume 44100
			//		iSegmentSize = 262144; // 2^18
			iFFTSize = 1024;
			//shNumBarkBands = 24;
		}
	}



	/**  Loads db config file and parses db related options
	 * Postcondition: this object's db vars are set.
	 * @throws std::string exception in case of error  */
	void parseDbOpts(std::string dboptfilename) {
		char** argv_;
		int argc_;

		// -------------- load file
		// read options file
		std::ifstream myfile(dboptfilename.c_str());
		std::string line;
		std::stringstream ssLines(std::stringstream::in
				| std::stringstream::out);
		int c = 1;
		if (myfile.is_open()) {
			while (!myfile.eof()) {
				getline(myfile, line);
				ssLines << line << std::endl;
				c++;
			}
			myfile.close();
			argv_ = new char*[c];
			c = 1;
			argv_[0] = new char[1];
			strcpy(argv_[0], "");
			// move arguments from options file to argv array
			// unless it is an empty string or the line starts with an # (=comment)
			while (!ssLines.eof()) {
				getline(ssLines, line);
				trimWhitespace(line);
				if (line != "" && line[0] != '#') {
					argv_[c] = new char[line.length() + 1];
					strcpy(argv_[c], line.c_str());
					c++;
				}
			}
			// do as if the options of the file would be the "real" command line arguments
			argc_ = c;
		} else {
			throw std::string("Error parsing db options file: Options file not found: " + dboptfilename);
		}


		// ---- parse options

		struct arg_str *arg_sdbhost = arg_str0(NULL, "dbhost", "HOST",
				std::string("Database host, default is " + strDbhost +  " (for database storage)").c_str() );
		struct arg_str *arg_sdbport = arg_str0(NULL, "dbport", "PORT",
				std::string("Database port, default is " + strDbport +  " (for database storage)").c_str() );
		struct arg_str *arg_sdbname = arg_str0(NULL, "dbname", "DATABASE",
				std::string("Database name, default is " + strDbname +  " (for database storage)").c_str());
		struct arg_str *arg_sdbuser = arg_str0(NULL, "dbuser", "USER",
				std::string("Database user, default is " + strDbuser +  " (for database storage)").c_str());
		struct arg_str *arg_sdbpwd = arg_str0(NULL, "dbpwd", "PASSWORD",
				std::string("Database password, default is " + strDbpwd +  " (for database storage)").c_str());
		struct arg_end *end = arg_end(20);

		void* argtable1[] = {arg_sdbhost, arg_sdbport, arg_sdbname, arg_sdbuser, arg_sdbpwd, end };


		int nerrors;

		/* verify the argtable[] entries were allocated sucessfully */
		if (arg_nullcheck(argtable1) != 0) {
			throw std::string("Error parsing db options file: insufficient memory");
		}


		// Parse the command line as defined by argtable[]
		nerrors = arg_parse(argc_, argv_, argtable1);



		if (nerrors == 0) {
			if (arg_sdbhost->count > 0) {
				strDbhost = std::string(arg_sdbhost->sval[0]);
			}
			if (arg_sdbport->count > 0) {
				strDbport = std::string(arg_sdbport->sval[0]);
			}
			if (arg_sdbname->count > 0) {
				strDbname = std::string(arg_sdbname->sval[0]);
			}
			if (arg_sdbuser->count > 0) {
				strDbuser = std::string(arg_sdbuser->sval[0]);
			}
			if (arg_sdbpwd->count > 0) {
				strDbpwd = std::string(arg_sdbpwd->sval[0]);
			}

		} else {
			arg_print_errors(stdout, end, "<progname>");
			throw std::string("Error parsing db options file. See above");
		}

		SMAFELOG_FUNC(SMAFELOG_INFO, "dbhost=" + stringify(strDbhost));
		SMAFELOG_FUNC(SMAFELOG_INFO, "dbport=" + stringify(strDbport));
		SMAFELOG_FUNC(SMAFELOG_INFO, "dbname=" + stringify(strDbname));
		SMAFELOG_FUNC(SMAFELOG_INFO, "dbuser=" + stringify(strDbuser));
		SMAFELOG_FUNC(SMAFELOG_INFO, "dbpwd=**************"); // ;-)


		arg_freetable(argtable1, sizeof(argtable1) / sizeof(argtable1[0]));

		for (int i = 0; i < argc_; i++) {
			delete[] argv_[i];
		}
		delete[] argv_;
	}


	/** Stores configuration in  database
	 * <b>Precondition: db name and details must be set</b> */
	void generateSQLCommandsForConfigStoreInDb(SMAFE_STORE_DB_CLASS* db, std::ofstream &outfile) {
		//SMAFELOG_FUNC(SMAFELOG_DEBUG, "Saving configuration in database " + strDbname + " on " + strDbhost + " with user " + strDbuser);

		// store the passphrase with special pp if we use encr
		if (strlen(verysecretpassphrase) > 0) {
			outfile << db->buildConfigRecord("the passphrase!", std::string(verysecretpassphrase), stdpassphrase);
		} else {
			outfile << db->buildConfigRecord("dummy", "dummy");
		}

		outfile << db->buildConfigRecord("uiSkipin", stringify(uiSkipin));
		outfile << db->buildConfigRecord("uiSkipout", stringify(uiSkipout));
		outfile << db->buildConfigRecord("uiStepwidth", stringify(uiStepwidth));
		outfile << db->buildConfigRecord("bReturnSegmentFeatures", stringify(bReturnSegmentFeatures));
		outfile << db->buildConfigRecord("bNormalizeFFTEnergy", stringify(bNormalizeFFTEnergy));;
		outfile << db->buildConfigRecord("bTransformDecibel", stringify(bTransformDecibel));;
		outfile << db->buildConfigRecord("bTransformSone", stringify(bTransformSone));
		outfile << db->buildConfigRecord("uiModAmplLimit", stringify(uiModAmplLimit));
		outfile << db->buildConfigRecord("bIncludeDC", stringify(bIncludeDC));
		outfile << db->buildConfigRecord("shNumBarkBands", stringify(shNumBarkBands));
		outfile << db->buildConfigRecord("bFluctuationStrengthWeighting", stringify(bFluctuationStrengthWeighting));
		outfile << db->buildConfigRecord("bBlurring1", stringify(bBlurring1));
		outfile << db->buildConfigRecord("bBlurring2", stringify(bBlurring2));
		//		outfile << db->buildConfigRecord("fv_output_files_prefix", fv_output_files_prefix);
		outfile << db->buildConfigRecord("sFileDest", sFileDest);
		outfile << db->buildConfigRecord("min_loglevel", stringify(min_loglevel));
		{
			std::stringstream ss(std::stringstream::in | std::stringstream::out);
			{
				boost::archive::text_oarchive oa(ss);
				oa << BOOST_SERIALIZATION_NVP(distancetype_ids);
			}
			outfile << db->buildConfigRecord("distancetype_ids", ss.str().c_str());
		}
		outfile << db->buildConfigRecord("top_k", stringify(top_k));
		outfile << db->buildConfigRecord("sCodebookfile", sCodebookfile);
		outfile << db->buildConfigRecord("dMaxBubbleVincinityFactor", stringify(dMaxBubbleVincinityFactor));
		{
			std::stringstream ss(std::stringstream::in | std::stringstream::out);
			{
				boost::archive::text_oarchive oa(ss);
				oa << BOOST_SERIALIZATION_NVP(mapExtractGenerally);
			}
			outfile << db->buildConfigRecord("mapExtractGenerally", ss.str().c_str());
		}
		outfile << db->buildConfigRecord("limittrackscount", stringify(limittrackscount));
		outfile << db->buildConfigRecord("limittrialdays", stringify(limittrialdays));
		outfile << db->buildConfigRecord("limitharddate", to_iso_extended_string(limitharddate));


		SMAFELOG_FUNC(SMAFELOG_DEBUG, "... configuration successfully saved.");

	}

	/** Loads configuration from database
	 * <b>Precondition: db name and details must be set</b> */
	void loadConfigFromDb() {
		tKeyValueMap mConfigs;
		SMAFE_STORE_DB_CLASS* db = NULL;
		std::string key_cmp;

		SMAFELOG_FUNC(SMAFELOG_DEBUG, "Loading configuration from database " + strDbname + " on " + strDbhost + " with user " + strDbuser);

		// Open db connection and store config in db
		db = new SMAFE_STORE_DB_CLASS();
		db->openConnection(strDbhost, strDbport,  strDbuser, strDbpwd, strDbname);

		// load all configs from database, still decrypted
		mConfigs = db->getConfigRecords();

		bool bFoundPP = false;
		// iterate through all and find the one that can be decrypted with the given hardcoded passphrase (this is the passphrase for all other options)
		for (tKeyValueMap::iterator iter = mConfigs.begin(); iter != mConfigs.end(); iter++) {
			if (!bFoundPP) {
				try {
					SMAFELOG_FUNC(SMAFELOG_DEBUG, "Trying: " + iter->second);
					std::string pptry = decryptString(iter->second.c_str(), stdpassphrase);
					// if we are still here, no exeception took place so we have our passphrase
					bFoundPP = true;
					if (pptry.size() <= 63) {
						strcpy(verysecretpassphrase, pptry.c_str());
						SMAFELOG_FUNC(SMAFELOG_DEBUG, "Individual passphrase found in db.");
						SMAFELOG_FUNC(SMAFELOG_DEBUG3, "here it is (psst!) : " + pptry);
					} else {
						SMAFELOG_FUNC(SMAFELOG_FATAL, "Individual passphrase found in db but is more than 63 characters.");
						exit(2);
					}
					mConfigs.erase(iter++);
				} catch (CryptoPP::Exception& e) {
					// this entry is not the passphrase
					//SMAFELOG_FUNC(SMAFELOG_DEBUG, "Error converting config value. :" + iter->second + " - " + e.GetWhat());
				}
			} // !bFoundPP
		}
		// check if pp is found
		if (!bFoundPP) {
			SMAFELOG_FUNC(SMAFELOG_DEBUG, "Individual passphrase NOT found in db.");
			SMAFELOG_FUNC(SMAFELOG_INFO, "Assuming that this database is not encrypted.");
		}

		// iterate and assign found values
		for (tKeyValueMap::iterator iter = mConfigs.begin(); iter != mConfigs.end(); /* iter++ --- that is done if we have found match and the element is deleted */ ) {
			try {
				bool processed = false;

				std::string key = decryptString(iter->first.c_str());
				std::string key_encrypted = iter->first.c_str();
				std::string val = decryptString(iter->second.c_str());

				key_cmp = "uiSkipin";
				if (key_cmp == key) {
					// we have processed something in this iteration
					processed = true;
					// remove from map
					mConfigs.erase(iter++);
					uiSkipin = boost::lexical_cast<unsigned int>(val);
				}
				key_cmp = "uiSkipout";
				if (key_cmp == key) {
					processed = true;
					mConfigs.erase(iter++);
					uiSkipout = boost::lexical_cast<unsigned int>(val);
				}
				key_cmp = "uiStepwidth";
				if (key_cmp == key) {
					processed = true;
					mConfigs.erase(iter++);
					uiStepwidth = boost::lexical_cast<unsigned int>(val);
				}
				key_cmp = "bReturnSegmentFeatures";
				if (key_cmp == key) {
					processed = true;
					mConfigs.erase(iter++);
					bReturnSegmentFeatures = boost::lexical_cast<bool>(val);
				}
				key_cmp = "bNormalizeFFTEnergy";
				if (key_cmp == key) {
					processed = true;
					mConfigs.erase(iter++);
					bNormalizeFFTEnergy = boost::lexical_cast<bool>(val);
				}
				key_cmp = "bTransformDecibel";
				if (key_cmp == key) {
					processed = true;
					mConfigs.erase(iter++);
					bTransformDecibel = boost::lexical_cast<bool>(val);
				}
				key_cmp = "bTransformSone";
				if (key_cmp == key) {
					processed = true;
					mConfigs.erase(iter++);
					bTransformSone = boost::lexical_cast<bool>(val);
				}
				key_cmp = "uiModAmplLimit";
				if (key_cmp == key) {
					processed = true;
					mConfigs.erase(iter++);
					uiModAmplLimit = boost::lexical_cast<unsigned int>(val);
				}
				key_cmp = "bIncludeDC";
				if (key_cmp == key) {
					processed = true;
					mConfigs.erase(iter++);
					bIncludeDC = boost::lexical_cast<bool>(val);
				}
				key_cmp = "shNumBarkBands";
				if (key_cmp == key) {
					processed = true;
					mConfigs.erase(iter++);
					shNumBarkBands = boost::lexical_cast<short>(val);
				}
				key_cmp = "bFluctuationStrengthWeighting";
				if (key_cmp == key) {
					processed = true;
					mConfigs.erase(iter++);
					bFluctuationStrengthWeighting = boost::lexical_cast<bool>(val);
				}
				key_cmp = "bBlurring1";
				if (key_cmp == key) {
					processed = true;
					mConfigs.erase(iter++);
					bBlurring1 = boost::lexical_cast<bool>(val);
				}
				key_cmp = "bBlurring2";
				if (key_cmp == key) {
					processed = true;
					mConfigs.erase(iter++);
					bBlurring2 = boost::lexical_cast<bool>(val);
				}
				key_cmp = "fv_output_files_prefix";
				if (key_cmp == key) {
					processed = true;
					mConfigs.erase(iter++);
					//					fv_output_files_prefix = std::string(val);
					// issue warning because not valid anymore
					SMAFELOG_FUNC(SMAFELOG_WARNING, "Ignoring " + key_cmp + ". Is admin option now.");
				}
				key_cmp = "sFileDest";
				if (key_cmp == key) {
					processed = true;
					mConfigs.erase(iter++);
					sFileDest = std::string(val);
				}
				key_cmp = "min_loglevel";
				if (key_cmp == key) {
					processed = true;
					mConfigs.erase(iter++);
					min_loglevel = boost::lexical_cast<int>(val);
				}
				key_cmp = "distancetype_ids";
				if (key_cmp == key) {
					processed = true;
					mConfigs.erase(iter++);
					{
						std::stringstream ss(std::stringstream::in
								| std::stringstream::out);
						ss << boost_s11n_workaround_135(val) << std::endl;
						boost::archive::text_iarchive ia(ss);
						ia >> BOOST_SERIALIZATION_NVP(distancetype_ids);
					}
				}
				key_cmp = "top_k";
				if (key_cmp == key) {
					processed = true;
					mConfigs.erase(iter++);
					top_k = boost::lexical_cast<size_t>(val);
				}
				key_cmp = "sCodebookfile";
				if (key_cmp == key) {
					processed = true;
					mConfigs.erase(iter++);
					sCodebookfile = std::string(val);
				}
				key_cmp = "dMaxBubbleVincinityFactor";
				if (key_cmp == key) {
					processed = true;
					mConfigs.erase(iter++);
					dMaxBubbleVincinityFactor = boost::lexical_cast<double>(val);
				}
				key_cmp = "mapExtractGenerally";
				if (key_cmp == key) {
					processed = true;
					mConfigs.erase(iter++);
					{
						std::stringstream ss(std::stringstream::in
								| std::stringstream::out);
						ss << boost_s11n_workaround_135(val) << std::endl;
						boost::archive::text_iarchive ia(ss);
						ia >> BOOST_SERIALIZATION_NVP(mapExtractGenerally);
					}
				}
				key_cmp = "limittrackscount";
				if (key_cmp == key) {
					processed = true;
					mConfigs.erase(iter++);
					limittrackscount = boost::lexical_cast<size_t>(val);
				}
				key_cmp = "limittrialdays";
				if (key_cmp == key) {
					processed = true;
					mConfigs.erase(iter++);
					limittrialdays = boost::lexical_cast<size_t>(val);
				}
				key_cmp = "limitharddate";
				if (key_cmp == key) {
					processed = true;
					mConfigs.erase(iter++);
					limitharddate = bg::from_simple_string(val);
				}
				key_cmp = "limitsoftdate";
				if (key_cmp == key) {
					processed = true;
					mConfigs.erase(iter++);
					limitsoftdate = bg::from_simple_string(val);
				}
				key_cmp = "lastuseddate";
				if (key_cmp == key) {
					processed = true;
					mConfigs.erase(iter++);
					lastuseddate = bg::from_simple_string(val);
					sLastuseddateKey = key_encrypted; // store for later, for updates
				}

				key_cmp = "dummy";
				if (key_cmp == key) {
					processed = true;
					mConfigs.erase(iter++);
					// dummy key, this is inserted if database is not encrypted and thus the passphrase is missing.
					// with that dummy key, the number of config records are the same in both cases (encrypted vs plain text database)
				}



				// not processed until now? -> unkown
				if (!processed) {
					SMAFELOG_FUNC(SMAFELOG_WARNING, "Unkown option key: " + key + ". Ignoring.");
					// remove from map
					mConfigs.erase(iter++);
				}
			} catch(boost::archive::archive_exception& e) {
				SMAFELOG_FUNC(SMAFELOG_WARNING, "Error converting config value. Using default value. key:" + key_cmp + " - " + e.what());
			} catch(boost::bad_lexical_cast& e) {
				SMAFELOG_FUNC(SMAFELOG_WARNING, "Error converting config value. Using default value. key:" + key_cmp);
			}catch (CryptoPP::Exception& e) {
				SMAFELOG_FUNC(SMAFELOG_WARNING, "Error converting config value. Using default value. key:" + iter->first + " - " + e.GetWhat());
				// if we get this exception, it has occured already before an iteration++, so do it here.
				iter++;
			}

		}



		SMAFELOG_FUNC(SMAFELOG_DEBUG, "... configuration successfully loaded.");

		delete db;
	}

	/** Creates a string that contains key value pairs for all options (used for log file)
	 * @return String in human readable format with all options
	 */
	std::string toString() {
		std::string s = "uiSkipin="+stringify(uiSkipin)+
				", uiSkipout="+stringify(uiSkipout)+
				", uiStepwidth="+stringify(uiStepwidth)+
				", bSpectralmasking=NA, bNormalizeFFTEnergy="+stringify(bNormalizeFFTEnergy)+
				", bTransformDecibel="+stringify(bTransformDecibel)+
				", bTransformPhon=NA, bTransformSone="+stringify(bTransformSone)+
				", uiModAmplLimit="+stringify(uiModAmplLimit)+
				", bIncludeDC="+stringify(bIncludeDC)+
				", shNumBarkBands="+stringify(shNumBarkBands)+
				",  bFluctuationStrengthWeighting="+stringify(bFluctuationStrengthWeighting)+
				", bBlurring1="+stringify(bBlurring1)+
				", bBlurring2="+stringify(bBlurring2)+
				", bNormalizeFFTEnergy="+stringify(bNormalizeFFTEnergy)+
				", sFileDest="+stringify(sFileDest)+
				", min_loglevel="+stringify(min_loglevel)+
				", top_k="+stringify(top_k)+
				", sCodebookfile="+stringify(sCodebookfile)+
				", dMaxBubbleVincinityFactor="+stringify(dMaxBubbleVincinityFactor)+
				", limittrackscount="+stringify(limittrackscount)+
				", limittrialdays="+stringify(limittrialdays)+
				", limitharddate="+bg::to_iso_extended_string(limitharddate)
		;

		std::stringstream ss(std::stringstream::in | std::stringstream::out);
		ss << ", distancetype_ids";
		std::copy(distancetype_ids.begin(), distancetype_ids.end(), std::ostream_iterator<long>(ss, ", "));

		ss << ", mapExtractGenerally";
		for(string_bool_map::const_iterator iter = mapExtractGenerally.begin(); iter != mapExtractGenerally.end(); iter++) {
			ss << iter->first << ":" << iter->second << ", ";
		} // end of iterator

		return std::string(s + ss.str());
	}


private:

	friend class boost::serialization::access;


	/** <p>version 0: basis version
	 * <p>version 1: includes also bNormalizeFFTEnergy
	 * <p>version 2: includes also limittrackscount
	 * <p>version 3: includes also limithardlimit & limitdaystrail
	 * */
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(uiSkipin);
		ar & BOOST_SERIALIZATION_NVP(uiSkipout);
		ar & BOOST_SERIALIZATION_NVP(uiStepwidth);
		ar & BOOST_SERIALIZATION_NVP(bReturnSegmentFeatures);
		if (version >= 1)
			ar & BOOST_SERIALIZATION_NVP(bNormalizeFFTEnergy);
		ar & BOOST_SERIALIZATION_NVP(bTransformDecibel);
		ar & BOOST_SERIALIZATION_NVP(bTransformSone);
		ar & BOOST_SERIALIZATION_NVP(uiModAmplLimit);
		ar & BOOST_SERIALIZATION_NVP(bIncludeDC);
		ar & BOOST_SERIALIZATION_NVP(shNumBarkBands);
		ar & BOOST_SERIALIZATION_NVP(bFluctuationStrengthWeighting);
		ar & BOOST_SERIALIZATION_NVP(bBlurring1);
		ar & BOOST_SERIALIZATION_NVP(bBlurring2);
		ar & BOOST_SERIALIZATION_NVP(sFileDest);
		ar & BOOST_SERIALIZATION_NVP(min_loglevel);
		ar & BOOST_SERIALIZATION_NVP(distancetype_ids);
		ar & BOOST_SERIALIZATION_NVP(top_k);
		ar & BOOST_SERIALIZATION_NVP(sCodebookfile);
		ar & BOOST_SERIALIZATION_NVP(dMaxBubbleVincinityFactor);
		ar & BOOST_SERIALIZATION_NVP(mapExtractGenerally);
		if (version >= 2)
			ar & BOOST_SERIALIZATION_NVP(limittrackscount);
		if (version >= 3) {
			ar & BOOST_SERIALIZATION_NVP(limittrialdays);
			ar & BOOST_SERIALIZATION_NVP(limitharddate);
		}
	}





};


BOOST_CLASS_VERSION(Smafeopt, 3)



