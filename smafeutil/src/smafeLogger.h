///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// smafeLogger.h
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





#pragma once

#include "smafeExportDefs.h"
#include "smafeutil.h"
#include "smafeProgress.h"

#include <cstring>
#include <ostream>
#include <fstream>



// macro to avoid overhead for not logged messages (rendering of log message)
// is surrounded by do {...} while (0) to make it possible to add ; at the end of macro
// usage: SMAFELOG_FUNC(LL_DEBUG, "debug message, etc");
#define SMAFELOG_FUNC_GENERIC(_smafelogger, _level, _output) do { if (SmafeLogger::_level >= (_smafelogger).getLoglevel()) { (_smafelogger).log(SmafeLogger::_level, _output); } } while(0)
//#define SMAFELOG_FUNC(_level, _output) SMAFELOG_FUNC_GENERIC(*smlog, _level, _output)
#define SMAFELOG_FUNC(_level, _output) SMAFELOG_FUNC_GENERIC(*SmafeLogger::smlog, _level, _output)

//#define SMAFELOG_TIME_FUNC(_begintime, _output) SMAFELOG_FUNC_GENERIC(*smlog, SMAFELOG_DEBUG, "lap time '" + std::string(_output) + "': " + stringify(diffclock(clock(),_begintime)) + " ms.")
#define SMAFELOG_TIME_FUNC(_begintime, _output) SMAFELOG_FUNC(SMAFELOG_DEBUG, "lap time '" + std::string(_output) + "': " + stringify(diffclock(clock(),_begintime)) + " ms.")

class DLLEXPORT SmafeLogger {
public:

	// properties
	// log levels
	const static int SMAFELOG_DEBUG3=0;		/** log all debug messages */
	const static int SMAFELOG_DEBUG2=1;		/** log more debug messages */
	const static int SMAFELOG_DEBUG=2;		/** log debug messages */
	const static int SMAFELOG_INFO=3;		/** information messages */
	const static int SMAFELOG_WARNING=4;	/** warnings */
	const static int SMAFELOG_ERROR=5;		/** not fatal errors (results in skipping file) */
	const static int SMAFELOG_FATAL=6;		/** fatal errors (program exits) */
	const static int SMAFELOG_SILENT=7;		/** silent mode */

	// "meta" levels
	const static int MIN_LOGLEVEL = SMAFELOG_DEBUG3;
	const static int DEFAULT_LOGLEVEL = SMAFELOG_INFO;
	const static int MAX_LOGLEVEL = SMAFELOG_SILENT;


	// static public instance
	static SmafeLogger *smlog;

	// methods
	SmafeLogger(int ll) :
		logfilename(""),
		dest(&std::cerr),
		desttmp(NULL),
		loglevel_chosen(ll)
		{}

	/** @throws std::string in case that the log file cannot be created */
	SmafeLogger(std::string logfilename, int ll) :
		logfilename(logfilename),
		desttmp(NULL),
		loglevel_chosen(ll)
		{
		// open stream
		std::ofstream* destfile = new std::ofstream();
		destfile->exceptions ( std::ofstream::eofbit | std::ofstream::failbit | std::ofstream::badbit );
		try {
			destfile->open(logfilename.c_str());
		} catch (std::ios_base::failure& e) {
			throw "Could not create log file: " + logfilename + " (" + std::string(e.what()) + ")";
		}
		if (!destfile->good())
			throw "Could not create log file " + logfilename;
		dest = destfile;
		}
	virtual ~SmafeLogger(void) {
		// if we have a filename, we close the file
		if (dest != std::cerr) {
			std::ofstream* destfile_ptr;
			destfile_ptr = static_cast<std::ofstream*>(dest);
			if (destfile_ptr->is_open()) {
				destfile_ptr->flush();
				destfile_ptr->close();
			}
		}
	}


	/** Sets new log level. This mehtod checks if requested log level is in range.
	 * If it is not valid, a Warning is logged and the level is not changed.
	 * @param ll the requested loglevel
	 */
	void setLoglevel(int ll) {
		if (ll != loglevel_chosen) {
			if (ll >= MIN_LOGLEVEL && ll <= MAX_LOGLEVEL) {
				loglevel_chosen = ll;
				log(SMAFELOG_INFO, "Log level changed to " + stringify(loglevel_chosen));
			} else {
				// not in range
				log(SMAFELOG_WARNING, "Could not change log level. Requested level is out of range: " + stringify(ll));
			}
		} else {
			// no change
			log(SMAFELOG_DEBUG2, "No change of loglevel.");
		}
	}
	/** Returns the current log level */
	int getLoglevel() {
		return loglevel_chosen;
	}

	/** Sets temporary location
	 * At the begin, if the argument is not NULL, add the given string with some bars */
	void setDesttmp(std::ostream* _desttmp, std::string oldlog, std::string sectionname) {
		desttmp = _desttmp;
		if (desttmp != NULL) {
			*desttmp << oldlog <<
					"-------------------------------\n" <<
					sectionname <<
					"\n-------------------------------\n";
		}
	}

	/** Sets temporary location to NULL
	 * At the begin, if the argument is not NULL, add the given string with some bars */
	void resetDesttmp() {
		setDesttmp(NULL, "", "");
	}


	/** Append a log message to the log with the given log level
	 * @param level The log level of this message
	 * @param text The log message
	 */
	void log(const int level, std::string text) {
		if (level >= loglevel_chosen) {
			const char* timeStringFormat = "%Y-%m-%d %H:%M:%S";
			const int timeStringLength = 20;
			char timeString[timeStringLength];

			time_t t = time(NULL);		// get current date and time
			tm *curTime = localtime(&t);

			char levelIndicator[3];
			switch (level) {
			case SMAFELOG_DEBUG3:
				strcpy(levelIndicator, "d3");
				break;
			case SMAFELOG_DEBUG2:
				strcpy(levelIndicator, "d2");
				break;
			case SMAFELOG_DEBUG:
				strcpy(levelIndicator, "dd");
				break;
			case SMAFELOG_INFO:
				strcpy(levelIndicator, "ii");
				break;
			case SMAFELOG_WARNING:
				strcpy(levelIndicator, "WW");
				break;
			case SMAFELOG_ERROR:
				strcpy(levelIndicator, "EE");
				break;
			case SMAFELOG_FATAL:
				strcpy(levelIndicator, "FF");
				break;
			default:
				strcpy(levelIndicator, "??");
			}

			strftime(timeString, timeStringLength, timeStringFormat, curTime);
			std::stringstream ss(std::stringstream::in | std::stringstream::out);
			ss << timeString << " " << levelIndicator << " " << text << std::endl;
			*dest << ss.str() << std::flush;
			// also log to temporary if not null
			if (desttmp != NULL) {
				*desttmp << ss.str();
			}
		}
	}


	void log_alloc(size_t size) {
		log(SMAFELOG_DEBUG2, "trying to allocate memory block (in MB): " + stringify(float(size) / 1024.0 / 1024.0));
	}

	void log_esttimeleft(const int level, SmafeProgress *progr) {
		unsigned long secs = progr->getEstTimeLeft();
		if (secs > 0) {

			time_t timetEst = time(NULL)+secs;
			tm *tmEst = localtime(&timetEst);
			const char* timeStringFormat = "%Y-%m-%d %H:%M:%S";
			const int timeStringLength = 20;
			char timeString[timeStringLength];
			strftime(timeString, timeStringLength, timeStringFormat, tmEst);

			log(level, "Estimated end time is " + std::string(timeString) + ". (" + stringify(double(secs)/double(60)) + " minutes)");
		}
	}

private:
	/** filename */
	std::string logfilename;
	/** the destination */
	std::ostream* dest;
	/** temporary destination (log snippets) */
	std::ostream* desttmp;
	/** the current loglevel */
	int loglevel_chosen;
};
