/* CONTRIB:  AussieCheckReq.c */

/* code to be included in LocalCheckRequirements() in Local.c */

/* This code enofrces the policy 'only one job per user per node' *
 * on SMP shared nodes                                            *
 */

/* uses global BQS and Job */


int ContribAussieCheckReq(

  job_t  *J,
  node_t *N,
  long    StartTime)

  {
  int    rindex;

  long   Overlap;

  res_t *R;

  DBG(4,fCONFIG) DPrint("ContribAussieCheckReq(%s,%s)\n",
    J->Name,
    N->Name);

  for (rindex = 0;rindex < MAX_RESERVATION;rindex++)
    {
    R = N->R[rindex];

    if (R == (res_t *)MAX_RES_DEPTH)
      continue;

    if (R == NULL)
      break;

    Overlap =
      MIN(R->EndTime,StartTime + J->SpecWCLimit[0]) -
      MAX(R->StartTime,StartTime);

    if (Overlap <= 0)
      {
      /* reservations do not overlap */

      continue;
      }

    if (R->J == NULL)
      {
      /* reservation is not a job reservation */

      continue;
      }

    if (((job_t *)R->J)->Cred.U != J->Cred.U)
      {
      /* users do not match */

      continue;
      }

    /* overlapping job reservation for same user found */

    return(FAILURE);
    }  /* END for (rindex) */

  return(SUCCESS);
  }  /* END ContribAussieCheckReq() */

/* END AussieCheckReq.c */
