///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// smafeTextfileOutput.h
//
// Output feature vectors in somlib format
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
#include "smafeAbstractOutput.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>


class SmafeTextfileOutput :	public SmafeAbstractOutput
{
public:
	// -------------------- constants
	/** fv output files suffix */
	static const std::string FV_OUTPUT_FILES_SUFFIX;

	SmafeTextfileOutput(void) {}
	virtual ~SmafeTextfileOutput(void) {}

	/** writes feature vectors to text file (somlib format)
	 * Preconditions: The following members
	 * must be set:
	 * <ul>
	 * <li>SmafeFVType.name
	 * <li>SmafeFVType.dimension_x
	 * <li>SmafeFVType.dimension_y
	 * <li>SmafeNumericFeatureVector.buffer (with appropriate size, ie., dim_x * dim_y
	 * <li>SmafeNumericFeatureVector.file_uri
	 * </ul>
	 * <p>
	 * NB: Information for header is taken from first member of vector. It is assumed
	 * that these information is common to all other members

	 * @param fvs std::vector of SmafeAbstractFeatureVector_Ptr s (the feature vectors, all of same type)
	 * @param filename name of output file
	 * @return true if file was written, false otherwise
	 */
	virtual bool output(std::vector< SmafeAbstractFeatureVector_Ptr > fvs, std::string filename)  {
		// check for emptyness
		if (fvs.size() == 0) {
			SMAFELOG_FUNC(SMAFELOG_INFO, "... no feature vectors, not writing file " + filename);
			return false;
		}

		std::ofstream outfile (filename.c_str());

		// check if good
		if (!outfile.good())
			throw std::string("Error writing to file ") + filename;

		// --- Header (NB: first member of vector is chosen to determine meta info!)
		(void) fvs[0]->writeSomlibFileHeader(outfile, fvs.size());

		// --- Data
		// iterate through vector of feature vectors
		for(std::vector<SmafeAbstractFeatureVector_Ptr>::iterator iter = fvs.begin(); iter < fvs.end(); iter++) {
			SmafeAbstractFeatureVector* theFv = iter->get();
			(void) theFv->writeSomlibFileEntry(outfile);

		} // end of iterator

		outfile.close();
		SMAFELOG_FUNC(SMAFELOG_INFO,  filename + " has been written ("+stringify(fvs.size())+" vectors).");
		return true;
	}


	/** wrapper function for textfile output that selects feature vectors of same type and calls output function
	 *
	 * @param fvs std::vector of SmafeNumericFeatureVector_Ptr s (the feature vectors, may be multiple types)
	 * @param fvtype_name identifier of feature vector type name (e.g. RP)
	 * @param filename_prefix Prefix for output filename
	 * @param filename_suffix Suffix for output filename (The final filename will be <code>filename_prefix.fvtype_name.filename_suffix</code>)
	 */
	virtual void output(std::vector< SmafeAbstractFeatureVector_Ptr > fvs, std::string fvtype_name, std::string filename_prefix, std::string filename_suffix)  {
		// check for emptyness
		/*
		if (fvs.size() == 0)
			return;
			*/

		std::vector<SmafeAbstractFeatureVector_Ptr> fvs_to_output;
		SmafeAbstractFeatureVector_Ptr fv_ptr;

		// iterate through vector and move elements whose fvtype.name equals the requested one
		for(std::vector<SmafeAbstractFeatureVector_Ptr>::iterator iter = fvs.begin(); iter < fvs.end(); iter++) {

			SmafeAbstractFeatureVector* theFv = iter->get();

			SMAFELOG_FUNC(SMAFELOG_DEBUG2, theFv->fvtype->name);

			if (theFv->fvtype->name == fvtype_name) {
				// copy this feature vector
				//fv_ptr.reset(theFv);
			//	fv_ptr = ;
				fvs_to_output.push_back(*(iter.operator ->()));
			}

		} // end of iterator


		// call other output method
		output(fvs_to_output, filename_prefix + std::string(".") + fvtype_name + filename_suffix);


	}


};
