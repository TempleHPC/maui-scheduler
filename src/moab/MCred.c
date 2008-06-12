/* HEADER */
 
/* Contains:                                         *
 *                                                   */

#include "moab.h"
#include "msched-proto.h"

extern mlog_t      mlog;
 
extern msched_t    MSched;
extern mgcred_t    *MUser[MAX_MUSER + MAX_MHBUF];
extern mgcred_t    MGroup[MAX_MGROUP + MAX_MHBUF];
extern mqos_t      MQOS[];
extern mattrlist_t MAList;
extern mpar_t      MPar[];
extern mclient_t   MClient[];

extern const char *MJobFlags[];
extern const char *MCredAttr[];
extern const char *MOServAttr[];
extern const char *MXO[];
extern const char *MXOC[];
extern const char *MCredCfgParm[];
extern const char *MCSAlgoType[];
extern const char *MSockProtocol[];
extern const char *MSysAttr[];



/* local prototypes */


int __MCredParseLimit(char *,int *,int *); 
char *__MCredShowLimit(mpu_t *,int,int,int);





int MCredLoadConfig(

  int   OIndex, 
  char *CName,
  char *ABuf,    
  char *EMsg)

  {
  char   IndexName[MAX_MNAME];

  char   Value[MAX_MLINE];

  char  *ptr;
  char  *head;

  void     *C;

  mfs_t    *CFSPtr = NULL;
  mcredl_t *CLPtr  = NULL;
 
  head = (ABuf != NULL) ? ABuf : MSched.ConfigBuffer;

  if (head == NULL)
    {
    return(FAILURE);
    }

  if (strstr(head,MCredCfgParm[OIndex]) == NULL)
    {
    return(SUCCESS);
    }

  if ((CName == NULL) || (CName[0] == '\0'))
    {
    /* load ALL cred config info */

    ptr = head;
 
    IndexName[0] = '\0';
 
    while (MCfgGetSVal(
             head,
             &ptr,
             MCredCfgParm[OIndex],
             IndexName,
             NULL,
             Value,
             sizeof(Value),
             0,
             NULL) != FAILURE)
      {
      if (MOGetObject(OIndex,IndexName,&C,mAdd) == FAILURE) 
        {
        /* unable to locate credential */

        IndexName[0] = '\0';     
 
        continue; 
        }

      if ((MOGetComponent(C,OIndex,(void *)&CFSPtr,mxoFS) == FAILURE) ||
          (MOGetComponent(C,OIndex,(void *)&CLPtr,mxoLimits) == FAILURE))
        {
        /* cannot get components */

        IndexName[0] = '\0';
 
        continue; 
        }

      MCredProcessConfig(C,OIndex,Value,CLPtr,CFSPtr);

      IndexName[0] = '\0';
      }  /* END while (MCfgGetSVal() != FAILURE) */ 
    }    /* END if ((CName == NULL) || (CName[0] == '\0')) */
  else
    {
    /* load specified cred config info */

    if (MCfgGetSVal(
          head,
          NULL,
          MCredCfgParm[OIndex],         
          CName,
          NULL,
          Value,
          sizeof(Value),
          0,
          NULL) == FAILURE)
      {
      /* cannot locate config info for specified cred */ 

      return(FAILURE);
      }
    
    if (MOGetObject(OIndex,CName,&C,mAdd) == FAILURE)
      {
      /* unable to add user */

      return(FAILURE);
      }

    if ((MOGetComponent(C,OIndex,(void *)&CFSPtr,mxoFS) == FAILURE) ||
        (MOGetComponent(C,OIndex,(void *)&CLPtr,mxoLimits) == FAILURE))
      {
      /* cannot get components */

      IndexName[0] = '\0';

      return(FAILURE);
      }

    MCredProcessConfig(C,OIndex,Value,CLPtr,CFSPtr);
    }  /* END else ((CName == NULL) || (CName[0] == '\0')) */
   
  return(SUCCESS); 
  }  /* END MCredLoadConfig() */




int MCredAdjustConfig(

  int   OIndex,  /* I */
  void *C)       /* I */

  {
  mgcred_t *U;
  mqos_t   *Q;

  if (C == NULL)
    {
    return(FAILURE);
    }

  switch (OIndex)
    {
    case mxoUser:

      U = (mgcred_t *)C;

      if ((U->F.QDef == NULL) || (U->F.QDef == &MQOS[0]))
        {
        if ((MSched.DefaultU != NULL) && 
            (MSched.DefaultU->F.QDef != NULL))
          {
          Q = (mqos_t *)MSched.DefaultU->F.QDef;

          memcpy(U->F.QAL,MSched.DefaultU->F.QAL,sizeof(U->F.QAL));

          if (Q != NULL)
            MUBMSet(Q->Index,U->F.QAL);     
          }
        }

      break;

    default:

      /* NO-OP */

      break;
    }  /* END switch(OIndex) */

  return(SUCCESS);
  }  /* END MCredAdjustConfig() */




int MOGetComponent(

  void      *O,      /* I */
  int        OIndex, /* I */
  void     **C,      /* O */
  int        CIndex) /* I */

  {
  if ((O == NULL) || (C == NULL))
    {
    return(FAILURE);
    }

  if (C != NULL)
    *C = NULL;

  switch(OIndex)
    {
    case mxoUser:

      if (CIndex == mxoLimits) 
        *C = (void *)&((mgcred_t *)O)->L;
      else if (CIndex == mxoFS)
        *C = (void *)&((mgcred_t *)O)->F;  
      else if (CIndex == mxoStats)
        *C = (void *)&((mgcred_t *)O)->Stat;   
 
      break;
 
    case mxoGroup:

      if (CIndex == mxoLimits)
        *C = (void *)&((mgcred_t *)O)->L;
      else if (CIndex == mxoFS)
        *C = (void *)&((mgcred_t *)O)->F;
      else if (CIndex == mxoStats)
        *C = (void *)&((mgcred_t *)O)->Stat;
 
      break;

    case mxoAcct:

      if (CIndex == mxoLimits)
        *C = (void *)&((mgcred_t *)O)->L;
      else if (CIndex == mxoFS)
        *C = (void *)&((mgcred_t *)O)->F;
      else if (CIndex == mxoStats)
        *C = (void *)&((mgcred_t *)O)->Stat;

      break;

    case mxoQOS:

      if (CIndex == mxoLimits)
        *C = (void *)&((mqos_t *)O)->L;
      else if (CIndex == mxoFS)
        *C = (void *)&((mqos_t *)O)->F;
      else if (CIndex == mxoStats)
        *C = (void *)&((mqos_t *)O)->Stat;

      break;

    case mxoClass:

      if (CIndex == mxoLimits)
        *C = (void *)&((mclass_t *)O)->L;
      else if (CIndex == mxoFS)
        *C = (void *)&((mclass_t *)O)->F;
      else if (CIndex == mxoStats)
        *C = (void *)&((mclass_t *)O)->Stat;

      break;

    case mxoPar:

      if (CIndex == mxoLimits)
        *C = (void *)&((mpar_t *)O)->L;
      else if (CIndex == mxoFS)
        *C = (void *)&((mpar_t *)O)->F;
      else if (CIndex == mxoStats)
        *C = (void *)&((mpar_t *)O)->S;

      break;

    case mxoSched:

      if (CIndex == mxoLimits)
        *C = (void *)&((msched_t *)O)->GP->L;
      else if (CIndex == mxoFS)
        *C = (void *)&((msched_t *)O)->GP->F; 
      else if (CIndex == mxoStats)
        *C = (void *)&((msched_t *)O)->GP->S; 

      break;

    case mxoSys:

      /* object is global partition */

      if (CIndex == mxoLimits)
        *C = (void *)&((mpar_t *)O)->L;
      else if (CIndex == mxoFS)
        *C = (void *)&((mpar_t *)O)->F;
      else if (CIndex == mxoStats)
        *C = (void *)&((mpar_t *)O)->S;

      break;

    default:

      /* NO-OP */

      break;
    }  /* END switch(OIndex) */

  if (*C == NULL)
    {
    return(FAILURE);
    }

  return(SUCCESS);
  }  /* END MOGetComponent() */




int CredInitialize(

  int   OIndex,
  void *C,
  char *CName)

  {
  int rc;

  if ((C == NULL) || 
      (CName == NULL) || 
      (CName[0] == '\0'))
    {
    return(FAILURE);
    }

  switch(OIndex)
    {
    case mxoUser:

      rc = MUserInitialize((mgcred_t *)C,CName);

      break;

    case mxoGroup:
 
      rc = MGroupInitialize((mgcred_t *)C,CName);
 
      break;

    case mxoAcct:
 
      rc = MAcctInitialize((mgcred_t *)C,CName);
 
      break;

    case mxoClass:
 
      rc = MClassInitialize((mclass_t *)C,CName);
 
      break;

    case mxoQOS:
 
      rc = MQOSInitialize((mqos_t *)C,CName);
 
      break;

    default:

      rc = FAILURE;

      break;
    }  /* END switch(OIndex); */

  return(rc);
  }  /* END CredInitialize() */




int MOGetObject(
 
  int    OIndex,
  char  *CName,
  void **C,
  int    Mode)
 
  {
  int rc;
 
  if ((C == NULL) || (CName == NULL)) 
    {
    return(FAILURE);
    }

  if (CName[0] == '\0')
    {
    if ((OIndex == mxoUser) || 
        (OIndex == mxoGroup) || 
        (OIndex == mxoAcct) || 
        (OIndex == mxoPar) || 
        (OIndex == mxoClass) || 
        (OIndex == mxoQOS))
      {
      /* object name required */

      return(FAILURE);
      }
    }
 
  switch(OIndex)
    {
    case mxoUser:

      if (Mode == mAdd) 
        rc = MUserAdd(CName,(mgcred_t **)C);
      else
        rc = MUserFind(CName,(mgcred_t **)C);    
 
      break;
 
    case mxoGroup:

      if (Mode == mAdd)
        rc = MGroupAdd(CName,(mgcred_t **)C);          
      else
        rc = MGroupFind(CName,(mgcred_t **)C);         
 
      break;
 
    case mxoAcct:

      if (Mode == mAdd)
        rc = MAcctAdd(CName,(mgcred_t **)C);          
      else
        rc = MAcctFind(CName,(mgcred_t **)C);    
 
      break;
 
    case mxoClass:

      if (Mode == mAdd)
        rc = MClassAdd(CName,(mclass_t **)C);          
      else
        rc = MClassFind(CName,(mclass_t **)C);     
 
      break;
 
    case mxoQOS:

      if (Mode == mAdd)
        rc = MQOSAdd(CName,(mqos_t **)C);          
      else
        rc = MQOSFind(CName,(mqos_t **)C);    
 
      break;

    case mxoPar:

      if (Mode == mAdd)
        rc = MParAdd(CName,(mpar_t **)C);
      else
        rc = MParFind(CName,(mpar_t **)C);

      break;

    case mxoSys:

      *C = (void *)&MPar[0];  

      rc = SUCCESS;

      break;

    case mxoSched:

      *C = (void *)&MSched;

      rc = SUCCESS;

      break;
  
    default:
 
      rc = FAILURE;
 
      break;
    }  /* END switch(OIndex); */
 
  return(rc);
  }  /* END MOGetObject() */





int MCredProcessConfig(

  void     *O,      /* I (modified) */
  int       OIndex, /* I */
  char     *Value,  /* I */
  mcredl_t *L,      /* I */
  mfs_t    *F)      /* I */

  {
  char *ptr;
  char *TokPtr;
  char *tail;

  int   aindex;
  int   index;

  char  ValLine[MAX_MNAME];
  char  ArrayLine[MAX_MNAME];

  if ((Value == NULL) ||
      (Value[0] == '\0') ||
      (L == NULL) ||
      (F == NULL))
    {
    return(FAILURE);
    }

  /* process value line */

  ptr = MUStrTok(Value," \t\n",&TokPtr);

  while(ptr != NULL)
    {
    /* parse name-value pairs */

    /* FORMAT:  <ATTR>=<VALUE>[,<VALUE>] */

    if (MUGetPair(
          ptr,
          (const char **)MCredAttr,
          &aindex,
	  ArrayLine,
          TRUE,
          NULL,
          ValLine,
          MAX_MNAME) == FAILURE)
      {
      if (aindex == -1)
        {
        /* value pair not located in standard cred set */

        /* NOTE:  attempt cred-specific flags */

        switch(OIndex)
          {
          case mxoClass:

            MClassProcessConfig((mclass_t *)O,ptr);

            break;

          case mxoGroup:

            MGroupProcessConfig((mgcred_t *)O,ptr);

            break;

          case mxoQOS:

            MQOSProcessConfig((mqos_t *)O,ptr);

            break;

          default:

            /* NO-OP */

            break;
          }  /* END switch(OIndex) */
        }    /* END if (aindex == -1) */
 
      ptr = MUStrTok(NULL," \t\n",&TokPtr);

      continue;
      }  /* END if (MUGetPair() == FAILURE) */
   
    switch(aindex)
      {
      case mcaADef:

        {
        if ((MAcctFind(ValLine,(mgcred_t **)&F->ADef) == SUCCESS) ||
            (MAcctAdd(ValLine,(mgcred_t **)&F->ADef) == SUCCESS))
          {
          /* add account to AList */

          /* NYI:  set F.AAL */
          }
        }

        break;

      case mcaAList:

        /* NYI */

        break;

      case mcaPriority:

        F->Priority = strtol(ValLine,NULL,0);

        F->IsLocalPriority = TRUE;

        DBG(4,fFS) DPrint("INFO:     cred priority set to %ld\n",
          F->Priority);

        break;

      case mcaMaxJob:

        {
        int tmpIS;
        int tmpIH;

        __MCredParseLimit(
          ValLine,
          &tmpIS,
          &tmpIH);

        if (ArrayLine[0] != '\0')
          {
          int oindex;

          char *OType;
          char *OName;

          char *TokPtr;

          /* add multidimensional policy */

          if (((OType = MUStrTok(ArrayLine,":",&TokPtr)) == NULL) ||
              ((oindex = MUGetIndex(ArrayLine,MXOC,TRUE,mxoNONE)) == mxoNONE))
            {
            break;
            }

          if ((OName = MUStrTok(NULL,":",&TokPtr)) != NULL)
            {
            /* specific policy specified */

            switch(oindex)
              {
              case mxoClass:

                {
                mclass_t *C = NULL;

                if (MClassAdd(OName,&C) == FAILURE)
                  {
                  break;
                  }

                if ((L->APC == NULL) &&
                   ((L->APC = (mpu_t *)calloc(1,sizeof(mpu_t) * MAX_MCLASS)) == NULL))
                  {
                  break;
                  }

                L->APC[C->Index].SLimit[mptMaxJob][0] = tmpIS;
                L->APC[C->Index].HLimit[mptMaxJob][0] = tmpIH;
                }  /* END BLOCK */

                break;

              case mxoQOS:

                {
                mqos_t *Q = NULL;

                if (MQOSAdd(OName,&Q) == FAILURE)
                  {
                  break;
                  }

                if ((L->APQ == NULL) &&
                   ((L->APQ = (mpu_t *)calloc(1,sizeof(mpu_t) * MAX_MQOS)) == NULL))
                  {
                  break;
                  }

                L->APQ[Q->Index].SLimit[mptMaxJob][0] = tmpIS;
                L->APQ[Q->Index].HLimit[mptMaxJob][0] = tmpIH;
                }  /* END BLOCK */

                break;

              case mxoGroup:

                {
                mgcred_t *G = NULL;

                if (MGroupAdd(OName,&G) == FAILURE)
                  {
                  break;
                  }

                if ((L->APG == NULL) &&
                   ((L->APG = (mpu_t *)calloc(1,sizeof(mpu_t) * (MAX_MGROUP + MAX_MHBUF))) == NULL))
                  {
                  break;
                  }

                L->APG[G->Index].SLimit[mptMaxJob][0] = tmpIS;
                L->APG[G->Index].HLimit[mptMaxJob][0] = tmpIH;
                }  /* END BLOCK */

                break;

              default:

                /* NO-OP */

                break;
              }  /* END switch(oindex) */
            }
          else
            {
            switch(oindex)
              {
              case mxoClass:

                /* general policy specified */

                if ((L->APC == NULL) &&
                   ((L->APC = (mpu_t *)calloc(1,sizeof(mpu_t) * MAX_MCLASS)) == NULL))
                  {
                  break;
                  }

                /* NYI */

                break;

              case mxoQOS:

                /* general policy specified */

                if ((L->APQ == NULL) &&
                   ((L->APQ = (mpu_t *)calloc(1,sizeof(mpu_t) * MAX_MQOS)) == NULL))
                  {
                  break;
                  }

                /* NYI */

                break;

              case mxoGroup:

                {
                int gindex;

                /* general policy specified */

                if ((L->APG == NULL) &&
                   ((L->APG = (mpu_t *)calloc(1,sizeof(mpu_t) * (MAX_MGROUP + MAX_MHBUF))) == NULL))
                  {
                  break;
                  }

                for (gindex = 0;gindex < MAX_MGROUP + MAX_MHBUF;gindex++)
                  {
                  L->APG[gindex].SLimit[mptMaxJob][0] = tmpIS;
                  L->APG[gindex].HLimit[mptMaxJob][0] = tmpIH;
                  }  /* END for (gindex) */
                }    /* END BLOCK */

                break;

              case mxoUser:

                {
                int uindex;

                /* general policy specified */

                if ((L->APU == NULL) &&
                   ((L->APU = (mpu_t *)calloc(1,sizeof(mpu_t) * (MAX_MUSER + MAX_MHBUF))) == NULL))
                  {
                  break;
                  }

                for (uindex = 0;uindex < MAX_MUSER + MAX_MHBUF;uindex++)
                  {
                  L->APU[uindex].SLimit[mptMaxJob][0] = tmpIS;
                  L->APU[uindex].HLimit[mptMaxJob][0] = tmpIH;
                  }  /* END for (uindex) */
                }    /* END BLOCK */

                break;

              default:

                /* NO-OP */

                break;
              }  /* END switch(oindex) */
            }    /* END else ((OName = MUStrTok(NULL,":",&TokPtr)) != NULL) */
          }      /* END if (ArrayLine[0] != '\0') */
        else
          {
          L->AP.SLimit[mptMaxJob][0] = tmpIS;
          L->AP.HLimit[mptMaxJob][0] = tmpIH;
          }
        }    /* END BLOCK */

        break;

      case mcaMaxJobPerUser:

        switch (OIndex)
          {
          case mxoClass:
          case mxoQOS:
 
            {
            int uindex;

            if (L->APU == NULL)
              L->APU = (mpu_t *)calloc(1,sizeof(mpu_t) * (MAX_MUSER + MAX_MHBUF));

            __MCredParseLimit(
              ValLine,
              &L->APU[0].SLimit[mptMaxJob][0],
              &L->APU[0].HLimit[mptMaxJob][0]);  

            for (uindex = 1;uindex < MAX_MUSER + MAX_MHBUF;uindex++)
              {
              L->APU[uindex].SLimit[mptMaxJob][0] = L->APU[0].SLimit[mptMaxJob][0];
              L->APU[uindex].HLimit[mptMaxJob][0] = L->APU[0].HLimit[mptMaxJob][0];
              }  /* END for (uindex) */

            break;
            }  /* END BLOCK */
        
          default:

            /* NO-OP */

            break;
          }  /* END switch (OIndex) */

        break;

      case mcaMaxNode:

        __MCredParseLimit(
          ValLine,
          &L->AP.SLimit[mptMaxNode][0],
          &L->AP.HLimit[mptMaxNode][0]);     
 
        break;

      case mcaMaxNodePerUser:
 
        switch (OIndex)
          {
          case mxoClass:
          case mxoQOS:
 
            {
            int uindex;
 
            if (L->APU == NULL)
              L->APU = (mpu_t *)calloc(1,sizeof(mpu_t) * (MAX_MUSER + MAX_MHBUF));
 
            __MCredParseLimit(
              ValLine,
              &L->APU[0].SLimit[mptMaxNode][0],
              &L->APU[0].HLimit[mptMaxNode][0]);
 
            for (uindex = 1;uindex < MAX_MUSER + MAX_MHBUF;uindex++)
              {
              L->APU[uindex].SLimit[mptMaxNode][0] = 
                L->APU[0].SLimit[mptMaxNode][0];

              L->APU[uindex].HLimit[mptMaxNode][0] = 
                L->APU[0].HLimit[mptMaxNode][0];
              }  /* END for (uindex) */
 
            break;
            }  /* END BLOCK */
 
          default:

            /* NO-OP */
 
            break;
          }  /* END switch(Oindex) */
 
        break;

      case mcaMaxPE:

        __MCredParseLimit(
          ValLine,
          &L->AP.SLimit[mptMaxPE][0],
          &L->AP.HLimit[mptMaxPE][0]);     

        break;

      case mcaMaxProc:

        {
        int tmpIS;
        int tmpIH;

        __MCredParseLimit(
          ValLine,
          &tmpIS,
          &tmpIH);

        if (ArrayLine[0] != '\0')
          {
          int oindex;
          
          char *OType;
          char *OName;

          char *TokPtr;

          /* add multidimensional policy */

          if (((OType = MUStrTok(ArrayLine,":",&TokPtr)) == NULL) ||
              ((oindex = MUGetIndex(ArrayLine,MXOC,TRUE,mxoNONE)) == mxoNONE))
            {
            break;
            }

          if ((OName = MUStrTok(NULL,":",&TokPtr)) != NULL)
            {
            /* specific policy specified */

            switch(oindex)
              {
              case mxoClass:
 
                {
                mclass_t *C = NULL;

                if (MClassAdd(OName,&C) == FAILURE)
                  {
                  break;
                  }

                if ((L->APC == NULL) &&
                   ((L->APC = (mpu_t *)calloc(1,sizeof(mpu_t) * MAX_MCLASS)) == NULL))
                  {
                  break;
                  }

                L->APC[C->Index].SLimit[mptMaxProc][0] = tmpIS;
                L->APC[C->Index].HLimit[mptMaxProc][0] = tmpIH;
                }  /* END BLOCK */

                break;

              case mxoQOS:

                {
                mqos_t *Q = NULL;

                if (MQOSAdd(OName,&Q) == FAILURE)
                  {
                  break;
                  }

                if ((L->APQ == NULL) &&
                   ((L->APQ = (mpu_t *)calloc(1,sizeof(mpu_t) * MAX_MQOS)) == NULL))
                  {
                  break;
                  }

                L->APQ[Q->Index].SLimit[mptMaxProc][0] = tmpIS;
                L->APQ[Q->Index].HLimit[mptMaxProc][0] = tmpIH;
                }  /* END BLOCK */

                break;

              case mxoGroup:

                {
                mgcred_t *G = NULL;

                if (MGroupAdd(OName,&G) == FAILURE)
                  {
                  break;
                  }

                if ((L->APG == NULL) &&
                   ((L->APG = (mpu_t *)calloc(1,sizeof(mpu_t) * (MAX_MGROUP + MAX_MHBUF))) == NULL))
                  {
                  break;
                  }

                L->APG[G->Index].SLimit[mptMaxProc][0] = tmpIS;
                L->APG[G->Index].HLimit[mptMaxProc][0] = tmpIH;
                }  /* END BLOCK */

                break;

              default:

                /* NO-OP */

                break;
              }  /* END switch(oindex) */
            }
          else
            {
            switch(oindex)
              {
              case mxoClass:

                /* general policy specified */

                if ((L->APC == NULL) &&
                   ((L->APC = (mpu_t *)calloc(1,sizeof(mpu_t) * MAX_MCLASS)) == NULL))
                  {
                  break;
                  }

                /* NYI */

                break;

              case mxoQOS:

                /* general policy specified */

                if ((L->APQ == NULL) &&
                   ((L->APQ = (mpu_t *)calloc(1,sizeof(mpu_t) * MAX_MQOS)) == NULL))
                  {
                  break;
                  }

                /* NYI */

                break;

              case mxoGroup:

                /* general policy specified */

                if ((L->APG == NULL) &&
                   ((L->APG = (mpu_t *)calloc(1,sizeof(mpu_t) * (MAX_MGROUP + MAX_MHBUF))) == NULL))
                  {
                  break;
                  }

                /* NYI */

                break;

              default:

                /* NO-OP */

                break;
              }  /* END switch(oindex) */
            }    /* END else ((OName = MUStrTok(NULL,":",&TokPtr)) != NULL) */
          }      /* END if (ArrayLine[0] != '\0') */
        else
          {
          L->AP.SLimit[mptMaxProc][0] = tmpIS;
          L->AP.HLimit[mptMaxProc][0] = tmpIH;
          }
        }    /* END BLOCK */

        break;

      case mcaMinProc:
 
        __MCredParseLimit(
          ValLine,
          &L->AP.SLimit[mptMinProc][0],
          &L->AP.HLimit[mptMinProc][0]);
 
        break;

      case mcaMaxProcPerUser:
 
        switch (OIndex)
          {
          case mxoClass:
          case mxoQOS:
  
            {
            int uindex;
 
            if (L->APU == NULL)
              L->APU = (mpu_t *)calloc(1,sizeof(mpu_t) * (MAX_MUSER + MAX_MHBUF));
 
            __MCredParseLimit(
              ValLine,
              &L->APU[0].SLimit[mptMaxProc][0],
              &L->APU[0].HLimit[mptMaxProc][0]);
 
            for (uindex = 1;uindex < MAX_MUSER + MAX_MHBUF;uindex++)
              {
              L->APU[uindex].SLimit[mptMaxProc][0] = L->APU[0].SLimit[mptMaxProc][0];
              L->APU[uindex].HLimit[mptMaxProc][0] = L->APU[0].HLimit[mptMaxProc][0];
              }  /* END for (uindex) */
 
            break;
            }  /* END BLOCK */
 
          default:

            /* NO-OP */
 
            break;
          }  /* END switch(OIndex) */
 
        break;

      case mcaMaxPS:

        __MCredParseLimit(
          ValLine,
          &L->AP.SLimit[mptMaxPS][0],
          &L->AP.HLimit[mptMaxPS][0]);     
 
        break;

      case mcaMaxWC:

        __MCredParseLimit(
          ValLine,
          &L->AP.SLimit[mptMaxWC][0],
          &L->AP.HLimit[mptMaxWC][0]);     
 
        break;

      case mcaMaxMem:

        __MCredParseLimit(
          ValLine,
          &L->AP.SLimit[mptMaxMem][0],
          &L->AP.HLimit[mptMaxMem][0]);     
 
        break;

      case mcaMaxIJob:

        if (L->IP == NULL)
          { 
	  L->IP = (mpu_t *)calloc(1,sizeof(mpu_t));
          }

        if (L->IP != NULL)
          {
          __MCredParseLimit(
            ValLine,
            &L->IP->SLimit[mptMaxJob][0],
            &L->IP->HLimit[mptMaxJob][0]);
          }
 
        break;
 
      case mcaMaxINode:

        if (L->IP == NULL)
          {
          L->IP = (mpu_t *)calloc(1,sizeof(mpu_t));
          }

        if (L->IP != NULL)       
          {
          __MCredParseLimit(
            ValLine,
            &L->IP->SLimit[mptMaxNode][0],
            &L->IP->HLimit[mptMaxNode][0]);
          }
 
        break;
 
      case mcaMaxIPE:

        if (L->IP == NULL)
          {
          L->IP = (mpu_t *)calloc(1,sizeof(mpu_t));
          }
 
	if (L->IP != NULL)
          {
          __MCredParseLimit(
            ValLine,
            &L->IP->SLimit[mptMaxPE][0],
            &L->IP->HLimit[mptMaxPE][0]);
          }

        break;
 
      case mcaMaxIProc:

        if (L->IP == NULL)
          {
          L->IP = (mpu_t *)calloc(1,sizeof(mpu_t));
          }

        if (L->IP != NULL)
          {
          __MCredParseLimit(
            ValLine,
            &L->IP->SLimit[mptMaxProc][0],
            &L->IP->HLimit[mptMaxProc][0]);
          }

        break;
 
      case mcaMaxIPS:

        if (L->IP == NULL)
          {
          L->IP = (mpu_t *)calloc(1,sizeof(mpu_t));
          }

        if (L->IP != NULL)
          {
          __MCredParseLimit(
            ValLine,
            &L->IP->SLimit[mptMaxPS][0],
            &L->IP->HLimit[mptMaxPS][0]);
          }

        break;
 
      case mcaMaxIWC:

        if (L->IP == NULL)
          {
          L->IP = (mpu_t *)calloc(1,sizeof(mpu_t));
          }

        if (L->IP != NULL)
          {
          __MCredParseLimit(
            ValLine,
            &L->IP->SLimit[mptMaxWC][0],
            &L->IP->HLimit[mptMaxWC][0]);
          }

        break;
 
      case mcaMaxIMem:

        if (L->IP == NULL)
          {
          L->IP = (mpu_t *)calloc(1,sizeof(mpu_t));
          }

        if (L->IP != NULL)
          {
          __MCredParseLimit(
            ValLine,
            &L->IP->SLimit[mptMaxMem][0],
            &L->IP->HLimit[mptMaxMem][0]);
          }

        break;

      case mcaOMaxJob:
 
        if (L->OAP == NULL)
          break;

        __MCredParseLimit(
          ValLine,
          &L->OAP->SLimit[mptMaxJob][0],
          &L->OAP->HLimit[mptMaxJob][0]);
 
        break;
 
      case mcaOMaxNode:
 
        if (L->OAP == NULL)
          break;

        __MCredParseLimit(
          ValLine,
          &L->OAP->SLimit[mptMaxNode][0],
          &L->OAP->HLimit[mptMaxNode][0]);
 
        break;
 
      case mcaOMaxPE:
 
        if (L->OAP == NULL)
          break;

        __MCredParseLimit(
          ValLine,
          &L->OAP->SLimit[mptMaxPE][0],
          &L->OAP->HLimit[mptMaxPE][0]);
 
        break;
 
      case mcaOMaxProc:
 
        if (L->OAP == NULL)
          break;

        __MCredParseLimit(
          ValLine,
          &L->OAP->SLimit[mptMaxProc][0],
          &L->OAP->HLimit[mptMaxProc][0]);
 
        break;
 
      case mcaOMaxPS:
 
        if (L->OAP != NULL)
          __MCredParseLimit(ValLine,&L->OAP->SLimit[mptMaxPS][0],&L->OAP->HLimit[mptMaxPS][0]);
 
        break;
 
      case mcaOMaxWC:
 
        if (L->OAP != NULL)
          __MCredParseLimit(ValLine,&L->OAP->SLimit[mptMaxWC][0],&L->OAP->HLimit[mptMaxWC][0]);
 
        break;
 
      case mcaOMaxMem:
 
        if (L->OAP != NULL)
          __MCredParseLimit(ValLine,&L->OAP->SLimit[mptMaxMem][0],&L->OAP->HLimit[mptMaxMem][0]);
 
        break;

      case mcaOMaxIJob:
 
        if (L->OIP != NULL)
          __MCredParseLimit(ValLine,&L->OIP->SLimit[mptMaxJob][0],&L->OIP->HLimit[mptMaxJob][0]);
 
        break;
 
      case mcaOMaxINode:
 
        if (L->OIP != NULL)
          __MCredParseLimit(ValLine,&L->OIP->SLimit[mptMaxNode][0],&L->OIP->HLimit[mptMaxNode][0]);
 
        break;
 
      case mcaOMaxIPE:
 
        if (L->OIP == NULL)
          break;

        __MCredParseLimit(
          ValLine,
          &L->OIP->SLimit[mptMaxPE][0],
          &L->OIP->HLimit[mptMaxPE][0]);
 
        break;
 
      case mcaOMaxIProc:
 
        if (L->OIP == NULL)
          break;

        __MCredParseLimit(
          ValLine,
          &L->OIP->SLimit[mptMaxProc][0],
          &L->OIP->HLimit[mptMaxProc][0]);
 
        break;
 
      case mcaOMaxIPS:
 
        if (L->OIP != NULL)
          __MCredParseLimit(ValLine,&L->OIP->SLimit[mptMaxPS][0],&L->OIP->HLimit[mptMaxPS][0]);
 
        break;
 
      case mcaOMaxIWC:
 
        if (L->OIP != NULL)
          __MCredParseLimit(ValLine,&L->OIP->SLimit[mptMaxWC][0],&L->OIP->HLimit[mptMaxWC][0]);
 
        break;
 
      case mcaOMaxIMem:
 
        if (L->OIP != NULL)
          __MCredParseLimit(ValLine,&L->OIP->SLimit[mptMaxMem][0],&L->OIP->HLimit[mptMaxMem][0]);
 
        break;

      case mcaOMaxJNode:
 
        if (L->OJP != NULL)
          __MCredParseLimit(ValLine,&L->OJP->SLimit[mptMaxNode][0],&L->OJP->HLimit[mptMaxNode][0]);
 
        break;
 
      case mcaOMaxJPE:
 
        if (L->OJP != NULL)
          __MCredParseLimit(ValLine,&L->OJP->SLimit[mptMaxPE][0],&L->OJP->HLimit[mptMaxPE][0]);
 
        break;
 
      case mcaOMaxJProc:
 
        if (L->OJP != NULL)
          __MCredParseLimit(ValLine,&L->OJP->SLimit[mptMaxProc][0],&L->OJP->HLimit[mptMaxProc][0]);
 
        break;
 
      case mcaOMaxJPS:
 
        if (L->OJP != NULL)
          __MCredParseLimit(ValLine,&L->OJP->SLimit[mptMaxPS][0],&L->OJP->HLimit[mptMaxPS][0]);
 
        break;
 
      case mcaOMaxJWC:
 
        if (L->OJP != NULL)
          __MCredParseLimit(ValLine,&L->OJP->SLimit[mptMaxWC][0],&L->OJP->HLimit[mptMaxWC][0]);
 
        break;
 
      case mcaOMaxJMem:
 
        if (L->OJP != NULL)
          __MCredParseLimit(ValLine,&L->OJP->SLimit[mptMaxMem][0],&L->OJP->HLimit[mptMaxMem][0]);
 
        break;

      case mcaFSTarget:

        MFSTargetFromString(F,ValLine);

        break;

      case mcaQDef:

        if ((tail = strchr(ValLine,'&')) != NULL)
          F->QALType = qalAND;
        else if ((tail = strchr(ValLine,'^')) != NULL)
          F->QALType = qalONLY;
        else
          F->QALType = qalOR;
       
        if (tail != NULL)
          *tail = '\0';
 
        if (MQOSFind(ValLine,(mqos_t **)&F->QDef) == FAILURE)
          {
          if (F->QDef != NULL)
            {
            /* empty slot located */

            MQOSAdd(ValLine,(mqos_t **)&F->QDef);

            MQOSInitialize((mqos_t *)F->QDef,ValLine);
            }
          else
            {
            F->QDef = (void *)&MQOS[MDEF_SYSQDEF];
            }
          }

        MUBMSet(((mqos_t *)F->QDef)->Index,F->QAL);  

        break;

      case mcaQList:

        if (strchr(ValLine,'&') != NULL)
          F->QALType = qalAND;
        else if (strchr(ValLine,'^') != NULL)
          F->QALType = qalONLY;
        else
          F->QALType = qalOR;
 
        if (MQOSListBMFromString(ValLine,F->QAL,mAdd) == FAILURE)
          {
          DBG(5,fFS) DPrint("ALERT:    cannot parse string '%s' as rangelist\n",
            ValLine);
          }
 
        DBG(2,fFS) DPrint("INFO:     QOS access list set to %s\n",
          MQOSBMToString(F->QAL));

        if (F->QDef != NULL)
          MUBMSet(((mqos_t *)F->QDef)->Index,F->QAL);   

        break;

      case mcaPDef:

        /* default partition specification */

        if ((tail = strchr(ValLine,'&')) != NULL)
          F->PALType = qalAND;
        else if ((tail = strchr(ValLine,'^')) != NULL)
          F->PALType = qalONLY;
        else
          F->PALType = qalOR;
 
        if (tail != NULL)
          *tail = '\0';
 
        if (MParFind(ValLine,(mpar_t  **)&F->PDef) == FAILURE)
          {
          if (F->PDef != NULL)
            {
            /* empty slot located */
 
            if (MParAdd(ValLine,(mpar_t **)&F->PDef) == SUCCESS)
              { 
              MParInitialize((mpar_t *)F->PDef,ValLine);
              }
            }
          else
            {
            F->PDef = (void *)&MPar[MDEF_SYSPDEF];
            }
          }
 
        MUBMSet(((mpar_t  *)F->PDef)->Index,F->PAL);
 
        DBG(4,fFS) DPrint("INFO:     default partition set to %s\n",
          ((mpar_t  *)F->PDef)->Name);

        break;

      case mcaPList:

        /* partition access list */

        if (strchr(ValLine,'&') != NULL)
          F->PALType = qalAND;
        else if (strchr(ValLine,'^') != NULL)
          F->PALType = qalONLY;
        else
          F->PALType = qalOR;
 
        if (MParListBMFromString(ValLine,F->PAL,mAdd) == FAILURE)
          {
          DBG(5,fFS) DPrint("ALERT:    cannot parse string '%s' as rangelist\n",
            ValLine);
          }
 
        DBG(2,fFS) DPrint("INFO:     partition access list set to %s\n",
          MParBMToString(F->PAL));
 
        if ((F->PDef != NULL) && (ValLine[0] != '\0'))
          {
          MUBMSet(((mpar_t *)F->PDef)->Index,F->PAL);
          }
 
        break;

      case mcaJobFlags:

        /* cred flag specification */
 
        for (index = 0;MJobFlags[index] != NULL;index++)
          {
          if (strstr(ValLine,MJobFlags[index]) != NULL)
            {
            F->JobFlags |= (1 << index);

            DBG(4,fFS) DPrint("INFO:     cred flag set to %s\n",
              MJobFlags[index]);
            }
          }    /* END for (index) */

        break;   

      case mcaOverrun:

        /* cred job overrun */

        F->Overrun = MUTimeFromString(ValLine);

        break;

      case mcaMaxWCLimit:
      case mcaDefWCLimit:
      case mcaMaxNodePerJob:
      case mcaMaxProcPerJob:

        MCredSetAttr(O,OIndex,aindex,(void **)ValLine,mdfString,mSet);

        break;

      default:

        DBG(4,fFS) DPrint("WARNING:  cred attribute '%s' not handled\n",
          MCredAttr[aindex]);

        break;
      }  /* END switch(aindex) */

    ptr = MUStrTok(NULL," \t\n",&TokPtr);       
    }  /* END while (ptr != NULL) */

  return(SUCCESS);
  }  /* END MCredProcessConfig() */




int __MCredParseLimit(

  char *ValLine,  /* I */
  int  *SLimit,   /* I */
  int  *HLimit)   /* I */

  {
  char *tail;

  *SLimit = (int)strtol(ValLine,&tail,0);
 
  if (*tail == ',')
    *HLimit = (int)strtol(tail + 1,NULL,0);
  else
    *HLimit = *SLimit;

  return(SUCCESS);
  }  /* END __MCredParseLimit() */




char *MCredShowAttrs(

  mpu_t *AP,
  mpu_t *IP,
  mpu_t *OAP,
  mpu_t *OIP,
  mpu_t *OJP,
  mfs_t *FS,
  long   Priority,
  long   Mode)

  {
  char *BPtr;
  int   BSpace;

  static char Line[MAX_MLINE];

  char *ptr;

  int pindex;

  int ShowUsage;

  if (Mode & (1 << mcsUsage))
    ShowUsage = TRUE;
  else
    ShowUsage = FALSE;

  MUSNInit(&BPtr,&BSpace,Line,sizeof(Line));

  for (pindex = mcaPriority;pindex <= mcaJobFlags;pindex++)
    {
    switch(pindex)
      {
      case mcaPriority:

        if (Priority != 0)
          {
          MUSNPrintF(&BPtr,&BSpace," %s=%ld",
            MCredAttr[pindex],
            Priority);
          }

        break;

      case mcaMaxJob:

        if ((Mode & (1 << mcsLimits)) &&
           ((ptr = __MCredShowLimit(AP,mptMaxJob,0,ShowUsage)) != NULL))
          {
          MUSNPrintF(&BPtr,&BSpace," %s=%s",
            MCredAttr[pindex],
            ptr);
          }

        break;

      case mcaMaxNode:

        if ((Mode & (1 << mcsLimits)) &&
           ((ptr = __MCredShowLimit(AP,mptMaxNode,0,ShowUsage)) != NULL))
          {
          MUSNPrintF(&BPtr,&BSpace," %s=%s",
            MCredAttr[pindex],
            ptr);
          }

        break;

      case mcaMaxPE:

        if ((Mode & (1 << mcsLimits)) &&
           ((ptr = __MCredShowLimit(AP,mptMaxPE,0,ShowUsage)) != NULL))
          {
          MUSNPrintF(&BPtr,&BSpace," %s=%s",
            MCredAttr[pindex],
            ptr);
          }

        break;

      case mcaMaxProc:

        if ((Mode & (1 << mcsLimits)) &&
           ((ptr = __MCredShowLimit(AP,mptMaxProc,0,ShowUsage)) != NULL))
          {
          MUSNPrintF(&BPtr,&BSpace," %s=%s",
            MCredAttr[pindex],
            ptr);
          }

        break;

      case mcaMaxPS:

        if ((Mode & (1 << mcsLimits)) &&
           ((ptr = __MCredShowLimit(AP,mptMaxPS,0,ShowUsage)) != NULL))
          {
          MUSNPrintF(&BPtr,&BSpace," %s=%s",
            MCredAttr[pindex],
            ptr);
          }

        break;

      case mcaMaxWC:

        if ((Mode & (1 << mcsLimits)) &&
           ((ptr = __MCredShowLimit(AP,mptMaxWC,0,ShowUsage)) != NULL))
          {
          MUSNPrintF(&BPtr,&BSpace," %s=%s",
            MCredAttr[pindex],
            ptr);
          }

        break;

      case mcaMaxMem:

        if ((Mode & (1 << mcsLimits)) &&
           ((ptr = __MCredShowLimit(AP,mptMaxMem,0,ShowUsage)) != NULL))
          {
          MUSNPrintF(&BPtr,&BSpace," %s=%s",
            MCredAttr[pindex],
            ptr);
          }

        break;

      case mcaMaxIJob:

        if ((Mode & (1 << mcsLimits)) &&
           ((ptr = __MCredShowLimit(IP,mptMaxJob,0,ShowUsage)) != NULL))
          {
          MUSNPrintF(&BPtr,&BSpace," %s=%s",
            MCredAttr[pindex],
            ptr);
          }

        break;

      case mcaMaxINode:

        if ((Mode & (1 << mcsLimits)) &&
           ((ptr = __MCredShowLimit(IP,mptMaxNode,0,ShowUsage)) != NULL))
          {
          MUSNPrintF(&BPtr,&BSpace," %s=%s",
            MCredAttr[pindex],
            ptr);
          }

        break;

      case mcaMaxIPE:

        if ((Mode & (1 << mcsLimits)) &&
           ((ptr = __MCredShowLimit(IP,mptMaxPE,0,ShowUsage)) != NULL))
          {
          MUSNPrintF(&BPtr,&BSpace," %s=%s",
            MCredAttr[pindex],
            ptr);
          }

        break;

      case mcaMaxIProc:

        if ((Mode & (1 << mcsLimits)) &&
           ((ptr = __MCredShowLimit(IP,mptMaxProc,0,ShowUsage)) != NULL))
          {
          MUSNPrintF(&BPtr,&BSpace," %s=%s",
            MCredAttr[pindex],
            ptr);
          }

        break;

      case mcaMaxIPS:

        if ((Mode & (1 << mcsLimits)) &&
           ((ptr = __MCredShowLimit(IP,mptMaxPS,0,ShowUsage)) != NULL))
          {
          MUSNPrintF(&BPtr,&BSpace," %s=%s",
            MCredAttr[pindex],
            ptr);
          }

        break;

      case mcaMaxIWC:

        if ((Mode & (1 << mcsLimits)) &&
           ((ptr = __MCredShowLimit(IP,mptMaxWC,0,ShowUsage)) != NULL))
          {
          MUSNPrintF(&BPtr,&BSpace," %s=%s",
            MCredAttr[pindex],
            ptr);
          }

        break;

      case mcaMaxIMem:

        if ((Mode & (1 << mcsLimits)) &&
           ((ptr = __MCredShowLimit(IP,mptMaxMem,0,ShowUsage)) != NULL))
          {
          MUSNPrintF(&BPtr,&BSpace," %s=%s",
            MCredAttr[pindex],
            ptr);
          }

        break;

      case mcaOMaxJob:

        if ((Mode & (1 << mcsLimits)) &&
           ((ptr = __MCredShowLimit(OAP,mptMaxJob,0,ShowUsage)) != NULL))
          {
          MUSNPrintF(&BPtr,&BSpace," %s=%s",
            MCredAttr[pindex],
            ptr);
          }

        break;

      case mcaOMaxNode:

        if ((Mode & (1 << mcsLimits)) &&
           ((ptr = __MCredShowLimit(OAP,mptMaxNode,0,ShowUsage)) != NULL))
          {
          MUSNPrintF(&BPtr,&BSpace," %s=%s",
            MCredAttr[pindex],
            ptr);
          }

        break;

      case mcaOMaxPE:

        if ((Mode & (1 << mcsLimits)) &&
           ((ptr = __MCredShowLimit(OAP,mptMaxPE,0,ShowUsage)) != NULL))
          {
          MUSNPrintF(&BPtr,&BSpace," %s=%s",
            MCredAttr[pindex],
            ptr);
          }

        break;

      case mcaOMaxProc:

        if ((Mode & (1 << mcsLimits)) &&
           ((ptr = __MCredShowLimit(OAP,mptMaxProc,0,ShowUsage)) != NULL))
          {
          MUSNPrintF(&BPtr,&BSpace," %s=%s",
            MCredAttr[pindex],
            ptr);
          }

        break;

      case mcaOMaxPS:

        if ((Mode & (1 << mcsLimits)) &&
           ((ptr = __MCredShowLimit(OAP,mptMaxPS,0,ShowUsage)) != NULL))
          {
          MUSNPrintF(&BPtr,&BSpace," %s=%s",
            MCredAttr[pindex],
            ptr);
          }

        break;

      case mcaOMaxWC:

        if ((Mode & (1 << mcsLimits)) &&
           ((ptr = __MCredShowLimit(OAP,mptMaxWC,0,ShowUsage)) != NULL))
          {
          MUSNPrintF(&BPtr,&BSpace," %s=%s",
            MCredAttr[pindex],
            ptr);
          }

        break;

      case mcaOMaxMem:

        if ((Mode & (1 << mcsLimits)) &&
           ((ptr = __MCredShowLimit(OAP,mptMaxMem,0,ShowUsage)) != NULL))
          {
          MUSNPrintF(&BPtr,&BSpace," %s=%s",
            MCredAttr[pindex],
            ptr);
          }

        break;

      case mcaOMaxIJob:

        if ((Mode & (1 << mcsLimits)) &&
           ((ptr = __MCredShowLimit(OIP,mptMaxJob,0,ShowUsage)) != NULL))
          {
          MUSNPrintF(&BPtr,&BSpace," %s=%s",
            MCredAttr[pindex],
            ptr);
          }

        break;

      case mcaOMaxINode:

        if ((Mode & (1 << mcsLimits)) &&
           ((ptr = __MCredShowLimit(OIP,mptMaxNode,0,ShowUsage)) != NULL))
          {
          MUSNPrintF(&BPtr,&BSpace," %s=%s",
            MCredAttr[pindex],
            ptr);
          }

        break;

      case mcaOMaxIPE:

        if ((Mode & (1 << mcsLimits)) &&
           ((ptr = __MCredShowLimit(OIP,mptMaxPE,0,ShowUsage)) != NULL))
          {
          MUSNPrintF(&BPtr,&BSpace," %s=%s",
            MCredAttr[pindex],
            ptr);
          }

        break;

      case mcaOMaxIProc:

        if ((Mode & (1 << mcsLimits)) &&
           ((ptr = __MCredShowLimit(OIP,mptMaxProc,0,ShowUsage)) != NULL))
          {
          MUSNPrintF(&BPtr,&BSpace," %s=%s",
            MCredAttr[pindex],
            ptr);
          }

        break;

      case mcaOMaxIPS:

        if ((Mode & (1 << mcsLimits)) &&
           ((ptr = __MCredShowLimit(OIP,mptMaxPS,0,ShowUsage)) != NULL))
          {
          MUSNPrintF(&BPtr,&BSpace," %s=%s",
            MCredAttr[pindex],
            ptr);
          }

        break;

      case mcaOMaxIWC:

        if ((Mode & (1 << mcsLimits)) &&
           ((ptr = __MCredShowLimit(OIP,mptMaxWC,0,ShowUsage)) != NULL))
          {
          MUSNPrintF(&BPtr,&BSpace," %s=%s",
            MCredAttr[pindex],
            ptr);
          }

        break;

      case mcaOMaxIMem:

        if ((Mode & (1 << mcsLimits)) &&
           ((ptr = __MCredShowLimit(OIP,mptMaxMem,0,ShowUsage)) != NULL))
          {
          MUSNPrintF(&BPtr,&BSpace," %s=%s",
            MCredAttr[pindex],
            ptr);
          }

        break;

      case mcaOMaxJNode:

        if ((Mode & (1 << mcsLimits)) &&
           ((ptr = __MCredShowLimit(OJP,mptMaxNode,0,ShowUsage)) != NULL))
          {
          MUSNPrintF(&BPtr,&BSpace," %s=%s",
            MCredAttr[pindex],
            ptr);
          }

        break;

      case mcaOMaxJPE:

        if ((Mode & (1 << mcsLimits)) &&
           ((ptr = __MCredShowLimit(OJP,mptMaxPE,0,ShowUsage)) != NULL))
          {
          MUSNPrintF(&BPtr,&BSpace," %s=%s",
            MCredAttr[pindex],
            ptr);
          }

        break;

      case mcaOMaxJProc:

        if ((Mode & (1 << mcsLimits)) &&
           ((ptr = __MCredShowLimit(OJP,mptMaxProc,0,ShowUsage)) != NULL))
          {
          MUSNPrintF(&BPtr,&BSpace," %s=%s",
            MCredAttr[pindex],
            ptr);
          }

        break;

      case mcaOMaxJPS:

        if ((Mode & (1 << mcsLimits)) &&
           ((ptr = __MCredShowLimit(OJP,mptMaxPS,0,ShowUsage)) != NULL))
          {
          MUSNPrintF(&BPtr,&BSpace," %s=%s",
            MCredAttr[pindex],
            ptr);
          }

        break;

      case mcaOMaxJWC:

        if ((Mode & (1 << mcsLimits)) &&
           ((ptr = __MCredShowLimit(OJP,mptMaxWC,0,ShowUsage)) != NULL))
          {
          MUSNPrintF(&BPtr,&BSpace," %s=%s",
            MCredAttr[pindex],
            ptr);
          }

        break;

      case mcaOMaxJMem:

        if ((Mode & (1 << mcsLimits)) &&
           ((ptr = __MCredShowLimit(OJP,mptMaxMem,0,ShowUsage)) != NULL))
          {
          MUSNPrintF(&BPtr,&BSpace," %s=%s",
            MCredAttr[pindex],
            ptr);
          }

        break;

      case mcaFSTarget:

        /* NYI */

        break;

      case mcaQList:

        break;

      case mcaQDef:

        break;

      case mcaPList:

        break;

      case mcaPDef:

        break;

      case mcaJobFlags:

        /* NO-OP */

        break;

      default:

        /* NO-OP */

        break;
      }  /* END switch(pindex) */
    }    /* END for (pindex) */

  return(Line);
  }  /* END MCredShowAttrs() */




char *__MCredShowLimit(

  mpu_t *P,
  int    PlIndex,
  int    PtIndex,
  int    ShowUsage)

  {
  static char Line[MAX_MNAME];

  if (P == NULL)
    {
    return(NULL);
    }

  if ((P->HLimit[PlIndex][PtIndex] == 0) &&
      (P->SLimit[PlIndex][PtIndex] == 0))
    {
    return(NULL);
    }

  if (ShowUsage == TRUE)
    {
    sprintf(Line,"%d:",
      P->Usage[PlIndex][PtIndex]);
    }
  else
    {
    Line[0] = '\0';
    }

  if ((P->SLimit[PlIndex][PtIndex] == 0) || 
      (P->SLimit[PlIndex][PtIndex] == P->HLimit[PlIndex][PtIndex]))
    {
    sprintf(Line,"%s%d",
      Line,
      P->HLimit[PlIndex][PtIndex]);
    }
  else
    {
    sprintf(Line,"%s%d,%d",
      Line,
      P->SLimit[PlIndex][PtIndex],
      P->HLimit[PlIndex][PtIndex]);
    }

  return(Line);
  }  /* END __MCredShowLimit() */




int MCredSetDefaults()

  {
  const char *FName = "MCredSetDefaults";

  DBG(4,fRM) DPrint("%s()\n",
    FName);

  memset(MUser,0,sizeof(MUser));
  memset(MGroup,0,sizeof(MGroup));

  /* initialize default credentials */
 
  MQOSInitialize(&MQOS[0],"DEFAULT");
  MQOSInitialize(&MQOS[1],ALL);
 
  MUserAdd("DEFAULT",&MSched.DefaultU);
 
  if (MGroupAdd("NOGROUP",NULL) == FAILURE)
    {
    DBG(1,fRM) DPrint("ERROR:    cannot add default group\n");
    }
 
  MGroupAdd("DEFAULT",&MSched.DefaultG);
 
  MSched.DefaultG->L.IP = (mpu_t *)calloc(1,sizeof(mpu_t));
 
  MAcctAdd("DEFAULT",&MSched.DefaultA);
 
  MSched.DefaultA->L.IP = (mpu_t *)calloc(1,sizeof(mpu_t));
 
  MQOSAdd("DEFAULT",&MSched.DefaultQ);
 
  MSched.DefaultQ->L.IP = (mpu_t *)calloc(1,sizeof(mpu_t));
 
  MClassAdd(NONE,NULL);
  MClassAdd(ALL,NULL);

  if ((MSched.DefaultC = (mclass_t *)calloc(1,sizeof(mclass_t))) != NULL)
    {
    strcpy(MSched.DefaultC->Name,"DEFAULT");

    MSched.DefaultC->L.IP = (mpu_t *)calloc(1,sizeof(mpu_t));
    }

  return(SUCCESS);
  }  /* END MCredSetDefaults() */




int MCredSetAttr(

  void     *O,      /* I (modified) */
  int       OIndex, /* I */
  int       AIndex, /* I */
  void    **Value,  /* I */
  int       Format, /* I */
  int       Mode)   /* I (mSet/mAdd/mClear) */
 
  {
  mcredl_t *L;
  must_t   *S;

  if ((O == NULL) || (Value == NULL))
    {
    return(FAILURE);
    }

  if (MOGetComponent(O,OIndex,(void *)&S,mxoStats) == FAILURE)
    {
    /* cannot get components */

    return(FAILURE);
    }

  if (MOGetComponent(O,OIndex,(void *)&L,mxoLimits) == FAILURE)
    {
    /* cannot get components */

    return(FAILURE);
    }

  switch(AIndex)
    {
    case mcaDefWCLimit:

      {
      long tmpL;

      mjob_t *J;

      if (L->JDef == NULL)
        L->JDef = (void *)calloc(1,sizeof(mjob_t));

      J = (mjob_t *)L->JDef;

      if (Format == mdfString)
        tmpL = MUTimeFromString((char *)Value);
      else
        tmpL = *(long *)Value;

      J->SpecWCLimit[0] = tmpL;
      }  /* END BLOCK */

      break;

    case mcaID:
  
      switch(OIndex)
        {
        case mxoUser:
 
          strcpy(((mgcred_t *)O)->Name,(char *)Value);
 
          break;
 
        case mxoGroup:
 
          strcpy(((mgcred_t *)O)->Name,(char *)Value);
 
          break;
 
        case mxoAcct:
 
          strcpy(((mgcred_t *)O)->Name,(char *)Value);
 
          break;
 
        case mxoQOS:
 
          strcpy(((mqos_t *)O)->Name,(char *)Value);
 
          break;
 
        case mxoPar:
 
          strcpy(((mpar_t *)O)->Name,(char *)Value);
 
          break;
 
        case mxoClass:
 
          strcpy(((mclass_t *)O)->Name,(char *)Value);
 
          break;
        }  /* END switch(OIndex) */
 
      break;

    case mcaMaxJob:

      {
      int tmpI;

      if (Format == mdfInt)
        {
        tmpI = *(int *)Value;
        }
      else
        {
        /* default format is 'string' */

        tmpI = (int)strtol((char *)Value,NULL,0);
        }

      if (tmpI > 100000)
	{
        /* unlimited */

        tmpI = 0;
        }
       
      L->AP.HLimit[mptMaxJob][0] = tmpI;
      L->AP.SLimit[mptMaxJob][0] = tmpI;
      }  /* END BLOCK */

      break;

    case mcaMaxProc:

      {
      int tmpI;

      if (Format == mdfInt)
        {
        tmpI = *(int *)Value;
        }
      else
        {
        /* default format is 'string' */

        tmpI = (int)strtol((char *)Value,NULL,0);
        }

      if (tmpI > 100000)
        {
        /* unlimited */

        tmpI = 0;
        }

      L->AP.HLimit[mptMaxProc][0] = tmpI;
      L->AP.SLimit[mptMaxProc][0] = tmpI;
      }  /* END BLOCK */

      break;

    case mcaMaxJobPerUser:

      {
      switch (OIndex)
        {
        case mxoClass:
        case mxoQOS:

          {
          int tmpS;
          int tmpL;

          int uindex;

          if (L->APU == NULL)
            L->APU = (mpu_t *)calloc(1,sizeof(mpu_t) * (MAX_MUSER + MAX_MHBUF));

          if (Format == mdfInt)
            {
            tmpS = *(int *)Value;
            tmpL = *(int *)Value;
            }
          else
            {
            /* default format is 'string' */

            __MCredParseLimit((char *)Value,&tmpS,&tmpL);
            }

          for (uindex = 0;uindex < (MAX_MUSER + MAX_MHBUF);uindex++)
            {
            L->APU[uindex].SLimit[mptMaxJob][0] = tmpS;
            L->APU[uindex].HLimit[mptMaxJob][0] = tmpL;
            }  /* END for (uindex) */
          }    /* END BLOCK */

          break;
        }  /* END switch (OIndex) */
      }    /* END BLOCK */

      break;

    case mcaMaxNodePerJob:

      {
      int     tmpI;

      mjob_t *J;

      if ((L->JMax == NULL) &&
         ((L->JMax = (void *)calloc(1,sizeof(mjob_t))) == NULL))
        {
        return(FAILURE);
        }

      J = (mjob_t *)L->JMax;

      if (Value == NULL)
        tmpI = 0;
      else if (Format == mdfInt)
        tmpI = *(int *)Value;
      else
        tmpI = (int)strtol((char *)Value,NULL,0);

      J->Request.NC = tmpI;
      }  /* END BLOCK */

      break;

    case mcaMaxProcPerJob:

      {
      int     tmpI;

      mjob_t *J;

      if ((L->JMax == NULL) &&
         ((L->JMax = (void *)calloc(1,sizeof(mjob_t))) == NULL))
        {
        return(FAILURE);
        }

      J = (mjob_t *)L->JMax;

      if (Value == NULL)
        tmpI = 0;
      else if (Format == mdfInt)
        tmpI = *(int *)Value;
      else
        tmpI = (int)strtol((char *)Value,NULL,0);

      J->Request.TC = tmpI;
      }  /* END BLOCK */

      break;

    case mcaMaxProcPerUser:

      switch (OIndex)
        {
        case mxoClass:
        case mxoQOS:

          {
          int tmpS;
          int tmpL;

          int uindex;

          if (L->APU == NULL)
            L->APU = (mpu_t *)calloc(1,sizeof(mpu_t) * (MAX_MUSER + MAX_MHBUF));

          if (Format == mdfInt)
            {
            tmpS = *(int *)Value;
            tmpL = *(int *)Value;
            }
          else
            {
            /* default format is 'string' */

            __MCredParseLimit((char *)Value,&tmpS,&tmpL);
            }

          for (uindex = 0;uindex < (MAX_MUSER + MAX_MHBUF);uindex++)
            {
            L->APU[uindex].SLimit[mptMaxProc][0] = tmpS;
            L->APU[uindex].HLimit[mptMaxProc][0] = tmpL;
            }  /* END for (uindex) */

          break;
          }    /* END BLOCK */

        default:

          /* NO-OP */

          break;
        }  /* END switch(OIndex) */

      break;

    case mcaMaxWCLimit:

      {
      long tmpL;

      mjob_t *J;

      if ((L->JMax == NULL) &&
         ((L->JMax = (void *)calloc(1,sizeof(mjob_t))) == NULL))
        {
        return(FAILURE);
        }

      J = (mjob_t *)L->JMax;

      if (Format == mdfString)
        tmpL = MUTimeFromString((char *)Value);
      else
        tmpL = *(long *)Value;

      J->SpecWCLimit[0] = tmpL;
      }  /* END BLOCK */

      break;

    default:

      /* not supported */

      return(FAILURE);

      /*NOTREACHED*/

      break;
    }  /* END switch(AIndex) */

  return(SUCCESS);
  }  /* END MCredSetAttr() */




int MCredAToString(

  void *O,       /* I */
  int   OIndex,  /* I */
  int   AIndex,  /* I */
  char *Buf,     /* O */
  int   Format)  /* I */

  {
  static mfs_t    *F;
  static mcredl_t *L;
  static must_t   *S;

  static void *CO = NULL;

  if (Buf == NULL)
    {
    return(FAILURE);
    }

  Buf[0] = '\0';

  if (O == NULL)
    {
    return(FAILURE);
    }

  if (O != CO)
    {
    if ((MOGetComponent(O,OIndex,(void *)&F,mxoFS) == FAILURE) ||
        (MOGetComponent(O,OIndex,(void *)&L,mxoLimits) == FAILURE) ||
        (MOGetComponent(O,OIndex,(void *)&S,mxoStats) == FAILURE))
      {
      /* cannot get components */
 
      return(FAILURE);
      }

    CO = O;
    }
  
  switch(AIndex)
    {
    case mcaID:

      switch(OIndex)
        {
        case mxoUser:
   
          strcpy(Buf,((mgcred_t *)O)->Name);

          break;

        case mxoGroup:

          strcpy(Buf,((mgcred_t *)O)->Name);      

          break;

        case mxoAcct:

          strcpy(Buf,((mgcred_t *)O)->Name);      

          break;

        case mxoQOS:

          strcpy(Buf,((mqos_t *)O)->Name);      

          break;

        case mxoPar:

          strcpy(Buf,((mpar_t *)O)->Name);      

          break;

        case mxoClass:

          strcpy(Buf,((mclass_t *)O)->Name);      

          break;
        }  /* END switch(OIndex) */

      break;

    case mcaDefWCLimit:

      if ((L->JDef == NULL) ||
        (((mjob_t *)L->JDef)->SpecWCLimit[0] == 0))
        break;

      strcpy(Buf,MULToTString(((mjob_t *)L->JDef)->SpecWCLimit[0]));

      break;

    case mcaMaxNodePerJob:

      {
      int tmpI = 0;

      if (L->JMax != NULL)
        tmpI = ((mjob_t *)L->JMax)->Request.NC;

      if (tmpI == 0)
        {
        break;
        }

      sprintf(Buf,"%d",tmpI);
      }  /* END BLOCK */

      break;

    case mcaMaxProcPerJob:

      {
      int tmpI = 0;

      /* NOTE:  temporarily map tasks to procs */

      if (L->JMax != NULL)
        tmpI = ((mjob_t *)L->JMax)->Request.TC;

      if (tmpI == 0)
        {
        break;
        }

      sprintf(Buf,"%d",tmpI);
      }  /* END BLOCK */

    case mcaMaxProcPerUser:

      if ((L->APU != NULL) && 
         ((L->APU[0].HLimit[mptMaxProc][0] != 0) || 
          (L->APU[0].SLimit[mptMaxProc][0] != 0)))
        {
        sprintf(Buf,"%d,%d",
          L->APU[0].HLimit[mptMaxProc][0],
          L->APU[0].SLimit[mptMaxProc][0]);
        }

      break;

    case mcaMaxNodePerUser:

      if ((L->APU != NULL) &&
         ((L->APU[0].HLimit[mptMaxNode][0] != 0) ||
          (L->APU[0].SLimit[mptMaxNode][0] != 0)))
        {
        sprintf(Buf,"%d,%d",
          L->APU[0].HLimit[mptMaxNode][0],
          L->APU[0].SLimit[mptMaxNode][0]);
        }

      break;

    case mcaMaxJobPerUser:

      if ((L->APU != NULL) &&
         ((L->APU[0].HLimit[mptMaxJob][0] != 0) ||
          (L->APU[0].SLimit[mptMaxJob][0] != 0)))
        {
        sprintf(Buf,"%d,%d",
          L->APU[0].HLimit[mptMaxJob][0],
          L->APU[0].SLimit[mptMaxJob][0]);
        }

      break;

    case mcaMaxWCLimit:

      if ((L->JMax == NULL) ||
        (((mjob_t *)L->JMax)->SpecWCLimit[0] == 0))
        {
        break;
        }

      strcpy(Buf,MULToTString(((mjob_t *)L->JMax)->SpecWCLimit[0]));

      break;

    default:

      /* attribute not handled */

      return(FAILURE);

      /*NOTREACHED*/

      break;
    }  /* END switch(AIndex) */

  return(SUCCESS);
  }  /* END MCredAToString() */




char *MOGetName(

  void  *O,
  int    OIndex,
  char **NameP)

  {
  char *ptr;

  if (O == NULL)
    {
    return(NULL);
    }

  switch(OIndex)
    {
    case mxoUser:

      ptr = ((mgcred_t *)O)->Name;

      break;

    case mxoGroup:

      ptr = ((mgcred_t *)O)->Name;

      break;

    case mxoAcct:

      ptr = ((mgcred_t *)O)->Name;

      break;

    case mxoQOS:

      ptr = ((mqos_t *)O)->Name;

      break;

    case mxoClass:

      ptr = ((mclass_t *)O)->Name;

      break;

    default:

      ptr = NULL;

      break;
    }  /* END switch(OIndex) */

  if (NameP != NULL)
    *NameP = ptr;

  return(ptr);
  }  /* END MOGetName() */




int MOFromString(

  void  *O,       /* I (modified) */
  int    OIndex,  /* I */
  char  *EString) /* I */

  {
  mxml_t *E = NULL;

  int rc;

  if ((O == NULL) || (EString == NULL))
    {
    return(FAILURE);
    }

  if (MXMLFromString(&E,EString,NULL,NULL) == FAILURE)
    {
    return(FAILURE);
    }

  rc = MOFromXML(O,OIndex,E);

  MXMLDestroyE(&E);

  return(rc);
  }  /* END MOFromString() */




int MOFromXML(
 
  void    *O,      /* I (modified) */
  int      OIndex, /* I */
  mxml_t *E)      /* I */
 
  {
  int aindex;
  int cindex;
 
  int caindex;

  static mfs_t    *F;
  static mcredl_t *L;
  static must_t   *S; 

  static void *CO = NULL;

  mxml_t *C;

  if ((O == NULL) || (E == NULL))
    {
    return(FAILURE);
    }
 
  /* NOTE:  do not initialize.  may be overlaying data */

  /* set attributes */

  if (OIndex == mxoSys)
    {
    for (aindex = 0;aindex < E->ACount;aindex++)
      {
      if ((caindex = MUGetIndex(E->AName[aindex],MSysAttr,FALSE,0)) != 0)
        {
        MSysSetAttr((msched_t *)O,caindex,(void **)E->AVal[aindex],mdfString,mSet);
        }
      }  /* END for (aindex) */
    }
  else
    { 
    for (aindex = 0;aindex < E->ACount;aindex++)
      {
      caindex = MUGetIndex(E->AName[aindex],MCredAttr,FALSE,0);
 
      if (caindex == 0)
        continue;
 
      MCredSetAttr(O,OIndex,caindex,(void **)E->AVal[aindex],mdfString,mSet);
      }  /* END for (aindex) */
    }

  if (O != CO)
    { 
    if ((MOGetComponent(O,OIndex,(void *)&F,mxoFS) == FAILURE) ||
        (MOGetComponent(O,OIndex,(void *)&L,mxoLimits) == FAILURE) ||
        (MOGetComponent(O,OIndex,(void *)&S,mxoStats) == FAILURE))
      {
      /* cannot get components */
 
      return(FAILURE);
      }

    CO = 0;
    }

  for (cindex = 0;cindex < E->CCount;cindex++)
    {
    C = E->C[cindex];
 
    caindex = MUGetIndex(C->Name,MXO,FALSE,0);
 
    if (caindex == 0)
      continue;
 
    switch (caindex) 
      {
      case mxoStats:
 
        MStatFromXML(S,C);
 
        break;

      case mxoLimits:

        MLimitFromXML(L,C);

        break;

      case mxoFS:

        MFSFromXML(F,C);

        break;
 
      default:
 
        break;
      }  /* END switch(caindex) */
    }    /* END for (cindex) */
 
  return(SUCCESS);
  }  /* END MOFromXML() */




int MOToXML(

  void     *O,      /* I */
  int       OIndex, /* I */
  mxml_t **EP)     /* O */

  {
  if ((O == NULL) || (EP == NULL))
    {
    return(FAILURE);
    }

  switch(OIndex)
    {
    case mxoFS:

      MFSToXML((mfs_t *)O,EP,NULL);

      break;

    case mxoLimits:

      MLimitToXML((mcredl_t *)O,EP,NULL);

      break;

    case mxoStats:

      MStatToXML((must_t *)O,EP,NULL);

      break;

    default:

      /* NOT HANDLED */

      return(FAILURE);

      /*NOTREACHED*/

      break;
    }  /* END switch(OIndex) */

  if (*EP == NULL)
    {
    return(FAILURE);
    }

  return(SUCCESS);
  }  /* END MOToXML() */




void *MOGetNextObject(

  void **O,       /* I */
  int    OIndex,
  int    OSize,
  void  *OEnd,
  char **NameP)

  { 
  if ((*O >= OEnd) && (OIndex != mxoSched))
    {
    *O = NULL;

    return(*O);
    }
 
  switch(OIndex) 
    {
    case mxoUser:  

      {
      mgcred_t **UP;
     
      for (UP = *(mgcred_t ***)O + 1;UP < (mgcred_t **)OEnd;UP += 1)
        {
        if ((*UP != NULL) && ((*UP)->Name[0] > '\1'))
          {    
          *O = (void *)UP;

          if (NameP != NULL)
            *NameP = (*UP)->Name;

          return((void *)*UP);
          }
        }  /* END for (OP) */
      }

      break;

    case mxoGroup:

      {
      mgcred_t *G;
 
      for (G = *(mgcred_t **)O + 1;G < (mgcred_t *)OEnd;G += 1)
        {
        if ((G != NULL) && (G->Name[0] > '\1'))
          {
          *O = (void *)G;
 
          if (NameP != NULL)
            *NameP = G->Name;
 
          return(*O);
          }
        }  /* END for (GP) */
      }
 
      break;

    case mxoAcct:

      {
      mgcred_t *A;
 
      for (A = *(mgcred_t **)O + 1;A < (mgcred_t *)OEnd;A += 1)
        {
        if ((A != NULL) && (A->Name[0] > '\1'))
          {
          *O = (void *)A;
 
          if (NameP != NULL)
            *NameP = A->Name;
 
          return(*O);
          }
        }  /* END for (A) */
      }
 
      break;

    case mxoQOS:

      {
      mqos_t *Q;
 
      for (Q = *(mqos_t **)O + 1;Q < (mqos_t *)OEnd;Q += 1)
        {
        if ((Q == NULL) || (Q->Name[0] == '\0'))
          continue;

        if ((Q->Name[0] > '\1'))
          {
          *O = (void *)Q;
 
          if (NameP != NULL)
            *NameP = Q->Name;
 
          return(*O);
          }
        }  /* END for (Q) */
      }

      break;

    case mxoClass:

      {
      mclass_t *C;
 
      for (C = *(mclass_t **)O + 1;C < (mclass_t *)OEnd;C += 1)
        {
        if ((C == NULL) || (C->Name[0] == '\0'))
          continue;
 
        if ((C->Name[0] > '\1'))
          {
          *O = (void *)C;
 
          if (NameP != NULL)
            *NameP = C->Name;
 
          return(*O);
          }
        }  /* END for (C) */
      }    /* END BLOCK */

      break;

    case mxoSched:

      {
      if (*O == &MSched)
        {
        /* initial pass */

        *O = NULL;
    
        return(&MSched);
        }
      else
        {
        *O = NULL;

        return(NULL);
        }

      return(*O);
      }  /* END BLOCK */

      break;

    default: 

      *NameP = NULL; 

      break; 
    }  /* END switch(OIndex) */

  *O = NULL;

  return(*O);
  }  /* END MOGetNextObject() */




int MCredConfigShow(

  void *O,       /* I */
  int   OIndex,  /* I */
  int   VFlag,   /* I */
  int   PIndex,  /* I */
  char *Buffer)  /* O */

  {
  char tmpLineC[MAX_MLINE];
  char tmpLineS[MAX_MLINE];

  char *OName = NULL;

  if ((O == NULL) || (Buffer == NULL))
    {
    return(FAILURE);
    }

  tmpLineC[0] = '\0';
  tmpLineS[0] = '\0';

  /* show common attributes */

  MCredConfigLShow(O,OIndex,VFlag,PIndex,tmpLineS);

  /* show cred specific attributes */

  switch(OIndex)
    {
    case mxoQOS:

      MQOSConfigLShow((mqos_t *)O,VFlag,PIndex,tmpLineS);

      break;

    case mxoClass:

      MClassConfigLShow((mclass_t *)O,VFlag,PIndex,tmpLineS);

      break;

    default:

      /* no-op */

      break;
    }  /* END switch(OIndex) */

  if ((tmpLineC[0] != '\0') || (tmpLineS[0] != '\0'))
    {
    /* display parameter */

    MOGetName(O,OIndex,&OName);

    sprintf(Buffer,"%s%s[%s] %s%s\n",
      Buffer,
      MCredCfgParm[OIndex],
      OName,
      tmpLineC,
      tmpLineS);
    }
 
  return(SUCCESS);
  }  /* END MCredConfigShow() */




int MCredConfigLShow(

  void *O,       /* I */
  int   OIndex,  /* I */
  int   VFlag,   /* I */
  int   PIndex,  /* I */
  char *Buf)     /* O */

  {
  int AList[] = {
    mcaDefWCLimit,
    mcaMaxNodePerJob,
    mcaMaxProcPerJob,
    mcaMaxProcPerUser,
    mcaMaxWCLimit,
    -1 };

  int aindex;

  char tmpString[MAX_MLINE];

  if ((O == NULL) || (Buf == NULL))
    {
    return(FAILURE);
    }

  /* clear buffer */

  Buf[0] = '\0';

  for (aindex = 0;AList[aindex] != -1;aindex++)
    {
    if ((MCredAToString(O,OIndex,AList[aindex],tmpString,0) == FAILURE) ||
        (tmpString[0] == '\0'))
      {
      continue;
      }

    sprintf(Buf,"%s %s=%s",
      Buf,
      MCredAttr[AList[aindex]],
      tmpString);
    }  /* END for (aindex) */

  return(SUCCESS);
  }  /* END MCredConfigLShow() */




int MOLoadPvtConfig(

  void  **O,               /* I */
  int     OIndex,
  char   *OName,
  mpsi_t *P,
  char   *ConfigPathName)  /* I (optional) */

  {
  char *ptr;
  char *curptr;
  char *TokPtr;

  char  IndexName[MAX_MNAME];
  char  tmpLine[MAX_MLINE];
  char  ValLine[MAX_MLINE];

  int   aindex;

  if (MSched.PvtConfigBuffer == NULL)
    {
    int count;
    int SC;

    if (ConfigPathName == NULL)
      {
      return(FAILURE);
      }

    /* load/cache private config data */

    if ((MSched.PvtConfigBuffer = MFULoad(
          ConfigPathName,
          1,
          macmWrite,
          &count,
          &SC)) == NULL)
      {
      DBG(6,fCONFIG) DPrint("WARNING:  cannot load file '%s'\n",
        ConfigPathName);

      return(FAILURE);
      }

    MCfgAdjustBuffer(&MSched.PvtConfigBuffer,TRUE);
    }  /* END if (MSched.PvtConfigBuffer == NULL) */

  curptr = MSched.PvtConfigBuffer;

  /* FORMAT:  [CSKEY=<X>][CSALGO=<X>][HOST=<X>][PORT=<X>][PROTOCOL=<X>][VERSION=<X>] */
  
  if (OIndex > mxoNONE)
    {
    /* obtain object name */

    switch(OIndex)
      {
      case mxoAM:

        sprintf(IndexName,"AM:%s",
          ((mam_t *)O)->Name);

        break;

      case mxoRM:

        sprintf(IndexName,"RM:%s",
          ((mrm_t *)O)->Name);

        break;

      case mxoNONE:
      default:

        /* not handled */

        return(FAILURE);

        /*NOTREACHED*/

        break;
      }  /* END switch(OIndex) */

    /* load config info */

    if (MCfgGetSVal(
          MSched.PvtConfigBuffer,
          &curptr,
          MParam[pClientCfg],
          IndexName,
          NULL,
          tmpLine,
          sizeof(tmpLine),
          0,
          NULL) == FAILURE)
      {
      /* cannot locate client config info */

      return(SUCCESS);
      }

    /* process value/set attributes */
   
    ptr = MUStrTok(tmpLine," \t\n",&TokPtr);

    while(ptr != NULL)
      {
      /* parse name-value pairs */

      /* FORMAT:  <ATTR>=<VALUE>[,<VALUE>] */

      if (MUGetPair(
            ptr,
            (const char **)MOServAttr,
            &aindex,
	    NULL,
            TRUE,
            NULL,
            ValLine,
            MAX_MNAME) == FAILURE)
        {
        ptr = MUStrTok(NULL," \t\n",&TokPtr);

        continue;
        }  /* END if (MUGetPair() == FAILURE) */

      switch(aindex)
        {
        case mosaCSKey:

          switch(OIndex)
            {
            case mxoRM:
 
              MRMSetAttr((mrm_t *)O,mrmaCSKey,(void **)ValLine,mdfString,0);

              break;

            case mxoAM:

              MAMSetAttr((mam_t *)O,mamaCSKey,(void **)ValLine,mdfString,0);

              break;

            case mxoNONE:
            default:

              /* NO-OP */

              break;
            }  /* END switch(OIndex) */

          break;

        case mosaCSAlgo:

          switch(OIndex)
            {
            case mxoRM:

              MRMSetAttr((mrm_t *)O,mrmaCSAlgo,(void **)ValLine,mdfString,0);

              break;

            case mxoAM:

              MAMSetAttr((mam_t *)O,mamaCSAlgo,(void **)ValLine,mdfString,0);

              break;

            default:

              break;
            }  /* END switch(OIndex) */

          break;
     
        default:

          /* NO-OP */

          break;
        }  /* END switch(AIndex) */

      ptr = MUStrTok(NULL," \t\n",&TokPtr);
      }    /* END while (ptr != NULL) */
    }      /* END if (OIndex > mxoNONE) */
  else
    {
    int index;

    mclient_t *C;

    /* load general client private attributes */

    if (OName != NULL)
      {
      strcpy(IndexName,OName);
      }
    else
      {
      IndexName[0] = '\0';
      }

    index = 0;

    while (MCfgGetSVal(
        MSched.PvtConfigBuffer,
        &curptr,
        MParam[pClientCfg],
        IndexName,
        NULL,
        tmpLine,
        sizeof(tmpLine),
        0,
        NULL) == SUCCESS)
      {
      /* FORMAT:  INDEXNAME:  <CLIENT_NAME> || RM:<RMNAME> */

      if (!strncmp("RM:",IndexName,strlen("RM:")) || 
          !strncmp("AM:",IndexName,strlen("AM:")))
        {
        if (OName == NULL)
          {
          IndexName[0] = '\0';
          }

        continue;
        }

      if (P != NULL)
        {
        /* process value/set attributes */

        ptr = MUStrTok(tmpLine," \t\n",&TokPtr);

        while (ptr != NULL)
          {
          /* parse name-value pairs */

          /* FORMAT:  <ATTR>=<VALUE>[,<VALUE>] */

          if (MUGetPair(
                ptr,
                (const char **)MOServAttr,
                &aindex,
                NULL,
                TRUE,
                NULL,
                ValLine,
                MAX_MNAME) == FAILURE)
            {
            ptr = MUStrTok(NULL," \t\n",&TokPtr);

            continue;
            }  /* END if (MUGetPair() == FAILURE) */

          switch(aindex)
            {
            case mosaCSAlgo:

              P->CSAlgo = MUGetIndex((char *)ValLine,MCSAlgoType,FALSE,P->CSAlgo);

              break;

            case mosaCSKey:

              MUStrDup(&P->CSKey,ValLine);

              break;

            case mosaHost:

              MUStrDup(&P->HostName,ValLine);

              break;

            case mosaPort:

              P->Port = (int)strtol(ValLine,NULL,0);

              break;

            case mosaProtocol:

              P->SocketProtocol = MUGetIndex(ValLine,MSockProtocol,FALSE,0);

              break;

            case mosaVersion:

              MUStrDup(&P->Version,ValLine);

              break;

            default:

              /* not handled */

              break;
            }  /* END switch(aindex) */

          ptr = MUStrTok(NULL," \t\n",&TokPtr);
          }    /* END while (ptr != NULL) */
        }      /* END if (P != NULL) */
      else
        {
        /* cache general client info */

        /* process value/set attributes */

        ptr = MUStrTok(tmpLine," \t\n",&TokPtr);

        while (ptr != NULL)
          {
          /* parse name-value pairs */

          /* FORMAT:  <ATTR>=<VALUE>[,<VALUE>] */

          if (MUGetPair(
                ptr,
                (const char **)MOServAttr,
                &aindex,
                NULL,
                TRUE,
                NULL,
                ValLine,
                MAX_MNAME) == FAILURE)
            {
            ptr = MUStrTok(NULL," \t\n",&TokPtr);

            continue;
            }  /* END if (MUGetPair() == FAILURE) */

          /* NOTE:  do not support algo specification */

          switch(aindex)
            {
            case mosaCSKey:

              C = &MClient[index];

              MUStrCpy(C->Name,IndexName,sizeof(MClient[index].Name));
              MUStrCpy(C->CSKey,ValLine,MAX_MNAME);

              index++;

              break;

            default:

              /* NO-OP */

              break;
            }  /* END switch(aindex) */

          ptr = MUStrTok(NULL," \t\n",&TokPtr);
          }  /* END while (ptr != NULL) */
        }    /* END else (P != NULL) */

      /* reset IndexName */

      if (OName == NULL)
        {
        IndexName[0] = '\0';
        }
      }      /* END while (MCfgGetSVal() == SUCCESS) */
    }        /* END else (OIndex > mxoNONE) */

  return(SUCCESS);
  }  /* END MOLoadPvtConfig() */




int MCredIsMatch(

  mcred_t *C, /* I */
  void    *O, /* I */
  int      OType) /* I */

  {
  if ((C == NULL) || (O == NULL))
    {
    return(FAILURE);
    }

  switch(OType)
    {
    case mxoUser:

      if (C->U == (mgcred_t *)O)
        {
        return(SUCCESS);
        }

      break;

    case mxoGroup:

      if (C->G == (mgcred_t *)O)
        {
        return(SUCCESS);
        }

      break;

    case mxoAcct:

      if (C->A == (mgcred_t *)O)
        {
        return(SUCCESS);
        }

      break;

    case mxoClass:

      if (C->C == (mclass_t *)O)
        {
        return(SUCCESS);
        }

      break;

    case mxoQOS:

      if (C->Q == (mqos_t *)O)
        {
        return(SUCCESS);
        }

      break;

    default:

      /* NO-OP */

      break;
    }  /* END switch(OType) */

  return(FAILURE);
  }  /* END MCredIsMatch() */
   
/* END MCred.c */

