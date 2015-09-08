///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// smafestoredb.h
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

#include "smafeExportDefs.h"
#include "smafeutil.h"
#include "tAudioformat.h"
#include "smafeFeatureVectorClasses.h"
#include "smafeFVType.h"
#include "smafeLogger.h"

#include <boost/shared_ptr.hpp>
#include <vector>
#include <fstream>
// for OpenMP
#include <omp.h>



/** Datatypes corresponding to table records */
struct tDistanceRecord {
	long track_a_id, track_b_id, fvt_id, dist_id;
	double d;
};
/** Smart pointer for this type */
typedef boost::shared_ptr<tDistanceRecord> tDistanceRecord_Ptr;
/** vector of distance records */
typedef std::vector<tDistanceRecord_Ptr> vector_tDistanceRecord_Ptr;

struct tSmafejob_addfileRecord {
	long id, priority;
	std::string file_uri, status, collection_name, log, external_key, guid;
};
/** Smart pointer for this type */
typedef boost::shared_ptr<tSmafejob_addfileRecord> tSmafejob_addfileRecord_Ptr;
/** vector of distance records */
typedef std::vector<tSmafejob_addfileRecord_Ptr> vector_tSmafejob_addfileRecord_Ptr;


struct tSmafejob_deletefileRecord {
	long id, priority, file_id;
	std::string status, collection_name, log;
};
/** Smart pointer for this type */
typedef boost::shared_ptr<tSmafejob_deletefileRecord> tSmafejob_deletefileRecord_Ptr;
/** vector of distance records */
typedef std::vector<tSmafejob_deletefileRecord_Ptr> vector_tSmafejob_deletefileRecord_Ptr;


struct tSmafejob_deletecollectionRecord {
	long id, priority;
	std::string status, collection_name, log;
};

struct tDistancejobRecord {
	long smafejob_addfile_id, track_id, featurevectortype_id, distancetype_id, priority;
	std::string status;
};

// -- smui jobs
struct tSmuijob_addtrackRecord {
	long id, priority, track_id;
	std::string status, log;
};
struct tSmuijob_deletetrackRecord {
	long id, priority, track_id;
	std::string status, log;
};

/** record for dbinfo */
struct tDbinfoRecord {
	long numberoflayers, dimx, dimy;
	std::string bubblesagginfo, labelsagginfo;
};



/** Class that encapsulates database communication */
class DLLEXPORT SmafeStoreDB
{
public:
	// -- constants regarding DDL
	/** reserved collection: removed */
	static const long RESERVEDCOLLECTIONS_REMOVED = 1;
	/** reserved collection: default */
	static const long RESERVEDCOLLECTIONS_DEFAULT = 2;
	/** array for collection names */
	static std::string aReservedCollectionNames[3];
	// status strings for job rows
	/** job finished without error */
	static std::string STATUSOK;
	/** currently not used */
	static std::string STATUSDIST;
	/** job failed */
	static std::string STATUSFAILED;
	/** add file job: feature extractoin done*/
	static std::string STATUSFEDONE;


	// -- constants
	/** Length of char arrays for sql statements */
	static const int MAXSQLLEN = 3000;



	// -- methods
	SmafeStoreDB(void);
	virtual ~SmafeStoreDB(void);


	//--------------------------------------- General methods -----------------------------------------

	/** Opens connection to database
	 * This method must set strDb* variables */
	virtual void openConnection(std::string dbhost, std::string dbport, std::string dbuser, std::string dbpwd, std::string dbdb) = 0;

	/** Opens connection to database using the stored connection details  */
	virtual void openConnection();

	/** Closes connection to database */
	virtual void closeConnection() = 0;


	/** Starts a transaction */
	virtual void startTransaction() = 0;

	/** Finishes a transaction, either as COMMIT or ROLLBACK, depending on bool
	 *	parameter
	 *	@param bCommit if true, the transaction is being committed. If false,
	 *			the transaction is being rolled back.
	 */
	virtual void finishTransaction(bool bCommit) = 0;


	/** Executes an SQL statement that does not return anything
	 *	@param sqlcmd the SQL statement
	 *	@return number of affected rows
	 */
	virtual long executeStatement(const char* sqlcmd) = 0;

	/** Executes an SQL statement that does not return anything
	 * 	If the statement fails because of a serialization error
	 *  this function returns false (rather than throwing an exception)
	 *  @param sqlcmd the SQL statement
	 *	@return true, if commit was successful; false if commit failed due to serialization error
	 */
	virtual bool executeStatement_serializationcheck(const char* sqlcmd) = 0;


	/** Escapes a string for use in an SQL statement.
	<p>In SmafeStoreDB the method is declared abstract as the
	escaping process is database dependent.
	<p>If an error occurs an exception is thrown
	@param from		Input string, \0 terminated as usual
	@param to		Output string, buffer already allocated.
					<b>Buffer must be at least double size of input + 1 byte </b>
	 */
	virtual void escapeString(const char* from, char* &to) = 0;


	/** Escapes a string for use in an SQL statement, using std::string
	<p>In SmafeStoreDB the method is declared abstract as the
	escaping process is database dependent.
	<p>If an error occurs an exception is thrown
	@param from		Input string
	@returns 		OUtput string, escaped

	 */
	virtual std::string escapeString(std::string from) = 0;





	//--------------------------------------- Change structure (DDL) ----------------------------------


	/** checks if cost intensive index in distance table exist */
	virtual bool distanceTableIndexExists() = 0;

	/** creates cost intensive index in distance table
	 * @throws std::string if db error occurs. */
	virtual void createDistanceTableIndex() = 0;

	/** drops cost intensive index in distance table  */
	virtual void dropDistanceTableIndex() = 0;



	//--------------------------------------- Get information -----------------------------------------

	/** Returns whether the given table exists in the database (empty or populated does not matter).
	 * @param tablename string: name of table
	 */
	virtual bool tableExists(std::string tablename) = 0;

	/** Returns whether the given table is empty.
	 * @param tablename string: name of table
	 */
	virtual bool tableIsEmpty(std::string tablename) = 0;


	/** Returns whether the given file hash value is already contained
	 * in the database.
	 * @param hash Hash value
	 * @return >= 0 (the file_id), if hash is already in database. < 0 otherwise
	 */
	virtual long isFileInDatabase(t_filehash hash) = 0;

	/** Returns whether all three items (the given file hash value, filename and external key) are already contained
	 * in the database.
	 * @param hash Hash value
	 * @param filename the name of the file (URI in the database)
	 * @param external_key external key, got from the addfile job in smafe_api
	 * @param guid guid, got from the addfile job in smafe_api
	 * @return >= 0 (the file_id), if hash, URI and ext key exist in database. < 0 otherwise
	 */
	virtual long isFileInDatabase(t_filehash hash, std::string filename, std::string external_key, std::string guid) = 0;

	/** Returns whether the given filename is already contained
	 * in the database.
	 * <p>note that this method uses a bit of heuristic (you could also specify not only
	 * a filename but (part of) path + filename. In this case the outcome is not defined.
	 * <p>The filename is used in a LIKE statement "...like '<filename>'...."
	 * @param filename the name of the file
	 * @return >= 0 (the file_id of first record), if there is one or more record in table "File"
	 * whose uri field ends with the given filename . < 0 otherwise
	 */
	virtual std::vector< long > isFileInDatabase(std::string filename) = 0;

	/** Returns whether the given fingerprint is already contained
	 * in the database.
	 * @param fp The fingerprint
	 * @return >= 0 (the track_id), if fingerprint is already in database (plus or minus a
	 *		given tolerance. < 0 otherwise
	 */
	virtual long isTrackInDatabase(t_fingerprint fp) = 0;

	/** Returns whether the given track is in the given collection
	 * This is the case if the track is referenced by at least one file that is in the collection
	 * @param track_id id of track
	 * @param collection_id id of collection
	 * @return true if track is in collection, false otherwise
	 */
	virtual bool isTrackInCollection(long track_id, long collection_id) = 0;


	/** Returns whether the given collection name is in the DB
	 * @param coll_name the collection name
	 * @return >= 0 (the collection_id), if record is in database. < 0 otherwise
	 */
	virtual long isCollectionInDatabase(std::string coll_name) = 0;

	/** Returns whether there is a distance record in table distance with specified constraints
	 * @param track_a_id... primary and foreign keys
	 * @return true if record is in distance table, false otherwise
	 */
	virtual bool isDistanceRecordInDatabase(long track_a_id, long track_b_id, long fvt_id, long distt_id) = 0;



	/** Returns track_id for given file_id
	 * @param file_id The file_id primary key
	 * @return >= 0 (the track_id)
	 */
	virtual long getTrackIDForFile(long file_id) = 0;


	/** Returns id of given collection name
	 * @param collection_name the name of the collection
	 * @return >= 1 (the collection_id)
	 */
	virtual long getCollectionId(std::string name) = 0;


	/** Returns number of track records
	 * @return >=0 : number of track records
	 */
	virtual size_t getTrackCount() = 0;

	/** Returns number of file records
	 * @return >=0 : number of  records
	 */
	virtual size_t getFileCount() = 0;


	/** Returns whether the given feature vector type is already contained
	 * in the database.
	 * NOTE (does not handle decryption!)
	 * @param todo todo
	 * @return >= 0 (the featurevectortype_id), if it is already in database ;
	 *		< 0 otherwise
	 */
	//virtual long isFeatVecTypeInDatabase(const char* name, int version, int dim_x, int dim_y, const char* params) = 0;

	/** Returns whether a feature vector exists in the database for a given track_id and
	 * featurevectortype_id
	 * @param todo todo
	 * @return true, or false
	 */
	virtual bool isFeatVecInDatabase(long track_id, long featurevectortype_id) = 0;

	/** Get information of open tasks (table distancejob).
	 * This function returns the total number of open tasks and the primary key
	 * of one task chosen (if there is at least one open task)
	 * @param distancetype_ids Vektor of ids to be filtered. If empty, all ids will be queried
	 * @param fvtype_ids Vektor of ids to be filtered. If empty, all ids will be queried
	 * @param lNumVacancies OUT number of open tasks
	 * @param lNumCurrentVac OUT number of open tasks mathcing current parameters
	 * @param currentJob_track_id OUT part of primary key of record of distancejob
	 * @param currentJob_fvt_id OUT part of primary key of record of distancejob
	 * @param currentJob_dist_id OUT part of primary key of record of distancejob
	 * @returns true if there is one open task (in this case the primary key is returned as OUT params);
	 * 			or false, if there is not open tasks
	 */
	virtual bool getOpenTaskInfo(std::vector<long> *distancetype_ids, std::vector<long> *fvtype_ids,
			long &lNumVacancies, long &lNumCurrentVac, long &currentJob_track_id,  long &currentJob_fvt_id,
			long &currentJob_dist_id) = 0;

	/** Get information of open tasks (table smafejob_addfile).
	 * This function returns the total number of open tasks and the primary key
	 * of one task chosen (if there is at least one open task)
	 * @param rec OUT record with info for the chosen task
	 * @param lNumVacancies OUT number of open tasks
	 * @param lNumCurrentVac OUT number of open tasks mathcing current parameters
	 * @returns true if there is one open task (in this case the primary key is returned as OUT params);
	 * 			or false, if there is not open tasks
	 */
	virtual bool getOpenTaskInfo_addfile(tSmafejob_addfileRecord &rec, long &lNumVacancies, long &lNumCurrentVac) = 0;

	/** Get information of open tasks (table smafejob_deletefile).
	 * This function returns the total number of open tasks and the primary key
	 * of one task chosen (if there is at least one open task)
	 * @param rec OUT record with info for the chosen task
	 * @param lNumVacancies OUT number of open tasks
	 * @param lNumCurrentVac OUT number of open tasks mathcing current parameters
	 * @returns true if there is one open task (in this case the primary key is returned as OUT params);
	 * 			or false, if there is not open tasks
	 */
	virtual bool getOpenTaskInfo_deletefile(tSmafejob_deletefileRecord &rec, long &lNumVacancies, long &lNumCurrentVac) = 0;

	/** Get information of open tasks (table smafejob_deletecollection).
	 * This function returns the total number of open tasks and the primary key
	 * of one task chosen (if there is at least one open task)
	 * @param rec OUT record with info for the chosen task
	 * @param lNumVacancies OUT number of open tasks
	 * @param lNumCurrentVac OUT number of open tasks mathcing current parameters
	 * @returns true if there is one open task (in this case the primary key is returned as OUT params);
	 * 			or false, if there is not open tasks
	 */
	virtual bool getOpenTaskInfo_deletecollection(tSmafejob_deletecollectionRecord &rec, long &lNumVacancies, long &lNumCurrentVac) = 0;

	/** Get information of open tasks (table smuijob_addtrack).
	 * This function returns the total number of open tasks and the primary key
	 * of one task chosen (if there is at least one open task)
	 * @param rec OUT record with info for the chosen task
	 * @param lNumVacancies OUT number of open tasks
	 * @param lNumCurrentVac OUT number of open tasks mathcing current parameters
	 * @returns true if there is one open task (in this case the primary key is returned as OUT params);
	 * 			or false, if there is not open tasks
	 */
	virtual bool getOpenTaskInfo_smuiaddtrack(tSmuijob_addtrackRecord &rec, long &lNumVacancies, long &lNumCurrentVac) = 0;

	/** Get information of open tasks (table smuijob_deletetrack).
	 * This function returns the total number of open tasks and the primary key
	 * of one task chosen (if there is at least one open task)
	 * @param rec OUT record with info for the chosen task
	 * @param lNumVacancies OUT number of open tasks
	 * @param lNumCurrentVac OUT number of open tasks mathcing current parameters
	 * @returns true if there is one open task (in this case the primary key is returned as OUT params);
	 * 			or false, if there is not open tasks
	 */
	virtual bool getOpenTaskInfo_smuideletetrack(tSmuijob_deletetrackRecord &rec, long &lNumVacancies, long &lNumCurrentVac) = 0;


	/** reads db information from table dbinfo and stores it in the record */
	virtual void getDbinfo(tDbinfoRecord &rec) = 0;


	/** Returns id of best matching bubble (=nearest, biggest)
	 * @param x x coordinate
	 * @param y y coordinate
	 * @param layer the number of layer (use table bubblesX)
	 * @param distparam the parameter used for Distwithin
	 * @return >= 0 (the id) or -1 in case of an error
	 */
	virtual long getNearestBubble(long x, long y, long layer, long distparam) = 0;

	/** Returns bubble id and count for the given layer and track
	 * @param IN l 			layer number (1 -> bubbles1, ...)
	 * @param IN track_id	track id
	 * @param OUT lBubbleId	the bubble id. -1 in case of error
	 * @param OUT lCount	count
	 */
	virtual void getBubbleInfo(long l, long track_id, long &lBubbleId, long &lCount) = 0;

	/** returns list of existing track_ids lower than the specified currentJob_track_id
	 * <p>NB: currently, distancetype_id is not used */
	virtual std::vector<longlongpair> getLowerTracksForDistanceCalc(long currentJob_track_id, long currentJob_fvt_id, long currentJob_dist_id) = 0;

	/** returns list of existing track_ids other than the specified currentJob_track_id if >= 0, and
	 * returns list of all track_ids if currentJob_track_id if < 0
	 * <p>NB: currently, distancetype_id is not used */
	virtual std::vector<longlongpair> getOtherTracksForDistanceCalc(long currentJob_track_id, long currentJob_fvt_id, long currentJob_dist_id) = 0;

	/** returns list of existing track_ids / segmentnr pairs
	 * <p>NB: currently, distancetype_id and currentJob_track_id are not used */
	virtual std::vector<longlongpair> getSegmentTracksForDistanceCalc(long currentJob_track_id, long currentJob_fvt_id, long currentJob_dist_id) = 0;


	/** returns list of existing track_a_ids from distance table, for given fvt id and dist id
	 *
	 * */
	virtual std::vector<long> getTrack_a_ids(long fvt_id, long dist_id) = 0;

	/** returns list of track ids from track table
	 * @param collection_name if not empty, return only tracks contained in that collection
	 * @return vector of track ids
	 *
	 * */
	virtual std::vector<long> getTrack_ids(std::string collection_name) = 0;

	/** returns track / collection assignment, except the _removed collection. For each collection_id, except _removed, we have a set of track_ids
	 * @param featurevectortype_id the featurevectortype id
	 * @param bSegments if fv of segments should be used
	 * @param tracks_collections OUT pointer to deque of sets of track_ids. Index for deque is collection_id
	 */
	virtual void getTracksAndCollections(const long featurevectortype_id, const bool bSegments, longlong_set_deque &tracks_collections) = 0;

	void getCollectionReferences(const long currentJob_fvt_id, const bool bUseSegmFvs, const tFVMap &fvbuffer, tlonglonglongsafvStructVectorMap &lllsafvMap);

	/** returns list of file_ids for the given tracks. May returned removed files or not.
	 * @param track_id the track id
	 * @param bAlsoRemoved if false, only files are returned that are not contained
	 * in the special REMOVED collection. If true, these files are returned as well.
	 * @return vector of file_ids
	 *
	 * */
	virtual std::vector<long> getFilesForTrack(long track_id, bool bAlsoRemoved) = 0;


	/** returns list of distancetype_ids from distancetype table
	 *
	 * */
	virtual std::vector<long> getDistance_ids() = 0;


	/** returns list of featurevectortype_ids from fvt table
	 * Note: does not need decryption (only ids are taken)
	 *
	 * */
	virtual std::vector<long> getFeaturevectortype_ids() = 0;

	/** returns map of config records from config table
	 * <b>Note:</b> This method returns the config stuff NOT decrypted, ie, encrypted, ie, as it is in the database
	 * @return map where the string key is the key from db and the string value is the value form the db
	 * */
	virtual tKeyValueMap getConfigRecords() = 0;

	/** Reads entry in FeatureVector table
	 * <p>Please dont forget to delete[] the buffer after use
	 * <p>If segmentnr is < 0, table featurevector is used, if >=0 featurevectorsegment is used
	 * @param track_id ... featurevectortype_id: foreign keys
	 * @param buffer OUT: pointer to \0 terminated data buffer (allocated in method) OR NULL if fv was not found in db
	 * @param buf_length OUT: length of buffer, including \0
	 * @param class_id OUT: class_id CURRENTLY NOT RETURNED!
	 * @param file_id OUT: the file_id foreign key
	 */
	virtual void readFeatureRecord_raw(long track_id,
			long segmentnr,
			long featurevectortype_id,
			char* &buffer,
			size_t &buf_length,
			std::string &class_id,
			long &file_id, bool load_file_id) = 0;

	/** Reads all fvs (whole track or segments) of one type.
	 *  <p>If segmentnr is < 0, table featurevector is used, if >=0 featurevectorsegment is used
	 * @param bSegments: if true, featurevectorsegments table is used. otherwise: featurevector table
	 * @param featurevectortype_id: foreign keys
	 * @param lLimitsize: if != 0: use LIMIT clause with given amount
	 * @param lOffset: if != 0: use OFFSET clause with given amount
	 * @param vBuffer OUT: pointer to a tRawFVMap structure (map of <track_id, segmentnr> pairs to buffer which holds the raw fv.
	 *  <p>The buffers are wrapped in a smart pointer
	 * @return false if no feature vectors are returned, true otherwise
	 */

	virtual bool readAllFeatureRecords_raw(bool bSegments, long featurevectortype_id, long lLimitsize, long lOffset,  tRawFVMap &vBuffer) = 0;


	/** Reads information about feature vector type and file uri
	 * @param fv OUT: (allocated) instance of SmafeAbstractFeatureVector. <p>A new instance of SmafeFVType will be created
	 * and attached to fv->fvtype which must be deleted
	 * @param track_id, fvt_id ... featurevectortype_id, track_id: prim/foreign keys
	 * @param file_id optional file primary key, if meta data should be derived from specific file (the given file_id)
	 * and not just the first one (taken if < 0)
	 */
	virtual void getFeatureVectorMetaInfo(SmafeAbstractFeatureVector &fv, long fvt_id, long track_id, long file_id) = 0;

	/** Reads feature vector recordset that is compatibel with
	 * SmafeAbstractFeatureVector class.
	 * A new instance of this class is created and returned.
	 * In this instance, the foreign keys track_id, and file_id are set
	 * @param fvt_id again, featurevectortype_id
	 * @param track_id guess :-)
	 * @param load_fvt_info If true, feature vector type metadata and file uri is loaded.
	 * @param file_id: optional primary key for file. Specifies if URI of certain file should be used (in case that
	 * 					we have two files for the same track.) A value < 0 means: no specific file.
	 * 					NB:  this parameter is only used if load_fv_info is true.
	 * @param segmentnr optional number of segment, if segment feature vector should be loaded. Set < 0 if whole-track fv should be loaded
	 * @param onErrorSilent: if true, no error handling is done (must be done by caller)
	 * @return if onErrorSilent==false the method always returns a valid result OR throws an exception.
	 * 			if onErrorSilent==true the method may return NULL
	 */
	SmafeAbstractFeatureVector* readFeatureVector(long fvt_id, long track_id, bool load_fvt_info, long file_id, long segmentnr, bool onErrorSilent, bool load_file_id);

	/** Reads feature vectors
	 * New instances of SmafeAbstractFeatureVector class are created and returned.
	 * In this instance, the foreign keys track_id is set. (segmentnr ?)
	 * @param fvt_id again, featurevectortype_id
	 * @param bSegments: if true, featurevectorsegments table is used. otherwise: featurevector table
	 * @param onErrorSilent: if true, no error handling is done (must be done by caller)
	 * @param fvbuffer OUT pointer to tFVMap buffer where the result is returned
	 * @param lLimitsize number of FVs to load at once. 0 .. all at once. Useful if we have low memory
	 * @return true if everyhitng went okay, false otherwise
	 */
	bool readAllFeatureVectors(long fvt_id, bool bSegments, bool onErrorSilent, tFVMap &fvbuffer, long lLimitsize);

	/** Reads feature vector recordset that is compatibel with
	 * SmafeAbstractFeatureVector class.
	 * A new instance of this class is created and returned.
	 * <p>Just delegate to other method, no metadata loaded
	 * @param fvt_id again, featurevectortype_id
	 * @param track_id guess :-)
	 */
	SmafeAbstractFeatureVector* readFeatureVector(long fvt_id, long track_id);

	/** Reads feature vector recordset that is compatibel with
	 * SmafeAbstractFeatureVector class.
	 * A new instance of this class is created and returned.
	 * <p>Just delegate to other method, no metadata loaded, use segment table
	 * @param fvt_id again, featurevectortype_id
	 * @param track_id guess :-)
	 * @param segmentnr number of segment
	 */
	SmafeAbstractFeatureVector* readSegmentFeatureVector(long fvt_id, long track_id, long segmentnr);

	/** Get vector with availble distancetypes
	 * @param nns OUT: vector of tDistancetype struct to hold the result
	 */
	virtual void getDistancetypes(std::vector< tDistancetype > &vDisttypes) = 0;

	/** Get vector with availble fv types
	 * @param nns OUT: vector of SmafeFVType struct to hold the result
	 */
	virtual void getFeaturevectortypes(std::vector< SmafeFVType_Ptr > &v) = 0;

	/** DB specific query for nearest neighbours according to distance
	 * @param nns OUT: vector for Nn_result_rs_Ptr to hold the result
	 * @param track_id, fvt_id, dist_id ... prim/foreign keys
	 * @param k number of neighbours
	 */
	virtual void query_nn(std::vector< Nn_result_rs_Ptr > &nns, long track_id, long fvt_id, long dist_id, std::string sCollectionName, int k);

	/** DB specific query for nearest neighbours according to distance
	 * @param nns OUT: vector for Nn_result_rs_Ptr to hold the result
	 * @param track_id, fvt_id, dist_id ... prim/foreign keys
	 * @param skip_k number of neighbours to skip from beginning
	 * @param k number of neighbours
	 */
	virtual void query_nn(std::vector< Nn_result_rs_Ptr > &nns, long track_id, long fvt_id, long dist_id, std::string sCollectionName, int skip_k, int k) = 0;

	/** DB specific query for nearest neighbours according to distance
	 * @param nns OUT: vector of doubles to hold the result
	 * @param track_id, fvt_id, dist_id ... prim/foreign keys
	 * @param skip_k number of neighbours to skip from beginning
	 * @param k number of neighbours
	 */
	t_double_deque query_nn(long track_id, long fvt_id, long dist_id, std::string sCollectionName, int skip_k, int k);


	//--------------------------------------- Store information -----------------------------------------


	/** Stores config record in the datase
	 * The data (key and value) is transparently encrypted useing the "verysecretpassphrase".
	 * This method works for both cases: - key not contained in config table, - key contained in config table.
	 * (Frist, an UDPATE is done, if now rows are affected, INSERT is executed.
	 * @param key string for the key, plaintext
	 * @param value string for the value, plaintext
	 * @return the key used (may be encrypted)
	 */
	virtual std::string storeConfigRecord(std::string key, std::string value) = 0;

	/** Stores config record in the datase
	 * The encrypted key is used to UDPATE the value
	 * @param key string for the key, plaintext
	 * @param value string for the value, plaintext
	 * @param key_encrypted existing encrypted key
	 */
	virtual void storeConfigRecord(std::string key, std::string value, std::string key_encrypted) = 0;

	/** Generates SQL commands to store configs.
	 * The data (key and value) is transparently encrypted useing the "verysecretpassphrase"
	 * @param key string for the key, plaintext
	 * @param value string for the value, plaintext
	 */
	virtual std::string buildConfigRecord(std::string key, std::string value);

	/** generates SQL commands to store configs.  The data (key and value) is transparently encrypted using the given passphrase
	 * @param key string for the key, plaintext
	 * @param value string for the value, plaintext
	 */
	virtual std::string buildConfigRecord(std::string key, std::string value, const char* pp);


	/** Stores new entry in file table
	 * @param track_id corresponding track_id (foreign key)
	 * @param hash hash of the file
	 * @param ad audio meta information
	 * @return primary key of inserted record
	 */
	virtual long storeFileRecord(long track_id, t_filehash hash, tAudioformat* ad, std::string guid, std::string external_key) = 0;


	/** Stores new entry in track table
	 * @param coll_name collection name
	 * @return primary key of inserted record
	 */
	virtual long storeCollectionRecord(std::string coll_name) = 0;


	/** Makes sure that the specified collectoin_id, file_id pair is in the DB
	 * @param collection_id the collection_id
	 * @param file_id the file_id
	 */
	virtual void ensureCollectionFileInDatabase(long collection_id, long file_id) = 0;

	/** Makes sure that the specified file_id is connected with a reserved collection
	 * @param which_coll Specifies which resreved collection should be used (constants)
	 * @param bOnlyIfNoOther Inserts the record only if there is no other collection_file record for this file
	 * @param file_id the file_id
	 */
	virtual void ensureCollectionFileInDatabase_reservedname(int which_coll, bool bOnlyIfNoOther, long file_id) = 0;

	/** Deletes the mapping from the given file id to the _removed collection if other mappings are present
	 * @param file_id the file id
	 * @return number of rows affected
	 */
	long removeFromRemovedCollectionIfNecessary(long file_id);

	/** Stores new entry in track table
	 * @param fingerprint fingerprint of the track
	 * @return primary key of inserted record
	 */
	virtual long storeTrackRecord(t_fingerprint fingerprint) = 0;

	/** Stores new entry in FeatureVectorType table
	 * @param (...) table columns
	 * @return primary key of inserted record
	 */
	virtual long storeFeatVecTypeRecord(const char* name, int version, int dimx, int dimy, const char* params, const char* class_id) = 0;


	/** Stores new entry in FeatureVector table
	 * @param track_id ... featurevectortype_id, file_id: foreign keys
	 * @param buffer data buffer
	 * @param buf_length length of buffer
	 */
	virtual void storeFeatureRecord(long track_id,
			long featurevectortype_id,
			const char* buffer,
			size_t buf_length,
			long file_id) = 0;

	/** Stores new entry in FeatureVector table from (SmafeAbstractFeatureVector) object
	 * @param track_id ... featurevectortype_id, file_id: foreign keys
	 * @param fv sublclass of SmafeAbstractFeatureVector
	 */
	void storeFeatureRecord(long track_id,
			long featurevectortype_id,
			const SmafeAbstractFeatureVector* fv,
			long file_id);

	/** Stores new entry in FeatureVectorSegment table from (SmafeAbstractFeatureVector) object
	 * @param track_id ... featurevectortype_id, file_id: foreign keys
	 * @param startsample, length info fields
	 * @param buffer data buffer
	 * @param buf_length length of buffer
	 */
	virtual void storeFeatureSegRecord(const long segmentnr, const long track_id,
			 long featurevectortype_id,
			const SmafeAbstractFeatureVector* fv,
			const long file_id,
			const long startsample,
			const long length);

	/** Stores new entry in FeatureVectorSegment table
	 * @param track_id ... featurevectortype_id, file_id: foreign keys
	 * @param startsample, length info fields
	 * @param buffer data buffer
	 * @param buf_length length of buffer
	 */
	virtual void storeFeatureSegRecord(long segmentnr, long track_id,
			long featurevectortype_id,
			const char* buffer,
			size_t buf_length,
			long file_id,
			long startsample,
			long length) = 0;

	/** Stores new rows in distancejob table
	 * @param djr struct of distancejob record. These fields must be filled:
	 * 			- smafejob_addfile_id
	 * 			- track_id
	 * 			- featurevectortype_id
	 * 			- priority
	 * The method will insert a NULL value if the smafejob_addfile_id is below 0
	 * @return primary key of inserted record
	 */
	virtual void storeDistancejobRecords(tDistancejobRecord* djr) = 0;

	/** Stores smuijob_addtrack job
	 * @param djr struct of  record. These fields must be filled:
	 * 			- track_id
	 * 			- priority
	 */
	virtual void storeSmuijob_addtrackRecord(tSmuijob_addtrackRecord* rec) = 0;

	/** Stores smuijob_deletetrack job
	 * @param djr struct of  record. These fields must be filled:
	 * 			- track_id
	 * 			- priority
	 */
	virtual void storeSmuijob_deletetrackRecord(tSmuijob_deletetrackRecord* rec) = 0;




	/** Sets the "id" member of the given fvtype to the respective featurevectortype_id from the database.
	 * <p>If this fv-type is not already in the db it will be inserted.
	 * @param fvtype IN/OUT smafeFVType class instance with all relevent members set (dims, parameters etc).
	 * 		<p>id will be set in this method.
	 * @return true if record has been inserted to db; false otherwise
	 */
	bool ensureRPFeatVecTypeRecord(SmafeFVType* fvtype);


	/** Stores feature vectors in DB
	 * <p>Throws exception, if vector is empty
	 * @param fvs std::vector of feature vectors
	 * @param segmentfvs segment features (RP etc)
	 * @param file_id, track_id: primary keys from database
	 * @return error code if error occured, 0 otherwise
	 */
	int store(std::vector<SmafeAbstractFeatureVector_Ptr> fvs, std::vector<SmafeAbstractFeatureVector_Ptr> segmentfvs,
			long file_id, long track_id, long addfilejob_id);



	/** Inserts distance record
	 * @param track_a_id db key
	 * @param track_b_id db key
	 * @param fvt_id db key
	 * @param dist_id db key
	 * @param d value for distance.
	 * */
	virtual void insertDistanceRecord(long track_a_id, long track_b_id, long fvt_id, long dist_id, double d);


	/** Deletes superfluous distance records, ie., all recors with a value worse that the top k
	 */
	virtual unsigned long purgeDistanceTable(long track_id, long fvt_id, long dist_id, size_t topk) = 0;


	/** Generates SQL commands to empty (truncate) config table
	 */
	virtual std::string clearConfigRecords();


protected:
	/** "get next job" calls since last real SQL query for open jobs */
	int iLastTimeJobQuery;

	// details about the connection opened
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



};
