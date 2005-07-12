#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define __MDPROC72

struct {
  char  *Name;
  int    JobCount;
  double PFrac;
  int    Duration;
  } JobDesc[] = {
  { "A", 75,  0.03125, 267 },  /* A */
  { "B", 9,   0.06250, 322 },  /* B */
  { "C", 3,   0.50000, 534 },  /* C */
  { "D", 3,   0.25000, 616 },  /* D */
  { "E", 3,   0.50000, 315 },  /* E */
  { "F", 9,   0.06250, 1846 }, /* F */
  { "G", 6,   0.12500, 1334 }, /* G */
  { "H", 6,   0.15820, 1067 }, /* H */
  { "I", 24,  0.03125, 1432 }, /* I */
  { "J", 24,  0.06250, 725 },  /* J */
  { "K", 15,  0.09570, 487 },  /* K */
  { "L", 36,  0.12500, 366 },  /* L */
  { "M", 15,  0.25000, 187 },  /* M */    
/* { "A", 2, 1.00000, 100 }, */      
  { 0, 0, 0 }};

/* NOTE:  submit 'Z' jobs at +00:24:00 and +02:00:00 */




int main(

  int    argc,
  char **argv)

  {
  int jindex;
  int index;

  int dindex;

  int JobCount;

  int JobList[1000];

  int SubmitTime;

  int SysSize;

  time_t StartTime;
  time_t Now;

  SysSize = atoi(argv[1]);

  /* populate list of potential jobs */

  jindex = 0;

  for (dindex = 0;JobDesc[dindex].JobCount > 0;dindex++)
    {
    for (index = 0;index < JobDesc[dindex].JobCount;index++)
      {
      JobList[jindex++] = dindex;      
      }
    }  /* END for (dindex) */

  JobCount = jindex;

  jindex = 0;

#if !defined(__LIVE)   
  printf("# workload trace for %d processor ESP test\n",
    SysSize);
#endif /* !__LIVE */

  time(&StartTime);

  SubmitTime = 1;

  while (jindex < JobCount)
    {
    index = (int)((double)JobCount * rand() / (RAND_MAX + 1.0));  

    if (JobList[index] == -1)
      continue;

#if !defined(__LIVE)

/* NOTE:  staggered job submission no longer required */

/*
    if (jindex > (JobCount * 2 / 3))
      SubmitTime = 20 * 60;
    else if  (jindex > (JobCount / 3))
      SubmitTime = 10 * 60;
    else
      SubmitTime = 1;
*/

    printf("job%s%03d 1 %d userA groupB %d Completed [batch:1] %d %d %d %d [NONE] [NONE] [NONE] >= 0 >= 0 [NONE] 1 1 0 0 0 accountC [NONE] [NONE] 0 0.0 DEFAULT 1 0 0 0 0 0 X 0 [NONE] [NONE] [NONE] [NONE] [NONE]\n",
      JobDesc[JobList[index]].Name,
      jindex,
      (int)(JobDesc[JobList[index]].PFrac * SysSize),
      JobDesc[JobList[index]].Duration,
      SubmitTime,
      SubmitTime,
      SubmitTime,
      SubmitTime + JobDesc[JobList[index]].Duration);
#else
    time(&Now);

    if (((jindex >= (int)(JobCount * 2 / 3)) && ((Now - StartTime) < 1200)) ||
        ((jindex >= (int)(JobCount * 1 / 3)) && ((Now - StartTime) < 600)))
      {
      sleep(1);

      continue;
      }

    /* NOTE:  add submission queue info */

    sprintf(SubmitCommand,"/usr/local/bin/qsub -l nodes=%d,walltime=%d %c",
      JobDesc[JobList[index]].ProcCount,
      JobDesc[JobList[index]].Duration,
      'A' + JobList[index] - 1);

    system(SubmitCommand);
#endif /* !defined(__LIVE) */

    JobList[index] = -1;

    jindex++;
    }  /* END while (jindex) */

#if !defined(__LIVE)
  /* submit Z jobs */

  printf("jobZ%03d 1 %d userA groupB %d Completed [super:1] %d %d %d %d [NONE] [NONE] [NONE] >= 0 >= 0 [NONE] 1 1 0 0 0 accountC [NONE] [NONE] 0 0.0 DEFAULT 1 0 0 0 0 0 X 0 [NONE] [NONE] [NONE] [NONE] [NONE]\n",
    jindex,
    SysSize,
    100,
    SubmitTime + 2400,
    SubmitTime + 2400,
    SubmitTime + 2400,
    SubmitTime + 2400 + 100);

  jindex++;

  printf("jobZ%03d 1 %d userA groupB %d Completed [super:1] %d %d %d %d [NONE] [NONE] [NONE] >= 0 >= 0 [NONE] 1 1 0 0 0 accountC [NONE] [NONE] 0 0.0 DEFAULT 1 0 0 0 0 0 X 0 [NONE] [NONE] [NONE] [NONE] [NONE]\n",
    jindex,
    SysSize,
    100,
    SubmitTime + 7200,
    SubmitTime + 7200,
    SubmitTime + 7200,
    SubmitTime + 7200 + 100);
#else
  sleep(2400);

  /* submit Z0 at 00:40:00 */

  /* NYI */

  sleep(4800);
  
  /* submit Z1 at 02:00:00 */

  /* NYI */
#endif

  exit(0);
  }  /* END main() */

