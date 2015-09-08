///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// smafeNumericFeatureVector.h
//
// Feature Vector consisting of array of double
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


#include "smafeAbstractFeatureVector.h"
#include "smafeutil.h"
#include <cstring>
#include <stdio.h>
#include <boost/shared_ptr.hpp>

//#include <boost/serialization/export.hpp>





/** Feature Vector consisting of array of double */
class DLLEXPORT SmafeNumericFeatureVector : public SmafeAbstractFeatureVector
{
public:
	SmafeNumericFeatureVector();


	/** creates instance and uses given buffer and information about fv type.
	 * @param copy should buffer be copied or not. If true, the constructor allocates memory for the buffer which the
	 * deconstructor de-allocates
	 */
	SmafeNumericFeatureVector(double* buffer_, SmafeFVType* fvtype_, bool copy);


	virtual ~SmafeNumericFeatureVector(void);

	virtual long sizeOf() const;
	virtual void writeSomlibFileHeader(std::ofstream &outfile, size_t vector_size);
	virtual void writeSomlibFileEntry(std::ofstream &outfile);

	/** allocates buflen_ bytes for buffer */
	void init(size_t buflen_);

	double* buffer;
	/** size of buffer (number of doubles) */
	size_t buflen;





private:
	/** creates instance  with given buffer length */
	//SmafeNumericFeatureVector(size_t len);


	/** creates instance and uses given buffer and buflen.
	 * @param copy should buffer be copied or not. If true, the constructor allocates memory for the buffer which the
	 * deconstructor de-allocates
	 */
	//SmafeNumericFeatureVector(double* buffer_, size_t buflen_, bool copy);



	/** deletes buffer if it has been allocated before */
	void deleteBuffer();


	bool buffer_allocated;



	// s11n stuff
	friend class boost::serialization::access;


template<class Archive>
void save(Archive & ar, const unsigned int version) const
 {
	 ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(SmafeAbstractFeatureVector);

	//std::cout << "in SmafeNumericFeatureVector.save()" << std::endl;

	// store length of buffer
	ar & BOOST_SERIALIZATION_NVP(buflen);
	// store data itself
	for (size_t j = 0; j < buflen; j++) {
		ar & buffer[j];
		//ar & boost::serialization::make_nvp("data", buffer[j]);
	}
}




template<class Archive>
void load(Archive & ar, const unsigned int version) {
	ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(SmafeAbstractFeatureVector);

	//std::cout << "in SmafeNumericFeatureVector.load()" << std::endl;

	// get length of buffer
	ar & BOOST_SERIALIZATION_NVP(buflen);

	// create buffer with appropriate length
	deleteBuffer();
	init(buflen);

	// read data into buffer
	for (size_t j = 0; j < buflen; j++)
		ar & buffer[j];
}

    BOOST_SERIALIZATION_SPLIT_MEMBER()






/*
	// just to try it with serialize alone:
	template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
		std::cout << "in SmafeNumericFeatureVector.serialize()" << std::endl;
		ar & BOOST_SERIALIZATION_NVP(buflen);
    }
*/


};

// moved to cpp file
//BOOST_CLASS_EXPORT_GUID(SmafeNumericFeatureVector, "SmafeNumericFeatureVector")


typedef boost::shared_ptr<SmafeNumericFeatureVector> SmafeNumericFeatureVector_Ptr;

