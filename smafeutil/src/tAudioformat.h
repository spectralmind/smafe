///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// tAudioformat.h
//
// struct that encapsulates info about audio stream (birate, etc)
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



#include <string>

#include "smafeExportDefs.h"

//------------------------------------------------
/** info about audio stream (birate, etc)*/
class DLLEXPORT tAudioformat{
public:
	int iChannels, iSamplerate, iDatasize, iBitsPerSample, iBitrate;
	unsigned long ulNumSamples;
	std::string label, encoding;

	std::string getSummary(void) {
		char ad_string[250];
		sprintf(ad_string, "samplefrequency: %i, channels: %i, bits/sample: %i, bitrate: %i, #samples: %ul, encoding: %s",
				iSamplerate,
				iChannels,
				iBitsPerSample,
				iBitrate,
				ulNumSamples,
				encoding.c_str()
				);

		return std::string(ad_string);

	}
};
