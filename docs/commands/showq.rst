
.. rubric:: showq
   :name: showq

**(Show Queue)**
**Synopsis**

::

    showq [-b] [-g] [-l] [-c|-i|-r] [-p partition] [-R rsvid] [-v] [-w <CONSTRAINT>]

**Overview**

Displays information about active, eligible, blocked, and/or recently
completed jobs.  Since the resource manager is not actually scheduling
jobs, the job ordering it displays is not valid. The **showq** command
displays the actual job ordering under the Moab Workload Manager.  When
used without flags, this command displays all jobs in active, idle, and
non-queued states.
**Access**

By default, this command can be run by any user.  However, the ``-c``,
``-i``, and ``-r`` flags can only be used by level 1, 2, or 3 Moab
administrators.
**Flags**

Flag
Description
`**-b** <>`__
display blocked jobs only
`**-c** <>`__
display details about recently
`completed <../3.2environment.html#jobstates>`__ jobs (see
`example <#completedexample>`__,
`JOBCPURGETIME <../a.fparameters.html#jobcpurgetime>`__).
`**-g** <>`__
display grid job and system id's for all jobs.
`**-i** <>`__
display extended details about idle jobs. (see
`example <#idleexample>`__)
`**-l** <>`__
display local/remote view. For use in a
`Grid <../17.0peertopeer.html>`__ environment, displays job usage of
both local and remote compute resources.
`**-p** <>`__
display only jobs assigned to the specified partition.
`**-r** <>`__
display extended details about active (running) jobs. (see
`example <#activeexample>`__)
`**-R** <>`__
display only jobs which overlap the specified reservation.
`**-v** <>`__
Display local and full resource manager job IDs as well as partitions.
If specified with the '**-i**' option, will display job reservation
time. .
`**-w** <>`__
display only jobs associated with the specified constraint. Valid
constraints include **user**, **group**, **acct**, **class**, and
**qos**. (see `showq -w <#whereexample>`__ example.)
**Details**

   Beyond job information, the **showq** command will also report if the
scheduler is stopped or paused or if a system reservation is in place.
Further, the showq command will also report public system messages.
**Examples**

-  `Example 1 <#defaultexample>`__: Default Report
-  `Example 2 <#activeexample>`__: Detailed Active/Running Job Report
-  `Example 3 <#idleexample>`__: Detailed Eligible/Idle Job Report
-  `Example 4 <#completedexample>`__: Detailed Completed Job Report
-  `Example 5 <#whereexample>`__: Filtered Job Report

**\ Example 1: Default Report**

   The output of this command is divided into three parts,
`Active <#active>`__ Jobs, `Eligible <#eligible>`__ Jobs, and
`Blocked <#blocked>`__ Jobs.


::

    > showq
     
    active jobs------------------------
    JOBID              USERNAME      STATE  PROC   REMAINING            STARTTIME
     
    12941               sartois    Running    25     2:44:11  Thu Sep  1 15:02:50
    12954                tgates    Running     4     2:57:33  Thu Sep  1 15:02:52
    12944                 eval1    Running    16     6:37:31  Thu Sep  1 15:02:50
    12946                tgates    Running     2  1:05:57:31  Thu Sep  1 15:02:50
     
    4 active jobs             47 of 48 processors active (97.92%)
                              32 of 32 nodes active      (100.00%)
     
    eligible jobs----------------------
    JOBID              USERNAME      STATE  PROC     WCLIMIT            QUEUETIME
     
    12956              cfosdyke       Idle    32     6:40:00  Thu Sep  1 15:02:50
    12969              cfosdyke       Idle     4     6:40:00  Thu Sep  1 15:03:23
    12939                 eval1       Idle    16     3:00:00  Thu Sep  1 15:02:50
    12940               mwillis       Idle     2     3:00:00  Thu Sep  1 15:02:50
    12947               mwillis       Idle     2     3:00:00  Thu Sep  1 15:02:50
    12949                 eval1       Idle     2     3:00:00  Thu Sep  1 15:02:50
    12953                tgates       Idle    10     4:26:40  Thu Sep  1 15:02:50
    12955                 eval1       Idle     2     4:26:40  Thu Sep  1 15:02:50
    12957                tgates       Idle    16     3:00:00  Thu Sep  1 15:02:50
    12963                 eval1       Idle    16  1:06:00:00  Thu Sep  1 15:02:52
    12964                tgates       Idle    16  1:00:00:00  Thu Sep  1 15:02:52
    12937               allendr       Idle     9  1:00:00:00  Thu Sep  1 15:02:50
    12962                aacker       Idle     6    00:26:40  Thu Sep  1 15:02:50
    12968               tamaker       Idle     1     4:26:40  Thu Sep  1 15:02:52
     
    14 eligible jobs
     
    blocked jobs-----------------------
    JOBID              USERNAME      STATE  PROC     WCLIMIT            QUEUETIME
     
     
    0 blocked jobs
     
    Total jobs:  18


The fields are as follows:

+----------------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Column                           | Description                                                                                                                                                                                    |
+==================================+================================================================================================================================================================================================+
| \ **JOBID**               | job identifier.                                                                                                                                                                                |
+----------------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| \ **USERNAME**            | `User <../3.5credoverview.html#user>`__ owning job.                                                                                                                                            |
+----------------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| \ **STATE**               | `Job State <../3.2environment.html#jobstates>`__.  Current batch state of the job.                                                                                                             |
+----------------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| \ **PROC**                | Number of processors being used by the job.                                                                                                                                                    |
+----------------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| \ **REMAINING/WCLIMIT**   | For active jobs, the time the job has until it has reached its wall clock limit or for idle/blocked jobs, the amount of time requested by the job. Time specified in [DD:]HH:MM:SS notation.   |
+----------------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| \ **STARTTIME**           | Time job started running.                                                                                                                                                                      |
+----------------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

\ **Active Jobs**

Active jobs are those that are
`Running <../3.2environment.html#jobstates>`__ or
`Starting <../3.2environment.html#jobstates>`__ and consuming resources.
Displayed are the job id\*, the job's owner, and the job state.  Also
displayed are the number of processors allocated to the job, the amount
of time remaining until the job completes (given in HH:MM:SS notation),
and the time the job started. All active jobs are sorted in "Earliest
Completion Time First" order.

|Note|
\*Job id's may be marked with a single character to to specify the
following conditions:

Character
Description
\_ (underbar)
job violates usage limit
\* (asterisk)
job is backfilled AND is preemptible
+ (plus)
job is backfilled AND is NOT preemptible
- (hyphen)
job is NOT backfilled AND is preemptible

+----------+---------------------------------------------------------------------------------+
| |Note|   | Detailed active job information can be obtained using the '`-r <#r>`__' flag.   |
+----------+---------------------------------------------------------------------------------+

\ **Eligible Jobs**

Eligible Jobs are those that are queued and eligible to be scheduled.
They are all in the Idle job state and do not violate any fairness
policies or have any job holds in place. The jobs in the Idle section
display the same information as the Active Jobs section except that the
wall clock CPULIMIT is specified rather than job time REMAINING, and job
QUEUETIME is displayed rather than job STARTTIME. The jobs in this
section are ordered by job priority. Jobs in this queue are considered
eligible for both scheduling and backfilling.

+----------+-----------------------------------------------------------------------------------+
| |Note|   | Detailed eligible job information can be obtained using the '`-i <#i>`__' flag.   |
+----------+-----------------------------------------------------------------------------------+

\ **Blocked Jobs**

Blocked jobs are those that are ineligible to be run or queued. Jobs
listed here could be in a number of states for the following reasons:

+------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| State            | Description                                                                                                                                                                                                                  |
+==================+==============================================================================================================================================================================================================================+
| **Idle**         | Job violates a fairness policy. Use ``diagnose -q`` for more information.                                                                                                                                                    |
+------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **UserHold**     | A *user* hold is in place.                                                                                                                                                                                                   |
+------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **SystemHold**   | An administrative or *system* hold is in place.                                                                                                                                                                              |
+------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **BatchHold**    | A scheduler *batch* hold is in place (used when the job cannot be run because the requested resources are not available in the system or because the resource manager has repeatedly failed in attempts to start the job).   |
+------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **Deferred**     | A scheduler *defer* hold is in place (a temporary hold used when a job has been unable to start after a specified number of attempts. This hold is automatically removed after a short period of time).                      |
+------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **NotQueued**    | Job is in the resource manager state NQ (indicating the job's controlling scheduling daemon in unavailable).                                                                                                                 |
+------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

A summary of the job queue's status is provided at the end of the
output.

**\ Example 2: Detailed Active/Running Job Report**


::

    > showq -r
     
    active jobs------------------------
    JOBID               S CCODE PAR  EFFIC  XFACTOR  Q      USER    GROUP        MHOST PROCS   REMAINING            STARTTIME
     
    12941               R     -   3 100.00      1.0  -   sartois   Arches       G5-014 25     2:43:31  Thu Sep  1 15:02:50
    12954               R     -   3 100.00      1.0 Hi    tgates   Arches       G5-016  4     2:56:54  Thu Sep  1 15:02:52
    12944               R     -   2 100.00      1.0 De     eval1  RedRock     P690-016 16     6:36:51  Thu Sep  1 15:02:50
    12946               R     -   3 100.00      1.0  -    tgates   Arches       G5-001  2  1:05:56:51  Thu Sep  1 15:02:50
     
    4 active jobs             47 of 48 processors active (97.92%)
                              32 of 32 nodes active      (100.00%)
     
    Total jobs:  4


The fields are as follows:

+-------------------------+----------------------------------------------------------------------------------------------------+
| Column                  | Description                                                                                        |
+=========================+====================================================================================================+
| \ **JOBID**      | Name of active job.                                                                                |
+-------------------------+----------------------------------------------------------------------------------------------------+
| **S**                   | `Job State <../3.2environment.html#jobstates>`__. Either "R" for Running or "S" for Starting.      |
+-------------------------+----------------------------------------------------------------------------------------------------+
| **PAR**                 | Partition in which job is running.                                                                 |
+-------------------------+----------------------------------------------------------------------------------------------------+
| **EFFIC**               | CPU efficiency of job.                                                                             |
+-------------------------+----------------------------------------------------------------------------------------------------+
| **XFACTOR**             | Current expansion factor of job, where XFactor = (QueueTime + WallClockLimit) / WallClockLimit     |
+-------------------------+----------------------------------------------------------------------------------------------------+
| **Q**                   | Quality Of Service specified for job.                                                              |
+-------------------------+----------------------------------------------------------------------------------------------------+
| \ **USERNAME**   | User owning job.                                                                                   |
+-------------------------+----------------------------------------------------------------------------------------------------+
| **GROUP**               | Primary group of job owner.                                                                        |
+-------------------------+----------------------------------------------------------------------------------------------------+
| **MHOST**               | Master Host running primary task of job.                                                           |
+-------------------------+----------------------------------------------------------------------------------------------------+
| **PROC**                | Number of processors being used by the job.                                                        |
+-------------------------+----------------------------------------------------------------------------------------------------+
| **REMAINING**           | Time the job has until it has reached its wall clock limit. Time specified in HH:MM:SS notation.   |
+-------------------------+----------------------------------------------------------------------------------------------------+
| **STARTTIME**           | Time job started running.                                                                          |
+-------------------------+----------------------------------------------------------------------------------------------------+

| After displaying the running jobs, a summary is provided indicating
  the number of jobs, the number of allocated processors, and the system
  utilization.

+-----------------+----------------------------------------------------------------------------------------------------------------+
| Column          | Description                                                                                                    |
+=================+================================================================================================================+
| **JobName**     | Name of active job.                                                                                            |
+-----------------+----------------------------------------------------------------------------------------------------------------+
| **S**           | Job State. Either "R" for Running or "S" for Starting.                                                         |
+-----------------+----------------------------------------------------------------------------------------------------------------+
| **CCode**       | Completion Code. The return/completion code given when a job completes. (Only applicable to completed jobs.)   |
+-----------------+----------------------------------------------------------------------------------------------------------------+
| **Par**         | Partition in which job is running.                                                                             |
+-----------------+----------------------------------------------------------------------------------------------------------------+
| **Effic**       | CPU efficiency of job.                                                                                         |
+-----------------+----------------------------------------------------------------------------------------------------------------+
| **XFactor**     | Current expansion factor of job, where XFactor = (QueueTime + WallClockLimit) / WallClockLimit                 |
+-----------------+----------------------------------------------------------------------------------------------------------------+
| **Q**           | Quality Of Service specified for job.                                                                          |
+-----------------+----------------------------------------------------------------------------------------------------------------+
| **User**        | User owning job.                                                                                               |
+-----------------+----------------------------------------------------------------------------------------------------------------+
| **Group**       | Primary group of job owner.                                                                                    |
+-----------------+----------------------------------------------------------------------------------------------------------------+
| **Nodes**       | Number of processors being used by the job.                                                                    |
+-----------------+----------------------------------------------------------------------------------------------------------------+
| **Remaining**   | Time the job has until it has reached its wall clock limit. Time specified in HH:MM:SS notation.               |
+-----------------+----------------------------------------------------------------------------------------------------------------+
| **StartTime**   | Time job started running.                                                                                      |
+-----------------+----------------------------------------------------------------------------------------------------------------+


::

    > showq -i
     
    eligible jobs----------------------
    JOBID                 PRIORITY  XFACTOR  Q      USER    GROUP  PROCS     WCLIMIT     CLASS      SYSTEMQUEUETIME
     
    12956*                      20      1.0  -  cfosdyke  RedRock     32     6:40:00     batch  Thu Sep  1 15:02:50
    12969*                      19      1.0  -  cfosdyke  RedRock      4     6:40:00     batch  Thu Sep  1 15:03:23
    12939                       16      1.0  -     eval1  RedRock     16     3:00:00     batch  Thu Sep  1 15:02:50
    12940                       16      1.0  -   mwillis   Arches      2     3:00:00     batch  Thu Sep  1 15:02:50
    12947                       16      1.0  -   mwillis   Arches      2     3:00:00     batch  Thu Sep  1 15:02:50
    12949                       16      1.0  -     eval1  RedRock      2     3:00:00     batch  Thu Sep  1 15:02:50
    12953                       16      1.0  -    tgates   Arches     10     4:26:40     batch  Thu Sep  1 15:02:50
    12955                       16      1.0  -     eval1  RedRock      2     4:26:40     batch  Thu Sep  1 15:02:50
    12957                       16      1.0  -    tgates   Arches     16     3:00:00     batch  Thu Sep  1 15:02:50
    12963                       16      1.0  -     eval1  RedRock     16  1:06:00:00     batch  Thu Sep  1 15:02:52
    12964                       16      1.0  -    tgates   Arches     16  1:00:00:00     batch  Thu Sep  1 15:02:52
    12937                        1      1.0  -   allendr  RedRock      9  1:00:00:00     batch  Thu Sep  1 15:02:50
    12962                        1      1.2  -    aacker  RedRock      6    00:26:40     batch  Thu Sep  1 15:02:50
    12968                        1      1.0  -   tamaker  RedRock      1     4:26:40     batch  Thu Sep  1 15:02:52
     
    14 eligible jobs
     
    Total jobs:  14


The fields are as follows:

+-----------------------+--------------------------------------------------------------------------------------------------+
| Column                | Description                                                                                      |
+=======================+==================================================================================================+
| **JOBID**             | Name of job.                                                                                     |
+-----------------------+--------------------------------------------------------------------------------------------------+
| **PRIORITY**          | Calculated job priority.                                                                         |
+-----------------------+--------------------------------------------------------------------------------------------------+
| **XFACTOR**           | Current expansion factor of job, where XFactor = (QueueTime + WallClockLimit) / WallClockLimit   |
+-----------------------+--------------------------------------------------------------------------------------------------+
| **Q**                 | Quality Of Service specified for job.                                                            |
+-----------------------+--------------------------------------------------------------------------------------------------+
| **USER**              | `User <../3.5credoverview.html#user>`__ owning job.                                              |
+-----------------------+--------------------------------------------------------------------------------------------------+
| **GROUP**             | Primary `group <../3.5credoverview.html#group>`__ of job owner.                                  |
+-----------------------+--------------------------------------------------------------------------------------------------+
| **PROCS**             | Minimum number of processors required to run job.                                                |
+-----------------------+--------------------------------------------------------------------------------------------------+
| **WCLIMIT**           | Wall clock limit specified for job. Time specified in HH:MM:SS notation.                         |
+-----------------------+--------------------------------------------------------------------------------------------------+
| **CLASS**             | `Class <../3.5credoverview.html#class>`__ requested by job.                                      |
+-----------------------+--------------------------------------------------------------------------------------------------+
| **SYSTEMQUEUETIME**   | Time job was admitted into the system queue.                                                     |
+-----------------------+--------------------------------------------------------------------------------------------------+

+----------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| |Note|   | An asterisk at the end of a job (job 12956\* in this example) indicates that the job has a job `reservation <7.1advancereservations.html>`__ created for it. The details of this reservation can be displayed using the `checkjob <checkjob.html>`__ command.   |
+----------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

**\ Example 4: Detailed Completed Job Report**


::

    > showq -c

    completed jobs------------------------
    JOBID               S CCODE  PAR  EFFIC  XFACTOR  Q  USERNAME    GROUP        MHOST PROC    WALLTIME            STARTTIME

    13098               C     0  bas  93.17      1.0  -   sartois   Arches       G5-014    25    2:43:31  Thu Sep  1 15:02:50
    13102               C     0  bas  99.55      2.2 Hi    tgates   Arches       G5-016     4    2:56:54  Thu Sep  1 15:02:52
    13103               C     2  tes  99.30      2.9 De     eval1  RedRock     P690-016    16    6:36:51  Thu Sep  1 15:02:50
    13115               C     0  tes  97.04      1.0  -    tgates   Arches       G5-001     2 1:05:56:51  Thu Sep  1 15:02:50

    3 completed jobs


The fields are as follows:

+-------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Column                  | Description                                                                                                                                                                  |
+=========================+==============================================================================================================================================================================+
| \ **JOBID**      | job id for completed job.                                                                                                                                                    |
+-------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| \ **S**          | `Job State <../3.2environment.html#jobstates>`__. Either "C" for `Completed <../3.2environment.html#completed>`__ or "V" for `Vacated <../3.2environment.html#vacated>`__.   |
+-------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| \ **CCODE**      | Completion code reported by the job.                                                                                                                                         |
+-------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **PAR**                 | Partition in which job ran.                                                                                                                                                  |
+-------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **EFFIC**               | CPU efficiency of job.                                                                                                                                                       |
+-------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **XFACTOR**             | Expansion factor of job, where XFactor = (QueueTime + WallClockLimit) / WallClockLimit                                                                                       |
+-------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **Q**                   | `Quality of Service <../7.3qos.html>`__ specified for job.                                                                                                                   |
+-------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| \ **USERNAME**   | `User <../3.5credoverview.html#user>`__ owning job.                                                                                                                          |
+-------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **GROUP**               | Primary `group <../3.5credoverview.html#group>`__ of job owner.                                                                                                              |
+-------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **MHOST**               | Master Host which ran the primary task of job.                                                                                                                               |
+-------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **PROC**                | Number of processors being used by the job.                                                                                                                                  |
+-------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| \ **WALLTIME**   | Wallclock time used by the job.  Time specified in [DD:]HH:MM:SS notation.                                                                                                   |
+-------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **STARTTIME**           | Time job started running.                                                                                                                                                    |
+-------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

After displaying the active jobs, a summary is provided indicating the
number of jobs, the number of allocated processors, and the system
utilization.

+----------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| |Note|   | If the `DISPLAYFLAGS <../a.fparameters.html#displayflags>`__ parameter is set to **ACCOUNTCENTRIC**, job group information will be replaced with job account information.   |
+----------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

**\ Example 5: Filtered Job Report**

   Show only jobs associated with user ``john`` and class ``benchmark``


::

    > showq -w class=benchmark -w user=john

    ...


**See Also**

-  `Moab Client Installation <../2.2installation.html#client>`__ -
   explains how to distribute this command to client nodes
-  `showbf <showbf.html>`__ - command to display resource availability.
-  `mdiag -j <mdiag.html>`__ - command to display detailed job
   diagnostics.
-  `checkjob <checkjob.html>`__ - command to check the status of a
   particular job.
-  `JOBCPURGETIME <../a.fparameters.html#jobcpurgetime>`__ - parameter
   to adjust the duration of time Moab preserves information about
   completed jobs
-  `DISPLAYFLAGS <../a.fparameters.html#displayflags>`__ - parameter to
   control what job information is displayed

