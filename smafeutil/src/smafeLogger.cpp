///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// smafeLogger.cpp
//
// Class for simple logging
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






#include "smafeLogger.h"


// definitions of static members
// msust be done outside in c++, seems so
SmafeLogger *SmafeLogger::smlog = new SmafeLogger(SmafeLogger::DEFAULT_LOGLEVEL);


