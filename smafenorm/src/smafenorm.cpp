///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// smafenorm.cpp
//
// SpectralMind Audio Feature Extraction Normalization tool
// Main file
// ------------------------------------------------------------------------
//
//
// Version $Id: smafequery.cpp 285 2009-07-12 14:46:51Z ewald $
//
//
//
//
//
//
///////////////////////////////////////////////////////////////////////////


// ------------------------------------------------------------------------
// includes

#include "config.h"

#include "argtable2.h"
#include <string>
#include "math.h"


#include "smafeutil.h"
#include "smafestoredb.h"
#include "smafeLogger.h"
#include "smafeopt.h"

#include "smafestore_specific_include_no_text.h"



// ------------------------------------------------------------------------
// constants
/** program name */
const char PROGNAME[] = "smafenorm";


// ------------------------------------------------------------------------
// global vars
/** vector of fvtype_ids, if this parameter is given at command line.
 * <p>If this vector remains empty all fv types are calculated
 */
std::vector<long> fvtype_ids;
/** db conn */
SMAFE_STORE_DB_CLASS* db =  NULL;
/** vecotr of fvts */
std::vector<SmafeFVType_Ptr> fvtypes;
/** options */
Smafeopt* so;


// -- to be moved to log options class at some time
/** loglevel requested */
int loglevel_requested;


/** Processes command line arguments using argtable library
 * <p>Note: this function sets variable ssFiles
 * @param argc number of command line arguments (from main())
 * @param argv array of c-strings (from main())
 */
void processCommandLineArguments(int argc, char* argv[]) {

	struct arg_int *arg_fvtype_ids		=	arg_intn("f", "fvtypes","ID", 1, 99, "Feature vector type id(s) to be calculated. Parameter may occur more than once.");
	struct arg_str *arg_dbconf = arg_str0(NULL, "dbconf", "DATABASE-CONFIGURATION-FILE",
			"Specify file that contains database connection details");

	struct arg_int *arg_verbose =			arg_int0("v", "verbosity",	"0-6", "Set verbosity level (=log level). The lower the value the more verbose. Default is 3");
	struct arg_lit *help  =					arg_lit0(NULL,"help",       "Print this help and exit");
	struct arg_end *end   = arg_end(20);

	void* argtable1[] = {arg_fvtype_ids, arg_dbconf,
			arg_verbose, help, end};

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
		exit(0);
	}

	if (nerrors == 0) {
		// verbosity level
		// must be on top
		if (arg_verbose->count > 0) {
			loglevel_requested = arg_verbose->ival[0];
			// change loglevel
			SmafeLogger::smlog->setLoglevel(loglevel_requested);
		} else {
			loglevel_requested = SmafeLogger::DEFAULT_LOGLEVEL;
		}

		// ---- db stuff
		// db options file
		if (arg_dbconf->count > 0) {
			SMAFELOG_FUNC(SMAFELOG_DEBUG, "Parsing db configuration file");
			so->parseDbOpts(std::string(arg_dbconf->sval[0]));
		}

		// Passphrase
		/*
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
		 */

		// fv types
		if (arg_fvtype_ids->count > 0) {
			// parameter given
			for (int i = 0; i < arg_fvtype_ids->count; i++) {
				fvtype_ids.push_back(arg_fvtype_ids->ival[i]);
				SMAFELOG_FUNC(SMAFELOG_INFO, "Will also calculate feature vector type_id " + stringify(fvtype_ids[i]));
			}
		} else {
			// param not given, default values
			// vector remains empty
			SMAFELOG_FUNC(SMAFELOG_INFO, "Will calculate all feature vector type ids");
		}




	} // if (nerrors == 0)

}




// ------------------------------------------------------------------------
// normalize one feature vector type
void doNorm(SmafeFVType* fvt) {
	std::vector< SmafeAbstractFeatureVector_Ptr > fvs;
	SmafeAbstractFeatureVector_Ptr fv_ptr;
	SmafeAbstractFeatureVector* fv;
	SmafeFVType newtype;

	// vec length
	size_t veclen = fvt->dimension_x * fvt->dimension_y;

	// max values
	double* maxvals = new double[veclen];

	// convenience
	long fvt_id = fvt->id;

	// create new fvt instance
	newtype = SmafeFVType(*fvt);
	// change name and params
	newtype.name = fvt->name + "-norm-attr";
	newtype.parameters = "Derived from fvt "+fvt->name+" (id=" + stringify(fvt->id) + "). Values are normalized according to attributewise absolute max values.";


	try {
		db = new SMAFE_STORE_DB_CLASS();
		// open connection
		db->openConnection(so->strDbhost, so->strDbport, so->strDbuser, so->strDbpwd, so->strDbname);

		// ------------------------------------------
		// read

		// get all track ids
		std::vector<long> vTracks = db->getTrack_ids("");

		// iterate through track ids
		for(std::vector<long>::iterator iter_tr = vTracks.begin(); iter_tr < vTracks.end(); iter_tr++) {
			SMAFELOG_FUNC(SMAFELOG_DEBUG, "Reading:  fvt_id = " + stringify(fvt_id) + ", track_id = " + stringify(*iter_tr));
			// read feature vector, but no meta data. Error handling here not in smafestore
			// no file id given and no segmentnr given
			// load file_id
			fv = db->readFeatureVector(fvt_id, *iter_tr, false, -1, -1, true, true);
			// if track was read, store it in vector, wrapped in a shared pointer
			if (fv != NULL) {
				fv_ptr.reset(fv);
				fvs.push_back(fv_ptr);
			} else {
				SMAFELOG_FUNC(SMAFELOG_DEBUG, "Track not found:  fvt_id = " + stringify(fvt_id) + ", track_id = " + stringify(*iter_tr));
			}
			// occasionally log the current number of fvs read
			if (fvs.size() % 100 == 0 && fvs.size() > 0)
				SMAFELOG_FUNC(SMAFELOG_INFO, "Read " + stringify(fvs.size()) + " feature vectors so far.");
		}
		SMAFELOG_FUNC(SMAFELOG_INFO, "Read " + stringify(fvs.size()) + " feature vectors for this type.");

		// ------------------------------------------
		// calculate max values

		// init to 0
		for (size_t i = 0; i < veclen; i++) { // i is position of vector element
			maxvals[i] = 0;
		}
		for(std::vector< SmafeAbstractFeatureVector_Ptr >::iterator iter_fvs = fvs.begin(); iter_fvs < fvs.end(); iter_fvs++) {
			SmafeNumericFeatureVector *snfv;
			double absval;

			snfv = dynamic_cast<SmafeNumericFeatureVector*>((*iter_fvs).get());

			// length of vectors must match
			assert(veclen == snfv->buflen);

			for (size_t i = 0; i < veclen; i++) { // i is position of vector element
				// get absolute value
				absval = fabs(snfv->buffer[i]);

				// max
				if (absval > maxvals[i])
					maxvals[i] = absval;
			}
		}

		// ------------------------------------------
		// calculate norm vectors

		for(std::vector< SmafeAbstractFeatureVector_Ptr >::iterator iter_fvs = fvs.begin(); iter_fvs < fvs.end(); iter_fvs++) {
			SmafeNumericFeatureVector *snfv;

			snfv = dynamic_cast<SmafeNumericFeatureVector*>((*iter_fvs).get());

			// length of vectors must match
			assert(veclen == snfv->buflen);

			for (size_t i = 0; i < veclen; i++) { // i is position of vector element
				// check for 0
				if (maxvals[i] != 0) {
					// divide
					snfv->buffer[i] = snfv->buffer[i] / maxvals[i];
				} else {
					SMAFELOG_FUNC(SMAFELOG_DEBUG2, "Zero max value at position " + stringify(i));
				}
			}
		}


		// ------------------------------------------
		// write

		// start transaction
		db->startTransaction();

		// insert fvt
		bool bInserted = db->ensureRPFeatVecTypeRecord(&newtype);
		if (!bInserted) {
			SMAFELOG_FUNC(SMAFELOG_WARNING, "Featurevectortype is already in db. Maybe the normalized version of the specified feature vector type has already been calculated and inserted to the database?");
		}

		// insert fv-s
		for(std::vector< SmafeAbstractFeatureVector_Ptr >::iterator iter_fvs = fvs.begin(); iter_fvs < fvs.end(); iter_fvs++) {
			SmafeAbstractFeatureVector* fv = (*iter_fvs).get();
			db->SmafeStoreDB::storeFeatureRecord(fv->track_id, newtype.id, fv, fv->file_id);
			//			db->SmafeStoreDB::storeFeatureRecord(fv->track_id, newtype.id, fv, 2);
			// occasionally log the current number of fvs written
			size_t pos = iter_fvs - fvs.begin();
			if (pos % 100 == 0 && pos > 0)
				SMAFELOG_FUNC(SMAFELOG_INFO, "Wrote " + stringify(pos) + " feature vectors so far.");
		}
		SMAFELOG_FUNC(SMAFELOG_INFO, "Wrote " + stringify(fvs.size()) + " feature vectors for this type.");

		// commit
		db->finishTransaction(true);

		SMAFELOG_FUNC(SMAFELOG_DEBUG, "Transaction successfully committed.");
		SMAFELOG_FUNC(SMAFELOG_INFO, "Feature vectors of new feature vector type " + newtype.name + " (id="+stringify(newtype.id)+") successfully inserted.");

		delete[] maxvals;
		delete db;
	} catch (std::string& s) {
		SMAFELOG_FUNC(SMAFELOG_FATAL, s);
		// rollback (unneccessary since program exits anyway, but well.
		db->finishTransaction(false);
		exit(1);
	}

}




// ------------------------------------------------------------------------
// main

/** well that's the entry point of this cute application */
int main(int argc, char* argv[]) {
	/*
	// test crypoto
	std::cout << encryptString("hello world!", "spectralmind") << std::endl;
	std::cout << encryptString("hello world!", "1") << std::endl;
	std::cout << encryptString("hello world!", "2") << std::endl;
	std::cout << encryptString("hello world!", "spectralmindspectralmindspectralmindspectralmind") << std::endl;
	std::cout << encryptString("hello world!hello world!hello world!hello world!hello world!hello world!hello world!hello world!hello world!hello world!hello world!hello world!hello world!hello world!hello world!", "spectralmind") << std::endl;
	std::cout << encryptString("hello world!hello world!", "spectralmind") << std::endl;
	std::cout << encryptString("hello world!1", "spectralmind") << std::endl;
	std::cout << encryptString("hello world!12", "spectralmind") << std::endl;
	std::cout << encryptString("hello world!122", "spectralmind") << std::endl;
	std::cout << encryptString("hello world!1222", "spectralmind") << std::endl;
	std::cout << encryptString("hello world!12222", "spectralmind") << std::endl;
	std::cout << encryptString("hello world!122222", "spectralmind") << std::endl;
	std::cout << encryptString("1", "spectralmind") << std::endl;
	std::cout << encryptString("12", "spectralmind") << std::endl;
	std::cout << encryptString("123", "spectralmind") << std::endl;
	std::cout << encryptString("1234", "spectralmind") << std::endl;
	std::cout << encryptString("12344", "spectralmind") << std::endl;
	std::cout << encryptString("126666", "spectralmind") << std::endl;
	std::cout << encryptString("1277777", "spectralmind") << std::endl;
	std::cout << encryptString("12777778", "spectralmind") << std::endl;
	std::cout << encryptString("127777799", "spectralmind") << std::endl;
	std::cout << encryptString("1277777010", "spectralmind") << std::endl;
	std::cout << encryptString("12777771111", "spectralmind") << std::endl;
	std::cout << encryptString("hello world!", "1") << std::endl;
	 */
	/*
	try {
	std::cout << decryptString("/qOjWk7WeGZcn+l5M3iZQonFbI+KK8K3gBjAeGhmDbrZYcPqfkdi9kbF7iTIBevAUQtTO9LQpvTgl3XNbjnwdJdwujoNCWFeX+YpLNc4T/MEGYlucsvJZHL35PQPbPLxxjJSVPruFxkupOLwkTjhXIeI6KQljMMRJ9+PaOe4xiEOMQDz8XLglGZHhk/ufAw1mJfxt1I9LOZrALqwoadHy0O15w7tOE+4eRPKtxiLkihwYj0bZYrXEMbpZ8Mj4lOxH6xXWRGMrDFg3F9ow9NPXMrkkwRmpngvMgXrs7yniZYNN8N7MvpgYOFwJvYcE8BJUj3mkjLNroYJYaaqB85L7ds02dsYzvH/Tdtkk/2p4MZ6MfbWPmmy0IMHJSy8a0GBybBYGewKnBC97Sepg2WzXd1gwslOFzMf0ENiUEpgJnLdX5ZLqk6aUzdQK4ikQndf1NmYWA2NGbMdQ3/y3VNKddndR460BEyA/5lXYMDzMuzjb9KSxyZ/4PjgVE5yfjXbmvrX4534Sur9MVRUEHLA7r45sStx7r2GmOh74a64VF+4RdkCsfQDHxVp6Te+fVFlbqmf9U9ZMuk6TLowhMzC8xMVSm+s2Z8A5oAzS+iWzlQfZBnPjBVBTIVhTSstIy0jfgRLse8wPN6PIJ42hk1pgV2atDBDqqaWuqId2zxD/P70266ZoOo5lzm9RpzinKbDVsa5Fx9DFrzvIGOAkegUFnGmYWgFR9ivfQ/N43s1arb1hOsGVvE+VwLTHbN84bf8Hi4CGTcjlDnTXlgf/oDdYBoOwnHi++U+5/yBiNnIf8Eeam5yXyZPxEYPRY+82tzmO1CRZbtRbgv8sA7tgF7Rcsz57JBc4BOqjleTSSf8lYBAxDTucceHsWyxIoc7ENAipEbFNM/9PFdfdcFTP00YWmnoBeZ1fSmG+y8GwrsQTvsX7p6lzOZkFAguHKPbc+VORt3s78GkT0oT7B1Xm3Zcwa6k3SWeLtaK+IPplF3/8kpl2HtQXJKVkx8wzKGWlMnWnmRF6z06B2cxofpdbiuzcpfX6Rv9TOK6RZPZkA/Ixe4EPwpOhGqAcKWoKLw/xhRmrivapvaXi5xIKYrDHh7NYElc5EoV3B7/H8eVB8mLnFT5vGK+9ZUX547G0Zs98qbNod0GILQHOxy2cvm+7sqzQv5OewmylJzdchJ93Ic9ren3m7u0nUOVWKrFgDIZdyttmKCnsf72G81FhcHs06TG0RiFlUroD0WYvlLeWjK+NLJSHEl9tZNm5MQ762Qxe9WlpRapAZYU8qX3FGRM+Ka84An7YO3tHYrfDJkzSzMlArhoj//ksVTHgvCXG8EAVq6oQSz1cDiQa7upwlVk7hZveJB78wssWEAkp4V6NJB7oa6/w92rYvTZTpEb/tljArndjBV5IimhhLdzk42NlqtYTXitiM7oRs/7YtXURmpPzE6K8npl8Oycq9ulsSbK2nuXPgVruRSAyESY9O7LT4uayPpQ4dk0Sw6VnMK5QPotHFRl5RQrKeEbbOAQgAbHsjVIBJwnwaVcp73/V4vv0zt8uZ1VBeE+oia0KDkUUzdR9IsAmHaEnUEBBwd5+0YrGt5QclGSlC/i+XYaMrnMZ17FMqFlNjHLuRiNX3elxUZuchMBZAWvA2uTG4b6VlNDb9RyCw/gukjWKKUrHJdyIr5xdMqkxM5FSzis81Dt3xrRimoKM/+SnS9ZnrK1HPWtZXfKH0LKxNulXL6wVV63Gw7T9tnoVylkf61EOdmwEHksftOi5O5MBgdt8SRTGAnPqR+D3Wp8r273UvQM72hL7rLuBqTAMnIjVJMgwlcKfO0blXxq4CGNniXXS1fQbnYK0c4NDydlj1Ld0T9Dnx7AEwYCRVB6glL8Optd8qF97u7Wtev9NbG5vHhOs5LGZHKlccDQ0rvoLz61YxOmJY7exclXDWf6eAKHHcDB", "soundminer") << std::endl;
	} catch (CryptoPP::Exception& e) {
		std::cout << "Error: " << e.GetWhat() << std::endl;
	}
	try {
	std::cout << decryptString("NCHAxPb51yIdF67kUFun3hGp2hkWMKQiRdS3TSDQ6YF+Gp5tzFoKwjTdwNFNqW9ED97mPf04gf/Dcu1U+rljaCfhv42vClAaLsvGsP3ODGiQCDJZ61aidxS7SACtkgdegIkM0QUfbt52WpRVxbgd5/lb3wwJmnjHSG6aIRFZFEAj4e+tSWMOMgabDRN3irh+EYu735yCEgcf2ong5FppecvEZ4FEKOtyuOBXKfNlgelDlvcGDt+XM7VEYJjc/RP8BgIl94gad5T+81KMB8sVxCWpIdvNoCHi8BEiWhBota1O5Am1FtDPhThHvuo9xsH3Badn+380KFDVN/84iOj46iuDRUheou9GN0YcnWHqoDpnpNCd5G7JakfmHTvc8DJHP/Q1on3KN7I7p7hWNxNroP65JNrfnN8vUz7oB3Z6imxeuzR4jMAao2OaoJ5XtWt3jtIyHsBfakDfS2WBHCbF1aOkdA0/LYs/aTHXeKWaahPH6+JmZ6f+t+COazJC8oY1n8pk5NOv5875pc8vMpTdDTyGIM/rMTgy4QzXjiwU8s7R+EVmRAOD/iyVyD+vWyCvTO1L7druDZVxx0Hi08g1USTLesiiurN1vHrRHHCCoIhDtfuorkklL6LcikyN1TYXstQhDA3GydtNyeJxl5Ib5Lc4xxDsuLnFim6KynkB04Pyvq7LIK7jDuYQkP09WYHzhX0+Q8KzucVQJrI5R3UKiq4mcovDswC9JA2kYH0TIVhFFkFjVFi3+CCE36CsuryPUlRnO1O4eBIeETxe8Vln+rLmh7FBxFs0Sxjd/ZW3LIH4d1mqR30XDUTKy+Uh5cIYMp1+gJUczlvqtIW1haCQ/g7V19C5dyBVcyVRDgoL04HBWRuiCQzIIRWULqb/tlgWrb3pZhLmhCytcUoniYkp4Wxa/rae3V+mtdu2Nu6PXNLK0kHJANO1LZ3SAYWVyfI1r4RR6kFBtkONU1IzD+L3ByNYEWTq3Zg2MGGt8QKhgFfr4PUf6w9gqQkkC3T6w8DaKrIvwYJ9bFUA37NQg2G8kWRHqfLmaX8GUfxcF/Kz5WPXOyjWa6m+SffRRuhdkmS6efnctGu87QTfHKLhnF0tFaOvLpUKoDsuxGGxo4zQoSEk4uw4Dd4KSTBhVhwZhwW1400AfrAtOpDpkI3Tv+e4tjRbMNMZTvDbTqVjVCyvTrZWq0GMLPcm59HmJ5ZIYCegI+hKtrCrb+ushQ7yRLG4VMjWQL9ilu5falp51w19QUAPMZlOmzDpUHmOeO8mz8iInaIixsolVbhV685mNjLuo730rUjD3GUqbVpWV1o7EcfT6MCDR1EAXQZqJFjPuUJ1RwuRY9tRGHK0eS641Ug2hXWri5SRrSO/MWtK3YUELWbU/NEJapv/TYrUWmSAeaPKXmD4jI4r+uylzGdGxcNylayTmZsFFws0EkDjhadeyzH/51lDm07NA191lm6zjQFUriOF+5xnAK1lPgv7wmrzIvmIMNlH9r7iqcPJN7eITAGbGc8LWidNHDhLQaEP5rLK7kaFfGxsBsYpsHb7xBpcUH5bazShkXe861mV1j8iGV1BEK1q7vLjz6vSs9DS11JMq3U1/F+L1TV/5Rp3pgzWNkQ4Kpoi4dkVdlyhI/vTX2Aqhn8dhd+KHty92YAcbfxIExF5jnheOKZPwk4epJrWnmRQIp63bsPUsW+MyGerzkSXUe5txBbWL39GvLFunuRWKxvVJTYj9ff9tyTWhcxzx03IU6/eDtK3XEHtBS5LUfvDNtjlMeOPqF3XEflzbyJ+C36zHLosndK/cqLmn5EY50J/5edX5iuJmYPEImqLj+vG8VXR4OCwHKNbpfW2OuFejVUhy6unY6B0cPAClpgA3Ov/t+ci+NZqtI6/HF5khurCqkCdSkyFAx4v/SEt2IEThaVJzaqA6wzTkiArLu7jnJFulPR7DVAb", "soundminer") << std::endl;
	} catch (CryptoPP::Exception& e) {
		std::cout << "Error: " << e.GetWhat() << std::endl;
	}
	exit(0);
	 */
	try {
		splashScreen("Normalization tool");

		// Process command line options
		so = new Smafeopt();
		try {
			processCommandLineArguments(argc, argv);
			so->loadConfigFromDb(); // we need the passphrase
		} catch (std::string& s) {
			SMAFELOG_FUNC(SMAFELOG_FATAL, s);
			exit(2);
		}

		try {
			db = new SMAFE_STORE_DB_CLASS();
			// open connection
			db->openConnection(so->strDbhost, so->strDbport, so->strDbuser, so->strDbpwd, so->strDbname);
			// get feature vector types
			db->getFeaturevectortypes(fvtypes);
			delete db;
			SMAFELOG_FUNC(SMAFELOG_INFO, "Found " + stringify(fvtypes.size()) + " feature vector types in db.");
		} catch (std::string& s) {
			SMAFELOG_FUNC(SMAFELOG_FATAL, s);
			exit(1);
		}

		for(std::vector<long>::iterator iter_fvt_id = fvtype_ids.begin(); iter_fvt_id < fvtype_ids.end(); iter_fvt_id++) {
			long fvt_id = *iter_fvt_id;
			bool bFvtFound = false;
			SmafeFVType* curFvt;

			// this fvtype is in db?
			for(std::vector<SmafeFVType_Ptr>::iterator iter = fvtypes.begin(); iter < fvtypes.end(); iter++) {
				if (iter->get()->id == fvt_id) {
					curFvt = iter->get();
					bFvtFound = true;
					// quit loop
					break;
				}
			}
			// if no, fatal error
			if (!bFvtFound) {
				SMAFELOG_FUNC(SMAFELOG_FATAL, "Feature vector type id " + stringify(fvt_id) + " not found in database.");
				exit(1);
			}

			SMAFELOG_FUNC(SMAFELOG_INFO, "Starting attribute-wise normalization of feature vector type " + curFvt->name + " (id=" + stringify(curFvt->id)+ ")");
			doNorm(curFvt);
		}
		delete so;

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
