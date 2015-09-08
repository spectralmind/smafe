///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// smafeExportDefs.h
//
// Platform specific constant for shared lib function eporting
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


/* define shared lib export stuff,
 * this is platform specific!
 */
#if defined( WIN64   ) || defined( _WIN64   ) || \
    defined( WIN32   ) || defined( _WIN32   ) || \
    defined( WINDOWS ) || defined( _WINDOWS )
 #define DLLEXPORT   __declspec(dllexport)
#else
 #define DLLEXPORT
#endif
