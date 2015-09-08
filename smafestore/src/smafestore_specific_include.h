///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// smafestore_specific_include.h
//
// Includes correct database header file, according to preprocessor constant defined
// Throws a compile error, if no constant or more than one is/are defined
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



/* database switch
	Exactly one of SMAFE_PQ or SMAFE_SL or SMAFE_NODB
	must be defined as preprocessor
	constants.

	Check if that is the case
	*/
#if (defined(SMAFE_PQ) && !defined(SMAFE_SL) && !defined(SMAFE_NODB)) || \
	(!defined(SMAFE_PQ) && defined(SMAFE_SL) && !defined(SMAFE_NODB)) || \
	(!defined(SMAFE_PQ) && !defined(SMAFE_SL) && defined(SMAFE_NODB))
#else
	#error "Please define exactly one of the following preprocessor constants: SMAFE_PQ, SMAFE_SL, SMAFE_NODB"
#endif




#if defined(SMAFE_PQ)
	#include "smafestoredb_pq.h"
	#define SMAFE_STORE_DB_CLASS SmafeStoreDB_PQ
#endif
#if defined(SMAFE_SL)
	#error "Sqlite is currently not supported!"
	#include "smafestoredb_sl.h"
	#define SMAFE_STORE_DB_CLASS SmafeStoreDB_SL
#endif
#if defined(SMAFE_NODB)
	#define SMAFE_STORE_DB_CLASS void
#endif




