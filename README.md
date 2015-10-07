# SEARCH by Sound Platform: SMAFE + SMINT

## Introduction

SEARCH by Sound takes any sample song and finds songs that sound similar to it.
It includes SMAFE, the audio analysis and similartiy computation engine and SMINT API, to query for similar songs.

SMAFE is the Spectralmind Audio Feature Extractor, the core of Spectralmind's music analysis software package. 
It analyzes the audio content of a WAV or MP3 file and performs sound-similarity searches based on audio elements capturing timbre, tempo and rhythmic feel. SMINT contains the REST API to add/query/delete similar sounding songs. 

SEARCH by Sound with SMAFE and SMINT are published under the MIT license (see LICENSE file in the same directory for the complete terms).

The source code in this repository contains the following modules:
* Feature Extraction / Analysis
* Persistence (PostgreSQL)
* Basic task scheduler
* Query Engine
* REST API (PHP)

### Spectralmind

Spectralmind was an innovative media technology company founded 2008 by a group of music enthusiasts and semantic audio analysis experts in Vienna, Austria:

Thomas Lidy, Ewald Peiszer, Johann Waldherr and Wolfgang Jochum

Spectralmind’s audio analysis and music discovery applications allow computers to hear music in a similar way as humans do and consequently to find and recommend music by its mere content. This technology is an enabler for solutions in media search, categorization and recommendation.

In addition to the SEARCH by Sound Platform for audio content analysis, Spectralmind also created music discovery applications for Web, iOS and Android, foremost [Sonarflow] (http://www.sonarflow.com) (also see below).

Spectralmind ceased operations as of September 2015 and published its software stack as open source software under the MIT license.

### Available software

Spectralmind's open source software is comprised of four repositories:

* [SEARCH by Sound Platform a.k.a. Smafe](https://www.github.com/spectralmind/smafe)
* [SEARCH by Sound Web Application a.k.a. Smint](https://www.github.com/spectralmind/smint)
* [Sonarflow iOS App](https://www.github.com/spectralmind/sonarflow-ios)
* [Sonarflow Android App](https://www.github.com/spectralmind/sonarflow-android)

### Resources

The following resources are available:

* `README` - this file
* `INSTALL` - information on building and deploying Smafe
* `doc/Search by sound backend documentation 0.9.pdf` - Short operations manual
* `doc/API-Documentation v1.1 - SEARCH by Sound release 1.3.pdf` - API documentation and system requirements

## Build process

SMAFE can be built on POSIX compatible systems. Most documentation refers to building it on Debian style Linux, e.g., Ubuntu.

Please refer to INSTALL file in the same directory for concrete instructions and a list of dependencies.

### Create deployment package

The script **mkdeploy** in subdirectory misc can be used to create a self-contained installation package for deployment on a different system.

Usage: `./mkdeploy <config file> [configure parameters]`

Note that the script is currently expected to be run from the misc subdirectory.

Influental environment vars are: INTEL_IPP_FOLDER, BOOST_LIB_FOLDER (location of Boost libs), MPG123_LIB_FOLDER, LIB_FOLDER (standard location of other libs).

Config file: 
An example can be found in /test/resources/test.opt. 

It creates bzipped2 tar files `$HOME/smafedeploytmp/smafedeploy.tar.bz2` and `$HOME/smafedeploytmp/smafedeploy-libs.tar.bz2`, and copies the INSTALL and smafesetup.sh file.

## Support

As Spectralmind ceased operation, no support can be given by the company. Please contact any active members on github, or otherwise you can still try technology@spectralmind.com .

## Acknowledgement

We wish to thank all the contributors to this software, with a special thank you to all former employees and freelancers of Spectralmind.

September 2015
The Founders of Spectralmind: Thomas Lidy, Ewald Peiszer, Johann Waldherr 



