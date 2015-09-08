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


#include "smafeutil.h"
#include "smafeLogger.h"



//  passphrase set to 0 (see header file)
char verysecretpassphrase[64] = { 0 };
char stdpassphrase[] = "x+x$Qg=O";
//#if defined(SMAFE_PASSPHRASE)
//char verysecretpassphrase[] = SMAFE_PASSPHRASE;
//#else
//char verysecretpassphrase[] = "";
//#endif



double DLLEXPORT diffclock(clock_t clock1,clock_t clock2) {
	double diffticks=clock1-clock2;
	double diffms=(diffticks)/(CLOCKS_PER_SEC/1000);
	/*
	SMAFELOG_FUNC(SMAFELOG_DEBUG, "clock1: " + stringify(clock1));
	SMAFELOG_FUNC(SMAFELOG_DEBUG, "clock2: " + stringify(clock2));
	SMAFELOG_FUNC(SMAFELOG_DEBUG, "diff: " + stringify(diffticks));
	SMAFELOG_FUNC(SMAFELOG_DEBUG, "CLOCKS_PER_SEC: " + stringify(CLOCKS_PER_SEC));
	 */
	return diffms;
}



std::string DLLEXPORT trimWhitespace( std::string& str)
{
	// Trim Both leading and trailing spaces
	size_t startpos = str.find_first_not_of(" \t\n\r"); // Find the first character position after excluding leading blank spaces
	size_t endpos = str.find_last_not_of(" \t\n\r"); // Find the first character position from reverse af

	// if all spaces or empty return an empty string
	if(( std::string::npos == startpos ) || ( std::string::npos == endpos))
	{
		str = "";
	}
	else
		str = str.substr( startpos, endpos-startpos+1 );
	return str;
}

bool DLLEXPORT endsWith(const std::string theString,  const std::string thePattern) {
	size_t i = theString.rfind(thePattern);
	return (i != std::string::npos) && (i == (theString.length() - thePattern.length()));
}

void DLLEXPORT tokenize(const std::string& str,
		std::vector<std::string>& tokens,
		const std::string& delimiters)
{
	// Skip delimiters at beginning.
	std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	// Find first "non-delimiter".
	std::string::size_type pos     = str.find_first_of(delimiters, lastPos);

	while (std::string::npos != pos || std::string::npos != lastPos)
	{
		// Found a token, add it to the vector.
		tokens.push_back(str.substr(lastPos, pos - lastPos));
		// Skip delimiters.  Note the "not_of"
		lastPos = str.find_first_not_of(delimiters, pos);
		// Find next "non-delimiter"
		pos = str.find_first_of(delimiters, lastPos);
	}
}

// TODO: out is empty after std::transform
std::string DLLEXPORT toLower(const std::string& str) {
	/*
	 *
	std::string out;
	SMAFELOG_FUNC(SMAFELOG_INFO, "out: "  +out);
	SMAFELOG_FUNC(SMAFELOG_INFO, "str: "  +str);
	std::transform(str.begin(), str.end(), out.begin(), charToLower);
	SMAFELOG_FUNC(SMAFELOG_INFO, "out: "  +out);

	return out;
	 */

	return str;
}



/** This function is a workaround for the Boost serialization version problem of vectors of primitive types (eg vectors of integers)
 * The workaround is required because Boost libs > 1.35 cannot correctly decode v 1.35 archives.
 * More information:
 * #299 in trac
 *
 * How does this procedure work?
 * 1) We check if the archive is from the affected version (4)
 * 2) if yes: we copy the header without version number (4), add the version number 5, add a zero (item_version), and add the remainder of the original string.
 * 		NB: we assume that the header length is fixed but we are aware that the size number can have more than one digits. (we search for the first space)
 *
 * NB: This workaround does not work for arbitrary classes but has only been tested for vectors of integers (SmafeNumericFeatureVector is known to NOT work
 */
std::string boost_s11n_workaround_135(std::string in) {

	if ((BOOST_VERSION) >= 103600 ) {

		if (in.substr(0, 27) == "22 serialization::archive 4") {

			std::string out = in.substr(0, 26) + " 5 ";

			std::string::size_type loc = in.find( " ", 28 );
			if( loc == std::string::npos ) {
				return "WARNING: space after position 28 not found (in boost_s11n_workaround_135)";
			}

			//		std::cout << loc << std::endl;

			out += in.substr(28, loc-28) + " 0 " + in.substr(loc, std::string::npos);

			SMAFELOG_FUNC(SMAFELOG_DEBUG, "Detected s11n version 4. Applied workaround to version 5. Result: " + out);

			return out;
		} else // not the old header
			return in;
	} else // not the old library
		return in;
}



void DLLEXPORT splashScreen(std::string progname) {
	SMAFELOG_FUNC(SMAFELOG_INFO, "\n\n\n");
	SMAFELOG_FUNC(SMAFELOG_INFO, "                                       _____           ");
	SMAFELOG_FUNC(SMAFELOG_INFO, "           ______   _____   _____    _/ ____\\   ____   ");
	SMAFELOG_FUNC(SMAFELOG_INFO, "          /  ___/  /     \\  \\__  \\   \\   __\\  _/ __ \\  ");
	SMAFELOG_FUNC(SMAFELOG_INFO, "          \\___ \\  |  Y Y  \\  / __ \\_  |  |    \\  ___/  ");
	SMAFELOG_FUNC(SMAFELOG_INFO, "         /____  > |__|_|  / (____  /  |__|     \\___  > ");
	SMAFELOG_FUNC(SMAFELOG_INFO, "              \\/        \\/       \\/                \\/  ");
	SMAFELOG_FUNC(SMAFELOG_INFO, "");
	SMAFELOG_FUNC(SMAFELOG_INFO, "Spectralmind Audio Feature Extraction");
	if (progname != "")
		SMAFELOG_FUNC(SMAFELOG_INFO, progname);
	SMAFELOG_FUNC(SMAFELOG_INFO, "");
	SMAFELOG_FUNC(SMAFELOG_INFO, "Version: " + stringify(PACKAGE_STRING ));
	SMAFELOG_FUNC(SMAFELOG_INFO, "\n");
}


void getHashvalue(char* audio, int len, t_filehash &hash) {
	byte digest[CryptoPP::Weak::MD5().DigestSize()];

	//SMAFELOG_FUNC(SMAFELOG_INFO, "CryptoPP::AES::BLOCKSIZE" + stringify(CryptoPP::AES::BLOCKSIZE));
	SMAFELOG_FUNC(SMAFELOG_DEBUG2, "CryptoPP::Weak::MD5()" + stringify(CryptoPP::Weak::MD5().DigestSize()));
	CryptoPP::Weak::MD5().CalculateDigest(digest, (byte*) audio, len);
	for (int di = 0; di < 16; ++di)
		sprintf(hash + di * 2, "%02x", digest[di]);
	hash[32] = '\0';
	SMAFELOG_FUNC(SMAFELOG_DEBUG2, "hash" + stringify(hash));

	/*
	md5_state_t state;
	md5_byte_t digest[16];

	md5_init(&state);
	md5_append(&state, (const md5_byte_t *)audio, len);
	md5_finish(&state, digest);
	for (int di = 0; di < 16; ++di)
		sprintf(hash + di * 2, "%02x", digest[di]);
	hash[32] = '\0';
	*/
}


void getFingerprint(char* audio, int len, t_fingerprint &fp) {
	getHashvalue(audio, len, fp);
}

void DLLEXPORT writeArrayAsCode(std::ostream &outfile, std::string varName, char* buffer, int buflen) {
	outfile << "char " << varName << "[] = {";
	for( int n = 0; n < buflen; ++n ) {
		outfile << int(buffer[n]);
		if (n < buflen - 1) outfile << ",";
		if (n % 20 == 0) outfile << "\n";
	}
	outfile << "};" << std::endl;
}

void DLLEXPORT writeArrayAsCode(std::ostream &outfile, std::string varName, double* buffer, int buflen) {
	outfile << "double " << varName << "[] = {";
	for( int n = 0; n < buflen; ++n ) {
		outfile << buffer[n];
		if (n < buflen - 1) outfile << ", ";
		//if (n % 20 == 0) outfile << "\n\t";
	}
	outfile << "};" << std::endl;
}


long DLLEXPORT my_getpid() {
#if defined( WIN64   ) || defined( _WIN64   ) || \
		defined( WIN32   ) || defined( _WIN32   ) || \
		defined( WINDOWS ) || defined( _WINDOWS )
	return _getpid();
#else
	return getpid();
#endif
}



// ------------------------------------------------------------------------
// crypto wrappers


std::string encryptString(const char *instr, const char *passPhrase)
{
	if (strlen(passPhrase) > 0) {
		std::string outstr;

		//	CryptoPP::DefaultEncryptorWithMAC encryptor(passPhrase, new CryptoPP::HexEncoder(new CryptoPP::StringSink(outstr)));
		//	CryptoPP::DefaultEncryptor encryptor(passPhrase, new CryptoPP::HexEncoder(new CryptoPP::StringSink(outstr)));
		CryptoPP::DefaultEncryptor encryptor(passPhrase, new CryptoPP::Base64Encoder(new CryptoPP::StringSink(outstr)));
		encryptor.Put((byte *)instr, strlen(instr));
		encryptor.MessageEnd();

		return outstr;
	} else {
		return std::string(instr);
	}
}

std::string decryptString(const char *instr, const char *passPhrase)
{
	if (strlen(passPhrase) > 0) {
		std::string outstr;

		SMAFELOG_FUNC(SMAFELOG_DEBUG2, "Trying to decrypt:");
		SMAFELOG_FUNC(SMAFELOG_DEBUG2, std::string(instr));


		//CryptoPP::HexDecoder decryptor(new CryptoPP::DefaultDecryptorWithMAC(passPhrase, new CryptoPP::StringSink(outstr)));
		CryptoPP::Base64Decoder decryptor(new CryptoPP::DefaultDecryptor(passPhrase, new CryptoPP::StringSink(outstr)));
		decryptor.Put((byte *)instr, strlen(instr));
		decryptor.MessageEnd();


		return outstr;
	} else {
		return std::string(instr);
	}
}



std::string encryptString(const char *instr)
{
	return encryptString(instr, verysecretpassphrase);

}

std::string decryptString(const char *instr)
{
	return decryptString(instr, verysecretpassphrase);
}



/*
// convenience functions for printing ipp vectors
genPRINT( 64f, " %f" )
genPRINT( 32f, " %f" )
genPRINT( 16s, " %d" )
genPRINTcplx( 64fc, " {%f,%f}" )
genPRINTcplx( 32fc, " {%f,%f}" )
genPRINTcplx( 16sc, " {%d,%d}" )
 */
