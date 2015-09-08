///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// smafeProgress.h
//
// Utility class to get time left estimations
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


#include <ctime>
#include <cmath>
#include <iostream>
#include <climits>

#include "smafeLogger.h"


class SmafeProgress {
public:

	/** default constructor
	 * sets all vars to 0, and start time to current time*/
	SmafeProgress(void) : total(0), processed(0), tmStart(time(NULL)) {}
	/** constructor: sets number of total events and start time to current time */
	SmafeProgress(unsigned long total_) : total(total_), processed(0), tmStart(time(NULL)) {}

	virtual ~SmafeProgress(void) {}

	/** sets nubmer of processed items */
	void setProcessed( unsigned long proc) {
		processed = proc;
	}

	/** returns estimated number of secs to go */
	unsigned long getEstTimeLeft() {
		time_t tmNow = time(NULL);
		double diff = difftime(tmNow, tmStart);
		// diff must be > 0
		if (diff > 0) {
			return (unsigned long) ceil(diff * double(total) / double(processed) - diff);
		} else {
			return 0; //LONG_MAX;
		}
	}

	/** returns estimated number of secs to go after preocessed item nubmers is set*/
	unsigned long getEstTimeLeft(unsigned long proc) {
		setProcessed(proc);
		return getEstTimeLeft();
	}



private:
	unsigned long total, processed;
	time_t tmStart;


};
