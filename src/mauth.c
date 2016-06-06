/* HEADER */

#include "moab.h"
#include "moab-proto.h"

#define MUNAMETOK  "AUTH="

void MAAuthorize(char *);

mlog_t mlog;

#include "MLog.c"
#include "../mcom/MSec.c"
#include "MFile.c"
#include "MGUtil.c"




int main(

  int    argc,
  char **argv)

  {
  char  KeyFile[MAX_MLINE];

  char  CSBuf[MAX_MLINE];

  char *Digest;

  char *Buf;
  char *ptr;

  int   i;

  if (argv[1] == NULL)
    {
    exit(1);
    }
   
  Buf = argv[1];

  /* determine key file */

  if ((ptr = getenv(MSCHED_ENVHOMEVAR)) != NULL) 
    { 
    sprintf(KeyFile,"%s/%s",
      ptr,
      MSCHED_KEYFILE);
    }
  else 
    { 
    sprintf(KeyFile,"%s/%s",
      MBUILD_HOMEDIR,
      MSCHED_KEYFILE);
    }

  /* check authorization */

  MAAuthorize(Buf);

  /* open keyfile */

  if ((Digest = MFULoad(KeyFile,1,macmRead,NULL,NULL)) == NULL)
    {
    fprintf(stderr, "ERROR:  cannot open keyfile '%s' for reading: %s\n", 
      KeyFile,
      strerror(errno));

    exit(errno);
    }

  for (i = strlen(Digest) - 1;i > 0;i--)
    {
    if (!isspace(Digest[i]))
      break;

    Digest[i] = '\0';
    }  /* END for (i) */

  /* compute keyed hash for data and key */

  MSecGetChecksum(
    Buf,
    strlen(Buf),
    CSBuf,
    NULL,
    mcsaHMAC,
    Digest);

  fprintf(stdout,"%s\n",
    CSBuf);

  return(0);
  }  /* END main() */





void MAAuthorize(
 
  char *Buf)

  {
  char *ptr;

  int   i;

  struct passwd *passwd_s;

  if ((ptr = strstr(Buf,MUNAMETOK)) == NULL)
    {
    fprintf(stderr,"ERROR:  data string must contain %s<user>\n",
      MUNAMETOK);

    exit(1);
    }

  if ((passwd_s = getpwuid(getuid())) == NULL)
    {
    fprintf(stderr,"ERROR:  cannot determine current user\n");

    exit(1);
    }

  ptr += strlen(MUNAMETOK);

  for (i = 0;i < strlen(ptr);i++)
    {
    if (isspace(ptr[i]))
      break;
    }  /* END for (i) */

  if (strncmp(passwd_s->pw_name,ptr,i))
    {
    fprintf(stderr,"ERROR:  %s is unauthorized to create a checksum for %s\n",
      passwd_s->pw_name, 
      ptr);

    exit(1);
    }

  return;
  }  /* END MAAuthorize() */

