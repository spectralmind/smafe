///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2011 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// smafeFileExternal.h
//
// SmafeFile subclass for files that require an external tool to be transcoded to WAV
// ------------------------------------------------------------------------
//
// $Id: $
//
//
//
//
//
///////////////////////////////////////////////////////////////////////////

#ifndef SMAFEFILEEXTERNAL_H_
#define SMAFEFILEEXTERNAL_H_


#include "smafefile.h"
#include <stdio.h>


/** Subclass of smafeFile
	for  files that will be transcoded externally */
class DLLEXPORT SmafeFileExternal : public SmafeFile {
public:
	/** @param filename_ the file to load */
	SmafeFileExternal(const char* filename_) : SmafeFile(filename_) {

		FILE *fp;
		int status;
		WavFileForIO *myWav = NULL;

		//		std::string sCommand = "pwd";
		//		std::string sArgs = "";
		std::string sCommand = "ffmpeg";
		std::string sArgs = "-i " + std::string(filename_) + "  -ac 1 -f wav pipe:1";

		std::string sCommandAndArgs = sCommand + " " + sArgs;

		SMAFELOG_FUNC(SMAFELOG_DEBUG2, "before process starting");

		// popen
		fp = popen(sCommandAndArgs.c_str(), "r"); // Linux Ubuntu 9.10: "rb" does not work!
		if (fp == NULL) {
			throw std::string("(@ popen)  Error executing ") + sCommandAndArgs;
		}
		SMAFELOG_FUNC(SMAFELOG_DEBUG2, "before new WavFileForIO");

		// "loading" from FILE* pointer (pipe)
		myWav = new WavFileForIO(fp);
		myWav->setPath(filename_);

		SMAFELOG_FUNC(SMAFELOG_DEBUG, "after new WavFileForIO");

		audiobuffer = myWav->myData;
		audioformat = myWav->getAudioformat();

		// close process
		status = pclose(fp);
		if (status == -1) {
			// Error reported by pclose()
			throw std::string("(@ pclose) Error executing ") + sCommandAndArgs;
		} else {
			if (status != 0) {
				throw std::string("Exit code = ")+stringify(status)+(". Error executing ") + sCommandAndArgs;
			}
		}

		SMAFELOG_FUNC(SMAFELOG_DEBUG2, "after process");

		// print a summary of the file
		SMAFELOG_FUNC(SMAFELOG_DEBUG, std::string("Format details: \n") + audioformat.getSummary());
	}

	virtual ~SmafeFileExternal() {
	}



private:


};


#endif /* SMAFEFILEEXTERNAL_H_ */
