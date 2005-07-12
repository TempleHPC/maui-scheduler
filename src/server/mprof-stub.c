/*
*/

#include "mclient-stub.c"

        
int JobPReserve(

  mjob_t *J,
  int     PIndex,
  int    *ResCount)

  {
  return(SUCCESS);
  }

int UHProcessRequest(

  msocket_t *S,
  char     *RBuffer)

  {
  return(SUCCESS);
  }  /* END UHProcessRequest() */

int UIQueueShowAllJobs(char *S,long *SS,mpar_t *P,char *U) { return(SUCCESS); }

/* END mprof-stub.c */

