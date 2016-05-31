/*
*/

int __OMCLoadArgs(

  char  Flag,    /* I */
  char *OptArg,  /* I */
  int  *sindex)

  {
  int index;

  switch ((char)Flag)
    {
    case 'a':

      switch(*sindex)
        {
        case svcDiagnose:

          /* a-ACCOUNT */

          DiagMode = mxoAcct;

          break;

        case svcShowBackfillWindow:

          /* a-ACCOUNT */

          if ((OptArg == NULL) || (OptArg[0] == '\0'))
            {
            MCShowUsage(*sindex);

            exit(1);
            }
          else
            {
            MUStrCpy(Account,OptArg,sizeof(Account));
            }

          break;

        case svcSetJobHold:
        case svcReleaseJobHold:

          /* a-ALL */

          HType = mhAll;

          break;

        case svcShowStats:

          /* a-ACCOUNT */

          ObjectType = mxoAcct;     

          if ((OptArg != NULL) && (OptArg[0] != '\0'))
            {
            MUStrCpy(ObjectID,OptArg,sizeof(ObjectID));
            }

          break;

        case svcResCreate:

          /* a-ACCOUNTLIST */

          if (OptArg == NULL)
            {
            MCShowUsage(*sindex);

            exit(1);
            }

          MUStrCpy(AccountList,OptArg,sizeof(AccountList));

          break;

        default:

          MCShowUsage(*sindex);

          exit(1);

          break;
        }  /* END switch(*sindex) */

      break;

    case 'A':

      switch(*sindex)
        {
        case svcJobShow:
        case svcNodeShow:

            /* A-AVP */

            Flags |= (1 << mcmParse);

            break;

          case svcShowBackfillWindow:

            /* A-ALL */

            strcpy(UserName,ALL);
            strcpy(Group,ALL);
            strcpy(Account,ALL);

            break;

          case svcResCreate:

            /* A-CHARGEACCOUNT */

            if (OptArg == NULL)
              {
              MCShowUsage(*sindex);

              exit(1);
              }

            MUStrCpy(ChargeAccount,OptArg,sizeof(ChargeAccount));

            break;

          default:

            MCShowUsage(*sindex);

            exit(1);

            break;
          }  /* END switch(*sindex) */

        break;

      case 'b':

        switch (*sindex)
          {
          case svcSetJobHold:
          case svcReleaseJobHold:

            /* b-BATCH */

            HType = mhBatch;

            break;

          case svcShowQ:

            /* b-BLOCKING */

            QueueMode = 3;

            break;

          default:

            MCShowUsage(*sindex);

            exit(1);
 
            break;
          }

        break;

      case 'c':

        switch(*sindex)
          {
          case svcShowStats:
 
            /* c-CLASS */
 
            ObjectType = mxoClass;
 
            if ((OptArg != NULL) && (OptArg[0] != '\0'))
              {
              MUStrCpy(ObjectID,OptArg,sizeof(ObjectID));
              }
 
            break;

          case svcMJobCtl:

            /* c-CANCEL */

            JobCtlMode = mjcmCancel;

            break;

          case svcMGridCtl:

            /* c-COMMIT */

            MGridMode = mcCommit;

            break;

          case svcRunJob:
 
            /* c-CLEAR */
 
            Flags = mcmClear;
 
            break;

          case svcBNFQuery:

            /* c-COMMAND */

            if (OptArg == NULL)
              {
              MCShowUsage(*sindex);

              exit(1);
              }

            MUStrCpy(QLine,OptArg,sizeof(QLine));

            break;

          case svcDiagnose:
 
            /* c-CLASS */
 
            DiagMode = mxoClass;
 
            break;

          case svcResCreate:

            /* c-CLASSLIST */

            if (OptArg == NULL)
              {
              MCShowUsage(*sindex);

              exit(1);
              }

            MUStrCpy(ClassList,OptArg,sizeof(ClassList));

            break;

          case svcShowBackfillWindow:

            if (!strcmp(OptArg,"ALL"))
              {
              strcpy(ClassString,ALL);
              }
            else
              {
              MUStrCpy(ClassString,OptArg,sizeof(ClassString));
              }

            break;

          default:

            MCShowUsage(*sindex);

            exit(1);

            break;
          }

        break;

      case 'd':

        switch(*sindex)
          {
          case svcShowStats:
 
            /* d-DEPTH (Requires Arg) */

            if (OptArg == NULL)
              {
              MCShowUsage(*sindex);

              exit(1);
              }

            if (OptArg[0] == '\0')
              {
              MCShowUsage(*sindex);

              exit(1);
              }

            Flags |= atoi(OptArg);

            break;

          case svcResCreate:
          case svcShowBackfillWindow:

            /* d-DURATION (Requires Arg) */

            if (OptArg == NULL)
              {
              MCShowUsage(*sindex);

              exit(1);
              }

            Duration = MUTimeFromString(OptArg);

            break;

          default:

            MCShowUsage(*sindex);

            exit(1);

            break;
          }  /* END switch(*sindex) */

        break;

      case 'e':

        switch(*sindex)
          {
          case svcResCreate:

            /* e-ENDTIME */

            if (OptArg == NULL)
              {
              MCShowUsage(*sindex);

              exit(1);
              }

            if (MUStringToE(OptArg,&EndTime) != SUCCESS)
              {
              MCShowUsage(*sindex);

              fprintf(stderr,"ERROR:    invalid endtime specified, '%s'\n",
                OptArg);

              exit(1);
              }

            break;

          default:

            MCShowUsage(*sindex);

            exit(1);

            break;
          }

        break;

      case 'f':

        switch(*sindex)
          {
          case svcSched:
 
            /* f-FAILURE */
 
            SchedMode = msctlFailure;
 
            if (OptArg != NULL)
              {
              MUStrCpy(SchedArg,OptArg,sizeof(SchedArg));
              }
 
            break;

          case svcDiagnose:

            /* f-FAIRSHARE */

            DiagMode = mxoFS;

            break;

          case svcRunJob:

            /* f-FORCE */

            Flags = mcmForce;

            break;

          case svcShowBackfillWindow:
          case svcResCreate:

            MUStrCpy(FeatureString,OptArg,sizeof(FeatureString));

            break;

          default:

            MCShowUsage(*sindex);

            exit(1);

            break;
          }  /* END switch(*sindex) */

        break;

      case 'F':

        /* F-LOGFACILITY */

        if (OptArg == NULL)
          {
          MCShowUsage(*sindex);

          exit(1);
          }

        mlog.FacilityList = atoi(OptArg);

        DBG(2,fCONFIG) DPrint("INFO:     LOGFACILITY set to %ld\n",
          mlog.FacilityList);

        break;
 
      case 'g':

        switch(*sindex)
          { 
          case svcDiagnose:

            /* g-GROUP */

            DiagMode = mxoGroup;

            break;

          case svcResCreate:

            /* g-GROUPLIST */

            if (OptArg == NULL)
              {
              MCShowUsage(*sindex);

              exit(1);
              }

            MUStrCpy(GroupList,OptArg,sizeof(GroupList));

            break;

          case svcShowStats:

            /* g-GROUP */

            ObjectType = mxoGroup;

            if ((OptArg != NULL) && (OptArg[0] != '\0'))
              {
              MUStrCpy(ObjectID,OptArg,sizeof(ObjectID));
              }

            break;

          case svcResShow:

            /* g-GREP */

            Mode |= (1 << mcmGrep);

            break;

          case svcShowBackfillWindow:

            /* g-GROUP */

            if ((OptArg == NULL) || (OptArg[0] == '\0'))
              {
              MCShowUsage(*sindex);

              exit(1);
              }

            MUStrCpy(Group,OptArg,sizeof(Group));

            break;

          default:

            MCShowUsage(*sindex);

            exit(1);
  
            break;
          }

        break;

      case 'i':

        switch(*sindex)
          {
          case svcShowQ:

            /* i-IDLE */

            QueueMode = 1;

            break;

          case svcSched:

            /* i-INIT */

            SchedMode = msctlInit;

	    if (OptArg[0] != '-')
              MUStrCpy(SchedArg,OptArg,sizeof(SchedArg));

            break;

          default:

            MCShowUsage(*sindex);

            exit(1);

            break;
          }

        break;

      case 'j':

        switch(*sindex)
          {
          case svcSched:
 
            /* j-Job */
 
            SchedMode = msctlSubmit;

            MUStrCpy(SchedArg,OptArg,sizeof(SchedArg));           
 
            break;

          case svcDiagnose:
         
            /* j-JOB */
 
            DiagMode = mxoJob;

            break;

          case svcResCreate:

            MUStrCpy(JobFeatureString,OptArg,sizeof(JobFeatureString));

            break;

          default:

            MCShowUsage(*sindex);

            exit(1);
  
            break;
          }  /* END switch(*sindex) */

        break;

      case 'k':

        switch(*sindex)
          {
          case svcSched:
          
            /* k-KILL */

            SchedMode = msctlKill;

            break;

          default:

            MCShowUsage(*sindex);

            exit(1);
 
            break;
          }

        break;

      case 'l':

        switch(*sindex)
          {
          case svcMGridCtl:

            /* l-LIST */

            MGridMode = mcList;

            break;

          case svcJobShow:
          case svcDiagnose:

            switch(OptArg[0])
              {
              case 'o':
              case 'O':

                /* o-OFFPOLICY */

                PType = ptOFF;

                break;

              case 'h':
              case 'H':

                /* h-HARDPOLICY */
 
                PType = ptHARD;

                break;

              case 's':
              case 'S':

                /* s-SOFTPOLICY */

                PType = ptSOFT;

                break;

              default:

                MCShowUsage(*sindex);

                exit(1);

                break;
              }

            break;

          case svcSched:
 
            /* l-LIST */
 
            SchedMode = msctlList;
 
            if (OptArg != NULL)
              {
              MUStrCpy(SchedArg,OptArg,sizeof(SchedArg));
              }
 
            break;

          default:

            MCShowUsage(*sindex);

            exit(1);

            break;
          }

        break;

      case 'm':

        switch(*sindex)
          {
          case svcMJobCtl:
 
            /* m-MODIFY */
 
            JobCtlMode = mjcmModify;
 
            break;

          case svcMGridCtl:

            /* m-MODIFY */

            MGridMode = mcModify;

            break;

          case svcDiagnose:

            /* m-FRAME */

            DiagMode = mxoFrame;

            break;

          case svcShowBackfillWindow:

            /* m-MEMORY */

            if (OptArg == NULL)
              {
              MCShowUsage(*sindex);

              exit(1);
              }

            if (isdigit(OptArg[0]))
              {
              /* if no comparison given */

              Memory = (int)strtol(OptArg,NULL,0);
              MIndex = mcmpGE;
              }
            else
              {
              char tmpLine[MAX_MLINE];

              /* if comparison given */

              for (index = 0;ispunct(OptArg[index]);index++);

              strncpy(tmpLine,OptArg,index);
              tmpLine[index] = '\0';

              DBG(1,fCONFIG) DPrint("INFO:     arg: '%s' comparison '%s' (%d)\n",
                OptArg,
                tmpLine,
                index);

              for (MIndex = 0;MComp[MIndex] != NULL;MIndex++)
                {
                if (!strcmp(tmpLine,MComp[MIndex]))
                  break;
                }

              if (MComp[MIndex] == NULL)
                MIndex = 0;

              sscanf((OptArg + index),"%d",
                &Memory);
              }

            break;              

          case svcSched:
 
            /* m-MODIFY */
 
            SchedMode = msctlModify;
 
            if (OptArg == NULL)
              {
              MCShowUsage(*sindex);
 
              exit(1);
              }
 
            MUStrCpy(SchedArg,OptArg,sizeof(SchedArg));
 
            break;

          default:

            MCShowUsage(*sindex);

            exit(1);

            break;
          }    /* END switch(*sindex) */

        break;

      case 'M':

        switch(*sindex)
          {
          case svcShowBackfillWindow:

            /* m-DMEMORY */

            if (OptArg == NULL)
              {
              MCShowUsage(*sindex);

              exit(1);
              }

            DMemory = atoi(OptArg);
 
            break;

         default:

            MCShowUsage(*sindex);

            exit(1);

            break;
          }    /* END switch(*sindex) */

        break;

      case 'n':

        switch(*sindex)
          {
          case svcResCreate:

            /* n-Reservation Name */

            if (OptArg == NULL)
              {
              MCShowUsage(*sindex);
 
              exit(1);
              }
            else
              {
              MUStrCpy(ObjectID,OptArg,sizeof(ObjectID));
              }
 
            break;

          case svcSched:
 
            /* n-NODETABLE */
 
            SchedMode = msctlNodeTable;
 
            break;           

          case svcRunJob:
          case svcJobShow:

            /* n-NODELIST */

            if (OptArg == NULL)
              {
              MCShowUsage(*sindex);

              exit(1);
              }

            if (strlen(OptArg) >= sizeof(NodeRegEx))
              {
              fprintf(stderr,"ERROR:    expression too long. (%d > %d)\n",
                (int)strlen(OptArg),
                (int)sizeof(NodeRegEx));

              exit(1);
              }

            MUStrCpy(NodeRegEx,OptArg,sizeof(NodeRegEx));

            break;

          case svcShowStats:
          case svcResShow:

            /* n-NODE (No Argument) */

            ObjectType = mxoNode;

            break;

          case svcDiagnose:

            /* n-NODE */

            DiagMode = mxoNode;

            break;

          case svcShowBackfillWindow:

            /* n-NODE */

            if (OptArg == NULL)
              {
              MCShowUsage(*sindex);

              exit(1);
              }

            NodeCount = atoi(OptArg);

            break;

          default:

            MCShowUsage(*sindex);

            exit(1);

            break;
          }

        break;

      case 'N':

        switch(*sindex)
          {
          case svcResCreate:

            if ((OptArg == NULL) || (OptArg[0] == '\0'))
              {
              MCShowUsage(*sindex);

              exit(1);
              }

            MUStrCpy(NodeSetString,OptArg,sizeof(NodeSetString));
              
            break;

          default:

            MCShowUsage(*sindex);

            exit(1);

            break;
          }

        break;

      case 'o':

        switch(*sindex)
          {
          default:

            MCShowUsage(*sindex);

            exit(1);

            break;
          }

        break;

      case 'p':

        switch(*sindex)
          {
          case svcResCreate:

            /* p-PARTITION (REQUIRED ARG) */

            if (OptArg == NULL)
              {
              MCShowUsage(*sindex);

              exit(1);
              }

            if (OptArg[0] != '\0')
              {
              MUStrCpy(ParName,OptArg,sizeof(ParName));
              }

            break;
 
          case svcShowQ:
          case svcResShow:
          case svcShowBackfillWindow:
          case svcRunJob:

            /* p-PARTITION (REQUIRED ARG) */

            if (OptArg == NULL)
              {
              MCShowUsage(*sindex);

              exit(1);
              }

            if (OptArg[0] != '\0')
              {
              MUStrCpy(ParName,OptArg,sizeof(ParName));
              }
            else
              {
              MCShowUsage(*sindex);

              exit(1);
              }
 
            DBG(4,fCONFIG) DPrint("INFO:     partition set to %s\n",
              ParName);
 
            break;

          case svcDiagnose:

            /* p-PRIORITY */

            DiagMode = mxoPriority;

            break;

          default:

            MCShowUsage(*sindex);

            exit(1);

            break;
          }  /* END switch(*sindex) */

        break;

      case 'q':

        switch(*sindex)
          {
          case svcShowStats:
 
            /* q-QOS */
 
            ObjectType = mxoQOS;
 
            if ((OptArg != NULL) && (OptArg[0] != '\0'))
              {
              MUStrCpy(ObjectID,OptArg,sizeof(ObjectID));
              }
 
            break;

          case svcResCreate:
 
            /* q-QUEUELIST */
 
            if (OptArg == NULL)
              {
              MCShowUsage(*sindex);
 
              exit(1);
              }
 
            MUStrCpy(ClassList,OptArg,sizeof(ClassList));
 
            break;

          case svcMGridCtl:

            /* q-QUERY */

            MGridMode = mcQuery;

            break;

          case svcDiagnose:

            /* q-QUEUE */

            DiagMode = mxoQueue;

            break;

          case svcShowBackfillWindow:

            /* q-QOS */

            MUStrCpy(QOSName,OptArg,sizeof(QOSName));

            break;
 
          default:

            MCShowUsage(*sindex);

            exit(1);

            break;
          }

        break;

      case 'Q':

        switch(*sindex)
          {
          case svcResCreate:

            /* Q-QOSLIST */
 
            if (OptArg == NULL)
              {
              MCShowUsage(*sindex);
 
              exit(1);
              }
 
            MUStrCpy(QOSList,OptArg,sizeof(QOSList));
 
            break;

          case svcDiagnose:

            /* q-QOS */

            DiagMode = mxoQOS;

            break;

          default:

            MCShowUsage(*sindex);

            exit(1);

            break;
          }

        break;
 
      case 'r':

        switch(*sindex)
          {
          case svcJobShow:

            /* reservation */
 
            MUStrCpy(ResList,OptArg,sizeof(ResList));
 
            break;

          case svcSetJobSystemPrio:

            /* r-RELATIVE */

            PrioMode = mjpRelative;
  
            break;

          case svcResCreate:

            /* r-RESOURCELIST */

            if (OptArg == NULL)
              {
              MCShowUsage(*sindex);

              exit(1);
              }

            MUStrCpy(ResourceList,OptArg,sizeof(ResourceList));

            break;

          case svcMGridCtl:

            /* r-RELEASE */

            MGridMode = mcRemove;

            break;

          case svcMJobCtl:
 
            /* r-RESUME */
 
            JobCtlMode = mjcmResume;
 
            break;

          case svcResShow:

            /* r-RELATIVEMODE */

            Mode |= (1 << mcmRelative);

            break;

          case svcShowBackfillWindow:

            /* r-pRoc */

            if (OptArg == NULL)
              {
              MCShowUsage(*sindex);

              exit(1);
              }

            ProcCount = atoi(OptArg);

            break;

          case svcDiagnose:

            /* r-RESERVATION */

            DiagMode = mxoRsv;

            break;

          case svcSched:

            /* r-RESUME */

            SchedMode = msctlResume;

            MUStrCpy(SchedArg,OptArg,sizeof(SchedArg));

            break;

          case svcShowQ:

            /* r-RUNNING */

            QueueMode = 2;

            break;

          default:

            MCShowUsage(*sindex);

            exit(1);

            break;
          }
      
        break;

      case 'R':

        switch(*sindex)
          {
          case svcDiagnose:
 
            /* R-RM */
 
            DiagMode = mxoRM;
 
            break;

          case svcSched:

            /* R-RECONFIG */

            SchedMode = msctlReconfig;

            break;

          default:

            MCShowUsage(*sindex);

            exit(1);

            break;
          }

        break;

      case 's':

        switch(*sindex)
          {
          case svcRunJob:
 
            /* s-SUSPEND */
 
            Flags = mcmBlock;
 
            break;

          case svcMJobCtl:
 
            /* s-SUBMIT */
 
            JobCtlMode = mjcmSubmit;
 
            break;

          case svcMGridCtl:

            /* s-SET */

            MGridMode = mcSet;

            break;

          case svcSetJobHold:
          case svcReleaseJobHold:

            /* s-SystemHold */

            HType = mhSystem;

            break;
 
          case svcShowStats:

            /* s-SCHEDULER */

            ObjectType = mxoSched;

            break;

          case svcResCreate:

            /* s-STARTTIME (Arg Required) */

            if (OptArg == NULL)
              {
              MCShowUsage(*sindex);

              exit(1);
              }

            if (MUStringToE(OptArg,&BeginTime) != SUCCESS)
              {
              fprintf(stderr,"ERROR:    invalid StartTime specified, '%s'\n",
                OptArg);

              exit(1);
              }

            break;

          case svcSched:

            SchedMode = msctlStop;

            if (OptArg == NULL)
              {
              MCShowUsage(*sindex);

              exit(1);
              }

            if (OptArg[0] != '-')
              MUStrCpy(SchedArg,OptArg,sizeof(SchedArg));

            break;

          case svcResShow:

            /* s-SUMMARY */

            Flags |= (1 << mcmSummary);

            break;

          default:

            MCShowUsage(*sindex);

            exit(1);

            break;
          }  /* END switch(*sindex) */

        break;

      case 'S':

        switch(*sindex)
          {
          case svcMJobCtl:
 
            /* S-SUSPEND */
 
            JobCtlMode = mjcmSuspend;
 
            break;

          case svcMGridCtl:

            /* S-STAGE */

            MGridMode = mcSubmit;

            break;

          case svcShowBackfillWindow:

            /* S-SMP */

            BFMode = 1;

            break;

          case svcSched:

            /* S-STEP */

            SchedMode = msctlStep; 

            if (OptArg == NULL)
              {
              MCShowUsage(*sindex);

              exit(1);
              }

            MUStrCpy(SchedArg,OptArg,sizeof(SchedArg));

            break;

          case svcShowStats:

            /* S-SUMMARY */

            Flags |= 1;

            break;

          case svcDiagnose:

            /* s-SYS */

            DiagMode = mxoSys;

            break;

          default:

            MCShowUsage(*sindex);

            exit(1);

            break;
          }  /* END switch(*sindex) */

        break;

      case 't':

        switch(*sindex)
          {
          case svcMGridCtl:

            /* t-TYPE (REQUIRED ARG) */

            if (OptArg == NULL)
              {
              MCShowUsage(*sindex);

              exit(1);
              }

            MUStrCpy(Type,OptArg,sizeof(Type));

            break;

          case svcResCreate:

            /* t-TASKCOUNT */

            if (OptArg == NULL)
              {
              MCShowUsage(*sindex);

              exit(1);
              }

            TaskCount = (int)strtol(OptArg,NULL,0);

            break;

          case svcDiagnose:

            /* p-PARTITION (REQUIRED ARG) */

            if (OptArg == NULL)
              {
              MCShowUsage(*sindex);

              exit(1);
              }

            if (OptArg[0] != '\0')
              {
              MUStrCpy(ParName,OptArg,sizeof(ParName));

              DBG(4,fCONFIG) DPrint("INFO:     partition set to %s\n",
                ParName);
              }
            else
              {
              DiagMode = mxoPar;
              }

            break;

          default:

            MCShowUsage(*sindex);

            exit(1);

            break;
          }  /* END switch(*sindex) */

        break;

      case 'T':

        switch(*sindex)
          {
          case svcMGridCtl:

            /* T-TIME (REQUIRED ARG) */

            if (OptArg == NULL)
              {
              MCShowUsage(*sindex);

              exit(1);
              }

            MUStrCpy(RangeList,OptArg,sizeof(RangeList));

            break;

          default:

            MCShowUsage(*sindex);

            exit(1);

            break;
          }

        break;

      case 'u':

        switch(*sindex)
          {
          case svcSetJobHold:
          case svcReleaseJobHold:

            /* u-USERHOLD */

            HType = mhUser;
   
            break;

          case svcResCreate:

            /* u-USERLIST */

            if (OptArg == NULL)
              {
              MCShowUsage(*sindex);

              exit(1);
              }

            MUStrCpy(UserList,OptArg,sizeof(UserList));

            break;

          case svcShowStats:

            /* u-USER */

            ObjectType = mxoUser;

            if ((OptArg != NULL) && (OptArg[0] != '\0'))
              {
              MUStrCpy(ObjectID,OptArg,sizeof(ObjectID));
              }
 
            break;

          case svcShowBackfillWindow:

            /* u-USER */

            if ((OptArg == NULL) || (OptArg[0] == '\0'))
              {
              MCShowUsage(*sindex);

              exit(1);
              }

            MUStrCpy(UserName,OptArg,sizeof(UserName));

            break;

          case svcDiagnose:

            /* u-USER */

            DiagMode = mxoUser;

            break;

          case svcShowQ:

            /* u-USER */ /* HvB */

            if ((OptArg == NULL) || (OptArg[0] == '\0'))
              {
              MCShowUsage(*sindex);

              exit(1);
              }

            MUStrCpy(ShowUserName,OptArg,sizeof(ShowUserName));

            break;

          default:

            MCShowUsage(*sindex);

            exit(1);
 
            break;
          }  /* END switch(*sindex) */ 

        break;

      case 'v':

        switch(*sindex)
          {
          case svcJobShow:
          case svcNodeShow:
          case svcDiagnose:
          case svcShowConfig:
          case svcShowStats:
          case svcResShow:
          case svcShowBackfillWindow:
          case svcShowQ:

            /* v-VERBOSE */

            Flags |= (1 << mcmVerbose);

            break;

          default:

            fprintf(stderr,"%s client version %s\n",
              MSCHED_SNAME,
              MSCHED_VERSION);

            exit(1);

            break;
          }  /* END switch(*sindex) */

        break;

      case 'V':

        switch(*sindex)
          {
          default:

            fprintf(stderr,"%s client version %s\n",
              MSCHED_SNAME,
              MSCHED_VERSION);

            exit(1);

            break;
          }  /* END switch(*sindex) */

        break;

      case 'x':

        switch(*sindex)
          {
          case svcRunJob:

            /* x-FORCE2 */

            Flags = mcmForce2;

            break;

          case svcResCreate:
 
            /* x-FLAGS */
 
            strncpy(ResFlags,OptArg,sizeof(ResFlags));
 
            break;

          case svcClusterShow:

            /* x-XML */
   
            UseXML = TRUE;

            break;

          default:

            MCShowUsage(*sindex);

            exit(1);

            break;
          }    /* END switch(*sindex) */

        break;

    default:

      DBG(1,fCONFIG) DPrint("WARNING:  unexpected flag detected '%c'\n",
        Flag);

      MCShowUsage(*sindex);

      exit(1);

      break;
    }  /* END switch (Flag) */

  return(SUCCESS);
  }  /* END __OMCLoadArgs() */




int __OMCProcessArgs(

  int   ArgCount,  /* I */
  char *Args[],    /* I */
  int  *sindex)    /* I */

  {
  int ArgIndex;

  int index;

  ArgIndex = 1;

  switch(*sindex)
    {
    case svcShowStats:

      if (ArgCount != 1)
        {
        MCShowUsage(*sindex);

        exit(1);
        }

      if (ObjectType == -1)
        ObjectType = mxoSched;

      if (ObjectID[0] == '\0')
        MUStrCpy(ObjectID,NONE,sizeof(ObjectID));

      if (ObjectType == mxoNode)
        {
        /* set default EvaluationDepth to MAX_MJOB */

        if (Flags < (1 << 1))
          Flags |= (MAX_MJOB << 1);
        }

      sprintf(MsgBuffer,"%d %s %s %d",
        ObjectType,
        ObjectID,
        ParName,
        Flags);

      break;

    case svcShowTasks:
    case svcShowQ:
      if (QueueMode == -1)
        QueueMode = 0;

      sprintf(MsgBuffer,"%d %s %d %s",
        QueueMode,
        ParName,
	Flags,
        ShowUserName);

      break;
 
    case svcSetJobSystemPrio:
    case svcSetJobUserPrio:

      /* Format:  <CMD> <JOB> <PRIORITY> <MODE> */

      if (ArgCount != 3)
        {
        MCShowUsage(*sindex);

        exit(1);
        }

      /* route through mjobctl */

      sprintf(MsgBuffer,"<schedrequest action=\"modify\" attr=\"SysPrio\" value=\"%s\" flag=\"set\" job=\"%s\" arg=\"%s\"></schedrequest>\n",
        Args[ArgIndex],
        Args[ArgIndex + 1],
        (PrioMode == mjpRelative) ? "relative" : "absolute");

      *sindex = svcMJobCtl;

      /*
      sprintf(MsgBuffer,"%s %s %d",
        Args[ArgIndex],
        Args[ArgIndex + 1],
        PrioMode);
      */

      DBG(2,fCONFIG) DPrint("INFO:     setting priority on job %s to %s (%d)\n",
        Args[ArgIndex + 1],
        Args[ArgIndex],
        PrioMode);

      break;

   case svcSetJobQOS:

      /* Format:  <CMD> <QOS> <JOB> */

      if (ArgCount != 3)
        {
        MCShowUsage(*sindex);

        exit(1);
        }

      /* route through mjobctl */

      sprintf(MsgBuffer,"<schedrequest action=\"modify\" attr=\"QOS\" value=\"%s\" flag=\"set\" job=\"%s\"></schedrequest>\n",
        Args[ArgIndex],
        Args[ArgIndex + 1]);

      *sindex = svcMJobCtl;

      /*
      sprintf(MsgBuffer,"%s %s",
        Args[ArgIndex],
        Args[ArgIndex + 1]);
      */

      DBG(2,fCONFIG) DPrint("INFO:     setting QOS on job %s to %s\n",
        Args[ArgIndex + 1],
        Args[ArgIndex]);

      break;

    case svcSetJobHold:
    case svcReleaseJobHold:

      /* Format:  <CMD> <JOB_REGEX> */

      if (ArgCount != 2)
        {
        MCShowUsage(*sindex);

        exit(1);
        }

      if (HType == -1)
        HType = mhAll;

      DBG(2,fCONFIG) DPrint("INFO:     hold type %s selected\n",
        MHoldType[HType]);

      /* route through mjobctl */

      sprintf(MsgBuffer,"<schedrequest action=\"modify\" attr=\"Hold\" value=\"%s\" flag=\"%s\" job=\"%s\"></schedrequest>\n",
        MHoldType[HType],
        (*sindex == svcSetJobHold) ? "set" : "unset",
        Args[ArgIndex]);

      *sindex = svcMJobCtl;

      /* NOTE:  HType not supported */
     
      /* 
      sprintf(MsgBuffer,"%s %d\n",
        Args[ArgIndex],
        HType);
      */

      DBG(3,fCONFIG) DPrint("INFO:     Buffer '%s' (%d)\n",
        MsgBuffer,
        (int)strlen(MsgBuffer));

      break;

    case svcSetJobDeadline:

      /* Format:  <CMD> <JOB> <DEADLINE> */

      if (ArgCount == 3)
        {
        if (MUStringToE(Args[ArgIndex + 1],&BeginTime) != SUCCESS)
          {
          fprintf(stderr,"ERROR:    Invalid Job Deadline Specified\n");

          exit(1);
          }
        }
      else
        {
        MCShowUsage(*sindex);

        exit(1);
        }

      sprintf(MsgBuffer,"%s %ld\n",
        Args[ArgIndex],
        BeginTime);
      
      DBG(2,fCONFIG) DPrint("INFO:     deadline %ld set on job %s\n",
        BeginTime,
        Args[ArgIndex + 1]);
      
      break;

    case svcReleaseJobDeadline:

      /* Format:  <CMD> <JOB> */

      if (ArgCount == 2)
        {
        MUStrCpy(MsgBuffer,Args[ArgIndex],sizeof(MsgBuffer));
        }
      else
        {
        MCShowUsage(*sindex);

        exit(1);
        }
 
      break;

    case svcSched:

      /* Format:  <CMD>  -k | -r <RESUMETIME> | -R | -s [<STOPITERATION>] | -S [<STEPITERATION>] ] */

      if (SchedMode == -1)
        {
        MCShowUsage(*sindex);

        exit(1);
        }

      sprintf(MsgBuffer,"%d %s",
        SchedMode,
        SchedArg);

      DBG(2,fCONFIG) DPrint("INFO:     sending schedctl command: '%s'\n",
        MsgBuffer);

      break;

    case svcDiagnose:

      if (DiagMode == -1)
        {
        MCShowUsage(*sindex);

        exit(1);
        }

      if (DiagMode == mxoQueue)
        {
        if (PType == -1)
          PType = ptSOFT;

        sprintf(MsgBuffer,"%d %d %s",
          DiagMode,
          PType,
          ParName);
        }
      else
        {
        sprintf(MsgBuffer,"%d %d %s %s",
          DiagMode,
          Flags,
          ParName,
          (Args[ArgIndex] == NULL) ? NONE : Args[ArgIndex]);
        }

      break;

    case svcResCreate:

      if (ArgCount != 2)
        {
        MCShowUsage(*sindex);

        exit(1);
        }

      {
      time_t tmpT;

      time(&tmpT);

      ClientTime = (long)tmpT;
      }  /* END BLOCK */

      if (BeginTime == 0)
        BeginTime = ClientTime;

      if (Duration != 0)
        EndTime = BeginTime + Duration;
      else if (EndTime == MAX_MTIME)
        EndTime = BeginTime + 100000000;
 
      DBG(6,fALL) DPrint("INFO:     ChargeAccount: %s\n",
        ChargeAccount);

      RegEx = Args[ArgIndex];

      if (strlen(RegEx) >= sizeof(NodeRegEx))
        {
        fprintf(stderr,"ERROR:    regular expression too long. (%d > %d)\n",
          (int)strlen(Args[ArgIndex]),
          (int)sizeof(NodeRegEx));

        exit(1);
        }

      sprintf(MsgBuffer,"%ld %ld %ld %s %s %s %s %s %s %s %s %s %s %s %s %s %d %s",
        ClientTime,
        BeginTime,
        EndTime,
        ParName,
        UserList,
        GroupList,
        AccountList,
        ClassList,
        QOSList,
        ObjectID,
        ResourceList,
        ChargeAccount,
        RegEx,
        FeatureString,
        NodeSetString,
        ResFlags,
        TaskCount,
        JobFeatureString);

      break;

    case svcResDestroy:

      /* copy reservation name into buffer */

      if (ArgCount >= 2)
        {
        MsgBuffer[0] = '\0';

        for (index = ArgIndex;index < ArgCount;index++)
          {
          MUStrCat(MsgBuffer,Args[index],sizeof(MsgBuffer));
          MUStrCat(MsgBuffer," ",sizeof(MsgBuffer));
          }
        }
      else 
        {
        MCShowUsage(*sindex);

        exit(1);
        }

      break;

    case svcResShow:

      /* copy job if provided */

      if (ArgCount > 2)
        {
        MCShowUsage(*sindex);

        exit(1);
        }

      if (ObjectType == -1)
        {
        ObjectType = mxoJob;
        }

      if (C.Format == mwpXML)
        Flags |= (1 << mcmXML);

      sprintf(MsgBuffer,"%d %s %d %s",
        ObjectType,
        ParName,
        Flags,
        (Args[ArgIndex] != NULL) ? Args[ArgIndex] : NONE);

      break;

    case svcShowJobDeadline:

      if (ArgCount == 2)
        {
        MUStrCpy(MsgBuffer,Args[ArgIndex],sizeof(MsgBuffer));
        }

      break;

    case svcNodeShow:

      /* copy node */

      if (ArgCount == 2)
        {
        sprintf(MsgBuffer,"%s %d",
          Args[ArgIndex],
          Flags);
        }
      else
        {
        MCShowUsage(*sindex);

        exit(1);
        }

      break;

    case svcJobShow:

      if (PType == -1)
        PType = ptSOFT;

      if (ArgCount == 2)
        {
        sprintf(MsgBuffer,"%d %s %d %s %s",
          PType,
          Args[ArgIndex],
          Flags,
          ResList,
          (NodeRegEx[0] != '\0') ? NodeRegEx : NONE);
        }
      else
        {
        MCShowUsage(*sindex);

        exit(1);
        }

      break;

    case svcShowEarliestDeadline:

      if (ArgCount == 2)
        {
        sprintf(MsgBuffer,"%s %s",
          Args[ArgIndex],
          ParName);
        }
      else
        {
        MCShowUsage(*sindex);

        exit(1);
        }

      break;

    case svcRunJob:

      /* copy job */

      if (ArgCount == 2)
        {
        sprintf(MsgBuffer,"%s %s %s %s",
          Args[ArgIndex],
          MClientMode[Flags],
          ParName,
          (NodeRegEx[0] != '\0') ? NodeRegEx : NONE);
        }
      else
        {
        MCShowUsage(*sindex);

        exit(1);
        }

      break;

    case svcCancelJob:

      if (ArgCount >= 2)
        {
        /* copy job regex into buffer */

        MsgBuffer[0] = '\0';

        for (index = ArgIndex;index < ArgCount;index++)
          {
          MUStrCat(MsgBuffer,Args[index],sizeof(MsgBuffer));
          MUStrCat(MsgBuffer," ",sizeof(MsgBuffer));
          }
        }
      else
        {
        MCShowUsage(*sindex);

        exit(1);
        }

      break;

    case svcShowGrid:

      /* Format:  <CMD> <STATISTICS TYPE> */

      if (ArgCount == 2)
        {
        /* Copy Statistics Type */

        MUStrCpy(MsgBuffer,Args[ArgIndex],sizeof(MsgBuffer));
        }
      else
        {
        MCShowUsage(*sindex);

        exit(1);
        }

      break;

    case svcChangeParameter:

      /* Format:  <CMD> <PARAMETER> <VALUE> [ <VALUES> ] ... */

      if (ArgCount < 3)
        {
        MCShowUsage(*sindex);

        exit(1);
        }

      /* copy parameter and all values into buffer */

      for (index = ArgIndex;index < ArgCount;index++)
        {
        MUStrCat(MsgBuffer,Args[index],sizeof(MsgBuffer));
        
        MUStrCat(MsgBuffer," ",sizeof(MsgBuffer));
        }

      /* terminate line with newline character */

      MUStrCat(MsgBuffer,"\n",sizeof(MsgBuffer));

      break;

    case svcMigrateJob:

      if (ArgCount != 3)
        {
        MCShowUsage(*sindex);

        exit(1);
        }

      /* Copy Job and Server into Buffer */

      sprintf(MsgBuffer,"%s %s",
        Args[ArgIndex],
        Args[ArgIndex + 1]);

      break;

    case svcShowEstimatedStartTime:

      if (ArgCount != 2)
        {
        MCShowUsage(*sindex);

        exit(1);
        }

      /* copy job into buffer */

      MUStrCpy(MsgBuffer,Args[ArgIndex],sizeof(MsgBuffer));

      break;

    case svcShowBackfillWindow:

      sprintf(MsgBuffer,"%s %s %s %s %ld %d %d %d %d %s %d %d %s %s %s",
        UserName,
        Group,
        Account,
        ParName,
        Duration,
        NodeCount,
        ProcCount,
        DMemory,
        Memory, 
        MComp[MIndex],
        BFMode,
        Flags,
        ClassString,
        FeatureString,
        QOSName);

      break;

    case svcShowConfig:

      /* allow verbose flag */

      if (Flags & (1 << mcmVerbose))
        {
        MUStrCpy(MsgBuffer,"VERBOSE",sizeof(MsgBuffer));
        }

      /* single optional parameter accepted */

      if (ArgCount == 2)
        {
        strcat(MsgBuffer," ");

        strcat(MsgBuffer,Args[ArgIndex]);
        }
      else if (ArgCount > 2)
        {
        MCShowUsage(*sindex);

        exit(1);
        }

      break;

    case svcResetStats:

      /* no arguments accepted */

      if (ArgCount != 1)
        {
        MCShowUsage(*sindex);

        exit(1);
        }

      break;

    case svcClusterShow:

      /* no arguments accepted */

      if (ArgCount != 1)
        {
        MCShowUsage(*sindex);

        exit(1);
        }

      sprintf(MsgBuffer,"%s",
        (UseXML == TRUE) ? "XML" : "DEFAULT");

      break;

    case svcBNFQuery:

      MUStrCpy(MsgBuffer,QLine,sizeof(MsgBuffer));

      break;

    case svcMJobCtl:

      if (JobCtlMode == mjcmNONE)
        {
        MCShowUsage(*sindex);
 
        exit(1);
        }
 
      switch (JobCtlMode)
        {
        case mjcmCancel:
 
          sprintf(MsgBuffer,"%s %s",
            MJobCtlCmds[JobCtlMode],
            Args[ArgIndex]);
 
          break;
 
        case mjcmSuspend:

          sprintf(MsgBuffer,"%s %s",
            MJobCtlCmds[JobCtlMode],
            Args[ArgIndex]);

        case mjcmSubmit:
 
          sprintf(MsgBuffer,"%s [NONE] [NONE] %s",
            MJobCtlCmds[JobCtlMode],
            Args[ArgIndex]);
 
          break;

        default:

          MCShowUsage(*sindex);
 
          exit(1);
  
          /*NOTREACHED*/
 
          break;
        }  /* END switch(JobCtlMode) */

      break;

    case svcMNodeCtl:
 
      /* FORMAT:  <SUBCOMMAND> <ARG> <NODEXP> */
 
      if (ArgCount != 4)
        {
        MCShowUsage(*sindex);
 
        exit(1);
        }

      sprintf(MsgBuffer,"%s %s %s",
        Args[ArgIndex],
        Args[ArgIndex + 1],
        Args[ArgIndex + 2]);
 
      break;

    case svcMGridCtl:

      if (MGridMode == mcNONE)
        {
        MCShowUsage(*sindex);

        exit(1);
        }

      switch (MGridMode)
        {
        case mcCommit:

          sprintf(MsgBuffer,"%s %s",
            MGridCtlCmds[MGridMode],
            Args[ArgIndex]);

          break;

        case mcList:

          sprintf(MsgBuffer,"%s %s %s %s",
            MGridCtlCmds[MGridMode],
            Args[ArgIndex],
            Args[ArgIndex + 1],
            (Args[ArgIndex + 2] == NULL) ? "ALL" : Args[ArgIndex + 2]);

          break;

        case mcModify:

          if (ArgCount != 3)
            {
            MCShowUsage(*sindex);
       
            exit(1);
            }

          sprintf(MsgBuffer,"%s %s %s",
            MGridCtlCmds[MGridMode],
            Args[ArgIndex],
            Args[ArgIndex + 1]);

          break;

        case mcRemove:

          sprintf(MsgBuffer,"%s %s %s",
            MGridCtlCmds[MGridMode],
            Args[ArgIndex],
            Args[ArgIndex + 1]);

          break;

        case mcSubmit:

          sprintf(MsgBuffer,"%s %s",
            MGridCtlCmds[MGridMode],
            Args[ArgIndex]);

          break;

        case mcSet:
 
          /* FORMAT:  <CMD> <TYPE> <RESDESC> */

          sprintf(MsgBuffer,"%s %s %s %s",
            MGridCtlCmds[MGridMode],
            Type,
            Args[ArgIndex],
            Args[ArgIndex + 1]);

          break;

        case mcQuery:

          /* FORMAT:  <CMD> <TYPE> <RANGE> <RESDESCTYPE> <RESDESC> */

          sprintf(MsgBuffer,"%s %s %s WIKI %s",
            MGridCtlCmds[MGridMode],
            Type,
            RangeList,
            (Args[ArgIndex] == NULL) ? "DEFAULT" : Args[ArgIndex]);

          break;
        }   /* END switch(MGridMode) */

      break;
 
    default:

      fprintf(stderr,"ERROR:    service %d not handled\n",
        *sindex);

      break;
    }  /* END switch(*sindex) */

  return(SUCCESS);
  }  /* END Initialize() */





int OMCShowUsage(

  int sindex)

  {
  int index;

  DBG(3,fCORE) DPrint("OMCShowUsage(%d)\n",
    sindex);

  switch(sindex)
    {
    case svcSetJobSystemPrio:

      fprintf(stderr,"Usage: %s [ -r ] <PRIORITY> <JOB>\n",
        MService[sindex]);

      break;

    case svcSetJobUserPrio:

      fprintf(stderr,"Usage: %s <PRIORITY> <JOB>\n",
        MService[sindex]);

      break;

    case svcSetJobQOS:

      fprintf(stderr,"Usage: %s <QOS> <JOB>\n",
        MService[sindex]);

      break;

    case svcSetJobHold:

/*
      fprintf(stderr,"Usage: %s [-a][-b][-s][-u] <JOBREGEX>\n",
        MService[sindex]);
*/
      fprintf(stderr,"Usage: %s [FLAGS] <JOB_REGEX>\n",
        MService[sindex]);

      fprintf(stderr,"          [ -b ] // BATCH\n");
      fprintf(stderr,"          [ -h ]\n");

      break;

    case svcReleaseJobHold:

/*
      fprintf(stderr,"Usage: %s [-a][-b][-s][-u] <JOBREGEX>\n",
        MService[sindex]);
*/
      fprintf(stderr,"Usage: %s [FLAGS] <JOB_REGEX>\n",
        MService[sindex]);

      fprintf(stderr,"          [ -a ] // ALL HOLD TYPES\n");
      fprintf(stderr,"          [ -h ]\n");

      break;

    case svcReleaseJobDeadline:
    case svcShowEarliestDeadline:

      fprintf(stderr,"Usage: %s <JOB>\n",
        MService[sindex]);

      fprintf(stderr,"          [ -h ]\n");

      break;

    case svcCancelJob:

      fprintf(stderr,"Usage: %s <JOB>[<JOB>]...\n",
        MService[sindex]);

      fprintf(stderr,"          [ -h ]\n");

      break;

    case svcRunJob:

      fprintf(stderr,"Usage: %s [ FLAGS ] <JOB>\n",
        MService[sindex]);

      fprintf(stderr,"          [ -c ] // CLEAR (clear stale job attributes)\n");
      fprintf(stderr,"          [ -f ] // FORCE (ignore policies)\n");
      fprintf(stderr,"          [ -n <NODE_REGEX>]\n");
      fprintf(stderr,"          [ -p <PARTITION>]\n");
      fprintf(stderr,"          [ -x ] // FORCE2 (ignore policies and reservations)\n");

      fprintf(stderr,"          [ -h ]\n");

      break;

    case svcShowJobDeadline:

      fprintf(stderr,"Usage: %s <JOB>\n",
        MService[sindex]);

      fprintf(stderr,"          [ -h ]\n");

      break;

    case svcShowJobHold:

      fprintf(stderr,"Usage: %s\n",
        MService[sindex]);

      fprintf(stderr,"          [ -h ]\n");

      break;

    case svcShowConfig:

      fprintf(stderr,"Usage: %s [ -v ] [<PARAMETER>]\n",
        MService[sindex]);

      break;

    case svcShowGrid:

      fprintf(stderr,"Usage: %s <STATISTICS TYPE>\n",
        MService[sindex]);

      fprintf(stderr,"\nvalid statistics types:\n\n");

      for (index = 0;MStatType[index] != NULL;index++)
        {
        fprintf(stderr,"%s\n",
          MStatType[index]);
        }  /* END for (index) */

      break;

    case svcSetJobDeadline:

      fprintf(stderr,"Usage: %s <JOB> <TIME>\n",
        MService[sindex]);

      break;

    case svcChangeParameter:

      fprintf(stderr,"Usage: %s <PARAMETER> <VALUE> [VALUE] ...\n",
        MService[sindex]);

      break;

    case svcMigrateJob:

      fprintf(stderr,"Usage: %s <JOB> <SERVER>\n",
        MService[sindex]);
      fprintf(stderr,"       %s -h\n",
        MService[sindex]);

      break;

    case svcShowEstimatedStartTime:

      fprintf(stderr,"Usage: %s <JOB>\n",
        MService[sindex]);
      fprintf(stderr,"Usage: %s -h\n",
        MService[sindex]);

      break;

    case svcResDestroy:

      fprintf(stderr,"Usage: %s <RESERVATION>\n",
        MService[sindex]);
      fprintf(stderr,"       %s -h\n",
        MService[sindex]);

      break;

    case svcBNFQuery:

      fprintf(stderr,"Usage: %s -c <COMMAND> |\n",
        MService[sindex]);
      fprintf(stderr,"       %s -h\n",
        MService[sindex]);

      break;

    default:

      fprintf(stderr,"no usage information for '%s'\n",
        MService[sindex]);

      break;
    }  /* END switch(index) */

  return(SUCCESS);
  }  /* END OMCShowUsage() */





int MCShowReservation(

  char *Buffer)  /* I */

  {
  int rc;

  const char *FName = "MCShowReservation";

  DBG(3,fUI) DPrint("%s(Buffer)\n",
    FName);

  switch(ObjectType)
    {
    case mxoNode:

      rc = MCShowReservedNodes(Buffer);

      break;

    case mxoJob:

      rc = MCShowJobReservation(Buffer);

      break;
   
    default:

      fprintf(stderr,"ERROR:  unable to show reservation type %d\n",
        ObjectType);

      return(FAILURE);

      /*NOTREACHED*/

      break; 
    }  /* END switch(ObjectType) */

  return(rc);
  }  /* END MCShowReservation() */



      


int MCShowReservedNodes(

  char *Buffer)  /* I */

  {
  long   StartTime;
  long   EndTime;

  char   NodeName[MAX_MNAME];
  char   ResName[MAX_MNAME];
  char   JState[MAX_MNAME];
  char   ResType[MAX_MNAME];
  char   DurationLine[MAX_MNAME];
  char   StartLine[MAX_MNAME];
  char   EndLine[MAX_MNAME];
  char   TaskString[MAX_MNAME];
  char   UserName[MAX_MNAME];

  char   tmpLine[MAX_MLINE];

  long   now;

  int    ncount;

  time_t tmpTime;

  mxml_t *E  = NULL;
  mxml_t *DE = NULL;
  mxml_t *C  = NULL;
  mxml_t *NE = NULL;
  mxml_t *RE = NULL;

  int      RTok;
  int      NTok;

  const char *FName = "MCShowReservedNodes";

  DBG(2,fUI) DPrint("%s(Buffer)\n",
    FName);

/* FORMAT:                                                                                 */
/*                                                                                         */

  DBG(5,fUI) DPrint("Buffer: '%s' (%d)\n",
    Buffer,
    (int)strlen(Buffer));

  if (Flags & (1 << mcmXML))
    {
    fprintf(stdout,"%s\n",
      Buffer);

    return(SUCCESS);
    }

  /* display standard node reservation results */

  if (MCExtractData(Buffer,&E,&DE) == FAILURE)
    {
    /* cannot extract XML data */

    return(FAILURE);
    }

  /* determine time */

  if ((MXMLGetChild(DE,(char *)MXO[mxoSys],NULL,&C) == SUCCESS) &&
      (MXMLGetAttr(C,(char *)MSysAttr[msysaPresentTime],NULL,tmpLine,sizeof(tmpLine)) == SUCCESS))
    {
    now = strtol(tmpLine,NULL,0);
    }
  else
    {
    time(&tmpTime);

    now = (long)tmpTime;
    }

  if (Flags & (1 << mcmSummary))
    {
    char *ptr;
    char *TokPtr;

    fprintf(stdout,"%20s %10s %c %12s %12s %5s\n\n",
      "JobName",
      "User",
      'S',
      "StartTime",
      "EndTime",
      "Count");

    RTok = -1;

    while (MXMLGetChild(DE,"res",&RTok,&RE) == SUCCESS)
      {
      MXMLGetAttr(RE,(char *)MResAttr[mraName],NULL,ResName,sizeof(tmpLine));
      MXMLGetAttr(RE,(char *)MResAttr[mraStartTime],NULL,tmpLine,sizeof(tmpLine));
      StartTime = strtol(tmpLine,NULL,0);

      MXMLGetAttr(RE,(char *)MResAttr[mraEndTime],NULL,tmpLine,sizeof(tmpLine));
      EndTime = strtol(tmpLine,NULL,0);

      MXMLGetAttr(RE,(char *)MResAttr[mraJState],NULL,JState,sizeof(JState));

      MXMLGetAttr(RE,(char *)MResAttr[mraTaskCount],NULL,TaskString,sizeof(TaskString));

      MXMLGetAttr(RE,(char *)MResAttr[mraCreds],NULL,tmpLine,sizeof(tmpLine));

      if ((ptr = strstr(tmpLine,"USER=")) != NULL)
        {
        ptr += strlen("USER=");

        ptr = MUStrTok(ptr," \t,\n",&TokPtr);

        strcpy(UserName,ptr);
        }
      else
        {
        strcpy(UserName,"N/A");
        }
 
      strcpy(StartLine,MULToTString(StartTime - now));
      strcpy(EndLine,MULToTString(EndTime - now));

      fprintf(stdout,"%20s %10s %1.1s %12s %12s %5s\n",
        ResName,
        UserName,
        JState,
        StartLine,
        EndLine,
        TaskString);
      }  /* END while (MXMLGetChild(DE,"res",&RTok,&RE) == SUCCESS) */

    MXMLDestroyE(&E);

    return(SUCCESS);
    }  /* END if (Flags & (1 << mcmSummary)) */

  fprintf(stdout,"reservations on %s\n",
    MULToDString((mulong *)&now));

  if (!(Mode & (1 << mcmRelative)))
    {
    fprintf(stdout,"%20s  %9s %18s %10s %4s %11s %11s %s%20s\n\n",
      "NodeName",
      "Type",
      "ReservationID",
      "JobState",
      "Task",
      "Start",
      "Duration",
      (Flags & (1 << mcmVerbose)) ? "     " : "",
      "StartTime");
    }
  else
    {
    fprintf(stdout,"%20s  %9s %18s %10s %4s %8s --> %8s  (%8s)\n\n",
      "NodeName",
      "Type",
      "ReservationID",
      "JobState",
      "Task",
      "Start",
      "End",
      "Duration");
    }

  NTok = -1;

  ncount = 0;

  while (MXMLGetChild(DE,"node",&NTok,&NE) == SUCCESS)
    {
    int rcount = 0;

    MXMLGetAttr(NE,"name",NULL,NodeName,sizeof(NodeName));

    RTok = -1;

    while (MXMLGetChild(NE,"nres",&RTok,&RE) == SUCCESS)
      {
      MXMLGetAttr(RE,(char *)MNResAttr[mnraName],NULL,ResName,sizeof(ResName));
      MXMLGetAttr(RE,(char *)MNResAttr[mnraStart],NULL,tmpLine,sizeof(tmpLine));
      StartTime = strtol(tmpLine,NULL,0);

      MXMLGetAttr(RE,(char *)MNResAttr[mnraEnd],NULL,tmpLine,sizeof(tmpLine));
      EndTime = strtol(tmpLine,NULL,0);

      MXMLGetAttr(RE,(char *)MNResAttr[mnraType],NULL,ResType,sizeof(ResType));

      MXMLGetAttr(RE,(char *)MNResAttr[mnraState],NULL,JState,sizeof(JState));

      MXMLGetAttr(RE,(char *)MNResAttr[mnraTC],NULL,TaskString,sizeof(TaskString));

      /* display node/res information */

      fprintf(stdout,"%20s  %9s %18s %10s %4s ",
        ((rcount == 0) || (Mode & (1 << mcmGrep))) ? NodeName : " ",
        ResType,
        ResName,
        JState,
        TaskString);

      /* display res timeframe */

      if (Mode & (1 << mcmRelative))
        {
        if ((EndTime - StartTime) > 8553600)
          fprintf(stdout,"%8ld --> %8s  (%8s)\n",
          (StartTime - now),
          "INFINITE",
          "INFINITE");
        else
          fprintf(stdout,"%8ld --> %8ld  (%08ld)\n",
            (StartTime - now),
            (EndTime - now),
            (EndTime - StartTime));
        }
      else
        {
        strcpy(StartLine,MULToTString(StartTime - now));

        if ((EndTime - StartTime) > 8553600)
          {
          strcpy(DurationLine,"INFINITE");
          }
        else
          {
          strcpy(DurationLine,MULToTString(EndTime - StartTime));

          }

        tmpTime = (time_t)StartTime;

        fprintf(stdout,"%11s %11s  %20s",
          StartLine,
          DurationLine,
          (Flags & (1 << mcmVerbose)) ?
             ctime(&tmpTime) : MULToDString((mulong *)&StartTime));
        }  /* END else (Mode & (1 << mcmRelative)) */

      rcount++;
      }  /* END while (MXMLGetChild(RE) == SUCCESS) */

    ncount++;
    }    /* END while (MXMLGetChild(NE) == SUCCESS) */

  fprintf(stdout,"%d nodes reserved\n",
    ncount);

  MXMLDestroyE(&E);

  return(SUCCESS);
  }  /* END MCShowReservedNodes() */





int MCShowJobReservation(

  char *Buffer)  /* I */

  {
  const char *FName = "MCShowJobReservation";

  DBG(2,fUI) DPrint("%s(Buffer)\n",
    FName);

  fprintf(stdout,"%s\n",
    Buffer);

  return(SUCCESS);
  }  /* END MCShowJobReservation() */





int MCSchedCtl(

  char *Buffer)

  {
  const char *FName = "MCSchedCtl";

  DBG(2,fUI) DPrint("%s(Buffer)\n",
    FName);

  fprintf(stdout,"%s\n",
    Buffer);

  return(SUCCESS);
  }  /* END MCSchedCtl() */





int MCDiagnose(

  char *Buffer)  /* I */

  {
  const char *FName = "MCDiagnose";

  DBG(2,fUI) DPrint("%s(Buffer)\n",
    FName);

  fprintf(stdout,"%s\n",
    Buffer);

  return(SUCCESS);
  }  /* END MCDiagnose() */





int MCShowIdle(

  char *Buffer)

  {
  char  *ptr;
  char   name[MAX_MNAME];
  long   qtime;
  int    minprocs;
  int    dmemory;
  int    WCLimit;
  int    count;
  int    priority;
  char   tmpQOS[MAX_MNAME];
  double TotalLoad;
  int    TotalProcs;
  char   jobState[MAX_MNAME];
  char   jobClass[MAX_MNAME];

  double xfactor;

  char   UserName[MAX_MNAME];
  char   GroupName[MAX_MNAME];

  char   SMPLine[MAX_MNAME];

  char  *TokPtr;

  long   StartTime;
  
  int    SMP;

  const char *FName = "MCShowIdle";

  DBG(2,fUI) DPrint("%s(Buffer)\n",
    FName);

  DBG(3,fUI) DPrint("Buffer: '%s'\n\n",
    Buffer);

  if (getenv(MSCHED_ENVSMPVAR) != NULL)
    SMP = TRUE;
  else
    SMP = FALSE;

  SMPLine[0] = '\0';

  /* display prioritized list of idle jobs */

  if (strcmp(ParName,GLOBAL_MPARNAME) != 0)
    {
    fprintf(stdout,"Partition: %s\n",
      ParName);
    }

  fprintf(stdout,"%18s %11s %8s %2s %9s %8s %6s %s%11s %9s %20s\n\n",
    "JobName",
    "Priority",
    "XFactor",
    "Q",
    "User",
    "Group",
    "Procs",
    (SMP == TRUE) ? "Memory " : "",
    "WCLimit",
    "Class",
    "SystemQueueTime");

  count = 0;

  /* get current time */

  sscanf(Buffer,"%ld %d\n",
    &StartTime,
    &TotalProcs);

  MUStrTok(Buffer,"\n",&TokPtr);

  TotalLoad = 0.0;

  while ((ptr = MUStrTok(NULL,"\n",&TokPtr)) != NULL)
    {
    if (strstr(ptr,"Truncated"))
      {
      fprintf(stderr,"WARNING:  idle job list is too long  (list truncated)\n");

      break;
      }

    count++;

    /* Format:  <JOBNAME> <USERNAME> <GROUPNAME> <QUEUETIME> <MINPROCS> <CPULIMIT> <PRIORITY> <QOS> <JOB-STATE> */

    sscanf(ptr,"%s %s %s %ld %d %d %d %d %s %s %s",
      name,
      UserName,
      GroupName,
      &qtime,
      &minprocs,
      &dmemory,
      &WCLimit,
      &priority,
      tmpQOS,
      jobState,
      jobClass);

    if (jobState[0] == 'X')
      jobState[0] = ' ';

    TotalLoad += (double)WCLimit * minprocs;

    xfactor = (double)(StartTime - qtime + WCLimit) / WCLimit;

    if (SMP == TRUE)
      sprintf(SMPLine," %6d",
        dmemory);

    /* display job */

    fprintf(stdout,"%18s%c%11d %8.1f %2.2s %9s %8s %6d%s%12s %9s %21s",
      name,
      jobState[0],
      priority,
      xfactor,
      tmpQOS,
      UserName,
      GroupName,
      minprocs,
      SMPLine,
      MULToTString(WCLimit),
      jobClass,
      MULToDString((mulong *)&qtime));

    DBG(3,fUI) DPrint("INFO:     Job[%03d] state: '%s' (char: '%c')\n",
      count,
      jobState,
      jobState[0]);
    }  /* END while (ptr) */

  fprintf(stdout,"\nJobs: %d  Total Backlog:  %.2f ProcHours  (%.2f Hours)\n",
    count,
    TotalLoad / 3600.0,
    (double)TotalLoad / 3600.0 / TotalProcs);

  return(SUCCESS);
  }  /* MCShowIdle() */




/* HvB */

int MCShowBlocked(
 
  char *Buffer)

  {
  char  *ptr;
  char   name[MAX_MNAME];
  char   Reason[MAX_MBUFFER];
  int    TotalProcs;
 
  char   UserName[MAX_MNAME];

  char  *TokPtr;

  long   StartTime;
  
  const char *FName = "MCShowBlocked";

  DBG(2,fUI) DPrint("%s(Buffer)\n",
    FName);

  DBG(3,fUI) DPrint("Buffer: '%s'",
    Buffer);

  fprintf(stdout,"%18s %9s %20s\n\n",
    "JobName",
    "User",
    "Reason");

  /* get current time */

  sscanf(Buffer,"%ld %d\n",
    &StartTime,
    &TotalProcs);

  MUStrTok(Buffer,"\n",&TokPtr);

  while ((ptr = MUStrTok(NULL,"\n",&TokPtr)) != NULL)
    {
    if (strstr(ptr,"Truncated"))
      {
      fprintf(stderr,"WARNING:  blocked job list is too long  (list truncated)\n");

      break;
      }

    /* Format:  <JOBNAME> <USERNAME> <DUMMY1> <DUMMY2> <REASON> */

    /*
     * DUMMY1 = "job" and DUMMY2 = <jobnr> 
     */

    sscanf(ptr,"%s %s %*s %*s %[^\n]s",
      name,
      UserName,
      Reason);

    /* display job */

    fprintf(stdout,"%18s %9s %s\n",
      name,
      UserName,
      Reason);
    }  /* END while (ptr) */

  return(SUCCESS);
  }  /* MCShowBlocked() */




int MCShowRun(

  char *Buffer)  /* I */

  {
  char  *ptr;
  char   name[MAX_MNAME];
  long   stime;
  long   qtime;
  long   AWallTime;

  int    procs;
  long   WCLimit;
  char   tmpQOS[MAX_MNAME];
  int    count;
  double xfactor;
  char   effic[MAX_MNAME];
  char   partition[MAX_MNAME];

  int    rc;

  double psdedicated;
  double psutilized;

  int    UpProcs;
  int    IdleProcs;
  int    BusyProcs;

  int    UpNodes;
  int    IdleNodes;
  int    BusyNodes;

  /* int    BusyNodes; */

  char   JState[MAX_MNAME];
  char   UserName[MAX_MNAME];
  char   GroupName[MAX_MNAME];

  char   MHostName[MAX_MNAME];

  long   Now;

  int    DMemory;
  int    DedicatedMemory;
  int    ConfiguredMemory;

  mbool_t SMP;

  char   SMPLine[MAX_MLINE];
  char   JobMarker;

  long   JFlags;
  long   Priority;

  char  *TokPtr;

  char   VLine[MAX_MLINE];

  const char *FName = "MCShowRun";

  DBG(2,fUI) DPrint("%s(Buffer)\n",
    FName);

  DBG(3,fUI) DPrint("Buffer: '%s'\n\n",
    Buffer);

  if (getenv(MSCHED_ENVSMPVAR) != NULL)
    SMP = TRUE;
  else
    SMP = FALSE;

  SMPLine[0] = '\0';

  /* get general state */

  sscanf(Buffer,"%ld %d %d %d %d %d %d\n",
    &Now,
    &UpProcs,
    &IdleProcs,
    &UpNodes,
    &IdleNodes,
    &ConfiguredMemory,
    &BusyProcs);

  BusyNodes = UpNodes - IdleNodes;

  BusyProcs = MIN(UpProcs,BusyProcs);

  /* display partition */

  if (strcmp(ParName,GLOBAL_MPARNAME) != 0)
    {
    fprintf(stdout,"partition: %s\n",
      ParName);
    }

  if (Flags & (1 << mcmVerbose))
    {
    sprintf(VLine," %6s",
      "prio");
    }
  else
    {
    VLine[0] = '\0';
    }

  /* display list of active jobs */

  fprintf(stdout,"%18s%c %1s%s %3s %6s %8s %2s %9s %8s %8s %5s %s%11s  %19s\n\n",
    "JobName",
    ' ',
    "S",
    VLine,
    "Par",
    "Effic",
    "XFactor",
    "Q",
    "User",
    "Group",
    "MHost",
    (C.DisplayFlags & (1 << dfNodeCentric)) ?
      "Nodes" : "Procs",
    (SMP == TRUE) ? "Memory " : "",
    "Remaining",
    "StartTime");

  count = 0;

  ptr = MUStrTok(Buffer,"\n",&TokPtr);

  DedicatedMemory = 0;

  /*  read all active jobs */

  while ((ptr = MUStrTok(NULL,"\n",&TokPtr)) != NULL)
    {
    count++;

    DBG(3,fUI) DPrint("line: '%s'\n",
      ptr);

    /* Format:  <JOBNAME> <STATE> <USERNAME> <GROUPNAME> <START TIME> <QUEUE TIME> <PROCS> <CPULIMIT> <QOS> <PSDEDICATED> <PSUTILIZED> <PARTITION> */

    rc = sscanf(ptr,"%s %s %s %s %ld %ld %d %ld %s %lf %lf %s %d %s %ld %ld %ld",
      name,
      JState,
      UserName,
      GroupName,
      &stime,
      &qtime,
      &procs,
      &WCLimit,
      tmpQOS,
      &psdedicated,
      &psutilized,
      partition,
      &DMemory,
      MHostName,
      &JFlags,
      &AWallTime,
      &Priority
      );

    if (rc != 17)
      {
      fprintf(stderr,"ALERT:  job data is corrupt (%d fields: '%s')\n",
        rc,
        ptr);

      continue;
      }
     
    DedicatedMemory += DMemory;

    xfactor = (double)((stime - qtime) + WCLimit) / WCLimit;

    if (psdedicated > 0.0)
      sprintf(effic,"%6.2f",(double)psutilized / psdedicated * 100.0);
    else
      strcpy(effic,"------");

    if (SMP == TRUE)
      sprintf(SMPLine,"%6d ",
        DMemory);
     
    /* display job */

    if (JFlags & (1 << mjfSPViolation))
      {
      JobMarker = '_';
      }
    else if (JFlags & (1 << mjfBackfill))
      {
      if (JFlags & (1 << mjfPreemptee))
        {
        JobMarker = '*';
        }
      else
        {
        JobMarker = '+';
        }
      }
    else if (JFlags & (1 << mjfPreemptee))
      {
      JobMarker = '-';
      }
    else
      {
      JobMarker = ' ';
      }

    if (Flags & (1 << mcmVerbose))
      {
      sprintf(VLine," %6ld",
        Priority);
      }
    else
      {
      VLine[0] = '\0';
      }

    fprintf(stdout,"%18s%c %c%s %3.3s %6s %8.1f %2.2s %9s %8s %8s %5d %s%11s  %19s",
      name,
      JobMarker,
      JState[0],
      VLine,
      partition,
      effic,
      xfactor,
      tmpQOS,
      UserName,
      GroupName,
      MHostName,
      procs,
      SMPLine,
      MULToTString(WCLimit - AWallTime),
      MULToDString((mulong *)&stime));
    }  /* END while (ptr) */

  if (C.DisplayFlags & (1 << dfNodeCentric))
    {
    fprintf(stdout,"\n%3d Jobs   %5d of %5d Nodes Active (%.2f%c)\n",
      count,
      BusyNodes,
      UpNodes,
      (double)BusyNodes / UpNodes * 100.0,
      '%');
    }
  else
    {
    fprintf(stdout,"\n%3d Jobs   %5d of %5d Processors Active (%.2f%c)\n",
      count,
      BusyProcs,
      UpProcs,
      (UpProcs > 0) ? (double)BusyProcs / UpProcs * 100.0 : 0.0,
      '%');
    }

  if ((SMP == TRUE) && (ConfiguredMemory > 0))
    {
    fprintf(stdout,"           %5d of %5d MB Memory in Use  (%.2f%c)\n",
      DedicatedMemory,
      ConfiguredMemory,
      (double)DedicatedMemory / ConfiguredMemory * 100.0,
      '%');
    }

  return(SUCCESS);
  }  /* END MCShowRun() */





int MCShowJobHold(

  char *Buffer)  /* I */

  {
  int index;

  char *ptr;
  char  Name[MAX_MNAME];
  int   Holds;
 
  const char *FName = "MCShowJobHold";
 
  DBG(2,fUI) DPrint("%s(Buffer)\n",
    FName);

  fprintf(stdout,"%16s %10s\n\n",
    "JOBNAME",
    "HOLDS");

  ptr = strtok(Buffer,"\n");
 
  while ((ptr = strtok(NULL,"\n")) != NULL)
    {
    sscanf(ptr,"%s %d",
      Name,
      &Holds);

    fprintf(stdout,"%16s ",
      Name);

    for (index = 0;MHoldType[index] != 0;index++)
       {
       if (Holds & (1 << index))
         fprintf(stdout,"%6s ",
           MHoldType[index]);
       else
         fprintf(stdout,"%6s ",
           "      ");
       }

    fprintf(stdout,"\n");
    }

  return(SUCCESS);
  }  /* END MSShowJobHold() */





int MCShowSchedulerStatistics(

  char *Buffer)

  {
  char  *ptr;

  long   Time;
  long   InitializationTime;
  int    SchedRunTime;
  int    IdleQueueSize;
  int    RunnableJobs;
  int    RunningJobs;
  int    AvailableNodes;
  int    AvailableProcs;
  int    AvailableMem;

  int    IdleNodes;
  int    IdleProcs;
  int    IdleMem;

  char   TimeString[MAX_MLINE];

  int    TotalJobsCompleted;
  int    SuccessfulJobsCompleted;
  double TotalProcHours;
  double DedicatedProcHours;
  double SuccessfulProcHours;
  long   QueuePS;
  unsigned long AvgQueuePH;
  double PSRun;

  double MSAvail;

  double WeightedCpuAccuracy;
  double CpuAccuracy;
  double AvgXFactor;
  int    Iteration;
  long   RMPollInterval;
  int    ActiveNodes;
  int    BusyProcs;
  int    BusyMem;

  double MinEfficiency;
  int    MinEffIteration;
  double MaxXFactor;

  double AvgQTime;
  double MaxQTime;

  double AvgBypass;
  int    MaxBypass;

  long   StartTime;

  double PSDedicated;
  double PSUtilized;
  double MSDedicated;
  double MSUtilized;
  int    JobsEvaluated;

  double PreemptPH;
  int    PreemptJobs;

  int    SMP;
  
  time_t tmpTime;

  const char *FName = "MCShowSchedulerStatistics";

  DBG(2,fUI) DPrint("%s(Buffer)\n",
    FName);

  if (getenv(MSCHED_ENVSMPVAR) != NULL)
    SMP = TRUE;
  else
    SMP = FALSE;

  ptr = strtok(Buffer,"\n");

  Time = strtol(ptr,NULL,10);

  ptr = strtok(NULL,"\n");

  /*          STT INT RT IQ RJ RJ AN AP AM IN IP IM TC SJ TPH DPH SPH MSA MSD QPS AQN WCA CAC PSX IT RPI MEF ME MXF ABP MB AQT MQT PSR PSD PSU MSA MSD JE */

  sscanf(ptr,"%ld %ld %d %d %d %d %d %d %d %d %d %d %d %d %lf %lf %lf %lf %lf %ld %lu %lf %lf %lf %d %ld %lf %d %lf %lf %d %lf %lf %lf %lf %lf %lf %lf %d %d %lf",
    &StartTime,
    &InitializationTime,
    &SchedRunTime,                /* Time in Hundredth of a Second */
    &IdleQueueSize,
    &RunnableJobs,
    &RunningJobs,
    &AvailableNodes,
    &AvailableProcs,
    &AvailableMem,
    &IdleNodes,
    &IdleProcs,
    &IdleMem,
    &TotalJobsCompleted,
    &SuccessfulJobsCompleted,
    &TotalProcHours,
    &DedicatedProcHours,
    &SuccessfulProcHours,
    &MSAvail,
    &MSDedicated,
    &QueuePS,
    &AvgQueuePH,
    &WeightedCpuAccuracy,
    &CpuAccuracy,
    &AvgXFactor,
    &Iteration,
    &RMPollInterval,
    &MinEfficiency,
    &MinEffIteration,
    &MaxXFactor,
    &AvgBypass,
    &MaxBypass,
    &AvgQTime,
    &MaxQTime,
    &PSRun,
    &PSDedicated,
    &PSUtilized,
    &MSDedicated,
    &MSUtilized,
    &JobsEvaluated,
    &PreemptJobs,
    &PreemptPH);

  ActiveNodes = AvailableNodes - IdleNodes;
  BusyProcs   = AvailableProcs - IdleProcs;
  BusyMem     = AvailableMem   - IdleMem;

  fprintf(stdout,"\n");

  if (Flags & (1 << mcmVerbose))
    {
    tmpTime = (time_t)Time;

    if (!strcmp(C.SchedulerMode,"SIMULATION"))
      {
      sprintf(TimeString,"current scheduler time: %s",
        ctime(&tmpTime));

      TimeString[strlen(TimeString) - 1] = '\0';

      sprintf(temp_str," (%ld)\n",
        Time);
      strcat(TimeString,temp_str);

      fprintf(stdout,"%s",
        TimeString);
      }
    else
      {
      fprintf(stdout,"current scheduler time: %s\n",
        ctime(&tmpTime));
      }
    }    /* END if (Flags & (1 << mcmVerbose)) */

  fprintf(stdout,"%s active for   %11s  stats initialized on %s",
    MSCHED_SNAME,
    MULToTString(SchedRunTime / 100),
    MULToDString((mulong *)&InitializationTime));

  if (Flags & (1 << mcmVerbose))
    {
    fprintf(stdout,"statistics for iteration %5d  scheduler started on %s",
      Iteration,
      MULToDString((mulong *)&StartTime));
    }

  fprintf(stdout,"\n");

  fprintf(stdout,"Eligible/Idle Jobs:            %9d/%-9d (%.3f%c)\n",
    RunnableJobs,
    IdleQueueSize,
    (IdleQueueSize > 0) ? ((double)RunnableJobs * 100.0 / IdleQueueSize) : 0.0,
    '%');

  fprintf(stdout,"Active Jobs:                   %9d\n",
    RunningJobs);
   
  fprintf(stdout,"Successful/Completed Jobs:     %9d/%-9d (%.3f%c)\n",
    SuccessfulJobsCompleted,
    TotalJobsCompleted,
    (TotalJobsCompleted > 0) ? ((double)SuccessfulJobsCompleted * 100.0 / TotalJobsCompleted) : 0.0,
    '%');

  if (Flags & (1 << mcmVerbose))
    {
    fprintf(stdout,"Preempt Jobs:                  %9d\n",
      PreemptJobs);
    }

  fprintf(stdout,"Avg/Max QTime (Hours):         %9.2f/%-9.2f\n",
    AvgQTime / 3600.0,
    MaxQTime / 3600.0);
                 
  fprintf(stdout,"Avg/Max XFactor:               %9.2f/%-9.2f\n",
    AvgXFactor,
    MaxXFactor);

  if (Flags & (1 << mcmVerbose))
    {
    fprintf(stdout,"Avg/Max Bypass:                %9.2f/%-9.2f\n",
      AvgBypass,
      (double)MaxBypass);
    }

  fprintf(stdout,"\n");

  fprintf(stdout,"Dedicated/Total ProcHours:     %9.2f/%-9.2f (%.3f%c)\n",
    DedicatedProcHours,
    TotalProcHours,
    (TotalProcHours != 0.0) ? (DedicatedProcHours * 100.0 / TotalProcHours) : 0.0,
    '%');

  if (SMP == TRUE)
    {
    fprintf(stdout,"Dedicated/Total Mem (GBHours): %9.2f/%-9.2f (%.3f%c)\n",
      MSDedicated / 3600.0 / 1024.0,
      MSAvail / 3600.0 / 1024.0,
      (MSAvail > 0.0) ? (MSDedicated * 100.0 / MSAvail) : 0.0,
      '%');
    }

  if (Flags & (1 << mcmVerbose))
    {
    fprintf(stdout,"Preempt/Dedicated ProcHours:   %9.2f/%-9.2f (%.3f%c)\n",
      PreemptPH,
      DedicatedProcHours,
      (DedicatedProcHours != 0.0) ? (PreemptPH * 100.0 / DedicatedProcHours) : 0.0,
      '%');
    }

  fprintf(stdout,"\n");

  fprintf(stdout,"Current Active/Total Procs:    %9d/%-9d (%.3f%c)\n",
    BusyProcs,
    AvailableProcs,
    (AvailableProcs > 0) ? (double)BusyProcs * 100.0 / AvailableProcs : 0.0,
    '%');

  if (SMP == TRUE)
    {
    fprintf(stdout,"Current Active/Total Mem (GB): %9.2f/%-9.2f (%.3f%c)\n",
      (double)BusyMem / 1024.0,
      (double)AvailableMem / 1024.0,
      (double)BusyMem * 100.0 / AvailableMem,
      '%');
    }

  if ((Flags & (1 << mcmVerbose)) && (AvailableProcs != AvailableNodes))
    {
    fprintf(stdout,"Current Active/Total Nodes:    %9d/%-9d (%.3f%c)\n",
      ActiveNodes,
      AvailableNodes,
      (double)ActiveNodes * 100.0 / AvailableNodes,
      '%');
    }

  fprintf(stdout,"\n");

  if (SuccessfulProcHours > 0.0)
    {
    fprintf(stdout,"Avg WallClock Accuracy:        %8.3f%c\n",
      (TotalJobsCompleted > 0) ? CpuAccuracy / TotalJobsCompleted * 100.0 : 0.0,
      '%');

    fprintf(stdout,"Avg Job Proc Efficiency:       %8.3f%c\n",
      (PSDedicated > 0.0) ? PSUtilized / PSDedicated * 100.0 : 0.0,
      '%');
    }
  else
    {
    fprintf(stdout,"Avg WallClock Accuracy:         %8s\n",
      "<N/A>");

    fprintf(stdout,"Avg Job Proc Efficiency:        %8s\n",
      "<N/A>");
    }

  if (Flags & (1 << mcmVerbose))
    {
    if (MinEffIteration > 0)
      {
      fprintf(stdout,"Min System Utilization:        %8.3f%c (on iteration %d)\n",
        MinEfficiency,
        '%',
        MinEffIteration);

      }
    else
      {
      fprintf(stdout,"Min System Utilization:         %8s\n",
        "<N/A>");
      }
    }

  if (SuccessfulProcHours != 0)
    {
    fprintf(stdout,"Est/Avg Backlog (Hours):        %8.2f/%-8.2f\n",
      (double)QueuePS * (CpuAccuracy / TotalJobsCompleted) /
      (DedicatedProcHours / TotalProcHours * 3600 * AvailableProcs),
      (double)AvgQueuePH / AvailableProcs);
    }
  else
    {
    fprintf(stdout,"Est/Avg Backlog (Hours):        %8s/%-8s\n",
      "<N/A>",
      "<N/A>");
    }

  DBG(2,fUI) DPrint("INFO:     idle backlog: %8lu seconds (%8lu hours)\n",
    QueuePS,
    QueuePS / 3600);

  DBG(3,fUI) DPrint("INFO:     X: %lf  WCA: %lf  CA: %lf\n",
    AvgXFactor,
    WeightedCpuAccuracy,
    CpuAccuracy);
 
  return(SUCCESS);
  }  /* END MCShowSchedulerStatistics() */




int MCResetStats(

  char *Buffer)

  {
  long   Time;

  const char *FName = "MCResetStats";

  DBG(2,fUI) DPrint("%s(Buffer)\n",
    FName);

  Time = strtol(Buffer,NULL,0);

  fprintf(stdout,"statistics reset on %s\n",
    MULToDString((mulong *)&Time));

  return(SUCCESS);
  }  /* END MCResetStats() */





int MCResCreate(

  char *Buffer)

  {
  const char *FName = "MCResCreate";

  DBG(2,fUI) DPrint("%s(Buffer)\n",
    FName);

  fprintf(stdout,"reservation created\n");

  fprintf(stdout,"\n\n%s\n\n",
    Buffer);

  return(SUCCESS);
  }  /* END MCResCreate() */





int MCReleaseReservation(

  char *Buffer)

  {
  const char *FName = "MCReleaseReservation";

  DBG(2,fUI) DPrint("%s(Buffer)\n",
    FName);

  fprintf(stdout,"\n\n%s\n\n",
    Buffer);

  return(SUCCESS);
  }  /* END MCReleaseReservation() */






int MCSetJobHold(

  char *Buffer)

  {
  const char *FName = "MCSetJobHold";

  DBG(2,fUI) DPrint("%s(Buffer)\n",
    FName);

  fprintf(stdout,"%s\n",
    Buffer);

  return(SUCCESS);
  }  /* END MCSetJobHold() */




int MCReleaseJobHold(

  char *Buffer)

  {
  const char *FName = "MCReleaseJobHold";

  DBG(2,fUI) DPrint("%s(Buffer)\n",
    FName);

  fprintf(stdout,"%s\n",
    Buffer);

  return(SUCCESS);
  }  /* END MCReleaseJobHold() */




int MCSetJobSystemPrio(

  char *Buffer)

  {
  DBG(2,fUI) DPrint("MCSetJobSystemPrio(Buffer)\n");

  fprintf(stdout,"system priority adjusted\n");

  return(SUCCESS);
  }  /* END MCSetJobSystemPrio() */





int MCSetJobUserPrio(

  char *Buffer)

  {
  DBG(2,fUI) DPrint("MCSetJobUserPrio(Buffer)\n");

  fprintf(stdout,"user priority adjusted\n");

  return(SUCCESS);
  }  /* END MCSetJobUserPrio() */





int MCStopScheduling(

  char *Buffer)

  {
  DBG(2,fUI) DPrint("MCStopScheduling(Buffer)\n");

  fprintf(stdout,"%s\n",
    Buffer);

  return(SUCCESS);
  }  /* END MCStopScheduling() */





int MCResumeScheduling(

  char *Buffer)

  {
  DBG(2,fUI) DPrint("MCResumeScheduling(Buffer)\n");

  fprintf(stdout,"%s\n",
    Buffer);

  return(SUCCESS);
  }  /* END MCResumeScheduling() */





int MCSetJobDeadline(

  char *Buffer)

  {
  DBG(2,fUI) DPrint("MCSetJobDeadline(Buffer)\n");

  fprintf(stdout,"deadline set on job %s\n",
    Buffer);

  return(SUCCESS);
  }  /* END MCSetJobDeadline() */




int MCReleaseJobDeadline(

  char *Buffer)

  {
  DBG(2,fUI) DPrint("MCReleaseJobDeadline(Buffer)\n");

  fprintf(stdout,"deadline released on job %s\n",
    Buffer);

  return(SUCCESS);
  }  /* END MCReleaseJobDeadline() */





int MCShowJobDeadline(

  char *Buffer)

  {
  char    Name[MAX_MNAME];
  char   *ptr;
  long    Deadline;
  long    CpuLimit;
  long    Now;
  
  DBG(2,fUI) DPrint("MCShowJobDeadline(Buffer)\n");

  ptr = strtok(Buffer,"\n");

  Now = strtol(ptr,NULL,0);

  fprintf(stdout,"%16s  %10s  (%12s)  %s\n",
    "Job Name",
    "CpuLimit",
    "StartSeconds",
    "DeadLine");

  while ((ptr = strtok(NULL,"\n")) != NULL)
    {
    sscanf(ptr,"%s %ld %ld",
      Name,
      &Deadline,
      &CpuLimit);

    if ((Deadline - Now) > 9999999)
      {
      fprintf(stdout,"%16s  %10ld  (NO DEADLINE)\n",
        Name,
        CpuLimit);
      }
    else
      {
      fprintf(stdout,"%16s  %10ld  (%12ld)  %s",
        Name,
        CpuLimit,
        (Deadline - Now - CpuLimit),
        MULToDString((mulong *)&Deadline));
      }
    }

  return(SUCCESS);
  }  /* END MCShowJobDeadline() */






int MCSetJobQOS(

  char *Buffer)

  {
  DBG(2,fUI) DPrint("MCSetJobQOS(Buffer)\n");

  fprintf(stdout,"%s\n",
    Buffer);

  return(SUCCESS);
  }  /* END MCSetJobQOS() */





int MCShowGrid(

char *Buffer)

  {
  DBG(2,fUI) DPrint("MCShowGrid(Buffer)\n");

  fprintf(stdout,"\n\n%s\n",
    Buffer);

  return(SUCCESS);
  }  /* END MCShowGrind() */





int MCShowBackfillWindow(

  char *Buffer)

  {
  DBG(2,fUI) DPrint("MCShowBackfillWindow(Buffer)\n");

  fprintf(stdout,"%s\n",
    Buffer);

  return(SUCCESS);
  }  /* END MCShowBackfillWindow() */





int MCShowConfig(

  char *Buffer)

  {
  DBG(2,fUI) DPrint("MCShowConfig(Buffer)\n");

  fprintf(stdout,"%s\n\n",
    Buffer);

  return(SUCCESS);
  }  /* END MCShowConfig() */





int MCRunJob(

  char *Buffer)

  {
  DBG(2,fUI) DPrint("MCRunJob(Buffer)\n");

  fprintf(stdout,"\n\n%s\n",
    Buffer);

  if (strstr(Buffer,"ERROR:") != NULL)
    return(FAILURE);

  return(SUCCESS);
  }  /* END MCRunJob() */





int MCCancelJob(

  char *Buffer)

  {
  DBG(2,fUI) DPrint("MCCancelJob(Buffer)\n");

  fprintf(stdout,"\n\n%s\n",
    Buffer);

  if (strstr(Buffer,"ERROR:") != NULL)
    return(FAILURE);

  return(SUCCESS);
  }  /* END MCCancelJob() */





int MCShowNodeStats(

  char *Buffer)

  {
  const char *FName = "MCShowNodeStats";

  DBG(2,fUI) DPrint("%s(Buffer)\n",
    FName);

  fprintf(stdout,"\n\n%s\n",
    Buffer);

  return(SUCCESS);
  }  /* END MCShowNodeStats() */





int MCChangeParameter(

  char *Buffer)

  {
  const char *FName = "MCChangeParameter";

  DBG(2,fUI) DPrint("%s(Buffer)\n",
    FName);

  fprintf(stdout,"\n\n%s\n",
    Buffer);

  return(SUCCESS);
  }  /* END MCChangeParameter() */





int MCShowEstimatedStartTime(

  char *Buffer)

  {
  const char *FName = "MCShowEstimatedStartTime";

  DBG(2,fUI) DPrint("%s(Buffer)\n",
    FName);

  fprintf(stdout,"\n\n%s\n",
    Buffer);

  return(SUCCESS);
  }  /* END MCShowEstimatedStartTime() */





int MCShowQueue(

  char *Buffer)  /* I */

  {
  const char *FName = "MCShowQueue";

  DBG(4,fUI) DPrint("%s(Buffer)\n",
    FName);

  switch(QueueMode)
    {
    case 0:

      MCShowQ(Buffer,0);

      break;

    case 1:

      MCShowIdle(Buffer);

      break;

    case 2:

      MCShowRun(Buffer);

      break;

    case 3:

      MCShowBlocked(Buffer);

      break;

    default:

      DBG(1,fUI) DPrint("ALERT:    unexpected showq type, %d\n",
        QueueMode);

      break;
    }  /* END switch(QueueMode) */

  return(SUCCESS);
  }  /* END MCShowQueue() */


/* query the total number of tasks running by a given user */


int showTasksPerUser(

  char *Buffer)
	{
  char  *ptr;
  char   name[MAX_MNAME];
  long   stime;
  long   qtime;
  int    procs;
  long   WCLimit;
  char   tmpQOS[MAX_MNAME];
  int    count;
  int    priority;
  int    state;

  long   Now;

  char   UserName[MAX_MNAME];

  int    UpProcs;
  int    IdleProcs;

  int    UpNodes;
  int    IdleNodes;
  int    ActiveNodes;

  int    BusyNodes;
  int    BusyProcs;

  int    acount;
  int    icount;
  int    ncount;

  int    rc;

  char   tmp[MAX_MLINE];

  const char *FName = "showTasksPerUser";
  int 	 sumOfProcs;

  sumOfProcs = 0;
  /* get present time */
  if(TempArgv == NULL)
  	return FAILURE;

  ptr = strtok(Buffer,"\n");
  char userName[40] = "tuf94753";
  /* get and sum each record */
  while ((ptr = strtok(NULL,"\n")) != NULL)
    {
    if (!strcmp(ptr,"[ENDACTIVE]"))
      break;

      rc = sscanf(ptr,"%s %s %ld %ld %d %ld %s %d %d",
        name,
        UserName,
        &stime,
        &qtime,
        &procs,
        &WCLimit,
        tmpQOS,
        &state,
        &priority);

      if (rc != 9)
        continue;

      /* just calculate the tasks for the desired user*/
    	if (!strcmp(TempArgv,UserName))
    		sumOfProcs = sumOfProcs + procs;
  	}

  fprintf(stdout,"The total number of tasks running by %s is %d\n", TempArgv, sumOfProcs);

  return(SUCCESS);
	}/* END showTasksPerUser() */




int MCShowQ(

  char *Buffer,
  int   Mode)

  {
  char  *ptr;
  char   name[MAX_MNAME];
  long   stime;
  long   qtime;
  int    procs;
  long   WCLimit;
  char   tmpQOS[MAX_MNAME];
  int    count;
  int    priority;
  int    state;

  long   Now;

  char   UserName[MAX_MNAME];

  int    UpProcs;
  int    IdleProcs;

  int    UpNodes;
  int    IdleNodes;
  int    ActiveNodes;
 
  int    BusyNodes;
  int    BusyProcs;

  int    acount;
  int    icount;
  int    ncount;

  int    rc;

  char   tmp[MAX_MLINE];

  const char *FName = "MCShowQ";

  DBG(2,fUI) DPrint("%s(Buffer)\n",
    FName);

  DBG(4,fUI) DPrint("Buffer: '%s'\n\n",
    Buffer);

  count = 0;

  /* get present time */

  rc = sscanf(Buffer,"%ld %d %d %d %d %d %d\n",
    &Now,
    &UpProcs,
    &IdleProcs,
    &UpNodes,
    &IdleNodes,
    &ActiveNodes,
    &BusyProcs);
 
  BusyNodes = ActiveNodes;

  BusyProcs = MIN(UpProcs,BusyProcs);

  ptr = strtok(Buffer,"\n");

  /* display list of active jobs */

  fprintf(stdout,"ACTIVE JOBS--------------------\n");

  fprintf(stdout,"%-18s %8s %10s %5s %11s %20s\n\n",
    "JOBNAME",
    "USERNAME",
    "STATE",
    "PROC",
    "REMAINING",
    "STARTTIME");

  /* read all active jobs */

  acount = 0;

  while ((ptr = strtok(NULL,"\n")) != NULL)
    {
    if (!strcmp(ptr,"[ENDACTIVE]"))
      break;

    acount++;
    count++;

    DBG(3,fUI) DPrint("line: '%s'\n",
      ptr);

    /* Format:  <JOBNAME> <USERNAME> <START TIME> <QUEUE TIME> <PROCS> <CPULIMIT> <QOS> <STATE> <PRIO> */

    rc = sscanf(ptr,"%s %s %ld %ld %d %ld %s %d %d",
      name,
      UserName,
      &stime,
      &qtime,
      &procs,
      &WCLimit,
      tmpQOS,
      &state,
      &priority);

    if (rc != 9)
      continue;

    /* display job */

    fprintf(stdout,"%-18s %8s %10s %5d %11s  %19s",
      name,
      UserName,
      MJobState[state],
      procs,
      MULToTString(WCLimit - (Now - stime)),
      MULToDString((mulong *)&stime));
    }  /* END while (ptr) */

  sprintf(tmp,"%d Active Job%c   ",
    acount,
    (acount == 1) ? ' ' : 's');

  fprintf(stdout,"\n%21s %4d of %4d Processors Active (%.2f%c)\n",
    tmp,
    BusyProcs,
    UpProcs,
    (UpProcs > 0) ? (double)BusyProcs / UpProcs * 100.0 : 0.0,
    '%');

  if ((UpNodes > 0) && (UpProcs != UpNodes))
    {
    fprintf(stdout,"%21s %4d of %4d Nodes Active      (%.2f%c)\n",
      " ",
      BusyNodes,
      UpNodes,
      (UpNodes > 0) ? (double)BusyNodes / UpNodes * 100.0 : 0.0,
      '%');
    }

  /* display list of idle jobs */

  fprintf(stdout,"\nIDLE JOBS----------------------\n");

  fprintf(stdout,"%-18s %8s %10s %5s %11s %20s\n\n",
    "JOBNAME",
    "USERNAME",
    "STATE",
    "PROC",
    "WCLIMIT",
    "QUEUETIME");

  /* read all idle jobs */

  icount = 0;

  while ((ptr = strtok(NULL,"\n")) != NULL)
    {
    if (!strcmp(ptr,"[ENDIDLE]"))
      break;

    count++;
    icount++;

    DBG(3,fUI) DPrint("line: '%s'\n",
      ptr);

    /* Format:  <JOBNAME> <USERNAME> <START TIME> <QUEUE TIME> <PROCS> <CPULIMIT> <QOS> <STATE> <PRIO> */

    rc = sscanf(ptr,"%s %s %ld %ld %d %ld %s %d %d",
      name,
      UserName,
      &stime,
      &qtime,
      &procs,
      &WCLimit,
      tmpQOS,
      &state,
      &priority);

    /* display job */

    if (rc != 9)
      continue;

    fprintf(stdout,"%-18s %8s %10s %5d %11s  %19s",
      name,
      UserName,
      MJobState[state],
      procs,
      MULToTString(WCLimit),
      MULToDString((mulong *)&qtime));
    }

  fprintf(stdout,"\n%d Idle Job%c\n",
    icount,
    (icount == 1) ? ' ' : 's');

  /* display list of non-queued jobs */

  fprintf(stdout,"\nBLOCKED JOBS----------------\n");

  fprintf(stdout,"%-18s %8s %10s %5s %11s %20s\n\n",
    "JOBNAME",
    "USERNAME",
    "STATE",
    "PROC",
    "WCLIMIT",
    "QUEUETIME");

  /* read all blocked jobs */

  ncount = 0;

  while ((ptr = strtok(NULL,"\n")) != NULL)
    {
    if (!strcmp(ptr,"[ENDBLOCKED]"))
      {
      break;
      }

    count++;
    ncount++;

    DBG(3,fUI) DPrint("line: '%s'\n",
      ptr);

    /* Format:  <JOBNAME> <USERNAME> <START TIME> <QUEUE TIME> <PROCS> <CPULIMIT> <QOS> <STATE> <PRIORITY>  */

    rc = sscanf(ptr,"%s %s %ld %ld %d %ld %s %d %d",
      name,
      UserName,
      &stime,
      &qtime,
      &procs,
      &WCLimit,
      tmpQOS,
      &state,
      &priority);

    if (rc != 9)
      continue;

    /* display job */

    fprintf(stdout,"%-18s %8s %10s %5d %11s  %19s",
      name,
      UserName,
      (state > 0) ? MJobState[state] : "-",
      procs,
      MULToTString(WCLimit),
      MULToDString((mulong *)&qtime));
    }  /* END while (ptr) */

  fprintf(stdout,"\nTotal Jobs: %d   Active Jobs: %d   Idle Jobs: %d   Blocked Jobs: %d\n",
    count,
    acount,
    icount,
    ncount);

  while ((ptr = strtok(NULL,"\n")) != NULL)
    {
    fprintf(stdout,"\n%s\n",
      ptr);
    }

  return(SUCCESS);
  }  /* END MCShowQ() */




/* order high to low */

int PSDedicatedComp(

  cstats *a,
  cstats *b)

  {
  static int tmp;

  tmp = (int)((b->PSDedicated - a->PSDedicated) * 1000);

  return(tmp);
  }




int MCUPSDedicatedComp(
 
  mgcred_t *a,
  mgcred_t *b)
 
  {
  static int tmp;
 
  tmp = (int)((b->Stat.PSDedicated - a->Stat.PSDedicated) * 1000);
 
  return(tmp);
  }




int MCGPSDedicatedComp(
 
  mgcred_t *a,
  mgcred_t *b)
 
  {
  static int tmp;
 
  tmp = (int)((b->Stat.PSDedicated - a->Stat.PSDedicated) * 1000);
 
  return(tmp);
  }



int MCAPSDedicatedComp(
 
  mgcred_t *a,
  mgcred_t *b)
 
  {
  static int tmp;
 
  tmp = (int)((b->Stat.PSDedicated - a->Stat.PSDedicated) * 1000);
 
  return(tmp);
  }



int MCQPSDedicatedComp(
 
  mqos_t *a,
  mqos_t *b)
 
  {
  static int tmp;
 
  tmp = (int)((b->Stat.PSDedicated - a->Stat.PSDedicated) * 1000);
 
  return(tmp);
  }




int MCCPSDedicatedComp(
 
  mclass_t *a,
  mclass_t *b)
 
  {
  static int tmp;
 
  tmp = (int)((b->Stat.PSDedicated - a->Stat.PSDedicated) * 1000);
 
  return(tmp);
  }

/* END omclient.c */


