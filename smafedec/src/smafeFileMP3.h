///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// smafeFileMP3.h
//
// SmafeFile subclass for MP3 files
// ------------------------------------------------------------------------
//
// $Id$
//
//
//
//
//
///////////////////////////////////////////////////////////////////////////

#ifndef SMAFEFILEMP3_H_
#define SMAFEFILEMP3_H_


#include "smafefile.h"
#include <mpg123.h>
#include <vector>
#include <climits>
//#include <sndfile.h>



/** Subclass of smafeFile
	for mp3 files */
class DLLEXPORT SmafeFileMP3 : public SmafeFile {
public:
	/** @param filename_ the file to load */
	SmafeFileMP3(const char* filename_) : SmafeFile(filename_) {


		mpg123_handle *mh = NULL;
		mpg123_frameinfo minfo;
		size_t done = 0;
		int  channels = 0, encoding = 0;
		long rate = 0;
		int  err  = MPG123_OK;
		/** temporary pointer for realloc */
		void* tmpPointer;
		/** additional buffer size for iterations. Usually a fraction of whole song; but if mean bitrate has to be
		 * calculated, it might be approx 1 frame */
		off_t additionalSize = 0;
		off_t expectedSize = 0;
		/** buffer size for next decode iteration */
		off_t buf_size_next_iteration;
		/** buffer offset, and buffer size, resp. */
		off_t buf_offset = 0;
		/** calculate mean bitrate (if vbr) */
		bool bCalculateBitrate = false;
		/** we need not take each frame for bitrate calculation.
		 * So: take every n'th frame
		 */
		const unsigned int BITRATE_CALC_FRAME_INTERVAL = 10;
		/** cumulative sum of bitrates */
		unsigned long bitrateSum = 0;
		/** number of bitrates summed up */
		unsigned int countBitrates = 0;

		err = mpg123_init();
		if( err != MPG123_OK || (mh = mpg123_new(NULL, &err)) == NULL
				/* Let mpg123 work with the file, that excludes MPG123_NEED_MORE messages. */
				|| mpg123_open(mh, filename) != MPG123_OK
				/* Peek into track and get first output format. */
				|| mpg123_getformat(mh, &rate, &channels, &encoding) != MPG123_OK )
		{
			// cleanup
			mpg123_close(mh);
			mpg123_delete(mh);
			mpg123_exit();

			throw std::string("libmpg123 error: ") + (mh==NULL ? std::string(mpg123_plain_strerror(err)) : std::string(mpg123_strerror(mh)));
		}

		// set desired output format
		mpg123_format_none(mh);
		//err = mpg123_format(mh, rate, MPG123_MONO, MPG123_ENC_SIGNED_16);
		err = mpg123_format(mh, rate, channels, encoding);
		if (err != MPG123_OK)
			SMAFELOG_FUNC(SMAFELOG_WARNING, "libmpg123: set format did not work!");


		//int out = mpg123_format_support  	(mh, rate, 	MPG123_ENC_SIGNED_16);
		//printf("out: %i\n", out);

		SMAFELOG_FUNC(SMAFELOG_DEBUG, "rate: " + stringify(rate) + ", channels: " + stringify(channels) + ", encoding: " + stringify(encoding));

		// check if rate, channels are 0
		if (rate < 1 || channels < 1) {
			throw std::string("Cannot decode file as mp3: ") + std::string(filename_);
		}

		// ----- create audioformat struct ------
		// get info
		mpg123_info(mh, &minfo);

		if (minfo.vbr == MPG123_ABR)
			audioformat.iBitrate = minfo.abr_rate;
		else if (minfo.vbr == MPG123_VBR) {
			bCalculateBitrate = true; // let's log the frames' bitrates and take their mean
			SMAFELOG_FUNC(SMAFELOG_DEBUG, "variable bitrate: calculating mean");
		}
		else
			audioformat.iBitrate = minfo.bitrate;

		audioformat.iChannels = channels;
		audioformat.iSamplerate = rate;
		audioformat.iBitsPerSample = 16;
		audioformat.label = std::string(filename);
		audioformat.encoding = std::string("MPEG layer ") + stringify(minfo.layer)  +
		(minfo.version == MPG123_1_0 ? std::string(" v1, ") :
			minfo.version == MPG123_2_0 ? std::string(" v2, ") : std::string(" v2.5, ")) +
			(minfo.vbr == MPG123_CBR ? std::string("cbr") :
				minfo.vbr == MPG123_VBR ? std::string("vbr") : std::string("abr"));



		// -------- initialisations ----------
		// get expected length in bytes!
		// * 2 because we have 16 bit encoding
		// * channels
		if (bCalculateBitrate) {
			additionalSize =
				expectedSize = mpg123_outblock( mh )*BITRATE_CALC_FRAME_INTERVAL;
		} else {
			expectedSize = mpg123_length(mh)*channels*2;
			additionalSize = expectedSize / 100;
		}

		// allocate buffer with expected size
		if (expectedSize != MPG123_ERR) {
			buf_size_next_iteration = expectedSize;
			SMAFELOG_FUNC(SMAFELOG_DEBUG, "Buffersize (bytes): " + stringify(buf_size_next_iteration) + ". Expected length (bytes): " + stringify(expectedSize));
		} else {
			// default to 3 minutes song
			buf_size_next_iteration = rate * channels * 2 * 60 * 3;
			SMAFELOG_FUNC(SMAFELOG_DEBUG, "Error querying for expected length (number of samples). Taking default initial buffer size.");
		}

		// check for estimataed size 0
		if (buf_size_next_iteration == 0)
			throw std::string("Expected size is 0.");

		// allocate initial buffer size
		audiobuffer = (char *) malloc( buf_size_next_iteration ); // c style, because we wnat to use realloc afterwards
		//bitrates = (int*) malloc (buf_size_next_iteration / 2 / channels);
		//buffer = new unsigned char[buffer_size]; // c++ style

		if (audiobuffer == NULL) throw std::string(" malloc( buf_size_next_iteration ) failed for buf_size_next_iteration==") + stringify(buf_size_next_iteration);


		SMAFELOG_FUNC(SMAFELOG_DEBUG, "Decoding started...");
		buf_offset = 0;
		do
		{
			// decode as much as possible to fill the buffer
			err = mpg123_read( mh, (unsigned char*) &(audiobuffer[buf_offset]), buf_size_next_iteration, &done );

			// debug message
			//samples += done/sizeof(short);
			//SMAFELOG_FUNC(DEBUG, "samples: " + stringify(samples));

			if (bCalculateBitrate) {
				// get info
				mpg123_info(mh, &minfo);
				//SMAFELOG_FUNC(DEBUG, "bitrate is " + stringify(minfo.bitrate));
				//bitrates.push_back(minfo.bitrate);
				// check for overflow
				if (ULONG_MAX - minfo.bitrate > bitrateSum) {
					bitrateSum += minfo.bitrate;
					countBitrates++;
				}
			}

			// increase buffer offset to new value
			buf_offset += done;

			//frameCounter++;

			if (err == MPG123_OK) {
				// buffer was not sufficient, so enlarge
				SMAFELOG_FUNC(SMAFELOG_DEBUG2, "buffer was not sufficient, so enlarge");

				buf_size_next_iteration = additionalSize;
				tmpPointer = realloc(audiobuffer, buf_offset + buf_size_next_iteration);
				if (tmpPointer != NULL) {
					audiobuffer = (char*) tmpPointer;
				} else {
					//free(audiobuffer);
					mpg123_close(mh);
					mpg123_delete(mh);
					mpg123_exit();
					throw std::string("realloc of buffer to ") + stringify(buf_offset + buf_size_next_iteration) +
					std::string(" bytes failed.");

				}

			}
			// We are not in feeder mode, so MPG123_OK, MPG123_ERR and MPG123_NEW_FORMAT are the only possibilities.
			// We do not handle a new format, MPG123_DONE is the end... so abort on anything not MPG123_OK.
		} while (err==MPG123_OK);



		// shrink buffer to real buffer size
		SMAFELOG_FUNC(SMAFELOG_DEBUG2, "final audiobuffer size (MB): " + stringify(double(buf_offset) / 1024.0/1024.0));
		SMAFELOG_FUNC(SMAFELOG_DEBUG, "length of audiobuffer (s): " + stringify(double(buf_offset) * sizeof(double) /  double(audioformat.iSamplerate) / double(audioformat.iChannels) / double(audioformat.iBitsPerSample)));
		tmpPointer =  realloc(audiobuffer, buf_offset);
		if (tmpPointer != NULL) {
			audiobuffer = (char*) tmpPointer;
		} else {
			//free(audiobuffer);
			mpg123_close(mh);
			mpg123_delete(mh);
			mpg123_exit();
			throw std::string("realloc of buffer to ") + stringify(buf_offset) +
			std::string(" bytes failed.");

		}

		// check for error
		if(err != MPG123_DONE) {
			SMAFELOG_FUNC(SMAFELOG_WARNING, std::string("libmpg123: Decoding ended prematurely because:")
			+ (mh==NULL ? std::string(mpg123_plain_strerror(err)) : std::string(mpg123_strerror(mh))));
		}

		// calculate mean bitrate if appropriate
		if (bCalculateBitrate) {
			audioformat.iBitrate = int(bitrateSum / countBitrates);
		}

		// rest of audioformat
		audioformat.iDatasize = buf_offset;
		audioformat.ulNumSamples = audioformat.iDatasize / (audioformat.iBitsPerSample /8) / audioformat.iChannels;

		//cleanup
		// don't check for error code anymore here
		mpg123_close(mh);
		mpg123_delete(mh);
		mpg123_exit();

		SMAFELOG_FUNC(SMAFELOG_DEBUG, "...decoding finished");

		// print a summary of the file
		SMAFELOG_FUNC(SMAFELOG_DEBUG, std::string("Format details: \n") + audioformat.getSummary());
	}

	virtual ~SmafeFileMP3() {
		free(audiobuffer); // this has been allocated with malloc and realloc!
	}



private:


};


#endif /* SMAFEFILEMP3_H_ */
