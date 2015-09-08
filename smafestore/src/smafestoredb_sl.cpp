///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// smafestoredb_sl.cpp
//
// SQLite specific db communication
// This file is only compiled if SMAFE_SL is defined
// ------------------------------------------------------------------------
//
// $Id$
//
//
//
//
//
///////////////////////////////////////////////////////////////////////////


#if defined(SMAFE_SL)

//#include "smafewrapheader.h"
#include "smafestoredb_sl.h"



SmafeStoreDB_SL::SmafeStoreDB_SL() {
}

SmafeStoreDB_SL::~SmafeStoreDB_SL()
{

}


void SmafeStoreDB_SL::checkOpenConnection() {
	if (db == NULL) throw string("Database connection not ready (SQLite)");
}


void SmafeStoreDB_SL::openConnection() {
	SMAFELOG_FUNC(INFO, string("Trying sqlite3_open..."));

	// WEIRD THINGS HAPPEN IN HERE...

	rc = 0;

	/* Make a connection to the database */
	rc = sqlite3_open("smafestore", &db);

	if( rc ){
		char tmp[250];
		sprintf(tmp, "Connection to database failed: %s", sqlite3_errmsg(db));
		sqlite3_close(db);
		throw(string(tmp));
	}
}



long SmafeStoreDB_SL::isFileInDatabase(t_filehash hash) {
	checkOpenConnection();

	bool b = false;


	return b;
}




int SmafeStoreDB_SL::isTrackInDatabase(t_fingerprint fp) {
	checkOpenConnection();

	int ret = -1;


	return ret;
}

#endif
