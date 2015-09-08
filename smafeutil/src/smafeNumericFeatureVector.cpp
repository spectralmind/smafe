///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// smafeNumericFeatureVector.cpp
//
// Abstract class for one feature vector type
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


#include "smafeNumericFeatureVector.h"
#include <boost/serialization/export.hpp>



SmafeNumericFeatureVector::SmafeNumericFeatureVector() {
	buffer_allocated = false;
	buflen = 0;
}



SmafeNumericFeatureVector::SmafeNumericFeatureVector(double* buffer_, SmafeFVType* fvtype_, bool copy) {
	//fvtype.reset(fvtype_);
	fvtype = fvtype_;
	buflen = fvtype->dimension_x * fvtype->dimension_y;
	if (copy) {
		init(buflen);
		memcpy(buffer, buffer_, buflen*sizeof(double));
	} else {
		buffer_allocated = false; // not allocated here
		buffer = buffer_;
	}
}


SmafeNumericFeatureVector::~SmafeNumericFeatureVector()
{
	if (buffer_allocated) {
		delete[] buffer;
		buffer_allocated = false;
	}
}

long SmafeNumericFeatureVector::sizeOf() const {
		return buflen * sizeof(double) + sizeof(this);
}

void SmafeNumericFeatureVector::writeSomlibFileHeader(std::ofstream &outfile, size_t vector_size) {
	std::string sDataType;
	// choose correct data type
	sDataType = std::string("audio-") + fvtype->name;
	std::transform(sDataType.begin(), sDataType.end(), sDataType.begin(), ::tolower);

	std::stringstream ss(std::stringstream::in | std::stringstream::out);

	// store in string stream
	ss << "$TYPE vec\n" <<
	"$DATA_TYPE " << sDataType << "\n" <<
	"$DATA_DIM " << fvtype->dimension_x << "x" << fvtype->dimension_y << "\n" <<
	"$XDIM " << vector_size << "\n" <<
	"$YDIM 1\n" <<
	"$VEC_DIM " << (fvtype->dimension_x * fvtype->dimension_y) << "\n";

	// move to outfile as encrypted
	outfile << encryptString(ss.str().c_str());
}

void SmafeNumericFeatureVector::writeSomlibFileEntry(std::ofstream &outfile) {
	std::stringstream ss(std::stringstream::in | std::stringstream::out);

	// iterate through feature vector
	for( int n = 0; n < fvtype->dimension_x * fvtype->dimension_y; ++n ) {
		ss << buffer[n] << " ";
	}
	// filename at end
	ss << file_uri << std::endl;

	// move stringstream contents to outifle, encrypted
	outfile << encryptString(ss.str().c_str());

	//outfile << "hello";
}


void SmafeNumericFeatureVector::init(size_t buflen_) {
	buflen = buflen_;
	buffer = new double[buflen];
	buffer_allocated = true;
}

void SmafeNumericFeatureVector::deleteBuffer() {
	if (buffer_allocated)
		delete[] buffer;
	buffer_allocated = false;
}



BOOST_CLASS_EXPORT_GUID(SmafeNumericFeatureVector, "SmafeNumericFeatureVector")

