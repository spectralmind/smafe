///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// smafestoredb_pq.h
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

#pragma once

#include "smafestoredb.h"
#include "libpq-fe.h"


// for ntohl/htonl
#include <netinet/in.h>





/** Class that encapsulates database communication */
class DLLEXPORT SmafeStoreDB_PQ: public SmafeStoreDB
{
public:
	SmafeStoreDB_PQ(void);
	~SmafeStoreDB_PQ(void);

	PGresult* myPQexec(PGconn *conn, const char *query);
	virtual void openConnection(std::string dbhost, std::string dbport, std::string dbuser, std::string dbpwd, std::string dbdb);
	virtual void closeConnection();

	virtual void startTransaction();
	virtual void finishTransaction(bool bCommit);
	virtual long executeStatement(const char* sqlcmd);
	virtual bool executeStatement_serializationcheck(const char* sqlcmd);
	virtual void escapeString(const char* from, char* &to);
	virtual std::string escapeString(std::string from);
	virtual bool distanceTableIndexExists();
	virtual void createDistanceTableIndex();
	virtual void dropDistanceTableIndex();

	virtual bool tableExists(std::string tablename);
	virtual bool tableIsEmpty(std::string tablename);
	virtual long isFileInDatabase(t_filehash hash);
	virtual long isFileInDatabase(t_filehash hash, std::string filename, std::string external_key, std::string guid);
	virtual std::vector< long > isFileInDatabase(std::string filename);
	virtual long isCollectionInDatabase(std::string coll_name);
	virtual bool isDistanceRecordInDatabase(long track_a_id, long track_b_id, long fvt_id, long distt_id);
	virtual long isTrackInDatabase(t_fingerprint fp);
	virtual bool isTrackInCollection(long track_id, long collection_id);
	// obsolete (does not handle decryption!)
	//virtual long isFeatVecTypeInDatabase(const char* name, int version, int dim_x, int dim_y, const char* params);
	virtual bool isFeatVecInDatabase(long track_id, long featurevectortype_id);
	virtual long getTrackIDForFile(long file_id);
	virtual long getCollectionId(std::string name);
	virtual size_t getTrackCount();
	virtual size_t getFileCount();
	virtual void getBubbleInfo(long l, long track_id, long &lBubbleId, long &lCount);
	virtual bool getOpenTaskInfo(std::vector<long> *distancetype_ids, std::vector<long> *fvtype_ids, long &lNumVacancies, long &lNumCurrentVac, long &currentJob_track_id,  long &currentJob_fvt_id, long &currentJob_dist_id);
	virtual bool getOpenTaskInfo_addfile(tSmafejob_addfileRecord &rec, long &lNumVacancies, long &lNumCurrentVac);
	virtual bool getOpenTaskInfo_deletefile(tSmafejob_deletefileRecord &rec, long &lNumVacancies, long &lNumCurrentVac);
	virtual bool getOpenTaskInfo_deletecollection(tSmafejob_deletecollectionRecord &rec, long &lNumVacancies, long &lNumCurrentVac);
	virtual bool getOpenTaskInfo_smuiaddtrack(tSmuijob_addtrackRecord &rec, long &lNumVacancies, long &lNumCurrentVac);
	virtual bool getOpenTaskInfo_smuideletetrack(tSmuijob_deletetrackRecord &rec, long &lNumVacancies, long &lNumCurrentVac);
	virtual void readFeatureRecord_raw(long track_id,
		long segmentnr,
		long featurevectortype_id,
		char* &buffer,
		size_t &buf_length, std::string &class_id, long &file_id, bool load_file_id);
	virtual bool readAllFeatureRecords_raw(bool bSegments, long featurevectortype_id, long lLimitsize, long lOffset,  tRawFVMap &vBuffer);
	virtual void getFeatureVectorMetaInfo(SmafeAbstractFeatureVector &fv, long fvt_id, long track_id, long file_id);

	virtual void getDbinfo(tDbinfoRecord &rec);

	virtual long getNearestBubble(long x, long y, long layer, long distparam);

	virtual std::vector<longlongpair> getLowerTracksForDistanceCalc(long currentJob_track_id, long currentJob_fvt_id, long currentJob_dist_id);
	virtual std::vector<longlongpair> getOtherTracksForDistanceCalc(long currentJob_track_id, long currentJob_fvt_id, long currentJob_dist_id);
	virtual std::vector<longlongpair> getSegmentTracksForDistanceCalc(long currentJob_track_id, long currentJob_fvt_id, long currentJob_dist_id);
	virtual std::vector<long> getTrack_a_ids(long fvt_id, long dist_id);
	virtual std::vector<long> getTrack_ids(std::string collection_name);
	virtual void getTracksAndCollections(const long featurevectortype_id, const bool bSegments, longlong_set_deque &tracks_collections);
	virtual std::vector<long> getDistance_ids();
	virtual std::vector<long> getFeaturevectortype_ids();
	virtual tKeyValueMap getConfigRecords();
	virtual std::string storeConfigRecord(std::string key, std::string value);
	virtual void storeConfigRecord(std::string key, std::string value, std::string key_encrypted);
	virtual std::vector<long> getFilesForTrack(long track_id, bool bAlsoRemoved);
	virtual void getDistancetypes(std::vector< tDistancetype > &vDisttypes);
	virtual void getFeaturevectortypes(std::vector< SmafeFVType_Ptr > &v);

//	virtual void storeConfigRecord(std::string key, std::string value);
//	virtual void storeConfigRecord(std::string key, std::string value, const char* pp);
	virtual long storeFileRecord(long track_id, t_filehash hash, tAudioformat* ad, std::string guid, std::string external_key);
	virtual long storeTrackRecord(t_fingerprint fingerprint);
	virtual long storeCollectionRecord(std::string coll_name);
	virtual void ensureCollectionFileInDatabase(long collection_id, long file_id);
	virtual void ensureCollectionFileInDatabase_reservedname(int which_coll, bool bOnlyIfNoOther, long file_id);
	virtual long storeFeatVecTypeRecord(const char* name, int version, int dimx, int dimy, const char* params, const char* class_id);
	virtual void storeFeatureRecord(long track_id,
		long featurevectortype_id,
		const char* buffer,
		size_t buf_length,
		long file_id);
	virtual void storeFeatureSegRecord(long segmentnr, long track_id,
		long featurevectortype_id,
		const char* buffer,
		size_t buf_length,
		long file_id,
		long startsample,
		long length);
	virtual void storeDistancejobRecords(tDistancejobRecord* djr);
	virtual void storeSmuijob_addtrackRecord(tSmuijob_addtrackRecord* rec);
	virtual void storeSmuijob_deletetrackRecord(tSmuijob_deletetrackRecord* rec);
	virtual void query_nn(std::vector< Nn_result_rs_Ptr > &nns, long track_id, long fvt_id, long dist_id, std::string sCollectionName, int skip_k, int k);
	/** Inserts distance record
	 * <p>This implementation collects all numbers and peforms a copy operatoin if the current transaction is to be closed
	 * @param track_a_id db key
	 * @param track_b_id db key
	 * @param fvt_id db key
	 * @param dist_id db key
	 * @param d value for distance.
	 * */
	virtual void insertDistanceRecord(long track_a_id, long track_b_id, long fvt_id, long dist_id, double d);
	virtual unsigned long purgeDistanceTable(long track_id, long fvt_id, long dist_id, size_t topk);


private:
	/** checks if connecton is open */
	void checkOpenConnection();

	/** interal error handling function. Expected to clean up and throw an std::string exception
	 * @param sql pointer to sql command (c string) which failed
	 * @param res pointer to PGresult instance (to be freed)
	 * @throws std::string
	 */
	void bailOut(const char* sql, PGresult  *res);

	/** Creates copy statement for distance recors from container contents */
	void copy_dists_to_db();


	PGconn     *conn;
	const char *conninfo;
    PGresult   *res;

    /** stores information about distances recors that are to be copied */
    vector_tDistanceRecord_Ptr dists_queue_for_copy;
    /** used for dists container */
    tDistanceRecord_Ptr distrec_ptr;



};
