///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// smafeutil
//
// Utiliy functions
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


/** created by configure script */
#include "config.h"

#include <iostream>
#include <vector>
#include <deque>
#include <set>
#include <sstream>
#include <algorithm>
//#include <istringstream>
//#include "md5.h"
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <cryptopp/md5.h>
#include <ctime>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <string>
// crypto++
#include "default.h"
#include <base64.h>

// to check boost version
#include <boost/version.hpp>


#include "smafeExportDefs.h"
#include "smafeAbstractFeatureVector.h"


#if defined( WIN64   ) || defined( _WIN64   ) || \
    defined( WIN32   ) || defined( _WIN32   ) || \
    defined( WINDOWS ) || defined( _WINDOWS )
	#include <process.h>
#else
	#include <unistd.h>
#endif




/* typedefs */
/** fingerprint type */
typedef char t_fingerprint[33];
/** hashvalue type (file) */
typedef char t_filehash[33];
/** long, long pair, part of primkey of track or tracksegment. First long is track_id, second long is segmentnr */
typedef std::pair<long, long> longlongpair;
/**  pair of double, longlongpair */
typedef std::pair<double, longlongpair> doublelonglongpairpair;
/** vector of doubles */
typedef std::vector<double> double_vector;
/** deque of doubles */
typedef std::deque<double> t_double_deque;
/** Maps a label to an instance of feature vector class
 * (it is assumed that all fvs are of the same fv type!) */
typedef std::map<std::string, SmafeAbstractFeatureVector_Ptr> tSomlibFVMap;
/** Maps of string and string (key value pair, eg, for config  */
typedef std::map<std::string, std::string> tKeyValueMap;
/** Map of string to bool */
typedef std::map<std::string, bool> string_bool_map;
/** deque of sets of longs (for track / collection assignments)  */
typedef std::deque<std::set<longlongpair> > longlong_set_deque;
/** Smart pointer for char buffer */
typedef boost::shared_array<char> char_ptr;
/** Maps a track_id, segmentnr pair  to a buffer containing the raw fv (serialized, enrypted)
 * (it is assumed that all fvs are of the same fv type!) */
typedef std::map<longlongpair, char_ptr  > tRawFVMap;
/** Maps a track_id, segmentnr pair to an instance of feature vector class
 * (it is assumed that all fvs are of the same fv type!) */
typedef std::map<longlongpair, SmafeAbstractFeatureVector_Ptr> tFVMap;
/** struct for track_id, segment nr and fv smart pointer instance */
struct tlonglongsafvStruct {
	long track_id, segmentnr;
	SmafeAbstractFeatureVector_Ptr fv;
};
/** vector of tFVStructs
 * (it is assumed that all fvs are of the same fv type!) */
typedef std::vector<tlonglongsafvStruct> tlonglongsafvStructVector;
/** map with key=collection id, value=vector of fvs (including their prim keys) */
typedef std::map<long, tlonglongsafvStructVector> tlonglonglongsafvStructVectorMap;

// --constants that are used by more than one subproject--
// live calc protocol
/** command to request options */
const std::string 	SMAFELIVEMODE_COMMAND_GETOPTS 	= "GETOPTS";
/** std port */
const int			SMAFELIVEMODE_STDPORT			= 1212;


/** the passphrase  for en/decryption.
 * Will be set in the respective program (smafewrapd etc)
 * if it remains set to 0's, encryption is disabled */
extern char verysecretpassphrase[];
/** the fixed standard passphrase, the "real" passphrase is encrypted with */
extern char stdpassphrase[];


// convenience macro for indexing 2dim array
// that are defined as [x*y]
#define INDEX2DARRAY(x,y,c) ((y)*(c) + (x))

// on mingw define sleep, since it does not exist
// see: http://www.mail-archive.com/cvs-all@haskell.org/msg21900.html
// see: http://www.xinotes.org/notes/note/439/
#if defined(__MINGW32__)
#include <windows.h>
#define sleep(t) Sleep((t) * 1000)
#endif

/** compute time difference */
double DLLEXPORT diffclock(clock_t clock1,clock_t clock2) ;


/** <p>Checks if the double variable specified is NaN
 * <p>We define this helper function explicitly since not all c++ compiler may provide it.
 * See <a href="http://www.parashift.com/c++-faq-lite/newbie.html#faq-29.15">
 * http://www.parashift.com/c++-faq-lite/newbie.html#faq-29.15</a>
 * <p>Note: inline functions must be defined in the header file (not .cpp)
 */
inline bool DLLEXPORT my_isnan(double x) {
	return x != x;
}


/** Returns md5 hash value for audio track
 * @param audio audio data
 * @param len length of audio data
 * @hash OUT: allocated array of char for hash. Length must be 33 or greater (outgoing)
 *
 */
void DLLEXPORT getHashvalue(char* audio, int len, t_filehash &hash);

/** Calculates fingerprint of audio track, from raw audio data
 * <p>Currently, only a dummy value is returned (md5 hash)
 *
 */
void DLLEXPORT getFingerprint(char* audio, int len, t_fingerprint &fp);


/** Prints splash screen
 * @param progname Name of the program (e.g. smafewrapd)
 */
void DLLEXPORT splashScreen(std::string progname);

/** Writes an array as c array definition code to output file. Type char
 *
 */
void DLLEXPORT writeArrayAsCode(std::ostream &outfile, std::string varName, char* buffer, int buflen);

/** Writes an array as c array definition code to output file. Type double
 *
 */
void DLLEXPORT writeArrayAsCode(std::ostream &outfile, std::string varName, double* buffer, int buflen);


/** Returns PID (cross-OS win and linux)
 *
 */
long DLLEXPORT my_getpid();


// ------------------------------------------------------------------------
// string helper functions (ought to be in the STL, IMO! but well....)

/** trims leading and trailing whitespace from a std::string */
std::string DLLEXPORT trimWhitespace( std::string& str);

bool DLLEXPORT endsWith(const std::string theString,  const std::string thePattern);


template<typename T>
inline std::string stringify(const T& x)
{
	std::ostringstream o;
	if (!(o << x))
		throw std::string("stringify(") + typeid(x).name() + ")";
	return o.str();
}


template<typename T>
inline void convert(const std::string& s, T& x,
                    bool failIfLeftoverChars = true)
{
  std::istringstream i(s);
  char c;
  if (!(i >> x) || (failIfLeftoverChars && i.get(c)))
    throw std::string("Bad conversion: " + s);
}

template<typename T>
inline T convertTo(const std::string& s,
                   bool failIfLeftoverChars = true)
{
  T x;
  convert(s, x, failIfLeftoverChars);
  return x;
}

/** tokenizes a string at given delimiters
 * @param str: the string
 * @param tokens: OUT: vector of strings (the tokens
 * @param delimiters: (optional): delimiters. Default is space
 */
void DLLEXPORT tokenize(const std::string& str,
                      std::vector<std::string>& tokens,
                      const std::string& delimiters = " ");


inline char charToLower(char c)
		{
		    return std::tolower(c);
		}

/** Converts string to lower case (even this simple task requires some hacking in C++ :-(
 * @param str the intput string
 * @return the output string, all chardacters lower case
 */
std::string DLLEXPORT toLower(const std::string& str);


/** This function is a workaround for the Boost serialization version problem of vectors of primitive types (eg vectors of integers)
 * The workaround is required because Boost libs > 1.35 cannot correctly decode v 1.35 archives.
 * More information:
 * #299 in trac
 *
 * How does this procedure work?
 * 1) We check if the archive is from the affected version (4)
 * 2) if yes: we copy the header without version number (4), add the version number 5, add a zero (item_version), and add the remainder of the original string.
 * 		NB: we assume that the header length is fixed but we are aware that the size number can have more than one digits. (we search for the first space)
 */
std::string DLLEXPORT boost_s11n_workaround_135(std::string in);



// ------------------------------------------------------------------------
// crypto wrappers

/** encrypts the given string with the given passphrase using the default encryptor of
 * cryptopp lib
 * This function checks if passPhrase is of non-zero length. If so, no en/decryption takes place
 * @param instr the string to encrypt as c string (array of chars)
 * @param passPhrase passphrase to use, c string
 * @return encrypted string, string object
 */
std::string encryptString(const char *instr, const char *passPhrase);

/** decrypts the given string with the given passphrase using the default decryptor of
 * cryptopp lib
 * This function checks if passPhrase is of non-zero length. If so, no en/decryption takes place
 * @param instr encrypted string, as c string (array of chars)
 * @param passPhrase passphrase to use, c string
 * @return plaintext string, string object
 * @throw may throw CryptoPP::Exception (and other?)
 */
std::string decryptString(const char *instr, const char *passPhrase);


/** wrapper that uses the verysecretpassphrase for en/decryption
 */
std::string encryptString(const char *instr);

/** wrapper that uses the verysecretpassphrase for en/decryption
 */
std::string decryptString(const char *instr);


// ------------------------------------------------------------------------
// structs

/** one near neighbour */
struct Nn_result_rs {
	long track_id;
	std::string uri;
	double dist;
};

typedef boost::shared_ptr<Nn_result_rs> Nn_result_rs_Ptr;

/** struct for admin params for smafewrapd */
struct tAdminparams {
	// set by command line in admin mode only
	std::string sCollectionName;
	std::string sSomlibfile;
	std::string fv_output_files_prefix;
	/** text mode, is set after loading config from db */
	bool bToTextfile;
	/** Special somlib mode (ie, files will not be loaded, assumed that the files are only labels from somlib file */
	bool bSomlibmode;
	/** jobs  mode: read jobs from database */
	bool bJobsMode;
	/** treat given file as file list ? */
	bool bIsFileList;
	std::string sFilename;
	/** only print open tasks? */
	bool bPrintStatsOnly;

	// set by command line
	/** identifier for this daemon process */
	std::string daemonId;
	/** poll interval in minutes */
	int pollInterval;
	/** name of logfile */
	std::string sLogfilename;
	/** loglevel requested */
	int loglevel_requested;

	// set later
	/** propagate jobs to smafedistd (distance table) ?
	 * is always true for now */
	bool bPropagateJobsToSmafedistd;
	/** propagate jobs to smui component (smuijob_XXX table) ?
	 * This should only happen if the db schema exists. This is checked by the existance
	 * of table dbinfo during startup */
	bool bPropagateJobsToSmui;
	/** log level chosen with command line argument
	 * is defined in smafeutil.cpp */
	//int loglevel_chosen;
	/** live mode */
	bool bLivemode;
	/** live port */
	int iLiveport;
	/** live host */
	std::string sLivehost;
	/** live file */
	std::string sLivefile;
	/** how many results return in live qurey */
	int iLivetopk;
	/** id of collection to be used as result */
	long lCollectionId;
};

/** ORM mapping distance type */
struct tDistancetype {
	long id;
	std::string name;
};









