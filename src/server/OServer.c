/* */
        
int ServerSetSignalHandlers()

  {
  /* trap TERM(15) QUIT(3) INT(2) HUP(1) */
 
  signal(SIGINT,   SIG_IGN);
  signal(SIGTERM,  MSysShutdown);
  signal(SIGQUIT,  MSysShutdown);
  signal(SIGIO,    MSysShutdown);
  signal(SIGURG,   MSysShutdown);
 
  /* handle SIGPIPE/SIGHUP/SIGSEGV/SIGILL */

  ServerLoadSignalConfig();
 
/* NOTE: HUP temporarily disabled  *
  signal(SIGHUP,   (void(*)(int))ReloadConfig);
*/
  signal(SIGHUP,   SIG_IGN);
  signal(SIGPIPE,  SIG_IGN);
  signal(SIGSEGV,  MSysShutdown);
  signal(SIGILL,   MSysShutdown);

  return(SUCCESS);
  }  /* END ServerSetSignalHandlers() */




int ServerDemonize()
{
  int   pid;

  const char *FName = "ServerDemonize";

  DBG(2,fALL) DPrint("%s()\n",
    FName);

  /* create private process group */

  if (MSched.Mode != msmSim)
    {
    if (setpgid(0,0) == -1)
      {
      perror("cannot set process group");

      DBG(0,fALL) DPrint("ERROR:    cannot setpgrp, errno: %d (%s)\n",
        errno,
        strerror(errno));
      }

    fflush(mlog.logfp);

    if (MSched.DebugMode == FALSE)
      {
      /* only background if not in debug mode */

      /* NOTE:  setsid() disconnects from controlling-terminal */

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

      if (setsid() == -1)
        {
        MDB(3,fALL) MLog("INFO:     could not disconnect from controlling-terminal, errno=%d - %s\n",
          errno,
          strerror(errno));
        }

      /* disconnect stdin */

      fclose(stdin);

      /* disconnect stdout */

      fclose(stdout);

      /* disconnect stderr */

      fclose(stderr);

      }
    }    /* END if (MSched.Mode != msmSim) */

  return(SUCCESS);
  }  /* END ServerDemonize() */






void CrashMode(

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
    MULToDString(&Time));
 
  if (MSched.Mode == msmSim)
    {
    DBG(1,fALL) DPrint("CRASH MODE:  SIMULATION TIME: (%ld) %s\n",
      MSched.Time,
      MULToDString(&MSched.Time));
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
    signal(SIGSEGV,CrashMode);
    }
  else if (signo == SIGILL)
    {
    signal(SIGILL,CrashMode);
    }

  DBG(0,fALL) DPrint("INFO:     exiting CRASH MODE\n");

  return;
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




void ReloadConfig(

  int signo)

  {
  DBG(2,fCONFIG) DPrint("ReloadConfig()\n");

  DBG(1,fCONFIG) DPrint("INFO:     received signal %d.  reloading config at next interval\n",
    signo);

  MSched.Reload = TRUE;

  signal(SIGHUP,ReloadConfig);

  return;
  }  /* END ReloadConfig() */

/* END OServer.c */
