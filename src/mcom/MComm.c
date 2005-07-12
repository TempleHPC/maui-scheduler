/* HEADER */

#include "moab.h"
#include "moab-proto.h"

extern const char *SUMKType[];

extern mlog_t mlog;




void MSUCommParserInit(

  sucomm_parser_t *parser)   /* I (modified) */

  {
  if (parser == NULL)
    {
    return;
    }

  parser->length  = 0;
  parser->state   = 0;
  parser->msgSize = 0;
  
  return;
  }  /* END MSUCommParserInit() */





int MSUCommParse(

  su_t            *su,
  sucomm_parser_t *Parser,  /* I */
  void            *data,
  size_t           size,
  mbool_t         *IsDone)  /* O (modified) */

  {

  *IsDone = FALSE;

  if (Parser == NULL)
    {
    return(FAILURE);
    }

  if (Parser->length + size >= MMAX_BUFFER) /* buffer overflow */
    {
    return(FAILURE);
    }

  memcpy(Parser->buffer + Parser->length, data, size);
  Parser->length += size;

  /* perform mcomm specific parsing */

  switch (Parser->state)
    {
    case 0: /* initial */

      if (Parser->length < 9)
        {
        return(SUCCESS);
        }

      Parser->buffer[8] = '\0';
      Parser->msgSize = strtoul(Parser->buffer, NULL, 10);

      if ((Parser->msgSize == 0) || (Parser->msgSize >= MMAX_BUFFER))
        {
        return(FAILURE);
        }

      memmove(Parser->buffer,Parser->buffer + 9,Parser->length - 9);

      Parser->length -= 9;

      Parser->state = 1;

      /* fall through */

    case 1: /* length known */

      if (Parser->length >= (int)Parser->msgSize) 
        {
        *IsDone = TRUE;

        Parser->buffer[Parser->length] = '\0';
        }

    return(SUCCESS);
    }  /* END switch (Parser->state) */

  return(FAILURE);
  }  /* END MSUCommParse() */




int MSUCommWrapMessage(

  su_t       *su,
  const char *cmd,
  const char *args,
  suclcred_t *Cred,
  char       *buffer,
  size_t      size)

  {
  char msg[MMAX_BUFFER];
  char cksum[MMAX_LINE];
  time_t Now;
  char backup;


  time(&Now);

  snprintf(msg,MMAX_BUFFER,"%s%ld ID=%s %s%s%s %s%s %s%s %s%s",
    SUMKType[sumkTimeStamp],
    (long)Now,
    su->Env->UserName,
    SUMKType[sumkData],
    SUMKType[sumkCommand],
    cmd,
    SUMKType[sumkAuth],
    (su->Env->UserName != NULL) ? su->Env->UserName : NONE,
    SUMKType[sumkClient],
    (su->Name != NULL) ? su->Name : NONE,
    SUMKType[sumkArgs],
    (args != NULL) ? args : "");

  MSecGetChecksum(
    msg,
    strlen(msg),
    cksum,
    NULL,
    mcsaDES,
    Cred->EncryptionData.CSKey);

  snprintf(buffer + 9,size - 9,"%s%s %s",
    SUMKType[sumkCheckSum],
    cksum,
    msg);

  backup = buffer[9];
  sprintf(buffer, "%08ld\n", (long)strlen(buffer+9));
  buffer[9] = backup;

  return(strlen(buffer));
  }  /* END MSUCommWrapMessage() */




int MSUCommMessageIsValid(

  su_t       *su,
  char       *msg,    /* I */
  suclcred_t *Cred)

  {
  char  *cksum = strstr(msg,SUMKType[sumkCheckSum]);
  char  *ts = strstr(msg,SUMKType[sumkTimeStamp]);
  char  *data = strstr(msg,SUMKType[sumkData]);
  char  *cksumEnd;
  char  *tsEnd;
  char   CKSum[MMAX_LINE];
  time_t TSVal;
  time_t Now;

  if (cksum == NULL)
    {
    MDB(1,fSOCK) MLog("ALERT:    cannot locate checksum '%s'\n",
      msg);

    return(FAILURE);
    }

  if (ts == NULL)
    {
    MDB(1,fSOCK) MLog("ALERT:    cannot locate timestamp '%s'\n",
      msg);

    return(FAILURE);
    }

  if (data == NULL)
    {
    MDB(1,fSOCK) MLog("ALERT:    cannot locate data '%s'\n",
      msg);

    return(FAILURE);
    }

  cksum += strlen(SUMKType[sumkCheckSum]);
  cksumEnd = strchr(cksum,' ');

  if (cksumEnd)
    *cksumEnd = 0;

  MSecGetChecksum(
    ts,
    strlen(ts),
    CKSum,
    NULL,
    mcsaDES,
    Cred->EncryptionData.CSKey);

  if (strcmp(cksum,CKSum) != 0)
    {
    MDB(1,fSOCK) MLog("ALERT:    checksum does not match (%s:%s)\n",
      cksum,
      CKSum);

    if (cksumEnd)
      *cksumEnd = ' ';

    return(FAILURE);
    }

  if (cksumEnd)
    *cksumEnd = ' ';

  ts += strlen(SUMKType[sumkTimeStamp]);
  tsEnd = strchr(ts, ' ');

  if (tsEnd)
    *tsEnd = 0;

  TSVal = strtoul(ts, NULL, 10);

  if (tsEnd)
    *tsEnd = ' ';

  time(&Now);

  if (((Now - TSVal) > 3600) || ((Now - TSVal) < -3600))
    {
    MDB(1,fSOCK) MLog("ALERT:    timestamp does not match (%ld:%ld)\n",
      (unsigned long)Now,
      (unsigned long)TSVal);

    return(FAILURE);
    }

  return(SUCCESS);
  }  /* END MSUCommMessageIsValid() */


/* END MComm.c */

