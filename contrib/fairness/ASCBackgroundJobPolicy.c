/* CONTRIB:  ASCBackgroundJobPolicy.c */

/* code to be included in LocalCheckFairnessPolicy() in Local.c */

#define BACKGROUNDJOBQOS 0

/* uses global Job */

int ContribASCBackgroundJobPolicy(

  job_t *SpecJ,
  long   StartTime,
  char  *Message)

  {
  int    jindex;
  job_t *J;
  int    BackgroundQOS;

  if (SpecJ == NULL)
    return(FAILURE);

  BackgroundQOS = BACKGROUNDJOBQOS;

  if (SpecJ->QOS != BackgroundQOS)
    return(SUCCESS);

  /* locate idle jobs with non-background QOS */

  for (jindex = Job[0].Next;jindex != 0;jindex = Job[jindex].Next)
    {
    J = &Job[jindex];

    if (J->State != jIdle)
      continue;

    if (J->QOS != BackgroundQOS)
      {
      /* idle non-background QOS job located */

      /* background job should not be allowed to run */

      return(FAILURE);
      }
    }  /* END for (jindex) */

  /* no idle non-background job located */

  return(SUCCESS);
  }  /* END ContribASCBackgroundJobPolicy() */

/* END ASCBackgroundJobPolicy.c */
