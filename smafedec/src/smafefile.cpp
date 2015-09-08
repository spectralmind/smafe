///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// smafefile.cpp
//
// Class representing one file for feature extraction, including
// filename, options, and eventually the features
// ------------------------------------------------------------------------
//
// $Id$
//
//
//
//
//
///////////////////////////////////////////////////////////////////////////


#include "smafefile.h"
#include "smafeFileWAV.h"
#include "smafeFileMP3.h"
#include "smafeFileExternal.h"


/** Singleton method to get correct type of file depending on the file name and/or contents.
 * @param filename_ c string with audio file path+name in local fs
 * @param dummy if true, only create stub instance but do not really process any file
 * @return SmafeFile instance of correct subtype. If dummy==false: includes loaded and normalized audiodata
 */
SmafeFile* SmafeFile::getInstance(const char* filename_, bool dummy) {
	// decide how to open the file

	std::string filename_str(filename_);

	SmafeFile* sf;

	try {
		if (dummy) {
			// create dummy SmafeFile
			sf =  new SmafeFile();
			char* s = new char[filename_str.size()+1];
			strcpy(s, filename_str.c_str());
			// set the filename string as data (to get unique MD5-hashes)
			sf->setAudiobuffer(s);
			// set the datasize accordingly (for md5)
			sf->getAudioformat_ptr()->iDatasize = filename_str.length();
			// set filename
			sf->getAudioformat_ptr()->label = filename_str;
			return sf;
		}

		// get lowercase version of filename
		std::string filename_lower = toLower(filename_str);
		//		std::transform(filename_str.begin(), filename_str.end(), filename_lower.begin(), charToLower);

		SMAFELOG_FUNC(SMAFELOG_INFO, filename_lower);

		if (endsWith(filename_lower, ".wav") ) {
			SMAFELOG_FUNC(SMAFELOG_INFO, "wav file format assumed as file ends in .wav");
			sf =  new SmafeFileWAV(filename_);
		} else
			if (endsWith(filename_lower, ".mp3")) {
				SMAFELOG_FUNC(SMAFELOG_INFO, "mp3 file format assumed as file ends in .mp3");
				sf =  new SmafeFileMP3(filename_);
			} else
				//if (endsWith(filename_lower, ".aac") || endsWith(filename_lower, ".m4a") ||endsWith(filename_lower, ".flac") || endsWith(filename_lower, ".ogg")    ) {
			{
				SMAFELOG_FUNC(SMAFELOG_INFO, "Using external tool for decoding.");
				sf =  new SmafeFileExternal(filename_);
			}
		//else
			//	throw std::string("File type not recognized: ") + filename_str;

		sf->normalize_audiobuffer();
	} catch (std::bad_alloc& ex) {
		throw std::string("Allocation of memory failed: ") + ex.what();
	}


	return sf;

}

/** Calls overloaded method with dummy set to false
 * @see #SmafeFile::getInstance(const char* filename_, bool dummy)
 */
SmafeFile* SmafeFile::getInstance(const char* filename_) {
	return SmafeFile::getInstance(filename_, false);
}


