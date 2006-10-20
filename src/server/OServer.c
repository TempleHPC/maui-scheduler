/* */
        
int ServerSetSignalHandlers()

  {
  /* trap TERM(15) QUIT(3) INT(2) HUP(1) */
 
#if defined(__AIX41) || defined(__AIX42) || defined(__AIX43) || defined(__AIX51) || defined(__IRIX) || defined(__LINUX) || defined(__HPUX) || defined(__SOLARIS) || defined(__OSF) || defined(__FREEBSD)
 
  signal(SIGINT,   SIG_IGN);
  signal(SIGTERM,  (void(*)(int))MSysShutdown);
  signal(SIGQUIT,  (void(*)(int))MSysShutdown);
  signal(SIGIO,    (void(*)(int))MSysShutdown);
  signal(SIGURG,   (void(*)(int))MSysShutdown);
 
#else /* ... */
 
  signal(SIGINT,   SIG_IGN);
  signal(SIGTERM,  MSysShutdown);
  signal(SIGQUIT,  MSysShutdown);
  signal(SIGIO,    MSysShutdown);
  signal(SIGURG,   MSysShutdown);
 
#endif /* ... */
 
  /* handle SIGPIPE/SIGHUP/SIGSEGV/SIGILL */
 
#if defined(__AIX41) || defined(__AIX42) || defined(__AIX43) || defined(__AIX51)

  {
  char             *ptr;
  struct sigaction  act;

  act.sa_handler = SIG_IGN;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
 
  if (sigaction(SIGPIPE,&act,NULL) == -1) 
    {
    DBG(0,fCORE) DPrint("ERROR:    cannot ignore SIGPIPE, errno: %d (%s)\n",
      errno,
      strerror(errno));
    }
 
/* NOTE:  HUP temporarily disabled
  signal(SIGHUP,   (void(*)(int))ReloadConfig);
*/
  signal(SIGHUP,SIG_IGN);
 
  /* set up SIGSEGV and SIGILL signal handlers */
 
  sigaction(SIGSEGV,0,&act);
 
  act.sa_flags |= SA_FULLDUMP;
 
  if ((ptr = getenv(MSCHED_ENVCRASHVAR)) == NULL)
    {
    act.sa_handler = (void(*)(int))SIG_DFL;
    }
  else if (!strcmp(ptr,"TRAP") || !strcmp(ptr,"trap"))
    {
    act.sa_handler = (void(*)(int))CrashMode;
    }
  else if (!strcmp(ptr,"IGNORE") || !strcmp(ptr,"ignore"))
    {
    act.sa_handler = (void(*)(int))SIG_IGN;
    }
  else if (!strcmp(ptr,"DIE") || !strcmp(ptr,"die"))
    {
    act.sa_handler = (void(*)(int))SIG_DFL;
    }
  else
    {
    act.sa_handler = (void(*)(int))ServerRestart;
    } 
 
  /* create full core dumps on SEGV and ILL */
 
  if (sigaction(SIGSEGV,&act,NULL) < 0)
    {
    perror("cannot set up SIGSEGV signal handler");
 
    DBG(0,fALL) DPrint("ERROR:    cannot set up SEGV error handler, errno: %d (%s)\n",
      errno,
      strerror(errno));
    }
 
  sigaction(SIGILL,0,&act);
 
  act.sa_flags |= SA_FULLDUMP;
 
  if ((ptr = getenv(MSCHED_ENVCRASHVAR)) == NULL)
    {
    act.sa_handler = (void(*)(int))SIG_DFL;
    }
  else if (!strcmp(ptr,"TRAP") || !strcmp(ptr,"trap"))
    {
    act.sa_handler = (void(*)(int))CrashMode;
    }
  else if (!strcmp(ptr,"IGNORE") || !strcmp(ptr,"ignore"))
    {
    act.sa_handler = (void(*)(int))SIG_IGN;
    }
  else if (!strcmp(ptr,"DIE") || !strcmp(ptr,"die"))
    {
    act.sa_handler = (void(*)(int))SIG_DFL;
    }
  else
    {
    act.sa_handler = (void(*)(int))ServerRestart;
    } 
 
  if (sigaction(SIGILL,&act,NULL) < 0)
    {
    perror("cannot set up SIGILL signal handler");
 
    DBG(0,fALL) DPrint("ERROR:    cannot set up SIGILL signal handler, errno: %d (%s)\n",
      errno,
      strerror(errno));
    }
  }    /* END BLOCK */ 

#elif defined(__LINUX) || defined(__IRIX) || defined(__HPUX) || defined(__SOLARIS) || defined(__OSF)

  ServerLoadSignalConfig();
 
#else /* PIPE/HUP/SEGV */
 
/* NOTE: HUP temporarily disabled  *
  signal(SIGHUP,   (void(*)(int))ReloadConfig);
*/
  signal(SIGHUP,   SIG_IGN);
  signal(SIGPIPE,  SIG_IGN);
  signal(SIGSEGV,  MSysShutdown);
  signal(SIGILL,   MSysShutdown);
 
#endif /* PIPE/HUP/SEGV */

  return(SUCCESS);
  }  /* END ServerSetSignalHandlers() */




int ServerDemonize()

  {
#ifndef __NT
  int   pid;
#endif /* __NT */

  const char *FName = "ServerDemonize";

  DBG(2,fALL) DPrint("%s()\n",
    FName);

  /* create private process group */

  if (MSched.Mode != msmSim)
    {
#if !defined(__NT) && !defined(__HPUX)
#if defined(__OSF) || defined(__FREEBSD)
    if (setpgrp((pid_t)0,(pid_t)0) == -1)
#else /* __OSF */
    if (setpgrp() == -1)
#endif /* __OSF */
      {
      perror("cannot set process group");

      DBG(0,fALL) DPrint("ERROR:    cannot setpgrp, errno: %d (%s)\n",
        errno,
        strerror(errno));
      }
#endif /* !defined(__NT) && !defined(__HPUX) */

    fflush(mlog.logfp);

    if (MSched.DebugMode == FALSE)
      {
      /* only background if not in debug mode */

#ifndef __NT

      if ((pid = fork()) == -1)
        {
        perror("cannot fork");

        DBG(0,fALL) DPrint("ALERT:    cannot fork into background, errno: %d (%s)\n",
          errno,
          strerror(errno));
        }

      if (pid != 0)
        {
        /* exit if parent */

        DBG(3,fALL) DPrint("INFO:     parent is exiting\n");

        fflush(mlog.logfp);

        exit(0);
        }
      else
        {
        DBG(3,fALL) DPrint("INFO:     child process in background\n");
        }

#endif /* __NT */
      }
    }    /* END if (MSched.Mode != msmSim) */

  return(SUCCESS);
  }  /* END ServerDemonize() */






int CrashMode(

  int signo)

  {
  long   Time;
  time_t tmpTime;

  char Line[MAX_MLINE];

  DBG(1,fALL) DPrint("CrashMode(%d)\n",
    signo);

  MSched.Schedule = FALSE;

  MSched.CrashMode = TRUE;

  time(&tmpTime);
  Time = (long)tmpTime;

  DBG(1,fALL) DPrint("CRASH MODE:  crash occurred on iteration %d (Debug: %d) time: %s\n",
    MSched.Iteration,
    mlog.Threshold,
    MULToDString((mulong *)&Time));
 
  if (MSched.Mode == msmSim)
    {
    DBG(1,fALL) DPrint("CRASH MODE:  SIMULATION TIME: (%ld) %s\n",
      MSched.Time,
      MULToDString((mulong *)&MSched.Time));
    }

  mlog.Threshold = 20;

  if (MSched.Mode == msmNormal)
    {
    MOSSyslog(LOG_ERR,"ERROR:  %s has crashed, signo: %d",
      MSCHED_SNAME,
      signo);
    }

  sprintf(Line,"scheduler has crashed.  Signo: %d",
    signo);

  MSysRegEvent(Line,0,0,1);

  fflush(mlog.logfp);

  /* save log file */

  MLogRoll(".crash",1,MSched.Iteration,MSched.LogFileRollDepth);

  fprintf(stderr,"ALERT:    created crash file\n");

  sleep(2);

  DBG(1,fALL) DPrint("CRASH MODE:  attempting to read socket connection\n");

  UIProcessClients(&MSched.ServerS,MSched.RMPollInterval);

  if (signo == SIGSEGV)
    {
#if defined(__AIX41) || defined(__AIX42) || defined(__AIX43) || defined(__AIX51) || defined(__LINUX) || defined(__HPUX) || defined(__IRIX) || defined(__SOLARIS) || defined(__OSF) || defined(__FREEBSD)

    signal(SIGSEGV,(void(*)(int))CrashMode);

#else

    signal(SIGSEGV,(CrashMode));

#endif
    }
  else if (signo == SIGILL)
    {
#if defined(__AIX41) || defined(__AIX42) || defined(__AIX43) || defined(__AIX51) || defined(__LINUX) || defined(__HPUX) || defined(__IRIX) || defined(__SOLARIS) || defined(__OSF) || defined(__FREEBSD)

    signal(SIGILL,(void(*)(int))CrashMode);

#else

    signal(SIGILL,(CrashMode));

#endif
    }

  DBG(0,fALL) DPrint("INFO:     exiting CRASH MODE\n");

  return(SUCCESS);
  }  /* END CrashMode() */




void ServerRestart(

  int signo)

  {
  int  rc;

  const char *FName = "ServerRestart";

  DBG(1,fALL) DPrint("%s(%d)\n",
    FName,
    signo);

  if ((rc = execv(MSched.Argv[0],MSched.Argv)) == -1)
    {
    /* exec failed */

    DBG(1,fCORE) DPrint("ERROR:    cannot restart scheduler '%s' rc: %d\n",
      MSched.Argv[0],
      rc);

    exit(1);
    }

  exit(0);
  }  /* END ServerRestart() */




int ReloadConfig(

  int signo)

  {
  DBG(2,fCONFIG) DPrint("ReloadConfig()\n");

  DBG(1,fCONFIG) DPrint("INFO:     received signal %d.  reloading config at next interval\n",
    signo);

  MSched.Reload = TRUE;

#if defined(__AIX41) || defined(__AIX42) || defined(__AIX43) || defined(__AIX51) || defined(__LINUX) || defined(__HPUX) || defined(__IRIX) || defined(__SOLARIS) || defined(__OSF) || defined(__FREEBSD)

  signal(SIGHUP,(void(*)(int))ReloadConfig);

#else

  signal(SIGHUP,ReloadConfig);

#endif

  return(SUCCESS);
  }  /* END ReloadConfig() */

/* END OServer.c */
