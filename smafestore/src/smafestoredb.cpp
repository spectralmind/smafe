///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// smafestoredb.cpp
//
// Class for database communication
// ------------------------------------------------------------------------
//
// $Id$
//
//
//
//
//
///////////////////////////////////////////////////////////////////////////


#include "smafestoredb.h"

std::string SmafeStoreDB::aReservedCollectionNames[3] = { std::string("(THIS IS NOT USED)"), std::string("_r"),
		std::string("_d") };
std::string SmafeStoreDB::STATUSOK = "OK";
//std::string SmafeStoreDB::STATUSDIST = "DIST";
std::string SmafeStoreDB::STATUSFAILED = "FAILED";
//std::string SmafeStoreDB::STATUSFEDONE = "FE_DONE";

SmafeStoreDB::SmafeStoreDB(): iLastTimeJobQuery(0) {
}

SmafeStoreDB::~SmafeStoreDB() {
}

void SmafeStoreDB::openConnection() {
	SMAFELOG_FUNC(SMAFELOG_DEBUG, "Trying db connection with saved details");
	this->openConnection(strDbhost, strDbport, strDbuser, strDbpwd, strDbname);
}

void SmafeStoreDB::storeFeatureRecord(long track_id, long featurevectortype_id,
		const SmafeAbstractFeatureVector* fv, long file_id) {

	std::string ss_enc;

	std::stringstream ss(std::stringstream::in | std::stringstream::out);
	{
		boost::archive::text_oarchive oa(ss);
		//boost::archive::xml_oarchive oa(ss);
		oa << BOOST_SERIALIZATION_NVP(fv);
	}

	// encryption
	ss_enc = encryptString(ss.str().c_str());

	// call db specific method
	storeFeatureRecord(track_id, featurevectortype_id, ss_enc.c_str(),
			ss_enc.size(), file_id);
}

void SmafeStoreDB::storeFeatureSegRecord(const long segmentnr, const long track_id,
		long featurevectortype_id,
		const SmafeAbstractFeatureVector* fv,
		const long file_id,
		const long startsample,
		const long length) {

	std::string ss_enc;

	std::stringstream ss(std::stringstream::in | std::stringstream::out);
	{
		boost::archive::text_oarchive oa(ss);
		//boost::archive::xml_oarchive oa(ss);
		oa << BOOST_SERIALIZATION_NVP(fv);
	}

	// encryption
	ss_enc = encryptString(ss.str().c_str());

	// call db specific method
	storeFeatureSegRecord(segmentnr, track_id, featurevectortype_id,
			ss_enc.c_str(), ss_enc.size(), file_id, startsample, length);
}

// second take
int SmafeStoreDB::store(std::vector<SmafeAbstractFeatureVector_Ptr> fvs, std::vector<SmafeAbstractFeatureVector_Ptr> segmentfvs,
		const long file_id, const long track_id, const long addfilejob_id) {

	// throw exception if no fvs present
	if (fvs.empty())
		throw std::string("No feature vectors to store.");

	//-------
	// Store Feature Vectors
	//

	// iterate through vector of feature vectors
	for (std::vector<SmafeAbstractFeatureVector_Ptr>::iterator iter =
			fvs.begin(); iter < fvs.end(); iter++) {

		SmafeAbstractFeatureVector* theFv = iter->get();

		SMAFELOG_FUNC(SMAFELOG_DEBUG, std::string("Writing feature vector record to database: ") + theFv->fvtype->name);

		storeFeatureRecord(track_id, theFv->fvtype->id, theFv, file_id);

		//  also insert the distancejob record
		// this has been moved to smafewrapd

	} // end of iterator


	// store seg fvs if present
	if (!segmentfvs.empty()) {
		SMAFELOG_FUNC(SMAFELOG_DEBUG, "Storing segment feature vectors...");

		// iterate through vector of feature vectors
		for (std::vector<SmafeAbstractFeatureVector_Ptr>::iterator iter =
				segmentfvs.begin(); iter < segmentfvs.end(); iter++) {

			const SmafeAbstractFeatureVector* theFv = iter->get();

			SMAFELOG_FUNC(SMAFELOG_DEBUG, std::string("Writing feature vector record to database: ") + theFv->fvtype->name);

			storeFeatureSegRecord(theFv->lSegmentnr, track_id, theFv->fvtype->id, theFv, file_id, theFv->lStartsample, theFv->lLength);

		} // end of iterator

	}

	return 0;
}

bool SmafeStoreDB::ensureRPFeatVecTypeRecord(SmafeFVType* fvtype) {

	std::vector<SmafeFVType_Ptr> currentFvts;
	bool bIsInDb = false;
	long primkey;

	getFeaturevectortypes(currentFvts);

	for (std::vector<SmafeFVType_Ptr>::iterator iter = currentFvts.begin(); iter < currentFvts.end(); iter++) {
		if (( *(iter->get()) == *fvtype )) {
			// found, save prim key!
			primkey = iter->get()->id;
			bIsInDb = true;
		}
	}


	if (!bIsInDb) {
		// write this type into the table
		SMAFELOG_FUNC(SMAFELOG_DEBUG, std::string("Writing featurevectortype record to database: ") + fvtype->name);
		fvtype->id = storeFeatVecTypeRecord(fvtype->name.c_str(),
				fvtype->version, fvtype->dimension_x, fvtype->dimension_y,
				fvtype->parameters.c_str(), fvtype->class_id.c_str());
		SMAFELOG_FUNC(SMAFELOG_DEBUG, "Primary key returned is " + stringify(fvtype->id));
		return true;
	} else {
		SMAFELOG_FUNC(SMAFELOG_DEBUG, "This feature vector type is already in the database: " + fvtype->name + " as id " + stringify(primkey));
		fvtype->id = primkey;
		return false;
	}
}



std::string SmafeStoreDB::buildConfigRecord(std::string key, std::string value) {
	return buildConfigRecord(key, value, (const char*) verysecretpassphrase);
}

std::string SmafeStoreDB::buildConfigRecord(std::string key, std::string value, const char* pp){
	// version with escaping:
	//	return "INSERT INTO config (key, value) VALUES ('" + escapeString(encryptString(key.c_str(), pp)) + "', '" + escapeString(encryptString(value.c_str(), pp)) + "');" + '\n';
	// version without escaping:
	return "INSERT INTO config (key, value) VALUES ('" + (encryptString(key.c_str(), pp)) + "', '" + (encryptString(value.c_str(), pp)) + "');" + '\n';
}

SmafeAbstractFeatureVector* SmafeStoreDB::readFeatureVector(long fvt_id,
		long track_id, bool load_fvt_info, long file_id, long segmentnr, bool onErrorSilent, bool load_file_id) {
	SmafeAbstractFeatureVector *safv;

	char* s11nbuf = NULL;
	size_t buf_len;
	std::string class_id;
	long file_id_db; // the file_id from the fv record in the db
	std::string s11nbuf_decrypted;

	try {

		// get feature vector as serialized  (raw)
		readFeatureRecord_raw(track_id, segmentnr, fvt_id, s11nbuf, buf_len, class_id,
				file_id_db, load_file_id);

		// check for NULL
		if (s11nbuf != NULL) {

			// decrypt if necessary
			try {
				s11nbuf_decrypted = decryptString(s11nbuf);
			} catch (CryptoPP::Exception& e) {
				if (onErrorSilent) {
					return NULL;
				} else {
					throw "Error decrypting feature vector: " + e.GetWhat();
				}
			}

			/*
			 // check the save operation
			 std::stringstream ssq (std::stringstream::in | std::stringstream::out);
			 {
			 const SmafeNumericFeatureVector rp(10);
			 // xml
			 //boost::archive::xml_oarchive xoa(ssq);
			 //xoa << BOOST_SERIALIZATION_NVP(rp);
			 //text
			 boost::archive::text_oarchive toa(ssq);
			 toa << BOOST_SERIALIZATION_NVP(rp);
			 }
			 */

			{

				// test
				//			strcpy(s11nbuf, "22 serialization::archive 4 0 25 SmafeNumericFeatureVector 1 0   0 0 0 0");
				//			strcpy(s11nbuf, "22 serialization::archive 4 0 25 SmafeNumericFeatureVector 1 0   0 0 60 15.750428559896203 15.835513677740561 14.431150283495295 9.1691797843426137 12.811824920752358 13.586624597153685 11.002659647379955 11.064217292864679 7.9797139825054986 9.9235703018379375 9.2886428424895762 8.0324006306601525 15.276850905877762 6.9980024541539478 6.5534074804978939 8.2741463327244738 5.5196410597310415 7.0535993338265381 12.179729021310999 7.614244901519533 8.2170004314708081 7.2385293826428949 7.670518295296743 7.4098476138708218 9.1962051599756762 22.083150886531712 7.1937839234320631 5.3740144890307757 9.4028271947731152 4.9368000302025195 5.54337365004546 12.558513608835014 5.0464122111050171 5.2942967940821957 5.1293983657093012 4.0150540410477955 4.1744641937907305 4.4020548029323763 6.5890940735936869 4.9097187529569455 3.4772659337619394 5.18100803316465 3.9710662965692674 3.4761336596970747 6.1217972638432858 2.9740695367326637 3.4272158068985425 5.1425586241272487 4.0814512317989635 5.4559700678194245 10.940316799899946 10.544862969781059 4.6729446737166676 3.1199454721889417 4.0808752427976582 3.2306116954569353 2.6467356438540484 4.4918005045229616 2.4698196267397305 2.414333588255281");
				//strcpy(s11nbuf, "22 serialization::archive 4 0 25 SmafeNumericFeatureVector 1 0\0120 0 0 60 15.750428559896203 15.835513677740561 14.431150283495295 9.1691797843426137 12.811824920752358 13.586624597153685 11.002659647379955 11.064217292864679 7.9797139825054986 9.9235703018379375 9.2886428424895762 8.0324006306601525 15.276850905877762 6.9980024541539478 6.5534074804978939 8.2741463327244738 5.5196410597310415 7.0535993338265381 12.179729021310999 7.614244901519533 8.2170004314708081 7.2385293826428949 7.670518295296743 7.4098476138708218 9.1962051599756762 22.083150886531712 7.1937839234320631 5.3740144890307757 9.4028271947731152 4.9368000302025195 5.54337365004546 12.558513608835014 5.0464122111050171 5.2942967940821957 5.1293983657093012 4.0150540410477955 4.1744641937907305 4.4020548029323763 6.5890940735936869 4.9097187529569455 3.4772659337619394 5.18100803316465 3.9710662965692674 3.4761336596970747 6.1217972638432858 2.9740695367326637 3.4272158068985425 5.1425586241272487 4.0814512317989635 5.4559700678194245 10.940316799899946 10.544862969781059 4.6729446737166676 3.1199454721889417 4.0808752427976582 3.2306116954569353 2.6467356438540484 4.4918005045229616 2.4698196267397305 2.414333588255281\012");


				std::stringstream ss(std::stringstream::in
						| std::stringstream::out);
				//				ss << boost_s11n_workaround_135(s11nbuf_decrypted) << std::endl;
				ss << s11nbuf_decrypted << std::endl;

				SMAFELOG_FUNC(SMAFELOG_DEBUG3, std::string(s11nbuf));

				//ss << stringvalues << endl;

				boost::archive::text_iarchive ia(ss);

				// restore  from the archive
				ia >> BOOST_SERIALIZATION_NVP(safv);
			}

			// get info about featurevectortype if desired
			if (load_fvt_info)
				getFeatureVectorMetaInfo(*safv, fvt_id, track_id, file_id);

			// store foreign keys;
			safv->track_id = track_id;
			// if a specific file_id is given, store this one
			// otherweise store the one that comes from the db
			if (file_id < 0)
				safv->file_id = file_id_db;
			else
				safv->file_id = file_id;

		} else { //if (s11nbuf != NULL)
			if (onErrorSilent) {
				return NULL;
			} else {
				throw std::string("Feature vector not found for track_id = "
						+ stringify(track_id) + ", featurevectortype_id = "
						+ stringify(fvt_id));
			}
		}//if (s11nbuf != NULL)

	} // try block
	catch (std::string& s) {
		if (onErrorSilent) {
			if (s11nbuf != NULL) delete[] s11nbuf;
			return NULL;
		} else {
			throw std::string("Ignoring track. More info: ") + s;
		}
	} // catch block
	catch (boost::archive::archive_exception& e) {
		if (onErrorSilent) {
			if (s11nbuf != NULL) delete[] s11nbuf;
			return NULL;
		} else {
			throw std::string("Ignoring track. More info: ") + e.what();
		}
	}

	delete[] s11nbuf;

	return safv;
}

// just delegate to other method, no metadata loaded, error handling done by smafestore, file_id loaded
SmafeAbstractFeatureVector* SmafeStoreDB::readFeatureVector(long fvt_id,
		long track_id) {
	return readFeatureVector(fvt_id, track_id, false, -1, -1, false, true);
}


SmafeAbstractFeatureVector* SmafeStoreDB::readSegmentFeatureVector(long fvt_id, long track_id, long segmentnr) {
	return readFeatureVector(fvt_id, track_id, false, -1, segmentnr, false, true);
}


bool SmafeStoreDB::readAllFeatureVectors(long fvt_id, bool bSegments, bool onErrorSilent, tFVMap &fvbuffer, long lLimitsize) {
	tRawFVMap rawfvbuffer;
	bool bRet = true; // return value of raw funciton
	bool bOurReturnvalue = true; // return value for this function
	std::string class_id;

	//long lLimitsize;			// size of bunch that is fetched in one piece

	try {
		// Calculate limit size
		// fix for now: one SSD fv requires 3.4 kB, and we got an error with 519 MB, 20 % puffer
		//				lLimitsize = 1; // for testing
		//		lLimitsize = long(floor(519.0*1024.0/3.4 * 0.8));
		//lLimitsize = 0;

		// loop through Haeppchen, as long as new feature vectors are returned
		for (long lCurrentOffset = 0; bRet == true; lCurrentOffset += lLimitsize ) {
			// get feature vector as serialized  (raw)
			bRet = readAllFeatureRecords_raw(bSegments, fvt_id, lLimitsize, lCurrentOffset, rawfvbuffer);
			SMAFELOG_FUNC(SMAFELOG_DEBUG, "... data has been fetched from db. Starting decrypting and deserializing...");

			// at least 1 fv returned
			if (bRet) {

				// iterate through map
				tRawFVMap::const_iterator iter = rawfvbuffer.begin();
				//				for (tRawFVMap::iterator iter = rawfvbuffer.begin(); iter != rawfvbuffer.end(); iter++) {
//#pragma omp parallel for
				for (size_t i = 0; i < rawfvbuffer.size(); i++) {

					try {

					// private for each thread
					long track_id;
					long segmentnr;
					char* s11nbuf;
					std::string s11nbuf_decrypted;
					SmafeAbstractFeatureVector *safv;
					SmafeAbstractFeatureVector_Ptr fv_ptr;

					SMAFELOG_FUNC(SMAFELOG_DEBUG, "Parallel execution started with " + stringify(omp_get_num_threads()) + " threads");

					// access to iterator only by one thread at a time
					// I am not *sure* if that is necessary
//#pragma omp critical
					{
						// check for end
						if (iter != rawfvbuffer.end()) {
							track_id = iter->first.first;
							segmentnr = iter->first.second;
							s11nbuf = iter->second.get();
							// advance iterator
							iter++;
						} else {
							SMAFELOG_FUNC(SMAFELOG_WARNING, "We have advanced over the end of the iterator. Size of rawfvbuffer is " + stringify(rawfvbuffer.size()) + " tracknr=" + stringify(track_id) );
						}
					}

					// check for NULL
					if (s11nbuf != NULL) {
						SMAFELOG_FUNC(SMAFELOG_DEBUG2, "Featurevector found for track_id=" + stringify(track_id));

						// decrypt if necessary
						try {
							s11nbuf_decrypted = decryptString(s11nbuf);
						} catch (CryptoPP::Exception& e) {
							if (onErrorSilent) {
								bOurReturnvalue = false;
							} else {
								throw "Error decrypting feature vector: " + e.GetWhat() + " | Featurevector data: " + stringify(s11nbuf);
							}
						}


						{

							std::stringstream ss(std::stringstream::in
									| std::stringstream::out);
							//						ss << boost_s11n_workaround_135(s11nbuf_decrypted) << std::endl;
							ss << (s11nbuf_decrypted) << std::endl;

							SMAFELOG_FUNC(SMAFELOG_DEBUG3, std::string(s11nbuf));

							//ss << stringvalues << endl;

							boost::archive::text_iarchive ia(ss);

							// restore  from the archive
							ia >> BOOST_SERIALIZATION_NVP(safv);
						}

						// get info about featurevectortype if desired
						// not implemented yet
						//					if (load_fvt_info)
						//						getFeatureVectorMetaInfo(*safv, fvt_id, track_id, file_id);

						// store foreign keys;
						safv->track_id = track_id;
						/* file_id currently not retrieved
					// if a specific file_id is given, store this one
					// otherweise store the one that comes from the db
					if (file_id < 0)
						safv->file_id = file_id_db;
					else
						safv->file_id = file_id;
						 */

						fv_ptr.reset(safv);

						// updates of map only one thread at a time
						// This is not ideal, however. We should use a local map and than merge them
//#pragma omp critical
						fvbuffer[longlongpair(safv->track_id, segmentnr)] = fv_ptr;

						// is shared pointer, so no delete!
						//delete[] iter->second; // new is in ..readAllFeatureRecords_raw(..)

					} else { //if (s11nbuf != NULL)
						SMAFELOG_FUNC(SMAFELOG_WARNING, "Featurevector NOT found for track_id=" + stringify(track_id));
						if (onErrorSilent) {
							bOurReturnvalue = false;
						} else {
							throw std::string("Feature vector not found for track_id = "
									+ stringify(track_id) + ", featurevectortype_id = "
									+ stringify(fvt_id));
						}
					}//if (s11nbuf != NULL)

					} // try block
					catch (std::string& s) {
							if (onErrorSilent) {
								bOurReturnvalue = false;
							} else {
								SMAFELOG_FUNC(SMAFELOG_ERROR, "Ignoring track. More info: " + s);
							}
						} // catch block
						catch (boost::archive::archive_exception& e) {
							if (onErrorSilent) {
								bOurReturnvalue = false;
							} else {
								SMAFELOG_FUNC(SMAFELOG_ERROR, "Ignoring track. More info: " + stringify(e.what()));
							}
						}
				} // for (tFVMap::iterator iter = rawfvbuffer.begin()
				// End of parallel execution!

				// if we did not get a bunch of size |limit| we know that there are no more records, so stop the loop
				// Also, if limitsize is 0, ie, all fvs are fetched at once
				if (rawfvbuffer.size() < lLimitsize || lLimitsize == 0)
					bRet = false;
				else {
					// empty the vector
					rawfvbuffer.clear();
					closeConnection();
					openConnection();
				}

			} //if (bRet)
		} // for loop

	} // try block
	catch (std::string& s) {
		if (onErrorSilent) {
			bOurReturnvalue = false;
		} else {
			throw std::string("Ignoring track. More info: ") + s;
		}
	} // catch block
	catch (boost::archive::archive_exception& e) {
		if (onErrorSilent) {
			bOurReturnvalue = false;
		} else {
			throw std::string("Ignoring track. More info: ") + e.what();
		}
	}
	return bOurReturnvalue;
}




long SmafeStoreDB::removeFromRemovedCollectionIfNecessary(long file_id) {
	char sqlcmd[MAXSQLLEN];

	sprintf(
			sqlcmd,
			"delete from collection_file cf1 where collection_id = %li and file_id = %li \
			and exists (select collection_id, file_id from collection_file cf2 where cf1.file_id=cf2.file_id and collection_id != %li);",
			SmafeStoreDB::RESERVEDCOLLECTIONS_REMOVED, file_id, SmafeStoreDB::RESERVEDCOLLECTIONS_REMOVED);

	return executeStatement(sqlcmd);
}

t_double_deque SmafeStoreDB::query_nn(long track_id, long fvt_id, long dist_id,
		std::string sCollectionName, int skip_k, int k) {
	std::vector<Nn_result_rs_Ptr> nns;
	t_double_deque ds;

	query_nn(nns, track_id, fvt_id, dist_id, sCollectionName, skip_k, k);

	for (std::vector<Nn_result_rs_Ptr>::iterator iter = nns.begin(); iter
	< nns.end(); iter++) {
		ds.push_back(iter->get()->dist);
	}

	return ds;
}

void SmafeStoreDB::query_nn(std::vector<Nn_result_rs_Ptr> &nns, long track_id,
		long fvt_id, long dist_id, std::string sCollectionName, int k) {
	query_nn(nns, track_id, fvt_id, dist_id, sCollectionName, 0, k);
}



void SmafeStoreDB::getCollectionReferences(const long currentJob_fvt_id, const bool bUseSegmFvs,  const tFVMap &fvbuffer, tlonglonglongsafvStructVectorMap &lllsafvMap) {
	longlong_set_deque tracks_collections;

	getTracksAndCollections(currentJob_fvt_id, bUseSegmFvs, tracks_collections);

	//	for (longlong_set_deque::iterator it_collection = tracks_collections.begin(); it_collection != tracks_collections.end(); it_collection++) {


	for (long c_id = 0; c_id < tracks_collections.size(); c_id++) {
		if (tracks_collections[c_id].size() > 0) {
			// set expected size for vector, will equal set
			lllsafvMap[c_id].reserve(tracks_collections[c_id].size());
			SMAFELOG_FUNC(SMAFELOG_DEBUG, "collection id " + stringify(c_id) + ", size=" + stringify(tracks_collections[c_id].size()));
			for (std::set<longlongpair>::const_iterator iter = (tracks_collections[c_id]).begin(); iter !=  (tracks_collections[c_id]).end(); iter++) {
				long track_id = iter->first;
				long segmentnr = iter->second;

				tFVMap::const_iterator iter_fv;
				iter_fv = fvbuffer.find(longlongpair(track_id, segmentnr));
				if (iter_fv != fvbuffer.end()) {

					tlonglongsafvStruct newstruct;
					// set values
					newstruct.track_id = track_id;
					newstruct.segmentnr = segmentnr;
					newstruct.fv = iter_fv->second;

					lllsafvMap[c_id].push_back(newstruct);
					SMAFELOG_FUNC(SMAFELOG_DEBUG, "collection id " + stringify(c_id) + ", pushed back one element");
				} else {
					// not found in fvbuffer
					SMAFELOG_FUNC(SMAFELOG_WARNING, "Fv for (" + stringify(track_id) + ", " + stringify(segmentnr) + ") was not found in fvbuffer!");
				}

			} // for loop track_id, segmentnr pairs
		} else {
			// no track for this collection
			SMAFELOG_FUNC(SMAFELOG_DEBUG, "No track found for collection id " + stringify(c_id));
		}
	} // for loop collections



}


// ---------------------------- writing --------------------------------

void SmafeStoreDB::insertDistanceRecord(long track_a_id, long track_b_id,
		long fvt_id, long dist_id, double d) {
	//checkOpenConnection();


	char sqlcmd[MAXSQLLEN];

	sprintf(
			sqlcmd,
			"INSERT INTO distance \
			(track_a_id, track_b_id, featurevectortype_id, distancetype_id, value) \
			VALUES (%li, %li, %li, %li, %f)",
			track_a_id, track_b_id, fvt_id, dist_id, d);

	executeStatement(sqlcmd);
}


std::string SmafeStoreDB::clearConfigRecords() {
	return "TRUNCATE TABLE config;\n";
}

