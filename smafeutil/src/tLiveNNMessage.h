///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2011 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// tLiveNNMessage.h
//
// Message for Nearest neighbour request - live calculation
// ------------------------------------------------------------------------
//
//
//
//
//
//
//
//
//
///////////////////////////////////////////////////////////////////////////
#pragma once


#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "smafeutil.h"
#include "smafeAbstractFeatureVector.h"
#include <cstring>
#include <stdio.h>
#include <boost/shared_ptr.hpp>









/** Message for Nearest neighbour request - live calculation
 * <p>This message contains
 * <ul>
 * <li>feature vector
 * <li>options
 * </ul>
 *   */
class DLLEXPORT tLiveNNMessage
{
public:
	tLiveNNMessage(void) {};
	tLiveNNMessage(SmafeAbstractFeatureVector* safv_, int iLivetopk_, long lCollectionId_): safv(safv_), iLivetopk(iLivetopk_),  lCollectionId(lCollectionId_) {};

	SmafeAbstractFeatureVector* safv;

	/** how many results return in live qurey */
	int iLivetopk;
	/** id of collection to be used as result */
	long lCollectionId;



private:
	// s11n stuff
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		// Developer note:
		// The s11n of this object is not persisted (=it is created and used immediately),
		// so it is probably not necessary to use versioning (=use version variable)
		//
		// If you change the serialization you must, however, also adapt the
		// resource files for the test: telnet-input-*.txt
		// They contain the encrypted serialized representation of an instance of this class.
		//
		// Example:
		// You can get the appropriate string from the logfile of the test:
		// Look out for "Start requesting nearest neighbour from daemon..." and copy the
		// text into telnet-input-1.txt


		// fv
		ar & BOOST_SERIALIZATION_NVP(safv);
		// options
		ar & BOOST_SERIALIZATION_NVP(iLivetopk);
		ar & BOOST_SERIALIZATION_NVP(lCollectionId);
	}

};

typedef boost::shared_ptr<tLiveNNMessage> tLiveNNMessage_Ptr;

