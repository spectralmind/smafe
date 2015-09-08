///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// smafefile.h
//
// Abstract superclass representing one file for feature extraction
// Has static factory member to create concrete subclasses
// ------------------------------------------------------------------------
//
// $Id$
//
//
//
//
//
///////////////////////////////////////////////////////////////////////////

#pragma once


#include "smafeutil.h"
#include "tAudioformat.h"
#include "smafeExportDefs.h"
#include "smafeLogger.h"
#include <fstream>








/** <p>Class representing one file for feature extraction<br>
	This class is responsible for dealing with the various file types
	and provides the raw audio stream.
	<p>The constructors of the concrete subclasses must prepare
	<ul>
	<li>audiobuffer member, and
	<li>audioformat member
	</ul>
	If something goes wrong it should throw a std::string with an informtive text
	</p>

	<p>
	<b>Note</b>: currently, only wav and mp3 files are supported
	</p>
 */
class DLLEXPORT SmafeFile {
public:
	/** Static factory method that creates instance of correct subclass depending on filename
	 * suffix.
	 */
	static SmafeFile* getInstance(const char* filename_, bool dummy);

	/** Static factory method that creates instance of correct subclass depending on filename
	 * suffix.
	 */
	static SmafeFile* getInstance(const char* filename_);

	/** Default constructur. Usually not used. Returns an instance with dummy values.
	 * Used for somlib extractor
	 */
	SmafeFile() : audiobuffer_norm(NULL), audiobuffer(NULL) {
		audioformat.iBitrate = 0;
		audioformat.iChannels = 0;
		audioformat.iSamplerate = 0;
		audioformat.iBitsPerSample = 0;
		audioformat.label = "dummy-somlib";
		audioformat.encoding = "NA";
		audioformat.ulNumSamples = 0;
		audioformat.iDatasize = 0;
	};

	/** Returns information about audio format */
	virtual tAudioformat getAudioformat(void) {
		return audioformat;
	}

	/** Returns pointer to information about audio format */
	virtual tAudioformat* getAudioformat_ptr(void) {
		return &audioformat;
	}

	/** Sets audio stream data. Data is not copied! */
	virtual void setAudiobuffer(char* buf) {
		audiobuffer = buf;
	}

	/** Returns the audio stream data */
	virtual char* getAudiobuffer(void) {
		return audiobuffer;
	}

	/** Returns the normalized audio stream data */
	virtual double* getAudiobuffer_normalized(void) {
		return audiobuffer_norm;
	}

	virtual ~SmafeFile() {
		delete[] audiobuffer_norm;
		//SMAFELOG_FUNC(SMAFELOG_DEBUG, "+++deleting audiobuffer_norm");
	};


protected:

	// ---------------- properties

	/** the file to load */
	char filename[2048];

	/** struct for audio format details */
	tAudioformat audioformat;

	/** normalisierter audiobuffer */
	double* audiobuffer_norm;

	/** buffer for audio stream*/
	char* audiobuffer;

	// --------- methods
	SmafeFile(const char* filename_)  : audiobuffer_norm(NULL), audiobuffer(NULL) {
		strcpy (filename, filename_);
		// check if file exists
		std::ifstream infile(filename);
		if (infile.fail())
			throw std::string("File not found: ") + std::string(filename);
		infile.close();
	}

	/** normalize buffer, so that amplitudes are in [-1, 1) */
	void normalize_audiobuffer() {
		// normalize buffer, so that amplitudes are in [-1, 1)
		// done in Matlab's waveread
		tAudioformat audiodata = getAudioformat();


		if (audiodata.iBitsPerSample==1*8) { // buggy..
			throw std::string("normalization for 1 byte per sample buggy");
			SmafeLogger::smlog->log_alloc(audiodata.ulNumSamples*sizeof(double));
			audiobuffer_norm = new double[audiodata.ulNumSamples];
			for (unsigned long i=0; i < audiodata.ulNumSamples; i++) {
				audiobuffer_norm[i] = double((audiobuffer[i]))/128.0;
			}
		} else
			if (audiodata.iBitsPerSample==2*8) {
				SmafeLogger::smlog->log_alloc(audiodata.ulNumSamples*sizeof(double));
				audiobuffer_norm = new double[audiodata.ulNumSamples];
				SMAFELOG_FUNC(SMAFELOG_DEBUG, "finished allocating memory block of " + stringify(float(audiodata.ulNumSamples)*sizeof(double) / 1024.0 / 1024.0));
				short* shortbuf = (short*) audiobuffer;
				// mono-stereo
				if (audiodata.iChannels == 2) {
					SMAFELOG_FUNC(SMAFELOG_INFO, std::string("mixing stereo wave to mono"));
					int pos_alt_l, pos_neu;
					// go through stream and mix left and right sample to same buffer
					// (overwriting is safe)
					for (unsigned long s = 0; s < audiodata.ulNumSamples; s++) {
						// position of left sample in stereo wave
						pos_alt_l = s * 2; // * 2:  because 2 channels
						// position of mono sample in "new" wave
						pos_neu = s;

						// mean of left and right
						shortbuf[pos_neu] = short((shortbuf[pos_alt_l] + shortbuf[pos_alt_l+1] ) / 2.0);
					}
					audiodata.iChannels = 1;
				}
				if (audiodata.iChannels > 2) {
					throw std::string("wave has more than 2 channels. Aborting.\n");
				}
				for (unsigned long i=0; i < audiodata.ulNumSamples; i++) {
					audiobuffer_norm[i] = double((shortbuf[i]))/32768.0;
				}

			} else if (audiodata.iBitsPerSample==3*8) {
				throw std::string("normalization for 3 bytes per sample not implemented yet");
			}
			else if (audiodata.iBitsPerSample==4*8) {
				throw std::string("normalization for 4 bytes per sample not implemented yet");
			} else throw std::string("more than 4 bytes per sample?!");



		/*
					(taken from matlab)
					dat.Data = (dat.Data-128)/128;  % [-1,1)
					elseif BytesPerSample==2,
					dat.Data = dat.Data/32768;      % [-1,1)
					elseif BytesPerSample==3,
					dat.Data = dat.Data/(2^23);     % [-1,1)
					elseif BytesPerSample==4,
					if wavefmt.wFormatTag ~= 3,    % Type 3 32-bit is already normalized
					dat.Data = dat.Data/32768; % [-1,1)
					end
					end
		 */


	}



};

// types
typedef boost::shared_ptr<SmafeFile> SmafeFile_Ptr;


