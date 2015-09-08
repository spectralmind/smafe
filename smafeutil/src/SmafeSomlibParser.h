///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2009 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// SmafeSomlibParser.h
//
// Class for parsing Somlib format
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
//#include "smafeProgress.h"

#include <string>

//#include <cstring>
//#include <ostream>
#include <fstream>




class DLLEXPORT SmafeSomlibParser {
public:
	// properties
	/** feature vector name (derived from DATA_TYPE, DATA_DIM) */
	std::string sFvName;
	/** # lines in file (XDIM*YDIM) */
	unsigned long ulFvCount;
	/** size of vectors */
	unsigned int uiFvSize;
	/** map where fvs are stored */



	// con/destructor
	SmafeSomlibParser(std::string filename_): filename(filename_) {
		// check if file exists
	}

	virtual ~SmafeSomlibParser(void) {
	}

	// methods
	/** Parses the header of the file.
	 * Sets sFvName, ulFvCount and uiFvSize.
	 * File is opened and closed again
	 */
	void parseHeader() {
		// check if filename not empty
		if (filename == "") {
			SMAFELOG_FUNC(SMAFELOG_FATAL, "Please specify somlib vector file if you enable somlib extractor.");
			exit(1);
		}

		sFvName = "not-set";
		ulFvCount = 0;
		uiFvSize = 0;

		std::string line;
		std::vector<std::string> vTokens;
		std::vector<std::string>::iterator iter;

		std::ifstream ifile(filename.c_str());

		if (ifile.is_open()) {
			while (! ifile.eof() ) {
				// read filenames from textfile and add to stream
				getline (ifile,line);
				trimWhitespace(line);
				// if not empty line and not a comment
				if (line != "" && line[0] != '#') {
					// clear vector
					vTokens.clear();

					// tokenize by whitespace
					tokenize(line, vTokens, " \t\n");

					if (vTokens[0] == "$DATA_TYPE") {
						SMAFELOG_FUNC(SMAFELOG_DEBUG, "$DATA_TYPE found: " + vTokens[1]);
						sFvName = vTokens[1];
					}
					if (vTokens[0] == "$XDIM") {
						SMAFELOG_FUNC(SMAFELOG_DEBUG, "$XDIM found: " + vTokens[1]);
						ulFvCount = convertTo<unsigned long>(vTokens[1]);
					}
					if (vTokens[0] == "$YDIM") {
						SMAFELOG_FUNC(SMAFELOG_DEBUG, "$YDIM found: " + vTokens[1]);
						if (convertTo<int>(vTokens[1]) != 1)
							throw "Expected $YDIM to be 1";
					}
					if (vTokens[0] == "$VEC_DIM") {
						SMAFELOG_FUNC(SMAFELOG_DEBUG, "$VEC_DIM found: " + vTokens[1]);
						uiFvSize = convertTo<unsigned long>(vTokens[1]);
					}
					if (vTokens[0] == "$DATA_DIM") {
						SMAFELOG_FUNC(SMAFELOG_DEBUG, "$DATA_DIM found: " + vTokens[1]);
						SMAFELOG_FUNC(SMAFELOG_INFO, "$DATA_DIM ignored.");
					}

				} // if (line != "" && line[0] != '$' && line[0] != '#')

			} // while (! ifile.eof() )

			ifile.close();

			if (ulFvCount == 0) {
				SMAFELOG_FUNC(SMAFELOG_WARNING, "$XDIM not found or parsed correctly. Continueing...");
			}
			if (uiFvSize == 0) {
				throw "$VEC_DIM not found or parsed correctly. Abort";
			}

		} else {
			throw "Cannot find somlib vector file " + filename;
		}


	}

	/** Parses the rest of the file.
	 * Fills the fv map
	 * File is opened and closed again
	 */
	void parseMain(tSomlibFVMap &fvmap, SmafeFVType *fvt_cur) {
		SmafeNumericFeatureVector_Ptr fvp;
		std::string line;
		std::vector<std::string> vTokens;
		std::vector<std::string>::iterator iter;
		// double array for feature vector values
		double buf[uiFvSize];
		unsigned int uiCntVecElem;
		unsigned long ulCntFvs = 0;;

		std::ifstream ifile(filename.c_str());

		if (ifile.is_open()) {
			while (! ifile.eof() ) {
				// read filenames from textfile and add to stream
				getline (ifile,line);
				trimWhitespace(line);
				// if not empty line and not a header line and not a comment
				if (line != "" && line[0] != '$' && line[0] != '#') {
					// clear vector
					vTokens.clear();
					uiCntVecElem = 0;

					// tokenize by whitespace
					tokenize(line, vTokens, " \t\n");

					// convert tokens to double and fill buffer
					for(iter = vTokens.begin(); uiCntVecElem < uiFvSize && iter < vTokens.end()-1; iter++) {
						buf[uiCntVecElem] = convertTo<double>(*iter);

						uiCntVecElem++;
					} // end of iterator

					//					uiCntVecElem++;

					// check
					assert(uiCntVecElem == uiFvSize);

					/*
					// check if label does not contain spaces
					if (uiCntVecElem +1 != vTokens.size()) {
						//SMAFELOG_FUNC(SMAFELOG_ERROR, "Label probably contains spaces. This is not allowed: " + *(++iter));
						throw "Label in Somlib file probably contains spaces. This is not allowed: " + *(++iter);
					}
					*/

					// create fv instance
					SmafeNumericFeatureVector *snfv = new SmafeNumericFeatureVector(buf, fvt_cur, true);
					// build file_uri
					std::string tmp;
					for( ;  iter < vTokens.end(); iter++) {
						tmp += ( std::string(*iter) + " " );
					} // end of iterator
					snfv->file_uri = trimWhitespace(tmp);
					SMAFELOG_FUNC(SMAFELOG_DEBUG, "File uri: " + snfv->file_uri);
					//					snfv->file_uri = std::string(*(++iter));

					// wrap in smart pointer
					fvp.reset(snfv);

					// add to map
					fvmap[snfv->file_uri] = fvp;

					ulCntFvs ++;
				} // if (line != "" && line[0] != '$' && line[0] != '#')

			} // while (! ifile.eof() )

			ifile.close();

			if (ulCntFvs != ulFvCount) {
				SMAFELOG_FUNC(SMAFELOG_WARNING, "Number of feature vectors do not match. Expected: " + stringify(ulFvCount) + ", found: " + stringify(ulCntFvs));
			}

		} else {
			throw "Cannot find somlib vector file " + filename;
		}
	}





private:
	/** filename */
	std::string filename;



};
