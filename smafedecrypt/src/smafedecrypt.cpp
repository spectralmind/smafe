///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008 spectralmind
// All rights reserved.
// ------------------------------------------------------------------------
// smafedecrypt.cpp
// SpectralMind Audio Feature Extraction Decryption Tool
// Main file
// ------------------------------------------------------------------------
// Version $Id$
///////////////////////////////////////////////////////////////////////////


// ------------------------------------------------------------------------
// includes
#include "config.h"
#include <string>
#include "argtable2.h"
#include <boost/algorithm/string/replace.hpp>
#include "smafeutil.h"
#include "smafeLogger.h"


// ------------------------------------------------------------------------
// constants
/** program name */
const char PROGNAME[] = "smafedecrypt";


// ------------------------------------------------------------------------
// global vars
/** Processes command line arguments using argtable library
 * <p>Note: this function sets variable ssFiles
 * @param argc number of command line arguments (from main())
 * @param argv array of c-strings (from main())
 */
void processCommandLineArguments(int argc, char* argv[]) {
	struct arg_str *arg_passphrase = arg_str1("p", "passphrase", "PASSPHRASE", "Passphrase");
	struct arg_lit *help  =					arg_lit0(NULL,"help",       "Print this help and exit");
	struct arg_end *end   = arg_end(20);
	void* argtable1[] = {arg_passphrase, help, end};
	int nerrors;
	if (arg_nullcheck(argtable1) != 0) {
		/* NULL entries were detected, some allocations must have failed */
		std::cerr << PROGNAME << ": insufficient memory" << std::endl;
		exit(2);
	}
	// check if any argument given
	if (argc > 1) {
		// Parse the command line as defined by argtable[]
		nerrors = arg_parse(argc,argv,argtable1);
	} else {
		// no argument given
		help->count = 1;
	}
	/* special case: '--help' takes precedence over error reporting */
	if (help->count > 0) {
		std::cout << "USAGE " << std::endl << std::endl << PROGNAME;
		arg_print_syntax(stdout,argtable1,"\n");
		arg_print_glossary(stdout,argtable1,"  %-22s %s\n");
		std::cout << std::endl << std::endl;
		exit(127);
	}
	if (nerrors == 0) {
		// Passphrase
		if (arg_passphrase->count > 0) {
			if (strlen(arg_passphrase->sval[0]) <= 63) {
				strcpy(verysecretpassphrase, arg_passphrase->sval[0]);
				SMAFELOG_FUNC(SMAFELOG_INFO, "Data encryption / decryption is enabled.");
			} else {
				SMAFELOG_FUNC(SMAFELOG_FATAL, "Passphrase too long. Max 63 characters.");
				exit(2);
			}
		} else {
			SMAFELOG_FUNC(SMAFELOG_INFO, "Data encryption / decryption is DISABLED!");
		}
	} else {
		arg_print_errors(stdout, end, PROGNAME);
		std::cout << "--help gives usage information" << std::endl;
		exit(1);
	}
}


// ------------------------------------------------------------------------
// main
/** well that's the entry point of this cute application */
int main(int argc, char* argv[]) {
	try {




		  	// This is a short hack to have a string encrypted with a specific password.
		  	// Howto:
		  	// - uncomment the lines
			// - replace plain text and password
			// - compile & execute
//			std::cout << encryptString("2013-09-01", "buek") << std::endl;
//			std::cout << encryptString("2014-03-01", "buek") << std::endl;
//			std::cout << encryptString("2014-09-01", "buek") << std::endl;
//			std::cout << encryptString("180", "buek") << std::endl;
//			std::cout << encryptString("2000000", "buek") << std::endl;
//			exit(99);





		splashScreen("Decryption tool");
		processCommandLineArguments(argc, argv);
		std::string in;
		getline( std::cin, in );

		// strip away the \012 that are in the string if it comes from the db (a convenience heuristic)
		boost::algorithm::replace_all(in, "\\012", "");
		SMAFELOG_FUNC(SMAFELOG_INFO, "Encrypted string, cleaned: " + in);
		try {
			std::cout << "\nCleartext:\n" << std::endl;
			std::cout << decryptString(in.c_str(), verysecretpassphrase) << std::endl;
		} catch (CryptoPP::Exception& e) {
			std::cout << "Error: " << e.GetWhat() << std::endl;
			exit(1);
		}
	} catch (...) {
		// Catch all
		// Try to log, if that does not work, write to stderr
		try {
			SMAFELOG_FUNC(SMAFELOG_FATAL, "Uncaught exception :-(");
		} catch (...) {
			std::cerr << "Uncaught exception :-(" << std::endl;
		}
		exit(1);
	}
}
