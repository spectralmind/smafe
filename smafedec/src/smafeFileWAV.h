///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// smafeFileWAV.h
//
// SmafeFile subclass for WAV files
// ------------------------------------------------------------------------
//
// $Id$
//
//
//
//
//
///////////////////////////////////////////////////////////////////////////

#ifndef SMAFEFILEWAV_H_
#define SMAFEFILEWAV_H_


#include "smafefile.h"
#include "wavIO.h"



class DLLEXPORT SmafeFileWAV : public SmafeFile {
public:
	/** @param filename_ the file to load */
	SmafeFileWAV(const char* filename_) : SmafeFile(filename_) {


		try {
			myWav = new WavFileForIO(filename);
			//SMAFELOG_FUNC(DEBUG, "+++creating myWav");

			audiobuffer = myWav->myData;
			audioformat = myWav->getAudioformat();

			// print a summary of the wav file
			SMAFELOG_FUNC(SMAFELOG_DEBUG, std::string("Wave details") + myWav->getSummary());
		} catch(std::string &s) {
			throw std::string("Error reading wav file: ") + std::string(filename) + ". " + s;
		} catch(...) {
			throw std::string("Error reading wav file: ") + std::string(filename);
		}


	}
	virtual ~SmafeFileWAV() {
		delete myWav;
		//SMAFELOG_FUNC(DEBUG, "+++deleting myWav");
	}

/*
	virtual void* getData(void) {
		return audiobuffer;
	}

	virtual tAudioformat getAudioformat(void) {
		return audioformat;
	}
	*/

private:
	/** the wav data */
	WavFileForIO *myWav;

};


#endif /* SMAFEFILEWAV_H_ */
