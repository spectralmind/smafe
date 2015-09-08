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

#include "smafestoredb.h"
#include <sqlite3.h>



/** Class that encapsulates database communication */
class SmafeStoreDB_SL: public SmafeStoreDB
{
public:
	SmafeStoreDB_SL(void);
	~SmafeStoreDB_SL(void);

	virtual void openConnection();
	virtual long isFileInDatabase(t_filehash hash);
	virtual long isTrackInDatabase(t_fingerprint fp);




private:
	void checkOpenConnection();

	sqlite3 *db;
	int rc;

	const char *conninfo;


	//void bailOut(




};
