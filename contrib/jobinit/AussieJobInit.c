/* CONTRIB:  AussieJobInit.c */

/* code to be included in LocalJobInit() in Local.c */

/* uses global BQS and Job */

enum { ajwcNONE = 0, ajwcConstant, ajwcLinear, ajwcQuadratic };

#define DEFAULTWCPOLICY  ajwcConstant


int ContribAussieJobInit(

  job_t *J)

  {
  int WCPolicy;
  int ProcCount;

  DBG(4,fCONFIG) DPrint("ContribAussieJobInit(%s)\n",
    J->Name);

  if ((J->SpecWCLimit[0] > 0) || (J->CPULimit <= 0))
    {
    /* wallclock time specified for job
       or
       no cpulimit specified for job */

    return(SUCCESS);
    }

  if ((J->Req[0]->TaskCount > 0) && (J->Req[0]->DRes.Procs > 0))
    {
    ProcCount = J->Req[0]->TaskCount * J->Req[0]->DRes.Procs;
    }
  else
    {
    /* should not happen */

    ProcCount = 1;
    }

  WCPolicy = DEFAULTWCPOLICY;

  switch(WCPolicy)
    {
    case ajwcLinear:

      J->SpecWCLimit[0] = J->CPULimit / ProcCount;

      break;

    case ajwcQuadratic: 

      J->SpecWCLimit[0] = (long)((double)J->CPULimit / pow((double)ProcCount,0.5));

      break;

    case ajwcConstant:
    default:

      J->SpecWCLimit[0] = J->CPULimit;

      break;
    }  /* END switch(WCPolicy) */

  J->SpecWCLimit[1] = J->SpecWCLimit[0];

  DBG(4,fCONFIG) DPrint("INFO:     job %s wallclock limit set to %s (%ld)\n",
    J->Name,
    StringTime(J->SpecWCLimit[1]),
    J->CPULimit);

  return(SUCCESS);
  }  /* END ContribAussieJobInit() */

/* END AussieJobInit.c */
