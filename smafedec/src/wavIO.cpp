/**
// by Evan X. Merz
// www.thisisnotalabel.com

// Example Wav file input and output
// this was written for educational purposes, but feel free to use it for anything you like
// as long as you credit me appropriately ("wav IO based on code by Evan Merz")

// if you catch any bugs in this, or improve upon it significantly, send me the changes
// at evan at thisisnotalabel dot com, so we can share your changes with the world
 */



#include "wavIO.h"


/**
// by Evan X. Merz
// www.thisisnotalabel.com

// <b>Example Wav file input and output</b>
// this was written for educational purposes, but feel free to use it for anything you like
// as long as you credit me appropriately ("wav IO based on code by Evan Merz")

// if you catch any bugs in this, or improve upon it significantly, send me the changes
// at evan at thisisnotalabel dot com, so we can share your changes with the world
 */



// get/set for the Path property
char* WavFileForIO::getPath()
{
	return myPath;
}
void WavFileForIO::setPath(const char* newPath)
{
	myPath = new char[FILENAME_MAX];
	strcpy(myPath, newPath);
}

WavFileForIO::~WavFileForIO()
{
	delete[] myPath;
	myChunkSize = NULL;
	mySubChunk1Size = NULL;
	myFormat = NULL;
	myChannels = NULL;
	mySampleRate = NULL;
	myByteRate = NULL;
	myBlockAlign = NULL;
	myBitsPerSample = NULL;
	myDataSize = NULL;
	delete[] myData; // epei
}

// empty constructor
WavFileForIO::WavFileForIO()
{
	myPath = new char[200];
}

// constructor takes a wav path
WavFileForIO::WavFileForIO(const char* tmpPath)
{
	setPath(tmpPath);
	read();
}

// constructor takes a strem
WavFileForIO::WavFileForIO(std::istream &buffer)
{
	readFromStream(buffer);
}

// constructor takes file*
WavFileForIO::WavFileForIO(FILE *inFile)
{
	readFromStream(inFile);
}

// read a wav file into this class
bool WavFileForIO::read()
{
	std::ifstream inFile( myPath, std::ios::in | std::ios::binary);

	bool b = readFromStream(inFile);

	inFile.close(); // close the input file

	return b; // this should probably be something more descriptive
}

// read a wav file into this class
bool WavFileForIO::readFromStream(std::istream &inFile)
{
	//printf("Reading wav file...\n"); // for debugging only

	inFile.seekg(4, std::ios::beg);
	inFile.read( (char*) &myChunkSize, 4 ); // read the ChunkSize
	SMAFELOG_FUNC(SMAFELOG_DEBUG, "myChunkSize: " + stringify(myChunkSize));

	inFile.seekg(16, std::ios::beg);
	inFile.read( (char*) &mySubChunk1Size, 4 ); // read the SubChunk1Size
	SMAFELOG_FUNC(SMAFELOG_DEBUG, "mySubChunk1Size: " + stringify(mySubChunk1Size));

	//inFile.seekg(20, ios::beg);
	inFile.read( (char*) &myFormat, sizeof(short) ); // read the file format.  This should be 1 for PCM
	SMAFELOG_FUNC(SMAFELOG_DEBUG, "myFormat: " + stringify(myFormat));

	//inFile.seekg(22, ios::beg);
	inFile.read( (char*) &myChannels, sizeof(short) ); // read the # of channels (1 or 2)
	SMAFELOG_FUNC(SMAFELOG_DEBUG, "myChannels: " + stringify(myChannels));

	//inFile.seekg(24, ios::beg);
	inFile.read( (char*) &mySampleRate, sizeof(int) ); // read the samplerate
	SMAFELOG_FUNC(SMAFELOG_DEBUG, "mySampleRate: " + stringify(mySampleRate));

	//inFile.seekg(28, ios::beg);
	inFile.read( (char*) &myByteRate, sizeof(int) ); // read the byterate
	SMAFELOG_FUNC(SMAFELOG_DEBUG, "myByteRate: " + stringify(myByteRate));

	//inFile.seekg(32, ios::beg);
	inFile.read( (char*) &myBlockAlign, sizeof(short) ); // read the blockalign
	SMAFELOG_FUNC(SMAFELOG_DEBUG, "myBlockAlign: " + stringify(myBlockAlign));

	//inFile.seekg(34, ios::beg);
	inFile.read( (char*) &myBitsPerSample, sizeof(short) ); // read the bitspersample
	SMAFELOG_FUNC(SMAFELOG_DEBUG, "myBitsPerSample: " + stringify(myBitsPerSample));

	// check if invalid values
	if (! (myBitsPerSample > 0)) {
		throw std::string("myBitsPerSample must be > 0, but is " + stringify(myBitsPerSample));
		SMAFELOG_FUNC(SMAFELOG_WARNING, std::string("myBitsPerSample must be > 0, but is ") + stringify(myBitsPerSample));
	}
	if (! (myChannels > 0)) {
		throw std::string("myChannels must be > 0, but is " + stringify(myChannels));
		SMAFELOG_FUNC(SMAFELOG_WARNING, std::string("myChannels must be > 0, but is ") + stringify(myChannels));
	}

	// check for beginn of data
	char tmp[3]; // space for 2 characters and \0
	inFile.seekg(40, std::ios::beg);
	inFile.read( (char*) tmp, 2 );
	tmp[2] = '\0'; // i love C :-) !
	if (0 == strcmp(tmp, "ta")) { // "ta" is remainder of "data" keyword
		// special format, data block starts at offset 0x2a (42)
		inFile.seekg(42, std::ios::beg);
		SMAFELOG_FUNC(SMAFELOG_WARNING, std::string("'Non-standard' format detected, trying heuristic"));
		SMAFELOG_FUNC(SMAFELOG_WARNING, std::string("\t (data block is assumed at offset 0x2a instead of 0x28)"));
	} else {
		// "standard", offset 0x28 (40)
		inFile.seekg(40, std::ios::beg);

	}
	inFile.read( (char*) &myDataSize, sizeof(int) ); // read the size of the data

	SMAFELOG_FUNC(SMAFELOG_DEBUG, "data size: " + stringify(myDataSize));

	// read the data chunk
	myData = new char[myDataSize];
	inFile.seekg(44, std::ios::beg);
	inFile.read(myData, myDataSize);

	return true; // this should probably be something more descriptive
}


// read a wav file from FILE * with unknown length (comes through pipe)
bool WavFileForIO::readFromStream(FILE *inFile)
{
	size_t sBytesread;
	// buffer for data chunks read from file (max 4 bytes)
	char cChunk[8];
	//printf("Reading wav file...\n"); // for debugging only

	// header 4 bytes
	sBytesread = fread (cChunk, 1, 4,inFile);
	if (sBytesread != 4) {
		throw std::string("fread failed for chunk 0. Bytes read: " + stringify(sBytesread));
	}

	//	inFile.seekg(4, std::ios::beg);
	sBytesread = fread ((char*) &myChunkSize, 1, 4,inFile);
	if (sBytesread != 4) {
		throw std::string("fread failed for chunksize. Bytes read: " + stringify(sBytesread));
	}
	SMAFELOG_FUNC(SMAFELOG_DEBUG, "myChunkSize: " + stringify(myChunkSize));

	// read bytes to advance file pointer
	// remember we cannot seek here
	// we are @ offset 8
	sBytesread = fread ((char*) &myChunkSize, 1, 8,inFile);
	if (sBytesread != 8) {
		throw std::string("fread failed for chunks format, subchunk1 id. Bytes read: " + stringify(sBytesread));
	}

	//	inFile.seekg(16, std::ios::beg);
	sBytesread = fread ((char*) &mySubChunk1Size, 1, 4,inFile);
	if (sBytesread != 4) {
		throw std::string("fread failed for chunks subchunksize. Bytes read: " + stringify(sBytesread));
	}
	SMAFELOG_FUNC(SMAFELOG_DEBUG, "mySubChunk1Size: " + stringify(mySubChunk1Size));

	//inFile.seekg(20, ios::beg);
	sBytesread = fread ((char*) &myFormat, 1, 2,inFile);
	if (sBytesread != 2) {
		throw std::string("fread failed for chunks subchunksize. Bytes read: " + stringify(sBytesread));
	}
	SMAFELOG_FUNC(SMAFELOG_DEBUG, "myFormat: " + stringify(myFormat));

	//inFile.seekg(22, ios::beg);
	sBytesread = fread ((char*) &myChannels, 1, 2,inFile);
	if (sBytesread != 2) {
		throw std::string("fread failed for chunk myChannels. Bytes read: " + stringify(sBytesread));
	}
	SMAFELOG_FUNC(SMAFELOG_DEBUG, "myChannels: " + stringify(myChannels));

	//inFile.seekg(24, ios::beg);
	sBytesread = fread ((char*) &mySampleRate, 1, 4,inFile);
	if (sBytesread != 4) {
		throw std::string("fread failed for chunk mySampleRate. Bytes read: " + stringify(sBytesread));
	}
	SMAFELOG_FUNC(SMAFELOG_DEBUG, "mySampleRate: " + stringify(mySampleRate));

	//inFile.seekg(28, ios::beg);
	sBytesread = fread ((char*) &myByteRate, 1, 4,inFile);
	if (sBytesread != 4) {
		throw std::string("fread failed for chunk myByteRate. Bytes read: " + stringify(sBytesread));
	}
	SMAFELOG_FUNC(SMAFELOG_DEBUG, "myByteRate: " + stringify(myByteRate));

	//inFile.seekg(32, ios::beg);
	sBytesread = fread ((char*) &myBlockAlign, 1, 2,inFile);
	if (sBytesread != 2) {
		throw std::string("fread failed for chunks myBlockAlign. Bytes read: " + stringify(sBytesread));
	}
	SMAFELOG_FUNC(SMAFELOG_DEBUG, "myBlockAlign: " + stringify(myBlockAlign));

	//inFile.seekg(34, ios::beg);
	sBytesread = fread ((char*) &myBitsPerSample, 1, 2,inFile);
	if (sBytesread != 2) {
		throw std::string("fread failed for chunk myBitsPerSample. Bytes read: " + stringify(sBytesread));
	}
	SMAFELOG_FUNC(SMAFELOG_DEBUG, "myBitsPerSample: " + stringify(myBitsPerSample));

	// check if invalid values
	if (! (myBitsPerSample > 0)) {
		throw std::string("myBitsPerSample must be > 0, but is " + stringify(myBitsPerSample));
		SMAFELOG_FUNC(SMAFELOG_WARNING, std::string("myBitsPerSample must be > 0, but is ") + stringify(myBitsPerSample));
	}
	if (! (myChannels > 0)) {
		throw std::string("myChannels must be > 0, but is " + stringify(myChannels));
		SMAFELOG_FUNC(SMAFELOG_WARNING, std::string("myChannels must be > 0, but is ") + stringify(myChannels));
	}

	// advance to offset 40 (decimal)
	sBytesread = fread (cChunk, 1, 4,inFile);
	if (sBytesread != 4) {
		throw std::string("fread failed for chunk subchunk2id. Bytes read: " + stringify(sBytesread));
	}

	/*
	// check for beginn of data
	char tmp[3]; // space for 2 characters and \0
//	inFile.seekg(40, std::ios::beg);
	inFile.read( (char*) tmp, 2 );
	tmp[2] = '\0'; // i love C :-) !
	if (0 == strcmp(tmp, "ta")) { // "ta" is remainder of "data" keyword
		// special format, data block starts at offset 0x2a (42)
		inFile.seekg(42, std::ios::beg);
		SMAFELOG_FUNC(SMAFELOG_WARNING, std::string("'Non-standard' format detected, trying heuristic"));
		SMAFELOG_FUNC(SMAFELOG_WARNING, std::string("\t (data block is assumed at offset 0x2a instead of 0x28)"));
	} else {
		// "standard", offset 0x28 (40)
		inFile.seekg(40, std::ios::beg);

	}
	inFile.read( (char*) &myDataSize, sizeof(int) ); // read the size of the data


	 */

	// advance to offset 44 (decimal), reading the datasize but we expect to find it 0 (cause the file is a pipe)
	sBytesread = fread ((char *) &myDataSize, 1, 4,inFile);
	if (sBytesread != 4) {
		throw std::string("fread failed for chunk subchunk2 size. Bytes read: " + stringify(sBytesread));
	}
	if (myDataSize == 0) {
		/** temporary pointer for realloc */
		void* tmpPointer;
		/** buffer size for next decode iteration */
		off_t buf_size_next_iteration;
		/** buffer offset, and buffer size, resp. */
		off_t buf_offset = 0;
		/** total size of memory allocated for buffer */
		off_t buf_size;
		/** true if loop should be stopped */
		bool bStoploop = false;

		SMAFELOG_FUNC(SMAFELOG_DEBUG, "data size is 0, reading blindfold");

		// re-alloc in 5 minutes steps (first guess is 5 minutes, then 10 minutes etc)
		buf_size_next_iteration = mySampleRate * myChannels * myBitsPerSample/8 * 60 * 5;

		// allocate initial buffer size
		myData = (char *) malloc( buf_size_next_iteration ); // c style, because we wnat to use realloc afterwards
		buf_size = buf_size_next_iteration;
		if (myData == NULL) throw std::string(" malloc( buf_size_next_iteration ) failed for buf_size_next_iteration==") + stringify(buf_size_next_iteration);

		do {
			int c;
			do {
				c = fgetc (inFile);
				if (c != EOF) {
					myData[buf_offset++] = c;
				}
			} while (c != EOF && buf_offset < buf_size);

			// Case 1: end of file
			if (c == EOF && feof(inFile)) {
				// shrink buffer to real buffer size
				SMAFELOG_FUNC(SMAFELOG_DEBUG2, "final audiobuffer size (MB): " + stringify(double(buf_offset) / 1024.0/1024.0));
				SMAFELOG_FUNC(SMAFELOG_DEBUG, "length of audiobuffer (s): " + stringify(double(buf_offset) * sizeof(double) /  double(mySampleRate) / double(myChannels) / double(myBitsPerSample)));
				tmpPointer =  realloc(myData, buf_offset-1); // buf_offset points to the current next free position

				buf_size = buf_offset;

				if (tmpPointer != NULL) {
					myData = (char*) tmpPointer;
				} else {
					throw std::string("realloc of buffer to ") + stringify(buf_offset) +
							std::string(" bytes failed.");
				}
				bStoploop = true;
			} else  {
				// Case 2: error
				if (c == EOF && ferror(inFile)) {
					throw std::string("Error reading from FILE*. Last character read = " + stringify(c) + ",  buf_offset = " + stringify(buf_offset) + ", buf_size = " + stringify(buf_size));

					bStoploop = true;
				} else {
					// Case 3: buffer was not sufficient, so enlarge
					if (buf_offset == buf_size) {
						SMAFELOG_FUNC(SMAFELOG_DEBUG2, "buffer was not sufficient, so enlarge");

						tmpPointer = realloc(myData, buf_offset + buf_size_next_iteration);
						buf_size = buf_offset + buf_size_next_iteration;
						if (tmpPointer != NULL) {
							myData = (char*) tmpPointer;
						} else {
							throw std::string("realloc of buffer to ") + stringify(buf_size) +
									std::string(" bytes failed.");

						}
					} else {
						// None of the 3 possible cases
						throw std::string("Unknown error. Last character read = " + stringify(c) + ",  buf_offset = " + stringify(buf_offset) + ", buf_size = " + stringify(buf_size));
					}
				}
			}
		} while (!bStoploop);

		myDataSize = buf_size;

	} else {
		SMAFELOG_FUNC(SMAFELOG_DEBUG, "data size: " + stringify(myDataSize));

		// read the data chunk
		myData = new char[myDataSize];
		sBytesread = fread (myData, 1, myDataSize,inFile);
		if (sBytesread != myDataSize) {
			throw std::string("fread failed for data. Bytes read: " + stringify(sBytesread));
		}
	}



	return true; // this should probably be something more descriptive
}

// write out the wav file
bool WavFileForIO::save()
{
	std::fstream myFile (myPath, std::ios::out | std::ios::binary);

	// write the wav file per the wav file format
	myFile.seekp (0, std::ios::beg);
	myFile.write ("RIFF", 4);
	myFile.write ((char*) &myChunkSize, 4);
	myFile.write ("WAVE", 4);
	myFile.write ("fmt ", 4);
	myFile.write ((char*) &mySubChunk1Size, 4);
	myFile.write ((char*) &myFormat, 2);
	myFile.write ((char*) &myChannels, 2);
	myFile.write ((char*) &mySampleRate, 4);
	myFile.write ((char*) &myByteRate, 4);
	myFile.write ((char*) &myBlockAlign, 2);
	myFile.write ((char*) &myBitsPerSample, 2);
	myFile.write ("data", 4);
	myFile.write ((char*) &myDataSize, 4);
	myFile.write (myData, myDataSize);

	return true;
}

// return a printable summary of the wav file
std::string  WavFileForIO::getSummary()
{
	char summary[250];
	sprintf(summary, " Format: %d\n Channels: %d\n SampleRate: %d\n ByteRate: %d\n BlockAlign: %d\n BitsPerSample: %d\n DataSize: %d\n", myFormat, myChannels, mySampleRate, myByteRate, myBlockAlign, myBitsPerSample, myDataSize);
	return std::string(summary);
}

// epei
// return a audiodata struct
tAudioformat WavFileForIO::getAudioformat()
{
	tAudioformat ad;
	ad.iChannels = myChannels;
	ad.iSamplerate = mySampleRate;
	ad.iDatasize = myDataSize;
	ad.iBitsPerSample = myBitsPerSample;

	// inspired by Matlab, but here it does not correspond to allocation of myData
	// which leads to a outofbounds problem
	// so we stick to the Datasize
	//ad.ulNumSamples = (myChunkSize *8) / (myBitsPerSample * myChannels);
	ad.ulNumSamples = myDataSize / (myBitsPerSample /8) / ad.iChannels;
	if (myPath != NULL)
		ad.label = std::string(myPath);
	else
		ad.label = "(NA)";
	ad.iBitrate = 0; // Not applicable for WAV
	ad.encoding = std::string("PCM");
	return ad;
}





