///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// smafestoredb_pq.cpp
//
// PostgreSQL specific db communication
// This file is only compiled if SMAFE_PQ is defined
// ------------------------------------------------------------------------
//
// $Id$
//
//
//
//
//
///////////////////////////////////////////////////////////////////////////


#if defined(SMAFE_PQ)

#include "smafestoredb_pq.h"

SmafeStoreDB_PQ::SmafeStoreDB_PQ() :
dists_queue_for_copy() {
	conn = NULL;
}

SmafeStoreDB_PQ::~SmafeStoreDB_PQ() {
	closeConnection();
}

PGresult* SmafeStoreDB_PQ::myPQexec(PGconn *conn, const char *query) {
	SMAFELOG_FUNC(SMAFELOG_DEBUG, "Executing SQL: " + std::string(query));
	return PQexec(conn, query);
}

void SmafeStoreDB_PQ::bailOut(const char* sql, PGresult *res) {
	char tmp[5250];
	sprintf(tmp, "'%s' failed: %s", sql, PQerrorMessage(conn));

	PQclear(res);
	throw(std::string(tmp));
}

void SmafeStoreDB_PQ::checkOpenConnection() {
	if (PQstatus(conn) != CONNECTION_OK)
		throw std::string("Database connection not ready (PostgreSQL)");
}

void SmafeStoreDB_PQ::openConnection(std::string dbhost, std::string dbport, std::string dbuser, std::string dbpwd, std::string dbdb) {
	SMAFELOG_FUNC(SMAFELOG_DEBUG, std::string("Trying PQconnectdb..."));

	// prepare connection string
	char strConn[MAXSQLLEN];
	sprintf(strConn, "host=%s port=%s dbname=%s user=%s password=%s", dbhost.c_str(), dbport.c_str(),  dbdb.c_str(),
			dbuser.c_str(), dbpwd.c_str());

	SMAFELOG_FUNC(SMAFELOG_DEBUG, "Connection string:");
	SMAFELOG_FUNC(SMAFELOG_DEBUG, std::string(strConn));

	/* Make a connection to the database */
	conn = PQconnectdb(strConn);
	//conn = PQconnectdb("dbname=postgres user=postgres password=postgres");

	/* Check to see that the backend connection was successfully made */
	if (PQstatus(conn) != CONNECTION_OK) {
		char tmp[MAXSQLLEN];
		sprintf(tmp, "Connection to database failed: %s", PQerrorMessage(conn));
		throw(std::string(tmp));
	}

	// save details for later
	strDbhost = dbhost;
	strDbport = dbport;
	strDbuser = dbuser;
	strDbpwd = dbpwd;
	strDbname = dbdb;
}


void SmafeStoreDB_PQ::closeConnection() {
	if (conn != NULL) PQfinish(conn);
}

void SmafeStoreDB_PQ::startTransaction() {
	checkOpenConnection();

	char sqlcmd[] = "START TRANSACTION;";

	res = myPQexec(conn, sqlcmd);

	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		bailOut(sqlcmd, res);
	}

	// empty distances
	// note if not empty
	if (dists_queue_for_copy.size() > 0) {
		SMAFELOG_FUNC(SMAFELOG_INFO,
				"dists_queue_for_copy not empty when starting transaction! Not clearing now.");
	}
	// no clearing because it might be on purpose
	//dists_queue_for_copy.clear();

	PQclear(res);
}

void SmafeStoreDB_PQ::finishTransaction(bool bCommit) {
	checkOpenConnection();

	char sqlcmd[1024];

	if (bCommit) {
		strcpy(sqlcmd, "COMMIT;");
		// copy distance records to db
		if (dists_queue_for_copy.size() > 0) {
			copy_dists_to_db();
			SMAFELOG_FUNC(SMAFELOG_DEBUG, "Clearing dists_queue_for_copy.");
			dists_queue_for_copy.clear();
		}
	} else {
		strcpy(sqlcmd, "ROLLBACK;");
		// clear dists container
		SMAFELOG_FUNC(SMAFELOG_DEBUG, "Clearing dists_queue_for_copy.");
		dists_queue_for_copy.clear();
	}

	res = myPQexec(conn, sqlcmd);

	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		bailOut(sqlcmd, res);
	}

	PQclear(res);
}

long SmafeStoreDB_PQ::executeStatement(const char* sqlcmd) {
	checkOpenConnection();

	// output every sql statement as debug level
	//SMAFELOG_FUNC(SMAFELOG_DEBUG, "Executing " + std::string(sqlcmd));

	res = myPQexec(conn, sqlcmd);

	if (PQresultStatus(res) != PGRES_COMMAND_OK && PQresultStatus(res)
	!= PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}

	long nTuples = atol(PQcmdTuples(res));

	PQclear(res);

	SMAFELOG_FUNC(SMAFELOG_DEBUG, " ... done");

	return nTuples;
}

bool SmafeStoreDB_PQ::executeStatement_serializationcheck(const char* sqlcmd) {
	checkOpenConnection();

	bool ret;

	res = myPQexec(conn, sqlcmd);

	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		// check for ser.. error
		// NB: Error code 40001 is "SERIALIZATION FAILURE"
		// using errcodes.h include file did not work as expected
		// so we have this string dropping off the sky
		if (strcmp(PQresultErrorField(res, PG_DIAG_SQLSTATE), "40001") == 0) {
			ret = false;
		} else {
			char tmp[1250];
			sprintf(tmp, "%s failed: %s (%s)", sqlcmd, PQerrorMessage(conn),
					PQresultErrorField(res, PG_DIAG_SQLSTATE));
			throw(std::string(tmp));
		}

	} else {
		// ok
		ret = true;
	}

	PQclear(res);
	return ret;
}

void SmafeStoreDB_PQ::escapeString(const char* from, char* &to) {
	int ret;
	(void) PQescapeStringConn(conn, to, from, strlen(from), &ret);
	if (ret != 0) {
		char tmp[2250];
		sprintf(tmp, "Escaping '%s'failed: %s", from, PQerrorMessage(conn));

		throw(std::string(tmp));
	}
}

std::string SmafeStoreDB_PQ::escapeString(std::string from) {
	char* str_escaped = new char[2 * from.size() + 1];
	escapeString(from.c_str(), str_escaped);
	std::string to = std::string(str_escaped);
	delete[] str_escaped;
	return to;
}

/*
 void SmafeStoreDB_PQ::escapeBytea(char* from, char* &to, size_t from_length, size_t &to_length) {
 to =  PQescapeByteaConn(conn, from, from_length, to_length);
 if (to == NULL) {
 char tmp[250];
 sprintf(tmp, "Escaping '%s'failed: %s", from,
 PQerrorMessage(conn));
 PQfinish(conn);
 throw(string(tmp));
 }
 }
 */

bool SmafeStoreDB_PQ::distanceTableIndexExists() {
	checkOpenConnection();

	bool ret = false;
	;
	char sqlcmd[MAXSQLLEN] = { 0 };

	sprintf(sqlcmd, "select * from distance_indexes_exist();");

	res = myPQexec(conn, sqlcmd);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}

	// PQgetvalue returns "t" in case of true
	ret = std::string(PQgetvalue(res, 0, 0)) == "t" ? true : false;

	PQclear(res);

	return ret;
}

void SmafeStoreDB_PQ::createDistanceTableIndex() {
	char sql[] = "select create_distance_indexes ();";
	executeStatement(sql);
}

void SmafeStoreDB_PQ::dropDistanceTableIndex() {
	char sql[] = "select drop_distance_indexes ();";
	executeStatement(sql);
}

bool SmafeStoreDB_PQ::tableExists(std::string tablename) {
	checkOpenConnection();

	bool ret;
	char sqlcmd[MAXSQLLEN] = { 0 };

	sprintf(sqlcmd, "SELECT COUNT(*) FROM pg_tables WHERE tablename='%s'",
			tablename.c_str());

	res = myPQexec(conn, sqlcmd);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}
	if (0 == PQntuples(res))
		throw std::string("Expected one row to be returned.");
	else
		// should be true, if the count value is larger 0
		ret = atol(PQgetvalue(res, 0, 0)) > 0;
	PQclear(res);

	return ret;
}

bool SmafeStoreDB_PQ::tableIsEmpty(std::string tablename) {
	checkOpenConnection();

	bool ret;
	char sqlcmd[MAXSQLLEN] = { 0 };

	sprintf(sqlcmd, "SELECT COUNT(*) FROM %s",
			tablename.c_str());

	res = myPQexec(conn, sqlcmd);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		// table does probably not exist
		//bailOut(sqlcmd, res);
		ret = false;
	}
	if (0 == PQntuples(res))
		throw std::string("Expected one row to be returned.");
	else
		// should be true, if the count value is 0
		ret = atol(PQgetvalue(res, 0, 0)) == 0;

	if (!ret)
		SMAFELOG_FUNC(SMAFELOG_DEBUG2, "# of rows in " + tablename + ": " + stringify(PQntuples(res)));

	PQclear(res);

	return ret;

}

long SmafeStoreDB_PQ::isFileInDatabase(t_filehash hash) {
	checkOpenConnection();

	long ret;
	char sqlcmd[MAXSQLLEN] = { 0 };

	sprintf(sqlcmd, "select * from file WHERE hash = '%s'", hash);

	res = myPQexec(conn, sqlcmd);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}
	if (0 == PQntuples(res))
		ret = -1;
	else
		// assume row 0 (hash is unique so there should be only one row)
		ret = atol(PQgetvalue(res, 0, PQfnumber(res, "id")));
	PQclear(res);

	return ret;
}

long SmafeStoreDB_PQ::isFileInDatabase(t_filehash hash, std::string filename, std::string external_key, std::string guid) {
	checkOpenConnection();

	long ret;
	//char sqlcmd[MAXSQLLEN] = { 0 };
	std::string sqlcmd;

	SMAFELOG_FUNC(SMAFELOG_DEBUG, "External key: " + external_key);
	SMAFELOG_FUNC(SMAFELOG_DEBUG, "matches '': " + stringify(external_key == ""));

	// include external_key into statement?: only if not empty
	if (external_key != "") {
		// use external key
		sqlcmd = "select * from file WHERE hash = '"+stringify(hash)+"' and uri = '"+this->escapeString(filename)+"' and external_key = '"+this->escapeString(external_key)+"' and guid = '"+this->escapeString(guid)+"';";
	} else {
		// external_key must be null
		sqlcmd = "select * from file WHERE hash = '"+stringify(hash)+"' and uri = '"+this->escapeString(filename) + "' and external_key IS NULL and guid = '"+this->escapeString(guid)+"';";
	}

	res = myPQexec(conn, sqlcmd.c_str());
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd.c_str(), res);
	}
	if (0 == PQntuples(res))
		ret = -1;
	else
		// assume row 0 (hash is unique so there should be only one row)
		ret = atol(PQgetvalue(res, 0, PQfnumber(res, "id")));

	PQclear(res);

	return ret;
}

std::vector<long> SmafeStoreDB_PQ::isFileInDatabase(std::string filename) {
	checkOpenConnection();

	std::vector<long> ret;
	char sqlcmd[MAXSQLLEN] = { 0 };

	char* uri_escaped = new char[2 * filename.size() + 1];
	escapeString(filename.c_str(), uri_escaped);

	sprintf(sqlcmd, "select id, uri from file WHERE uri like '%%%s%%'",
			uri_escaped);

	res = myPQexec(conn, sqlcmd);
	delete[] uri_escaped;
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}
	for (int i = 0; i < PQntuples(res); i++) {
		// only one column
		ret.push_back(atol(PQgetvalue(res, i, PQfnumber(res, "id"))));
		// log uri as debug message:
		SMAFELOG_FUNC(SMAFELOG_DEBUG, "LIKE operand %" + filename
				+ "% expands to " + std::string(PQgetvalue(res, i, PQfnumber(
						res, "uri"))));
	}
	PQclear(res);

	return ret;
}

long SmafeStoreDB_PQ::isCollectionInDatabase(std::string coll_name) {
	checkOpenConnection();

	long ret;
	char sqlcmd[MAXSQLLEN];
	char* name_escaped = new char[2 * coll_name.size() + 1];

	escapeString(coll_name.c_str(), name_escaped);

	sprintf(sqlcmd, "select * from collection WHERE collection_name = '%s'",
			name_escaped);

	res = myPQexec(conn, sqlcmd);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}
	if (0 == PQntuples(res))
		ret = -1;
	else
		// assume row 0
		ret = atol(PQgetvalue(res, 0, PQfnumber(res, "id")));

	PQclear(res);
	delete[] name_escaped;

	return ret;
}

bool SmafeStoreDB_PQ::isDistanceRecordInDatabase(long track_a_id,
		long track_b_id, long fvt_id, long distt_id) {
	checkOpenConnection();

	bool ret;
	char sqlcmd[MAXSQLLEN];

	sprintf(
			sqlcmd,
			"select * from distance WHERE \
			track_a_id = %li and \
			track_b_id = %li and \
			distancetype_id = %li and \
			featurevectortype_id = %li;",
			track_a_id, track_b_id, distt_id, fvt_id);

	res = myPQexec(conn, sqlcmd);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}
	if (0 == PQntuples(res))
		ret = false;
	else
		ret = true;

	PQclear(res);

	return ret;
}

long SmafeStoreDB_PQ::isTrackInDatabase(t_fingerprint fp) {
	checkOpenConnection();

	long ret;
	char sqlcmd[MAXSQLLEN];

	sprintf(sqlcmd, "select * from track WHERE fingerprint = '%s'", fp);

	res = myPQexec(conn, sqlcmd);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}
	if (0 == PQntuples(res))
		ret = -1;
	else
		// assume row 0 (fingerprint is unique so there should be only one row)
		ret = atol(PQgetvalue(res, 0, PQfnumber(res, "id")));

	PQclear(res);

	return ret;
}

bool SmafeStoreDB_PQ::isTrackInCollection(long track_id, long collection_id) {
	checkOpenConnection();

	bool ret;
	char sqlcmd[MAXSQLLEN];

	sprintf(
			sqlcmd,
			"select f.id from collection_file cf, file f, track t WHERE cf.collection_id=%li and cf.file_id = f.id and f.track_id = t.id and t.id=%li;",
			collection_id, track_id);

	res = myPQexec(conn, sqlcmd);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}
	if (0 == PQntuples(res))
		ret = false;
	else
		ret = true;

	PQclear(res);

	return ret;
}

long SmafeStoreDB_PQ::getTrackIDForFile(long file_id) {
	checkOpenConnection();

	long ret;
	char sqlcmd[MAXSQLLEN];

	sprintf(sqlcmd, "select track_id from file WHERE id = %li", file_id);

	res = myPQexec(conn, sqlcmd);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}
	if (0 == PQntuples(res))
		ret = -1;
	else
		// only one row, only one column
		ret = atol(PQgetvalue(res, 0, 0));

	PQclear(res);

	return ret;
}

long SmafeStoreDB_PQ::getCollectionId(std::string name) {
	checkOpenConnection();

	long ret;
	char sqlcmd[MAXSQLLEN];

	sprintf(sqlcmd, "select id from collection WHERE collection_name = '%s'",
			escapeString(name).c_str());

	res = myPQexec(conn, sqlcmd);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}
	if (0 == PQntuples(res))
		ret = -1;
	else
		// only one row, only one column
		ret = atol(PQgetvalue(res, 0, 0));

	PQclear(res);

	return ret;
}

size_t SmafeStoreDB_PQ::getTrackCount() {
	checkOpenConnection();

	size_t ret;
	char sqlcmd[MAXSQLLEN];

	sprintf(sqlcmd, "select count(*) from track");

	res = myPQexec(conn, sqlcmd);
	if (PQresultStatus(res) != PGRES_TUPLES_OK || 0 == PQntuples(res)) {
		bailOut(sqlcmd, res);
	}
	// only one row, only one column
	ret = atol(PQgetvalue(res, 0, 0));
	PQclear(res);
	return ret;
}
size_t SmafeStoreDB_PQ::getFileCount() {
	checkOpenConnection();

	size_t ret;
	char sqlcmd[MAXSQLLEN];

	sprintf(sqlcmd, "select count(*) from file");

	res = myPQexec(conn, sqlcmd);
	if (PQresultStatus(res) != PGRES_TUPLES_OK || 0 == PQntuples(res)) {
		bailOut(sqlcmd, res);
	}
	// only one row, only one column
	ret = atol(PQgetvalue(res, 0, 0));
	PQclear(res);
	return ret;
}


void SmafeStoreDB_PQ::getBubbleInfo(long l, long track_id, long &lBubbleId,
		long &lCount) {
	checkOpenConnection();

	char sqlcmd[MAXSQLLEN];

	sprintf(
			sqlcmd,
			"select b.id, count from bubbles%li b, track t WHERE t.bubbles%li_id = b.id and t.id = %li",
			l, l, track_id);

	res = myPQexec(conn, sqlcmd);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}
	if (0 == PQntuples(res))
		lBubbleId = -1;
	else {
		// only one row, only one column
		lBubbleId = atol(PQgetvalue(res, 0, PQfnumber(res, "id")));
		lCount = atol(PQgetvalue(res, 0, PQfnumber(res, "count")));
	}

	PQclear(res);
}
/*
 * (does not handle decryption!)
 long SmafeStoreDB_PQ::isFeatVecTypeInDatabase(const char* name, int version,
 int dim_x, int dim_y, const char* params) {
 checkOpenConnection();

 long ret;
 char sqlcmd[MAXSQLLEN];
 char* params_escaped = new char[2 * strlen(params) + 1];

 escapeString(params, params_escaped);

 sprintf(
 sqlcmd,
 "SELECT * FROM featurevectortype \
			WHERE name = '%s' \
			AND version = %i \
			AND dimension_x = %i AND dimension_y = %i \
			AND parameters = '%s'",
 name, version, dim_x, dim_y, params_escaped);

 res = myPQexec(conn, sqlcmd);
 if (PQresultStatus(res) != PGRES_TUPLES_OK) {
 bailOut(sqlcmd, res);
 }
 if (0 == PQntuples(res))
 ret = -1;
 else
 // assume row 0 (name and parameters are assumed to be unique
 // so there should be only one row)
 ret = atol(PQgetvalue(res, 0, PQfnumber(res, "id")));

 PQclear(res);
 delete[] params_escaped;

 return ret;
 }
 */

bool SmafeStoreDB_PQ::isFeatVecInDatabase(long track_id,
		long featurevectortype_id) {
	checkOpenConnection();

	long ret;
	char sqlcmd[MAXSQLLEN];

	sprintf(
			sqlcmd,
			"SELECT * FROM featurevector \
			WHERE track_id = %li AND featurevectortype_id = %li",
			track_id, featurevectortype_id);

	res = myPQexec(conn, sqlcmd);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}
	if (0 == PQntuples(res))
		ret = -1;
	else
		ret = 0;

	PQclear(res);

	return ret == 0;
}

bool SmafeStoreDB_PQ::getOpenTaskInfo(std::vector<long> *distancetype_ids,
		std::vector<long> *fvtype_ids, long &lNumVacancies,
		long &lNumCurrentVac, long &currentJob_track_id,
		long &currentJob_fvt_id, long &currentJob_dist_id) {

	checkOpenConnection();

	bool ret;
	char sqlcmd[MAXSQLLEN];



	// create where clause if we have a distid filter
	std::string where_clause_dist_ids, where_clause_fv_ids;
	if (distancetype_ids->size() > 0) {
		where_clause_dist_ids = " AND (";
		for (std::vector<long>::iterator iter = distancetype_ids->begin(); iter
		!= distancetype_ids->end(); iter++) {
			long disttype_id = *iter;
			where_clause_dist_ids += " distancetype_id=" + stringify(
					disttype_id) + " OR ";
		}
		where_clause_dist_ids = where_clause_dist_ids.substr(0,
				where_clause_dist_ids.length() - 3);
		where_clause_dist_ids += ") ";
	}
	if (fvtype_ids->size() > 0) {
		where_clause_fv_ids = " AND (";
		for (std::vector<long>::iterator iter = fvtype_ids->begin(); iter
		!= fvtype_ids->end(); iter++) {
			long fvtype_id = *iter;
			where_clause_fv_ids += " featurevectortype_id=" + stringify(
					fvtype_id) + " OR ";
		}
		where_clause_fv_ids = where_clause_fv_ids.substr(0,
				where_clause_fv_ids.length() - 3);
		where_clause_fv_ids += ") ";
	}

	// get one task currently not assigned
	sprintf(
			sqlcmd,
			"SELECT * FROM distancejob \
			WHERE status IS NULL %s %s \
			ORDER BY featurevectortype_id, distancetype_id, track_id  \
			LIMIT 1",
			where_clause_dist_ids.c_str(), where_clause_fv_ids.c_str());
	res = myPQexec(conn, sqlcmd);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}
	if (0 != PQntuples(res)) {
		currentJob_track_id = atol(PQgetvalue(res, 0, PQfnumber(res,
				"track_id")));
		currentJob_fvt_id = atol(PQgetvalue(res, 0, PQfnumber(res,
				"featurevectortype_id")));
		currentJob_dist_id = atol(PQgetvalue(res, 0, PQfnumber(res,
				"distancetype_id")));
		ret = true;

		// check for count query
		if (iLastTimeJobQuery % 10 == 0) {
			iLastTimeJobQuery = 1;
			sprintf(sqlcmd, "select count(*) from distancejob where status is null");
			PQclear(res); // clear old res
			res = myPQexec(conn, sqlcmd);
			if (PQresultStatus(res) != PGRES_TUPLES_OK) {
				bailOut(sqlcmd, res);
			}
			//only one row, one column
			lNumCurrentVac = lNumVacancies = atol(PQgetvalue(res, 0, 0));

			// get number of taken tasks
			// select count(*) from distancejob where status is not null

			// get number
			sprintf(sqlcmd,
					"SELECT count(*) FROM distancejob WHERE status IS NULL %s %s",
					where_clause_dist_ids.c_str(), where_clause_fv_ids.c_str());
			PQclear(res); // clear old res
			res = myPQexec(conn, sqlcmd);
			if (PQresultStatus(res) != PGRES_TUPLES_OK) {
				bailOut(sqlcmd, res);
			}
			lNumCurrentVac = atol(PQgetvalue(res, 0, 0));

		} else {
			// do not query for count this time
			SMAFELOG_FUNC(SMAFELOG_DEBUG, "Do not query for count this time (since iLastTimeJobQuery=" + stringify(iLastTimeJobQuery) + ")");
			// one more call here
			iLastTimeJobQuery++;
			// vlaue indicates "not known"
			lNumCurrentVac = lNumVacancies = -1;
		}
	} else {
		// no job for these params returned
		lNumCurrentVac = 0;
		lNumVacancies = -1; // todo
		ret = false;
	}

	PQclear(res);

	return ret;
}

bool SmafeStoreDB_PQ::getOpenTaskInfo_addfile(tSmafejob_addfileRecord &rec,
		long &lNumVacancies, long &lNumCurrentVac) {

	checkOpenConnection();

	bool ret;
	char sqlcmd[MAXSQLLEN];

	sprintf(
			sqlcmd,
			"SELECT * FROM smafejob_addfile \
			WHERE status IS NULL \
			ORDER BY priority DESC, created ASC  \
	LIMIT 1");
	res = myPQexec(conn, sqlcmd);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}
	if (0 != PQntuples(res)) {
		rec.id = atol(PQgetvalue(res, 0, PQfnumber(res, "id")));
		rec.priority = atol(PQgetvalue(res, 0, PQfnumber(res, "priority")));
		rec.file_uri = (PQgetvalue(res, 0, PQfnumber(res, "file_uri")));
		rec.collection_name = (PQgetvalue(res, 0, PQfnumber(res,
				"collection_name")));
		rec.log = (PQgetvalue(res, 0, PQfnumber(res, "log")));
		rec.external_key = (PQgetvalue(res, 0, PQfnumber(res,
				"external_key")));
		rec.guid = (PQgetvalue(res, 0, PQfnumber(res, "guid")));
		ret = true;


		// check for count query
		if (iLastTimeJobQuery % 10 == 0) {
			iLastTimeJobQuery = 1;
			sprintf(sqlcmd,
					"select count(*) from smafejob_addfile where status is null");
			PQclear(res); // clear old res
			res = myPQexec(conn, sqlcmd);
			if (PQresultStatus(res) != PGRES_TUPLES_OK) {
				bailOut(sqlcmd, res);
			}
			//only one row, one column
			lNumCurrentVac = lNumVacancies = atol(PQgetvalue(res, 0, 0));
		} else {
			// do not query for count this time
			SMAFELOG_FUNC(SMAFELOG_DEBUG, "Do not query for count this time (since iLastTimeJobQuery=" + stringify(iLastTimeJobQuery) + ")");
			// one more call here
			iLastTimeJobQuery++;
			// vlaue indicates "not known"
			lNumCurrentVac = lNumVacancies = -1;
		}
	} else {
		// could not get a single open job
		lNumCurrentVac = lNumVacancies = 0;
		ret = false;
	}

	PQclear(res);

	return ret;
}

bool SmafeStoreDB_PQ::getOpenTaskInfo_deletefile(
		tSmafejob_deletefileRecord &rec, long &lNumVacancies,
		long &lNumCurrentVac) {

	checkOpenConnection();

	bool ret;
	char sqlcmd[MAXSQLLEN];

	sprintf(sqlcmd,
			"select count(*) from smafejob_deletefile where status is null");
	res = myPQexec(conn, sqlcmd);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}
	//only one row, one column
	lNumCurrentVac = lNumVacancies = atol(PQgetvalue(res, 0, 0));

	// only if at least one task
	if (lNumVacancies > 0) {

		lNumCurrentVac = atol(PQgetvalue(res, 0, 0));

		sprintf(
				sqlcmd,
				"SELECT * FROM smafejob_deletefile \
				WHERE status IS NULL \
				ORDER BY priority DESC, created ASC  \
		LIMIT 1");
		PQclear(res); // clear old res
		res = myPQexec(conn, sqlcmd);
		if (PQresultStatus(res) != PGRES_TUPLES_OK) {
			bailOut(sqlcmd, res);
		}
		if (0 != PQntuples(res)) {
			rec.id = atol(PQgetvalue(res, 0, PQfnumber(res, "id")));
			rec.priority = atol(PQgetvalue(res, 0, PQfnumber(res, "priority")));
			rec.file_id = atol(PQgetvalue(res, 0, PQfnumber(res, "file_id")));
			rec.collection_name = (PQgetvalue(res, 0, PQfnumber(res,
					"collection_name")));
			rec.log = (PQgetvalue(res, 0, PQfnumber(res, "log")));
			ret = true;
		} else {
			// count said we have jobs, but the concrete query did not return results
			ret = false;
		}
	} else
		ret = false;

	PQclear(res);

	return ret;
}

bool SmafeStoreDB_PQ::getOpenTaskInfo_deletecollection(
		tSmafejob_deletecollectionRecord &rec, long &lNumVacancies,
		long &lNumCurrentVac) {

	checkOpenConnection();

	bool ret;
	char sqlcmd[MAXSQLLEN];

	sprintf(sqlcmd,
			"select count(*) from smafejob_deletecollection where status is null");
	res = myPQexec(conn, sqlcmd);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}
	//only one row, one column
	lNumCurrentVac = lNumVacancies = atol(PQgetvalue(res, 0, 0));

	// only if at least one task
	if (lNumVacancies > 0) {

		lNumCurrentVac = atol(PQgetvalue(res, 0, 0));

		sprintf(
				sqlcmd,
				"SELECT * FROM smafejob_deletecollection \
				WHERE status IS NULL \
				ORDER BY priority DESC, created ASC  \
		LIMIT 1");
		PQclear(res); // clear old res
		res = myPQexec(conn, sqlcmd);
		if (PQresultStatus(res) != PGRES_TUPLES_OK) {
			bailOut(sqlcmd, res);
		}
		if (0 != PQntuples(res)) {
			rec.id = atol(PQgetvalue(res, 0, PQfnumber(res, "id")));
			rec.priority = atol(PQgetvalue(res, 0, PQfnumber(res, "priority")));
			rec.collection_name = (PQgetvalue(res, 0, PQfnumber(res,
					"collection_name")));
			rec.log = (PQgetvalue(res, 0, PQfnumber(res, "log")));
			ret = true;
		} else {
			// count said we have jobs, but the concrete query did not return results
			ret = false;
		}
	} else
		ret = false;

	PQclear(res);

	return ret;
}

bool SmafeStoreDB_PQ::getOpenTaskInfo_smuiaddtrack(
		tSmuijob_addtrackRecord &rec, long &lNumVacancies, long &lNumCurrentVac) {

	checkOpenConnection();

	bool ret;
	char sqlcmd[MAXSQLLEN];

	sprintf(sqlcmd,
			"select count(*) from smuijob_addtrack where status is null");
	res = myPQexec(conn, sqlcmd);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}
	//only one row, one column
	lNumCurrentVac = lNumVacancies = atol(PQgetvalue(res, 0, 0));

	// only if at least one task
	if (lNumVacancies > 0) {

		lNumCurrentVac = atol(PQgetvalue(res, 0, 0));

		sprintf(
				sqlcmd,
				"SELECT * FROM smuijob_addtrack \
				WHERE status IS NULL \
				ORDER BY priority DESC, created ASC  \
		LIMIT 1");
		PQclear(res); // clear old res
		res = myPQexec(conn, sqlcmd);
		if (PQresultStatus(res) != PGRES_TUPLES_OK) {
			bailOut(sqlcmd, res);
		}
		if (0 != PQntuples(res)) {
			rec.id = atol(PQgetvalue(res, 0, PQfnumber(res, "id")));
			rec.priority = atol(PQgetvalue(res, 0, PQfnumber(res, "priority")));
			rec.track_id = atol(PQgetvalue(res, 0, PQfnumber(res, "track_id")));
			rec.log = (PQgetvalue(res, 0, PQfnumber(res, "log")));
			ret = true;
		} else {
			// count said we have jobs, but the concrete query did not return results
			ret = false;
		}
	} else
		ret = false;

	PQclear(res);

	return ret;
}

bool SmafeStoreDB_PQ::getOpenTaskInfo_smuideletetrack(
		tSmuijob_deletetrackRecord &rec, long &lNumVacancies,
		long &lNumCurrentVac) {

	checkOpenConnection();

	bool ret;
	char sqlcmd[MAXSQLLEN];

	sprintf(sqlcmd,
			"select count(*) from smuijob_deletetrack where status is null");
	res = myPQexec(conn, sqlcmd);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}
	//only one row, one column
	lNumCurrentVac = lNumVacancies = atol(PQgetvalue(res, 0, 0));

	// only if at least one task
	if (lNumVacancies > 0) {

		lNumCurrentVac = atol(PQgetvalue(res, 0, 0));

		sprintf(
				sqlcmd,
				"SELECT * FROM smuijob_deletetrack \
				WHERE status IS NULL \
				ORDER BY priority DESC, created ASC  \
		LIMIT 1");
		PQclear(res); // clear old res
		res = myPQexec(conn, sqlcmd);
		if (PQresultStatus(res) != PGRES_TUPLES_OK) {
			bailOut(sqlcmd, res);
		}
		if (0 != PQntuples(res)) {
			rec.id = atol(PQgetvalue(res, 0, PQfnumber(res, "id")));
			rec.priority = atol(PQgetvalue(res, 0, PQfnumber(res, "priority")));
			rec.track_id = atol(PQgetvalue(res, 0, PQfnumber(res, "track_id")));
			rec.log = (PQgetvalue(res, 0, PQfnumber(res, "log")));
			ret = true;
		} else {
			// count said we have jobs, but the concrete query did not return results
			ret = false;
		}
	} else
		ret = false;

	PQclear(res);

	return ret;
}

void SmafeStoreDB_PQ::getDbinfo(tDbinfoRecord &rec) {
	checkOpenConnection();

	char sqlcmd[MAXSQLLEN];

	sprintf(sqlcmd, "select * from dbinfo;");
	res = myPQexec(conn, sqlcmd);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}

	// should be only one row
	if (PQntuples(res) == 0) {
		PQclear(res);
		throw std::string("dbinfo does not contain a row.");
	}
	if (PQntuples(res) > 1) {
		SMAFELOG_FUNC(SMAFELOG_WARNING, "dbinfo table contains more than one row. Taking 1st.");
	}
	rec.numberoflayers = atol(PQgetvalue(res, 0, PQfnumber(res,
			"numberoflayers")));
	rec.dimx = atol(PQgetvalue(res, 0, PQfnumber(res, "dimx")));
	rec.dimy = atol(PQgetvalue(res, 0, PQfnumber(res, "dimy")));
	rec.bubblesagginfo = (PQgetvalue(res, 0, PQfnumber(res, "bubblesagginfo")));
	rec.labelsagginfo = (PQgetvalue(res, 0, PQfnumber(res, "labelsagginfo")));

	PQclear(res);
}

long SmafeStoreDB_PQ::getNearestBubble(long x, long y, long layer,
		long distparam) {
	checkOpenConnection();

	long ret;
	char sqlcmd[MAXSQLLEN];

	// query more than necessary for debug reasons - for the beginning we maybe want to read more in the log.
	// later a select id from is enough
	sprintf(
			sqlcmd,
			"SELECT id, ST_X(b.geom) as x, ST_Y(b.geom) as y, size \
			FROM bubbles%li b  \
			WHERE ST_DWithin(geom, ST_SetSRID(ST_MakePoint(%li, %li),4326), %li ) \
			ORDER BY  ST_Distance(geom,ST_SetSRID(ST_MakePoint(%li,%li),4326)), size desc \
			LIMIT 5",
			layer, x, y, distparam, x, y);

	res = myPQexec(conn, sqlcmd);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}
	if (0 == PQntuples(res))
		ret = -1;
	else
		// take first column, first row
		ret = atol(PQgetvalue(res, 0, 0));

	PQclear(res);

	return ret;
}

std::vector<longlongpair> SmafeStoreDB_PQ::getLowerTracksForDistanceCalc(
		long currentJob_track_id, long currentJob_fvt_id,
		long currentJob_dist_id) {

	checkOpenConnection();

	std::vector<longlongpair> ret;
	char sqlcmd[MAXSQLLEN];

	sprintf(
			sqlcmd,
			"SELECT track_id FROM featurevector  \
			WHERE  track_id < %li AND featurevectortype_id = %li",
			currentJob_track_id, currentJob_fvt_id);

	res = myPQexec(conn, sqlcmd);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}

	for (int i = 0; i < PQntuples(res); i++) {
		// only one column
		ret.push_back(longlongpair(atol(PQgetvalue(res, i, 0)), -1)); // dummy segmentnr of -1
	}

	PQclear(res);

	return ret;
}

std::vector<longlongpair> SmafeStoreDB_PQ::getOtherTracksForDistanceCalc(
		long currentJob_track_id, long currentJob_fvt_id,
		long currentJob_dist_id) {

	checkOpenConnection();

	std::vector<longlongpair> ret;
	char sqlcmd[MAXSQLLEN];

	if (currentJob_track_id >= 0) {
		sprintf(
				sqlcmd,
				"SELECT track_id FROM featurevector  \
				WHERE  track_id <> %li AND featurevectortype_id = %li",
				currentJob_track_id, currentJob_fvt_id);
	} else {
		sprintf(
				sqlcmd,
				"SELECT track_id FROM featurevector  \
				WHERE  featurevectortype_id = %li",
				currentJob_fvt_id);
	}

	res = myPQexec(conn, sqlcmd);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}

	for (int i = 0; i < PQntuples(res); i++) {
		// only one column
		ret.push_back(longlongpair(atol(PQgetvalue(res, i, 0)), -1)); // dummy segmentnr of -1
	}

	PQclear(res);

	return ret;
}

std::vector<longlongpair> SmafeStoreDB_PQ::getSegmentTracksForDistanceCalc(
		long currentJob_track_id, long currentJob_fvt_id,
		long currentJob_dist_id) {

	checkOpenConnection();

	std::vector<longlongpair> ret;
	char sqlcmd[MAXSQLLEN];

	sprintf(
			sqlcmd,
			"SELECT track_id, segmentnr FROM featurevectorsegment  \
			WHERE featurevectortype_id = %li",
			currentJob_fvt_id);

	res = myPQexec(conn, sqlcmd);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}

	for (int i = 0; i < PQntuples(res); i++) {
		ret.push_back(longlongpair(atol(PQgetvalue(res, i, 0)),  atol(PQgetvalue(res, i, 1))));
	}

	PQclear(res);

	return ret;
}

std::vector<long> SmafeStoreDB_PQ::getTrack_a_ids(long fvt_id, long dist_id) {
	checkOpenConnection();

	std::vector<long> ret;
	char sqlcmd[MAXSQLLEN];

	sprintf(
			sqlcmd,
			"SELECT DISTINCT track_a_id FROM distance  \
			WHERE  featurevectortype_id = %li and distancetype_id = %li",
			fvt_id, dist_id);

	res = myPQexec(conn, sqlcmd);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}

	for (int i = 0; i < PQntuples(res); i++) {
		// only one column
		ret.push_back(atol(PQgetvalue(res, i, 0)));
	}

	PQclear(res);

	return ret;
}

std::vector<long> SmafeStoreDB_PQ::getTrack_ids(std::string collection_name) {
	checkOpenConnection();

	std::vector<long> ret;
	char sqlcmd[MAXSQLLEN];

	if (collection_name == "") {
		sprintf(sqlcmd, "SELECT id FROM track");
	} else {
		sprintf(
				sqlcmd,
				"SELECT t.id FROM track t, file f, collection_file cf, collection c where t.id=f.track_id and f.id=cf.file_id and cf.collection_id=c.id and c.collection_name='%s'",
				escapeString(collection_name).c_str());
	}

	res = myPQexec(conn, sqlcmd);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}

	for (int i = 0; i < PQntuples(res); i++) {
		// only one column
		ret.push_back(atol(PQgetvalue(res, i, 0)));
	}

	PQclear(res);

	return ret;
}


void SmafeStoreDB_PQ::getTracksAndCollections(const long featurevectortype_id, const bool bSegments, longlong_set_deque &tracks_collections) {
	checkOpenConnection();

	char sqlcmd[MAXSQLLEN];

	if (bSegments) {
		sprintf(sqlcmd,
				"SELECT cf.collection_id, fvs.track_id, fvs.segmentnr FROM featurevectorsegment fvs, track t, file f, collection_file cf where featurevectortype_id=%li and fvs.track_id=t.id and t.id = f.track_id and cf.file_id=f.id and cf.collection_id != %li;",
				featurevectortype_id, SmafeStoreDB::RESERVEDCOLLECTIONS_REMOVED);
	} else {
		sprintf(sqlcmd,
				"SELECT cf.collection_id, fv.track_id FROM featurevector fv, track t, file f, collection_file cf where featurevectortype_id=%li and fv.track_id=t.id and t.id = f.track_id and cf.file_id=f.id and cf.collection_id != %li;",
				featurevectortype_id, SmafeStoreDB::RESERVEDCOLLECTIONS_REMOVED);

	}

	res = myPQexec(conn, sqlcmd);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}

	SMAFELOG_FUNC(SMAFELOG_DEBUG, "Number tuples returned: " + stringify( PQntuples(res)));
	long max_c_id = 0;
	// Find max collection_id
	for (int i = 0; i < PQntuples(res); i++) {
		max_c_id = std::max(max_c_id, atol(PQgetvalue(res, i, 0)));
	}
	SMAFELOG_FUNC(SMAFELOG_DEBUG2, "Max Collection_id found: " + stringify(max_c_id));
	// resize to max collection_id + some spare sets to allow for "holes"
	tracks_collections.resize(max_c_id+10);
	SMAFELOG_FUNC(SMAFELOG_DEBUG2, "Size of tracks_collections: " + stringify(tracks_collections.size()));

	// go through datarecords and
	for (int i = 0; i < PQntuples(res); i++) {
		// add track_ids to the respective set (defined by collection_id)
		long c_id, t_id, segnr;
		c_id = atol(PQgetvalue(res, i, 0));
		t_id = atol(PQgetvalue(res, i, 1));
		if (bSegments) {
			segnr = atol(PQgetvalue(res, i, 2));
		} else {
			segnr = -1;
		}
		SMAFELOG_FUNC(SMAFELOG_DEBUG2, "Collection_id=" + stringify(c_id) + ", track_id=" + stringify(t_id) + ", segmentnr=" + stringify(segnr));
		//		tracks_collections[c_id].insert(t_id); // does not work?!
		tracks_collections.at(c_id).insert(longlongpair(t_id, segnr));
	}
	SMAFELOG_FUNC(SMAFELOG_DEBUG2, "Size of tracks_collections: " + stringify(tracks_collections.size()));

	PQclear(res);
}

std::vector<long> SmafeStoreDB_PQ::getFilesForTrack(long track_id,
		bool bAlsoRemoved) {
	checkOpenConnection();

	std::vector<long> ret;
	char sqlcmd[MAXSQLLEN];

	if (bAlsoRemoved)
		sprintf(sqlcmd, "select file.id from file where track_id = %li;",
				track_id);
	else
		sprintf(
				sqlcmd,
				"select *, f.id from file f where track_id = %li and not exists (select * from collection_file cf, collection c, file f2 where f.id=f2.id  and cf.file_id = f2.id and  c.id = cf.collection_id and c.collection_name='%s');",
				track_id,
				SmafeStoreDB::aReservedCollectionNames[SmafeStoreDB::RESERVEDCOLLECTIONS_REMOVED].c_str());

	res = myPQexec(conn, sqlcmd);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}

	for (int i = 0; i < PQntuples(res); i++) {
		// only one column
		ret.push_back(atol(PQgetvalue(res, i, 0)));
	}

	PQclear(res);

	return ret;
}

std::vector<long> SmafeStoreDB_PQ::getDistance_ids() {
	checkOpenConnection();

	std::vector<long> ret;
	char sqlcmd[MAXSQLLEN];

	sprintf(sqlcmd, "SELECT id FROM distancetype");

	res = myPQexec(conn, sqlcmd);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}

	for (int i = 0; i < PQntuples(res); i++) {
		// only one column
		ret.push_back(atol(PQgetvalue(res, i, 0)));
	}

	PQclear(res);

	return ret;
}

std::vector<long> SmafeStoreDB_PQ::getFeaturevectortype_ids() {
	checkOpenConnection();

	std::vector<long> ret;
	char sqlcmd[MAXSQLLEN];

	sprintf(sqlcmd, "SELECT id FROM featurevectortype");

	res = myPQexec(conn, sqlcmd);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}

	for (int i = 0; i < PQntuples(res); i++) {
		// only one column
		ret.push_back(atol(PQgetvalue(res, i, 0)));
	}

	PQclear(res);

	return ret;
}

tKeyValueMap SmafeStoreDB_PQ::getConfigRecords() {
	checkOpenConnection();

	tKeyValueMap ret;
	char sqlcmd[MAXSQLLEN];

	sprintf(sqlcmd, "SELECT key, value FROM config;");

	res = myPQexec(conn, sqlcmd);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}

	for (int i = 0; i < PQntuples(res); i++) {
		//		try {
		//			std::string key_decr = decryptString(PQgetvalue(res, i, 0));
		//			std::string value_decr = decryptString(PQgetvalue(res, i, 1));
		std::string key = (PQgetvalue(res, i, 0));
		std::string value = (PQgetvalue(res, i, 1));

		if (ret.find(key) != ret.end()) {
			// key is already in the map, so issue a warning
			SMAFELOG_FUNC(SMAFELOG_WARNING, "Key " + key + " already found in config map. Not overwriting.");
		} else {
			// store in map, encrypted
			ret[key] = value;
		}
		//		} catch (CryptoPP::Exception& e) {
		//			SMAFELOG_FUNC(SMAFELOG_WARNING, "Error decrypting config record: " + e.GetWhat());
		//		}
	}

	PQclear(res);

	return ret;
}

std::string SmafeStoreDB_PQ::storeConfigRecord(const std::string key, const std::string value) {
	checkOpenConnection();

	char sqlcmd[MAXSQLLEN];
	std::string key_encrypted = encryptString(key.c_str());

	sprintf( // EDIT also INSERT statement below!
			sqlcmd,
			"UPDATE config SET value='%s' WHERE key = '%s'; ",
			escapeString(encryptString(value.c_str())).c_str(),
			escapeString(key_encrypted).c_str());


	res = myPQexec(conn, sqlcmd);

	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		bailOut(sqlcmd, res);
	}
	if (atol(PQcmdTuples(res)) == 0) {
		// no tuples affected: use insert statement

		PQclear(res);
		sprintf( // EDIT also UPDATE statement above!
				sqlcmd,
				"INSERT INTO config (key, value) \
				VALUES ('%s', '%s');",
				escapeString(encryptString(key.c_str())).c_str(), escapeString(encryptString(value.c_str())).c_str());

		res = myPQexec(conn, sqlcmd);

		if (PQresultStatus(res) != PGRES_COMMAND_OK) {
			bailOut(sqlcmd, res);
		}
	}

	PQclear(res);

	SMAFELOG_FUNC(SMAFELOG_DEBUG, "Storing config pair in database (" + key + ", " + value + ")");

	return key_encrypted;
}


void SmafeStoreDB_PQ::storeConfigRecord(std::string key, std::string value, std::string key_encrypted) {
	checkOpenConnection();

	char sqlcmd[MAXSQLLEN];

	sprintf(
			sqlcmd,
			"UPDATE config SET value='%s' WHERE key = '%s'; ",
			escapeString(encryptString(value.c_str())).c_str(),
			escapeString(key_encrypted).c_str());


	res = myPQexec(conn, sqlcmd);

	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		bailOut(sqlcmd, res);
	}
	if (atol(PQcmdTuples(res)) == 0) {
		// no tuples affected: unexpected
		SMAFELOG_FUNC(SMAFELOG_WARNING, "Storing (update) config pair in database (" + key + "|" + key_encrypted + ", " + value + ") FAILED.");
	} else
		SMAFELOG_FUNC(SMAFELOG_DEBUG, "Storing (update) config pair in database (" + key  + "|" + key_encrypted + ", " +  value + ")");

	PQclear(res);


}


long SmafeStoreDB_PQ::storeFileRecord(long track_id, t_filehash hash,
		tAudioformat* ad, std::string guid, std::string external_key) {
	checkOpenConnection();

	int ret;
	char sqlcmd[MAXSQLLEN];

	// if external key is not empty, surround it by single quotes and escape it
	// otherwise set it to the string NULL
	external_key = !external_key.empty() ? "'" + escapeString(external_key) + "'" : "NULL";

	// if guid is not empty, surround it by single quotes and escape it
	// otherwise set it to the string NULL
	guid = !guid.empty() ? "'" + escapeString(guid) + "'" : "NULL";

	sprintf(
			sqlcmd,
			"INSERT INTO file (hash, track_id, input_format, uri, samplef, bitrate, channels, encoding, samplebit, external_key, guid) \
			VALUES ('%s', %li, '%s', '%s', %i, %i, %i, '%s', %i, %s, %s) RETURNING id",
			hash, track_id, ad->getSummary().c_str(), escapeString(ad->label).c_str(),
			ad->iSamplerate, ad->iBitrate, ad->iChannels, ad->encoding.c_str(),
			ad->iBitsPerSample, external_key.c_str(), guid.c_str());

	res = myPQexec(conn, sqlcmd);

	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}
	ret = atol(PQgetvalue(res, 0, 0));
	PQclear(res);
	return ret;
}

long SmafeStoreDB_PQ::storeCollectionRecord(std::string coll_name) {
	checkOpenConnection();

	int ret;
	char sqlcmd[MAXSQLLEN];
	char* name_escaped = new char[2 * coll_name.size() + 1];

	escapeString(coll_name.c_str(), name_escaped);

	sprintf(
			sqlcmd,
			"INSERT INTO collection (collection_name) \
			VALUES ('%s') RETURNING id",
			name_escaped);

	res = myPQexec(conn, sqlcmd);
	delete[] name_escaped;

	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}
	ret = atol(PQgetvalue(res, 0, 0));
	PQclear(res);
	return ret;
}

void SmafeStoreDB_PQ::ensureCollectionFileInDatabase(long collection_id,
		long file_id) {
	checkOpenConnection();

	char sqlcmd[MAXSQLLEN];

	// First: check if record is in DB

	sprintf(
			sqlcmd,
			"select * from collection_file WHERE collection_id= %li AND file_id = %li",
			collection_id, file_id);

	res = myPQexec(conn, sqlcmd);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}
	if (0 == PQntuples(res)) {
		// no record in DB, so insert one
		PQclear(res);

		sprintf(
				sqlcmd,
				"INSERT INTO collection_file (collection_id, file_id) \
				VALUES (%li, %li)",
				collection_id, file_id);

		res = myPQexec(conn, sqlcmd);

		if (PQresultStatus(res) != PGRES_COMMAND_OK) {
			bailOut(sqlcmd, res);
		}
	}

	PQclear(res);
}

void SmafeStoreDB_PQ::ensureCollectionFileInDatabase_reservedname(
		int which_coll, bool bOnlyIfNoOther, long file_id) {
	checkOpenConnection();

	char sqlcmd[MAXSQLLEN];
	// insert the record or not
	bool bInsertIt = true;
	int nTuples;

	// First: check if record is in DB
	sprintf(
			sqlcmd,
			"select * from collection_file cf, collection c WHERE cf.collection_id=c.id AND c.collection_name = '%s' AND file_id = %li",
			SmafeStoreDB::aReservedCollectionNames[which_coll].c_str(), file_id);

	res = myPQexec(conn, sqlcmd);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}
	nTuples = PQntuples(res);
	PQclear(res);

	if (0 == nTuples) {
		// no record in DB, so go ahead
		if (bOnlyIfNoOther) {
			// check for other records
			sprintf(
					sqlcmd,
					"select count(*) from collection_file cf WHERE file_id = %li",
					file_id);
			res = myPQexec(conn, sqlcmd);
			if (PQresultStatus(res) != PGRES_TUPLES_OK) {
				bailOut(sqlcmd, res);
			}
			// insert only if count is 0
			bInsertIt = atol(PQgetvalue(res, 0, 0)) == 0;
			PQclear(res);
		}

		if (bInsertIt) {

			sprintf(
					sqlcmd,
					"INSERT INTO collection_file (collection_id, file_id) \
					VALUES ( (SELECT id from collection where collection_name='%s') , %li)",
					SmafeStoreDB::aReservedCollectionNames[which_coll].c_str(),
					file_id);

			res = myPQexec(conn, sqlcmd);

			if (PQresultStatus(res) != PGRES_COMMAND_OK) {
				bailOut(sqlcmd, res);
			}
			PQclear(res);
		} else {
			SMAFELOG_FUNC(SMAFELOG_DEBUG,
					"Not inserting collection_file record because there are already other records.");
		}
	}
}

/*
void SmafeStoreDB_PQ::storeConfigRecord(std::string key, std::string value) {
		storeConfigRecord(key, value, (const char*) verysecretpassphrase);
}

void SmafeStoreDB_PQ::storeConfigRecord(std::string key, std::string value, const char* pp){
	checkOpenConnection();

	char sqlcmd[MAXSQLLEN];

	sprintf(sqlcmd,
			"INSERT INTO config (key, value) \
			VALUES ('%s', '%s')",
			escapeString(encryptString(key.c_str(), pp)).c_str(), escapeString(encryptString(value.c_str(), pp)).c_str()   );

	res = myPQexec(conn, sqlcmd);

	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		bailOut(sqlcmd, res);
	}
	PQclear(res);
}
 */

long SmafeStoreDB_PQ::storeTrackRecord(t_fingerprint fingerprint) {
	checkOpenConnection();

	int ret;
	char sqlcmd[MAXSQLLEN];

	sprintf(sqlcmd,
			"INSERT INTO track (fingerprint) \
			VALUES ('%s') RETURNING id",
			fingerprint);

	res = myPQexec(conn, sqlcmd);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}
	ret = atol(PQgetvalue(res, 0, 0));
	PQclear(res);
	return ret;
}

long SmafeStoreDB_PQ::storeFeatVecTypeRecord(const char* name, int version,
		int dimx, int dimy, const char* params, const char* class_id) {
	checkOpenConnection();

	int ret;
	char sqlcmd[MAXSQLLEN];
	char* params_escaped = new char[2 * strlen(params) + 1];

	escapeString(params, params_escaped);

	sprintf(
			sqlcmd,
			"INSERT INTO featurevectortype (name, version, dimension_x, dimension_y, parameters, class_id) \
			VALUES ('%s', %i, %i, %i, '%s', '%s')  RETURNING id",
			encryptString(name).c_str(), version, dimx, dimy, encryptString(
					params_escaped).c_str(), class_id);

	res = myPQexec(conn, sqlcmd);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}
	delete[] params_escaped;
	ret = atol(PQgetvalue(res, 0, 0));
	PQclear(res);
	return ret;
}

void SmafeStoreDB_PQ::storeFeatureRecord(long track_id,
		long featurevectortype_id, const char* buffer, size_t buf_length,
		long file_id) {

	checkOpenConnection();

	char sqlcmd[MAXSQLLEN];

	sprintf(
			sqlcmd,
			"INSERT INTO featurevector \
			(track_id, featurevectortype_id, data, file_id) \
			VALUES (%li, %li, $1::bytea, %li)",
			track_id, featurevectortype_id, file_id);

	const char* params[] = { buffer };
	const int params_length[] = { buf_length };
	const int params_format[] = { 1 }; /* 0: text, 1: binary */

	SMAFELOG_FUNC(SMAFELOG_DEBUG,
			"About to execute storeFeatureRecord() for track_id=" + stringify(
					track_id) + ", featurevectortype_id=" + stringify(
							featurevectortype_id));
	SMAFELOG_FUNC(SMAFELOG_DEBUG, std::string(sqlcmd));

	// 1 param
	// OIDs not specified
	// "result" 1 (not sure what's that for
	res = PQexecParams(conn, sqlcmd, 1, NULL, params, params_length,
			params_format, 1);

	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		bailOut(sqlcmd, res);
	}
	SMAFELOG_FUNC(SMAFELOG_DEBUG, "storeFeatureRecord() for track_id="
			+ stringify(track_id) + ", featurevectortype_id=" + stringify(
					featurevectortype_id) + " successfully finished.");

	PQclear(res);
}

void SmafeStoreDB_PQ::storeFeatureSegRecord(long segmentnr, long track_id,
		long featurevectortype_id, const char* buffer, size_t buf_length,
		long file_id, long startsample, long length) {

	checkOpenConnection();

	char sqlcmd[MAXSQLLEN];

	sprintf(
			sqlcmd,
			"INSERT INTO featurevectorsegment \
			(segmentnr, track_id, featurevectortype_id, data, file_id, startsample, length) \
			VALUES (%li, %li, %li, $1::bytea, %li, %li, %li)",
			segmentnr, track_id, featurevectortype_id, file_id, startsample,
			length);

	const char* params[] = { buffer };
	const int params_length[] = { buf_length };
	const int params_format[] = { 1 }; /* 0: text, 1: binary */

	// 1 param
	// OIDs not specified
	// "result" 1 (not sure what's that for
	res = PQexecParams(conn, sqlcmd, 1, NULL, params, params_length,
			params_format, 1);
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		bailOut(sqlcmd, res);
	}

	PQclear(res);
}

void SmafeStoreDB_PQ::storeDistancejobRecords(tDistancejobRecord* djr) {
	checkOpenConnection();
	std::vector<long> disttypes;
	char sqlcmd[MAXSQLLEN];
	std::string str_smafejob_addfile_id;

	// prepare possible foreign key
	if (djr->smafejob_addfile_id >= 0)
		str_smafejob_addfile_id = stringify(djr->smafejob_addfile_id);
	else
		str_smafejob_addfile_id = "NULL";

	// First query: get all distancetype ids
	sprintf(sqlcmd, "select id from distancetype order by id");
	res = myPQexec(conn, sqlcmd);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}
	for (int i = 0; i < PQntuples(res); i++) {
		// only one column
		disttypes.push_back(atol(PQgetvalue(res, i, 0)));
	}
	PQclear(res);

	for (std::vector<long>::iterator iter = disttypes.begin(); iter
	< disttypes.end(); iter++) {

		sprintf(
				sqlcmd,
				"insert into distancejob (smafejob_addfile_id, track_id, featurevectortype_id, distancetype_id, priority) \
				VALUES (%s, %li, %li, %li, %li)",
				str_smafejob_addfile_id.c_str(), djr->track_id,
				djr->featurevectortype_id, *iter, djr->priority);

		res = myPQexec(conn, sqlcmd);
		if (PQresultStatus(res) != PGRES_COMMAND_OK) {
			bailOut(sqlcmd, res);
		}
		PQclear(res);

	} // end of iterator
}

void SmafeStoreDB_PQ::storeSmuijob_addtrackRecord(tSmuijob_addtrackRecord* rec) {
	checkOpenConnection();

	char sqlcmd[MAXSQLLEN];

	sprintf(
			sqlcmd,
			"insert into smuijob_addtrack (track_id, priority) \
			VALUES (%li, %li)",
			rec->track_id, rec->priority);

	res = myPQexec(conn, sqlcmd);
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		bailOut(sqlcmd, res);
	}
	PQclear(res);
}

void SmafeStoreDB_PQ::storeSmuijob_deletetrackRecord(
		tSmuijob_deletetrackRecord* rec) {
	checkOpenConnection();

	char sqlcmd[MAXSQLLEN];

	sprintf(
			sqlcmd,
			"insert into smuijob_deletetrack (track_id, priority) \
			VALUES (%li, %li)",
			rec->track_id, rec->priority);

	res = myPQexec(conn, sqlcmd);
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		bailOut(sqlcmd, res);
	}
	PQclear(res);
}

// version with myPQexecParams
void SmafeStoreDB_PQ::readFeatureRecord_raw(long track_id, long segmentnr,
		long featurevectortype_id, char* &buffer, size_t &buf_length,
		std::string &class_id, long &file_id, bool load_file_id) {

	checkOpenConnection();

	char sqlcmd[MAXSQLLEN];

	if (segmentnr < 0) {
		sprintf(
				sqlcmd,
				"SELECT data FROM featurevector  \
				WHERE  track_id = %li AND featurevectortype_id = %li",
				track_id, featurevectortype_id);
	} else {
		SMAFELOG_FUNC(
				SMAFELOG_DEBUG2,
				"Using featurevectorsegment table.");
		sprintf(
				sqlcmd,
				"SELECT data FROM featurevectorsegment  \
				WHERE  track_id = %li AND segmentnr = %li AND featurevectortype_id = %li",
				track_id, segmentnr, featurevectortype_id);
	}

	res = PQexecParams(conn, sqlcmd, 0, // no param
			NULL, // can be NULL
			NULL, // can be NULL
			NULL, // can be NULL
			NULL, // can be NULL
			1); // ask for binary results

	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}
	int nTuples = PQntuples(res);
	if (1 == nTuples) {
		// one row, and data is the first column also
		// so get the length
		buf_length = PQgetlength(res, 0, PQfnumber(res, "data")) + 1; // care for \0!!!
		// allocate buffer
		buffer = new char[buf_length];
		// copy result to new  buffer
		memcpy(buffer, PQgetvalue(res, 0, PQfnumber(res, "data")), buf_length-1);
		// terminate by \0
		buffer[buf_length-1] = '\0';

		/*
		 // other fields
		 class_id = ""; // std::string(PQgetvalue(res, 0, PQfnumber(res, "class_id")));

		 // file_id
		 // since the result is in binary format we have to do some weird stuff to get a
		 // real long from it
		 // (byte order is reversed, for instance)
		 size_t buf_length2 = PQgetlength(res, 0, PQfnumber(res, "file_id"));
		 // allocate buffer
		 char* buffer2 = new char[buf_length2];
		 // copy result to new  buffer
		 memcpy(buffer2, PQgetvalue(res, 0, PQfnumber(res, "file_id")), buf_length2);
		 // get long in host byte order
		 // see http://www.postgresql.org/docs/8.2/interactive/libpq-example.html
		 // example 3
		 file_id = ntohl(*((int32_t *) buffer2));
		 */

	} else {
		// error handling to be done by calling function
		buffer = NULL;
	}

	// if buffer is not NULL, make second query for ids etc
	// we do not want to handle them in one query because we would have to deal with network byte order
	// numbers. For those we would have to use a function like nthol to convert them to host byte order.
	// However, this creates an additional dependency to a system specific header

	if (buffer != NULL) {

		if (load_file_id) {

			if (segmentnr < 0) {
				sprintf(
						sqlcmd,
						"SELECT file_id FROM featurevector  \
						WHERE  track_id = %li AND featurevectortype_id = %li",
						track_id, featurevectortype_id);
			} else {
				SMAFELOG_FUNC(
						SMAFELOG_DEBUG2,
						"Using featurevectorsegment table.");
				sprintf(
						sqlcmd,
						"SELECT file_id FROM featurevectorsegment  \
						WHERE  track_id = %li AND segmentnr = %li AND featurevectortype_id = %li",
						track_id, segmentnr, featurevectortype_id);
			}




			PQclear(res); // clear old res
			res = myPQexec(conn, sqlcmd);

			if (PQresultStatus(res) != PGRES_TUPLES_OK) {
				delete[] buffer;
				bailOut(sqlcmd, res);
			}

			if (1 != PQntuples(res)) {
				// error handling to be done by calling function
				buffer = NULL;
			} else {
				// other fields
				class_id = ""; // std::string(PQgetvalue(res, 0, PQfnumber(res, "class_id")));

				// file_id
				file_id = atol(PQgetvalue(res, 0, PQfnumber(res, "file_id")));
			}
		} else {
			// do not load file id so insert dummy illegal value
			file_id = -1;
		}
	}
	PQclear(res);
}

// version with myPQexecParams
bool SmafeStoreDB_PQ::readAllFeatureRecords_raw(bool bSegments, long featurevectortype_id, long lLimitsize, long lOffset,  tRawFVMap &vBuffer) {

	checkOpenConnection();

	char sqlcmd[MAXSQLLEN];
	bool ret = true;
	char* buffer2;
	size_t buf_length2;
	//char* buffer;

	size_t buf_length;
	std::string sOrderClause, sLimitClause, sOffsetClause;

	// add clauses if parameters are set
	if (lLimitsize > 0) {
		sLimitClause = " LIMIT " + stringify(lLimitsize)  + " ";
	}
	if (lOffset > 0) {
		sOffsetClause = " OFFSET " + stringify(lOffset)  + " ";
	}
	if (!bSegments) {
		// order by is needed for limit restrictions, otherwise the order is not defined
		if (lLimitsize > 0 || lOffset > 0) {
			sOrderClause = " ORDER BY track_id";
		}
		sprintf(
				sqlcmd,
				"SELECT track_id, data FROM featurevector  \
				WHERE featurevectortype_id = %li %s %s %s",
				featurevectortype_id, sOrderClause.c_str(), sLimitClause.c_str(), sOffsetClause.c_str());
	} else {
		// order by is needed for limit restrictions, otherwise the order is not defined
		if (lLimitsize > 0 || lOffset > 0) {
			sOrderClause = " ORDER BY track_id, segmentnr";
		}
		SMAFELOG_FUNC(
				SMAFELOG_DEBUG2,
				"Using featurevectorsegment table.");
		sprintf(
				sqlcmd,
				"SELECT track_id, segmentnr, data FROM featurevectorsegment  \
				WHERE  featurevectortype_id = %li %s %s %s",
				featurevectortype_id, sOrderClause.c_str(), sLimitClause.c_str(), sOffsetClause.c_str());
	}

	SMAFELOG_FUNC(SMAFELOG_DEBUG, "Executing " + stringify(sqlcmd));

	res = PQexecParams(conn, sqlcmd, 0, // no param
			NULL, // can be NULL
			NULL, // can be NULL
			NULL, // can be NULL
			NULL, // can be NULL
			1); // ask for binary results

	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}
	int nTuples = PQntuples(res);
	SMAFELOG_FUNC(SMAFELOG_DEBUG, "Returned "+stringify(nTuples)+" feature vectors");
	if (nTuples > 0) {
		for (int i = 0; i < PQntuples(res); i++) {
			long segmentnr, track_id;

			// track_id
			// since the result is in binary format we have to do some weird stuff to get a
			// real long from it
			// (byte order is reversed, for instance)
			buf_length2 = PQgetlength(res, i, PQfnumber(res, "track_id"));
			// allocate buffer
			buffer2 = new char[buf_length2];
			// copy result to new  buffer
			memcpy(buffer2, PQgetvalue(res, i, PQfnumber(res, "track_id")), buf_length2);
			// get long in host byte order
			// see http://www.postgresql.org/docs/8.2/interactive/libpq-example.html
			// example 3
			track_id = ntohl(*((int32_t *) buffer2));
			delete[] buffer2;

			if (bSegments) {
				// segmentnr
				// since the result is in binary format we have to do some weird stuff to get a
				// real long from it
				// (byte order is reversed, for instance)
				buf_length2 = PQgetlength(res, i, PQfnumber(res, "segmentnr"));
				// allocate buffer
				buffer2 = new char[buf_length2];
				// copy result to new  buffer
				memcpy(buffer2, PQgetvalue(res, i, PQfnumber(res, "segmentnr")), buf_length2);
				// get long in host byte order
				// see http://www.postgresql.org/docs/8.2/interactive/libpq-example.html
				// example 3
				segmentnr = ntohl(*((int32_t *) buffer2));
				delete[] buffer2;
			} else {
				segmentnr = -1;
			}



			SMAFELOG_FUNC(SMAFELOG_DEBUG, "Storing featurevector in buffer (track_id=" + stringify(track_id) + ", segmentnr=" + stringify(segmentnr) + ")");
			// so get the length of data
			buf_length = PQgetlength(res, i, PQfnumber(res, "data")) + 1; // care for \0!!!

			// allocate buffer
			char_ptr buffer_ptr(new char[buf_length]);

			// check if allocation worked
			if (buffer_ptr != NULL) {
				// copy result to new  buffer
				memcpy(buffer_ptr.get(), PQgetvalue(res, i, PQfnumber(res, "data")), buf_length-1);
				// terminate by \0
				buffer_ptr.get()[buf_length-1] = '\0';

				vBuffer[longlongpair(track_id, segmentnr)] = buffer_ptr;

				SMAFELOG_FUNC(SMAFELOG_DEBUG2, "buffer contents: " + stringify(buffer_ptr.get()));

			} else {
				SMAFELOG_FUNC(SMAFELOG_WARNING, "Could not allocate buffer for featurevector of track_id " + stringify(track_id));
			}
		} // for loop
	} else { // if (nTuples > 0)
		ret = false;
	}// if (nTuples > 0), else branch

	PQclear(res);
	return ret;
}

void SmafeStoreDB_PQ::getFeatureVectorMetaInfo(SmafeAbstractFeatureVector &fv,
		long fvt_id, long track_id, long file_id) {
	checkOpenConnection();

	char sqlcmd[MAXSQLLEN];
	char file_id_where[50] = { 0 };

	if (file_id > 0)
		sprintf(file_id_where, " AND f.id = %li ", file_id);

	sprintf(
			sqlcmd,
			"select f.uri, fvt.name, fvt.version, fvt.dimension_x, fvt.dimension_y, fvt.parameters, fvt.name, fvt.class_id\
			from featurevectortype fvt, file f, track t \
			WHERE f.track_id = t.id and \
			t.id = %li and \
			fvt.id = %li \
			%s;",
			track_id, fvt_id, file_id_where);

	res = myPQexec(conn, sqlcmd);

	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}

	if (0 == PQntuples(res)) {
		PQclear(res);
		// throw exception
		throw("Could not find FeatureVectorType record.");
	} else {
		// only one row since we query for primary key
		// read data and store in object

		// debug message if more than one row
		if (PQntuples(res) > 1)
			SMAFELOG_FUNC(
					SMAFELOG_DEBUG,
					"getFeatureVectorMetaInfo: more than one file for track: I take the data from the first record");

		SmafeFVType* fvt = new SmafeFVType();
		try {
			fvt->name = decryptString(
					PQgetvalue(res, 0, PQfnumber(res, "name")));
			fvt->version = atol(PQgetvalue(res, 0, PQfnumber(res, "version")));
			fvt->dimension_x = atol(PQgetvalue(res, 0, PQfnumber(res,
					"dimension_x")));
			fvt->dimension_y = atol(PQgetvalue(res, 0, PQfnumber(res,
					"dimension_y")));
			fvt->parameters = decryptString(PQgetvalue(res, 0, PQfnumber(res,
					"parameters")));
			fvt->class_id = std::string(PQgetvalue(res, 0, PQfnumber(res,
					"class_id")));
			fv.fvtype = fvt;
			// stored in fv
			fv.file_uri
			= std::string(PQgetvalue(res, 0, PQfnumber(res, "uri")));
		} catch (CryptoPP::Exception& e) {
			PQclear(res);
			throw "Error decrypting feature vector type: " + e.GetWhat();
		}
	}
	PQclear(res);

}

void SmafeStoreDB_PQ::getDistancetypes(std::vector<tDistancetype> &vDisttypes) {
	checkOpenConnection();

	char sqlcmd[] = "SELECT id, name FROM distancetype ORDER BY id ASC;";

	res = myPQexec(conn, sqlcmd);

	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}

	int nTuples = PQntuples(res);

	for (int i = 0; i < nTuples; i++) {
		tDistancetype dt;
		dt.id = atol(PQgetvalue(res, i, 0));
		dt.name = std::string(PQgetvalue(res, i, 1));
		vDisttypes.push_back(dt);
	}
	PQclear(res);
}

void SmafeStoreDB_PQ::getFeaturevectortypes(std::vector<SmafeFVType_Ptr> &v) {
	checkOpenConnection();

	SmafeFVType_Ptr varptr;

	char
	sqlcmd[] =
			"SELECT id, name, version, dimension_x, dimension_y, parameters, class_id FROM featurevectortype ORDER BY id ASC;";

	res = myPQexec(conn, sqlcmd);

	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}

	int nTuples = PQntuples(res);

	try {

		for (int i = 0; i < nTuples; i++) {
			varptr.reset(new SmafeFVType);
			varptr->id = atol(PQgetvalue(res, i, 0));
			varptr->name = decryptString(PQgetvalue(res, i, 1));
			varptr->version = atol(PQgetvalue(res, i, 2));
			varptr->dimension_x = atol(PQgetvalue(res, i, 3));
			varptr->dimension_y = atol(PQgetvalue(res, i, 4));
			varptr->parameters = decryptString(PQgetvalue(res, i, 5));
			varptr->class_id = std::string(PQgetvalue(res, i, 6));
			v.push_back(varptr);
		}
	} catch (CryptoPP::Exception& e) {
		PQclear(res);
		throw "Error decrypting feature vector type: " + e.GetWhat();
	}
	PQclear(res);
}

void SmafeStoreDB_PQ::query_nn(std::vector<Nn_result_rs_Ptr> &nns,
		long track_id, long fvt_id, long dist_id, std::string sCollectionName,
		int skip_k, int k) {

	checkOpenConnection();

	char sqlcmd[MAXSQLLEN];

	// check if collection should be considered
	if (sCollectionName == "") {
		// no collection id restriction
		sprintf(
				sqlcmd,
				"select d.track_a_id, d.track_b_id, f.uri, d.value as distance \
				from distance d, file f \
				where \
				d.distancetype_id = %li and \
				d.featurevectortype_id = %li and \
				d.track_a_id = %li and \
				f.track_id = d.track_b_id \
				order by distance \
				limit %i \
				offset %i;",
				dist_id, fvt_id, track_id, k, skip_k);
	} else {
		// only if collection id matches

		SMAFELOG_FUNC(SMAFELOG_DEBUG, "Restricting to collection "
				+ sCollectionName);

		sprintf(
				sqlcmd,
				"select d.track_a_id, d.track_b_id, f.uri, d.value as distance \
				from distance d, file f, track t, collection_file cf, collection c \
				where \
				c.collection_name='%s' and c.id=cf.collection_id and cf.file_id = f.id and t.id = d.track_b_id and \
				d.distancetype_id = %li and \
				d.featurevectortype_id = %li and \
				d.track_a_id = %li and \
				f.track_id = d.track_b_id \
				order by distance \
				limit %i \
				offset %i;",
				sCollectionName.c_str(), dist_id, fvt_id, track_id, k, skip_k);

		SMAFELOG_FUNC(SMAFELOG_DEBUG2, "SQL: " + std::string(sqlcmd));

	}

	res = myPQexec(conn, sqlcmd);

	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}

	int nTuples = PQntuples(res);

	Nn_result_rs_Ptr nn_ptr;
	int track_a_id;

	for (int i = 0; i < nTuples; i++) {
		nn_ptr.reset(new Nn_result_rs);
		// Decide whether we use track_a_id or track_b_id as the track_id of the neighbour
		// check which one is the id of the query track, so the other one must be the neighbour's
		track_a_id = atol(PQgetvalue(res, i, 0));
		if (track_a_id == track_id)
			nn_ptr->track_id = atol(PQgetvalue(res, i, 1));
		else
			nn_ptr->track_id = track_a_id;
		nn_ptr->uri = std::string(PQgetvalue(res, i, 2));
		nn_ptr->dist = atof(PQgetvalue(res, i, 3));
		nns.push_back(nn_ptr);
	}

	PQclear(res);
}

void SmafeStoreDB_PQ::insertDistanceRecord(long track_a_id, long track_b_id,
		long fvt_id, long dist_id, double d) {
	checkOpenConnection();

	// create new entry
	tDistanceRecord* distrec = new tDistanceRecord();
	// assign values
	distrec->track_a_id = track_a_id;
	distrec->track_b_id = track_b_id;
	distrec->fvt_id = fvt_id;
	distrec->dist_id = dist_id;
	distrec->d = d;
	// wrap in smart pointer
	distrec_ptr.reset(distrec);
	// add to vector
	dists_queue_for_copy.push_back(distrec_ptr);

	SMAFELOG_FUNC(SMAFELOG_DEBUG2, "added distance record. I have now "
			+ stringify(dists_queue_for_copy.size()) + "records");
}

void SmafeStoreDB_PQ::copy_dists_to_db() {
	checkOpenConnection();

	SMAFELOG_FUNC(SMAFELOG_DEBUG, "Starting copy command for " + stringify(
			dists_queue_for_copy.size()) + " records");

	const size_t MAXCOPYBUFFER = 5 * 1024 * 1024; // 10 MB;

	char sqlcmd[MAXSQLLEN];
	char copyline[1000];
	char* copybuf;
	//char copybuf[MAXCOPYBUFFER] = {0};
	size_t linelen, buflen;
	int status;

	sprintf(
			sqlcmd,
			"COPY distance (track_a_id, track_b_id, featurevectortype_id, distancetype_id, value) FROM stdin;");

	res = myPQexec(conn, sqlcmd);

	if (PQresultStatus(res) != PGRES_COPY_IN) {
		bailOut(sqlcmd, res);
	}

	buflen = 0;
	copybuf = new char[MAXCOPYBUFFER];
	// make "empty string"
	copybuf[0] = '\0';
	for (vector_tDistanceRecord_Ptr::iterator iter =
			dists_queue_for_copy.begin(); iter < dists_queue_for_copy.end(); iter++) {
		tDistanceRecord* drec = iter->get();

		sprintf(copyline, "%li\t%li\t%li\t%li\t%f\n", drec->track_a_id,
				drec->track_b_id, drec->fvt_id, drec->dist_id, drec->d);
		SMAFELOG_FUNC(SMAFELOG_DEBUG2, "copyline: " + stringify(copyline));

		linelen = strlen(copyline);

		if (buflen + linelen > MAXCOPYBUFFER) {
			// buffer cannot hold this new copyline
			// so we pass on the current buffer to pg
			status = PQputCopyData(conn, copybuf, buflen);
			if (status != 1) {
				delete[] copybuf;
				bailOut(copyline, res);
			}
			// empty buffer
			copybuf[0] = '\0';
			buflen = 0;
		}

		// add line to buffer
		strcat(copybuf, copyline);
		// correct length (no need to do another strlen of the buffer)
		buflen += linelen;
		//std::cout << stringify(buflen) << " - " << stringify(strlen(copybuf)) << '\n';
		// assertion!
		assert(strlen(copybuf) == buflen);
	}
	// flush the rest
	if (buflen > 0) {
		status = PQputCopyData(conn, copybuf, buflen);
		if (status != 1) {
			delete[] copybuf;
			bailOut(copyline, res);
		}
	}
	delete[] copybuf;

	// EOF
	status = PQputCopyEnd(conn, NULL);
	if (status != 1) {
		bailOut("at PQputCopyEnd()", res);
	}

	// wait for result and check if ok
	PGresult *res2;

	// The documention at
	// http://www.postgresql.org/docs/8.3/static/libpq-copy.html
	// and
	// http://www.postgresql.org/docs/8.3/static/libpq-async.html
	// does not reall explain what result stati are coming back.
	// I tried it with normal opreation (no error) and with
	// an arranged error.
	// As a matter of fact in the first case a PGRES_COMMAND_OK, followed
	// by a NULL result was returned
	// In the error case  PGRES_FATAL_ERROR, followed by a NULL
	// was returned.
	// Thus, I do not see any senses in waiting for a NULL result, as described in the doc
	// 		"PQgetResult must be called repeatedly until it returns a null pointer, indicating that the command is done."

	res2 = PQgetResult(conn);
	if (res2 == NULL) {
		SMAFELOG_FUNC(
				SMAFELOG_WARNING,
				"PQgetResult returned a NULL pointer at the first invocation after COPY end. Now wth does that mean?! I continue...");
	} else {
		if (PQresultStatus(res2) != PGRES_COMMAND_OK) {
			PQclear(res);
			bailOut("after copy end", res2);
		}
	}
	PQclear(res);
	PQclear(res2);

	SMAFELOG_FUNC(SMAFELOG_DEBUG, "Finished copy command for " + stringify(
			dists_queue_for_copy.size()) + " records");

}

unsigned long SmafeStoreDB_PQ::purgeDistanceTable(long track_id, long fvt_id,
		long dist_id, size_t topk) {
	checkOpenConnection();

	char sqlcmd[MAXSQLLEN];
	unsigned long ret;

	sprintf(
			sqlcmd,
			"DELETE FROM distance WHERE track_a_id=%li and \
			track_b_id IN ( \
			select track_b_id from distance where featurevectortype_id=%li and distancetype_id=%li and track_a_id=%li order by value offset %lu) \
			and featurevectortype_id=%li and distancetype_id=%li;",
			track_id, fvt_id, dist_id, track_id, (unsigned long) topk, fvt_id,
			dist_id);

	res = myPQexec(conn, sqlcmd);

	if (PQresultStatus(res) != PGRES_COMMAND_OK && PQresultStatus(res)
	!= PGRES_TUPLES_OK) {
		bailOut(sqlcmd, res);
	}

	ret = atol(PQcmdTuples(res));

	PQclear(res);

	return ret;
}

#endif
