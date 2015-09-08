///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// smafestore_specific_include_no_text.h
//
// Includes correct database header file, according to preprocessor constant defined
// Throws a compile error, if no constant or more than one is/are defined
// ------------------------------------------------------------------------
//
// $Id: smafestore_specific_include.h 153 2009-03-11 19:39:54Z wolfgang $
//
//
//
//
//
///////////////////////////////////////////////////////////////////////////

#pragma once


/* database switch
	One of SMAFE_PQ or SMAFE_SL
	must be defined as preprocessor
	constants.

	Check if that is the case
 */
#if defined(SMAFE_PQ) || defined(SMAFE_SL)
#else
#error "Please define one of the following preprocessor constants: SMAFE_PQ, SMAFE_SL"
#endif



#if defined(SMAFE_PQ)
#include "smafestoredb_pq.h"
#define SMAFE_STORE_DB_CLASS SmafeStoreDB_PQ
#endif
#if defined(SMAFE_SL)
#include "smafestoredb_sl.h"
#define SMAFE_STORE_DB_CLASS SmafeStoreDB_SL
#endif

