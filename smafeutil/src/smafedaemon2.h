///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2010 spectralmind
// All rights reserved.
//
// ------------------------------------------------------------------------
// smafedaemon1.h
//
// Daemon preparation
// Include file / header file - functions for daemonizing
// ------------------------------------------------------------------------
//
//
// Version $Id: smafequery.cpp 166 2009-03-24 21:25:13Z ewald $
//
//
//
//
//
//
///////////////////////////////////////////////////////////////////////////


#pragma once

//-----------------------------------------------------------------------------
#if defined(SMAFEDISTD_REAL_DAEMON)
//-----------------------------------------------------------------------------

/** start smafedistd as normal process and not a daemon?
 * This can be set using a command line argument
 */
bool bNoDaemon = false;

static void child_handler(int signum)
{
	SMAFELOG_FUNC(SMAFELOG_DEBUG, "Signal caught: " + stringify(strsignal(signum)) + " (" + stringify(signum) +  ")");
	switch(signum) {
		case SIGALRM: exit(0); break;
		case SIGUSR1:
		SMAFELOG_FUNC(SMAFELOG_DEBUG2, "Exiting PID " + stringify(getpid()));
		exit(0);
		break;
		case SIGCHLD: exit(0); break;
		case SIGTERM:
		SMAFELOG_FUNC(SMAFELOG_INFO, "Will stop daemon after next job is finished.");
		b_should_terminate = true;
		break;
	}
}

static void daemonize( void )
{
	pid_t pid, sid, parent;

	/* already a daemon */
	if ( getppid() == 1 ) return;

	/* Drop user if there is one, and we were run as root */
	/* Do not change user now
	 if ( getuid() == 0 || geteuid() == 0 ) {
	 struct passwd *pw = getpwnam(RUN_AS_USER);
	 if ( pw ) {
	 syslog( LOG_NOTICE, "setting user to " RUN_AS_USER );
	 setuid( pw->pw_uid );
	 }
	 }
	 */

	/* Trap signals that we expect to recieve */
	signal(SIGCHLD,child_handler);
	signal(SIGUSR1,child_handler);
	signal(SIGALRM,child_handler);

	/* Fork off the parent process */
	pid = fork();
	if (pid < 0) {
		SMAFELOG_FUNC(SMAFELOG_FATAL, "unable to fork daemon, code=" + stringify(errno) + " (" +
				stringify(strerror(errno) ) + ")");
		exit(1);
	}
	/* If we got a good PID, then we can exit the parent process. */
	if (pid > 0) {

		/* Wait for confirmation from the child via SIGTERM or SIGCHLD, or
		 for two seconds to elapse (SIGALRM).  pause() should not return. */
		alarm(2);
		pause();
		exit(1);
	}

	/* At this point we are executing as the child process */
	parent = getppid();

	/* Cancel certain signals */
	signal(SIGCHLD,SIG_DFL); /* A child process dies */
	signal(SIGTSTP,SIG_IGN); /* Various TTY signals */
	signal(SIGTTOU,SIG_IGN);
	signal(SIGTTIN,SIG_IGN);
	signal(SIGHUP, SIG_IGN); /* Ignore hangup signal */
	//	signal(SIGTERM,SIG_DFL); /* Die on SIGTERM */
	signal(SIGTERM,child_handler); /* handle TERM signal */

	/* Change the file mode mask */
	umask(0);

	/* Create a new SID for the child process */
	sid = setsid();
	if (sid < 0) {
		SMAFELOG_FUNC(SMAFELOG_FATAL, "unable to create a new session, code=" + stringify(errno) + " (" +
				stringify(strerror(errno) ) + ")");
		exit(1);
	}

	/* Change the current working directory.  This prevents the current
	 directory from being locked; hence not being able to remove it. */
	if ((chdir("/")) < 0) {
		SMAFELOG_FUNC(SMAFELOG_FATAL, "unable to change directory to /, code=" + stringify(errno) + " (" +
				stringify(strerror(errno) ) + ")");
		exit(1);
	}

	// output pid for convencience
	std::cout << "PID of this daemon instance is " << getpid() << std::endl;
	SMAFELOG_FUNC(SMAFELOG_INFO, "PID of this daemon instance is " + stringify(getpid()));

	/* Redirect standard files to /dev/null */
	//	if (!SMAFEDISTD_DEBUG) {
	FILE * fret;
	fret = freopen( "/dev/null", "r", stdin);
	if (fret == NULL) {
		SMAFELOG_FUNC(SMAFELOG_WARNING, "unable to redirect stdin stream");
	}
	fret = freopen( "/dev/null", "w", stdout);
	if (fret == NULL) {
		SMAFELOG_FUNC(SMAFELOG_WARNING, "unable to redirect stdout stream");
	}
	fret = freopen( "/dev/null", "w", stderr);
	if (fret == NULL) {
		SMAFELOG_FUNC(SMAFELOG_WARNING, "unable to redirect stderr stream");
	}
	//	}

	/* Tell the parent process that we are A-okay */
	//kill( parent, SIGUSR1 );

	SMAFELOG_FUNC(SMAFELOG_DEBUG, "Now I am a daemon!");
}
//-----------------------------------------------------------------------------
#else
//-----------------------------------------------------------------------------

// daemon mode is NOT possible

/** start smafedistd as normal process and not a daemon?
 * This can be set using a command line argument
 */
const bool bNoDaemon = true;

//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------

