///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2010 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// smafedaemon1.h
//
// Daemon preparation
// Include file / header file - Params and includes
// ------------------------------------------------------------------------
//
//
// Version $Id: smafequery.cpp 166 2009-03-24 21:25:13Z ewald $
//
//
//
//
//
//
///////////////////////////////////////////////////////////////////////////


#pragma once

// check if we are gonne be a real daemon. Only if we have fork() on the system
#if defined(HAVE_WORKING_FORK)
#define SMAFEDISTD_REAL_DAEMON
#else
#undef SMAFEDISTD_REAL_DAEMON
#endif


// for debugging: comile as 'normal' program
//#undef SMAFEDISTD_REAL_DAEMON


// include stuff for daemon
#if defined(SMAFEDISTD_REAL_DAEMON)
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#endif

