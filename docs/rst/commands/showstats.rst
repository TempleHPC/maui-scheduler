
.. rubric:: showstats
   :name: showstats

**(Show Statistics)**
**Synopsis**

::

    showstats 
    showstats -a [accountid] [-v] [-t <TIMESPEC>]
    showstats -c [classid] [-v] [-t <TIMESPEC>]
    showstats -f
    showstats -g [groupid] [-v] [-t <TIMESPEC>]
    showstats -j [jobtemplate] [-t <TIMESPEC>]
    showstats -n [nodeid] [-t <TIMESPEC>]
    showstats -q [qosid] [-v] [-t <TIMESPEC>]
    showstats -s
    showstats -T [leafid | tree-level]
    showstats -u [userid] [-v] [-t <TIMESPEC>]

**Overview**

This command shows various accounting and resource usage statistics for
the system. Historical statistics cover the timeframe from the most
recent execution of the `resetstats <resetstats.html>`__ command.
**Access**

By default, this command can be run by any Moab level 1, 2, or 3
Administrator.
**Parameters**

+--------------------------------------+--------------------------------------+
| Flag                                 | Description                          |
+======================================+======================================+
| \ **-a** [<ACCOUNTID>]        | display account statistics           |
+--------------------------------------+--------------------------------------+
| \ **-c** [<CLASSID>]          | display class statistics             |
+--------------------------------------+--------------------------------------+
| \ **-f**                      | display **full** matrix statistics   |
|                                      | (see `showstats                      |
|                                      | -f <showstatsf.html>`__ for full     |
|                                      | details)                             |
+--------------------------------------+--------------------------------------+
| \ **-g** [<GROUPID>]          | display group statistics             |
+--------------------------------------+--------------------------------------+
| \ **-j** [<JOBTEMPLATE>]      | display template statistics          |
+--------------------------------------+--------------------------------------+
| \ **-n** [<NODEID>]           | display node statistics              |
|                                      | (`ENABLEPROFILING <../12.2nodeattrib |
|                                      | utes.html#enableprofiling>`__        |
|                                      | must be set)                         |
+--------------------------------------+--------------------------------------+
| \ **-q** [<QOSID>]            | display QoS statistics               |
+--------------------------------------+--------------------------------------+
| \ **-s**                      | display general scheduler statistics |
+--------------------------------------+--------------------------------------+
| \ **-t**                      | display statistical information from |
|                                      | the specified timeframe:             |
|                                      |                                      |
|                                      | .. raw:: html                        |
|                                      |                                      |
|                                      |    <div class="example">             |
|                                      |                                      |
|                                      | ::                                   |
|                                      |                                      |
|                                      |     <TIME>[,<TIME>]                  |
|                                      |             (ABSTIME: [HH[:MM[:SS]]] |
|                                      | [_MO[/DD[/YY]]] ie 14:30_06/20)      |
|                                      |             (RELTIME: +[[[DD:]HH:]MM |
|                                      | :]SS)                                |
|                                      |                                      |
|                                      | .. raw:: html                        |
|                                      |                                      |
|                                      |    </div>                            |
|                                      |                                      |
|                                      | +----------+------------------------ |
|                                      | ------------------------------------ |
|                                      | ------------------------------------ |
|                                      | ------------------------------------ |
|                                      | ------------------------------------ |
|                                      | ------------------------------------ |
|                                      | ------------------------------------ |
|                                      | ------------------------------------ |
|                                      | ------------------------------------ |
|                                      | --------------+                      |
|                                      | | |Note|   | Profiling must be enabl |
|                                      | ed for the credential type you want  |
|                                      | statistics for. See `Credential Stat |
|                                      | istics <../3.5credoverview.html#stat |
|                                      | istics>`__ for information on how to |
|                                      |  enable profiling. Also, **-t** is n |
|                                      | ot a stand-alone option. It must be  |
|                                      | used in conjuction with the **-a**,  |
|                                      | **-c**, **-g**, **-n**, **-q**, or * |
|                                      | *-u** flag.   |                      |
|                                      | +----------+------------------------ |
|                                      | ------------------------------------ |
|                                      | ------------------------------------ |
|                                      | ------------------------------------ |
|                                      | ------------------------------------ |
|                                      | ------------------------------------ |
|                                      | ------------------------------------ |
|                                      | ------------------------------------ |
|                                      | ------------------------------------ |
|                                      | --------------+                      |
+--------------------------------------+--------------------------------------+
| \ **-T**                      | display fairshare tree statistics    |
+--------------------------------------+--------------------------------------+
| \ **-u** [<USERID>]           | display user statistics              |
+--------------------------------------+--------------------------------------+
| \ **-v**                      | display verbose information          |
+--------------------------------------+--------------------------------------+

**Example 1**


::

    > showstats -a

    Account Statistics Initialized Tue Aug 26 14:32:39

                  |----- Running ------|--------------------------------- Completed ----------------------------------|
      Account     Jobs Procs ProcHours Jobs    %   PHReq    %    PHDed    %   FSTgt  AvgXF  MaxXF  AvgQH  Effic  WCAcc
       137651       16    92   1394.52  229  39.15 18486  45.26 7003.5  41.54 40.00   0.77   8.15   5.21  90.70  34.69
       462212       11    63    855.27   43   7.35  6028  14.76 3448.4  20.45  6.25   0.71   5.40   3.14  98.64  40.83
       462213        6    72    728.12   90  15.38  5974  14.63 3170.7  18.81  6.25   0.37   4.88   0.52  82.01  24.14
       005810        3    24    220.72   77  13.16  2537   6.21 1526.6   9.06 -----   1.53  14.81   0.42  98.73  28.40
       175436        0     0      0.00   12   2.05  6013  14.72  958.6   5.69  2.50   1.78   8.61   5.60  83.64  17.04
       000102        0     0      0.00    1   0.17    64   0.16    5.1   0.03 -----  10.85  10.85  10.77  27.90   7.40
       000023        0     0      0.00    1   0.17    12   0.03    0.2   0.00 -----   0.04   0.04   0.19  21.21   1.20


This example shows a statistical listing of all active accounts. The top
line (Account Statistics Initialized...) of the output indicates the
beginning of the timeframe covered by the displayed statistics.

The statistical output is divided into two categories, Running and
Completed. Running statistics include information about jobs that are
currently running. Completed statistics are compiled using historical
information from both running and completed jobs.

The fields are as follows:

+--------------------------------------+--------------------------------------+
| Column                               | Description                          |
+======================================+======================================+
| Account                              | Account Number.                      |
+--------------------------------------+--------------------------------------+
| Jobs                                 | Number of running jobs.              |
+--------------------------------------+--------------------------------------+
| Procs                                | Number of processors allocated to    |
|                                      | running jobs.                        |
+--------------------------------------+--------------------------------------+
| ProcHours                            | Number of proc-hours required to     |
|                                      | complete running jobs.               |
+--------------------------------------+--------------------------------------+
| Jobs\*                               | Number of jobs completed.            |
+--------------------------------------+--------------------------------------+
| %                                    | Percentage of total jobs that were   |
|                                      | completed by account.                |
+--------------------------------------+--------------------------------------+
| PHReq\*                              | Total proc-hours requested by        |
|                                      | completed jobs.                      |
+--------------------------------------+--------------------------------------+
| %                                    | Percentage of total proc-hours       |
|                                      | requested by completed jobs that     |
|                                      | were requested by account.           |
+--------------------------------------+--------------------------------------+
| PHDed                                | Total proc-hours dedicated to active |
|                                      | and completed jobs. The proc-hours   |
|                                      | dedicated to a job are calculated by |
|                                      | multiplying the number of allocated  |
|                                      | procs by the length of time the      |
|                                      | procs were allocated, regardless of  |
|                                      | the job's CPU usage.                 |
+--------------------------------------+--------------------------------------+
| %                                    | Percentage of total proc-hours       |
|                                      | dedicated that were dedicated by     |
|                                      | account.                             |
+--------------------------------------+--------------------------------------+
| FSTgt                                | Fairshare target. An account's       |
|                                      | fairshare target is specified in the |
|                                      | ``fs.cfg`` file. This value should   |
|                                      | be compared to the account's         |
|                                      | node-hour dedicated percentage to    |
|                                      | determine if the target is being     |
|                                      | met.                                 |
+--------------------------------------+--------------------------------------+
| AvgXF\*                              | Average expansion factor for jobs    |
|                                      | completed. A job's XFactor           |
|                                      | (expansion factor) is calculated by  |
|                                      | the following formula: (QueuedTime + |
|                                      | RunTime) / WallClockLimit.           |
+--------------------------------------+--------------------------------------+
| MaxXF\*                              | Highest expansion factor received by |
|                                      | jobs completed.                      |
+--------------------------------------+--------------------------------------+
| AvgQH\*                              | Average queue time (in hours) of     |
|                                      | jobs.                                |
+--------------------------------------+--------------------------------------+
| Effic                                | Average job efficiency. Job          |
|                                      | efficiency is calculated by dividing |
|                                      | the actual node-hours of CPU time    |
|                                      | used by the job by the node-hours    |
|                                      | allocated to the job.                |
+--------------------------------------+--------------------------------------+
| WCAcc\*                              | Average wall clock accuracy for jobs |
|                                      | completed. Wall clock accuracy is    |
|                                      | calculated by dividing a job's       |
|                                      | actual run time by its specified     |
|                                      | wall clock limit.                    |
|                                      |                                      |
|                                      | +----------+------------------------ |
|                                      | ------------------------------------ |
|                                      | ------------------------------------ |
|                                      | ------------------------------------ |
|                                      | +                                    |
|                                      | | |Note|   | A job's wallclock accur |
|                                      | acy is capped at 100% so even if a j |
|                                      | ob exceeds its requested walltime it |
|                                      |  will report an accuracy of 100%.    |
|                                      | |                                    |
|                                      | +----------+------------------------ |
|                                      | ------------------------------------ |
|                                      | ------------------------------------ |
|                                      | ------------------------------------ |
|                                      | +                                    |
+--------------------------------------+--------------------------------------+

+----------+------------------------------------------------------------------------------+
| |Note|   | \* These fields are empty until an account has completed at least one job.   |
+----------+------------------------------------------------------------------------------+

**Example 2**


::

    > showstats -g

    Group Statistics Initialized Tue Aug 26 14:32:39

                  |----- Running ------|--------------------------------- Completed ----------------------------------|
    GroupName  GID Jobs Procs ProcHours Jobs    %   PHReq    %    PHDed    %   FSTgt  AvgXF  MaxXF  AvgQH  Effic  WCAcc
         univ  214   16    92   1394.52  229  39.15 18486  45.26 7003.5  41.54 40.00   0.77   8.15   5.21  90.70  34.69
          daf  204   11    63    855.27   43   7.35  6028  14.76 3448.4  20.45  6.25   0.71   5.40   3.14  98.64  40.83
        dnavy  207    6    72    728.12   90  15.38  5974  14.63 3170.7  18.81  6.25   0.37   4.88   0.52  82.01  24.14
         govt  232    3    24    220.72   77  13.16  2537   6.21 1526.6   9.06 -----   1.53  14.81   0.42  98.73  28.40
          asp  227    0     0      0.00   12   2.05  6013  14.72  958.6   5.69  2.50   1.78   8.61   5.60  83.64  17.04
        derim  229    0     0      0.00   74  12.65   669   1.64  352.5   2.09 -----   0.50   1.93   0.51  96.03  32.60
       dchall  274    0     0      0.00    3   0.51   447   1.10  169.2   1.00 25.00   0.52   0.88   2.49  95.82  33.67
          nih  239    0     0      0.00   17   2.91   170   0.42  148.1   0.88 -----   0.95   1.83   0.14  97.59  84.31
        darmy  205    0     0      0.00   31   5.30   366   0.90   53.9   0.32  6.25   0.14   0.59   0.07  81.33  12.73
      systems   80    0     0      0.00    6   1.03    67   0.16   22.4   0.13 -----   4.07   8.49   1.23  28.68  37.34
          pdc  252    0     0      0.00    1   0.17    64   0.16    5.1   0.03 -----  10.85  10.85  10.77  27.90   7.40
        staff    1    0     0      0.00    1   0.17    12   0.03    0.2   0.00 -----   0.04   0.04   0.19  21.21   1.20


This example shows a statistical listing of all active groups. The top
line (Group Statistics Initialized...) of the output indicates the
beginning of the timeframe covered by the displayed statistics.

The statistical output is divided into two categories, Running and
Completed. Running statistics include information about jobs that are
currently running. Completed statistics are compiled using historical
information from both running and completed jobs.

The fields are as follows:

+--------------------------------------+--------------------------------------+
| Column                               | Description                          |
+======================================+======================================+
| GroupName                            | Name of group.                       |
+--------------------------------------+--------------------------------------+
| GID                                  | Group ID of group.                   |
+--------------------------------------+--------------------------------------+
| Jobs                                 | Number of running jobs.              |
+--------------------------------------+--------------------------------------+
| Procs                                | Number of procs allocated to running |
|                                      | jobs.                                |
+--------------------------------------+--------------------------------------+
| ProcHours                            | Number of proc-hours required to     |
|                                      | complete running jobs.               |
+--------------------------------------+--------------------------------------+
| Jobs\*                               | Number of jobs completed.            |
+--------------------------------------+--------------------------------------+
| %                                    | Percentage of total jobs that were   |
|                                      | completed by group.                  |
+--------------------------------------+--------------------------------------+
| PHReq\*                              | Total proc-hours requested by        |
|                                      | completed jobs.                      |
+--------------------------------------+--------------------------------------+
| %                                    | Percentage of total proc-hours       |
|                                      | requested by completed jobs that     |
|                                      | were requested by group.             |
+--------------------------------------+--------------------------------------+
| PHDed                                | Total proc-hours dedicated to active |
|                                      | and completed jobs. The proc-hours   |
|                                      | dedicated to a job are calculated by |
|                                      | multiplying the number of allocated  |
|                                      | procs by the length of time the      |
|                                      | procs were allocated, regardless of  |
|                                      | the job's CPU usage.                 |
+--------------------------------------+--------------------------------------+
| %                                    | Percentage of total proc-hours       |
|                                      | dedicated that were dedicated by     |
|                                      | group.                               |
+--------------------------------------+--------------------------------------+
| FSTgt                                | Fairshare target. A group's          |
|                                      | fairshare target is specified in the |
|                                      | ``fs.cfg`` file. This value should   |
|                                      | be compared to the group's node-hour |
|                                      | dedicated percentage to determine if |
|                                      | the target is being met.             |
+--------------------------------------+--------------------------------------+
| AvgXF\*                              | Average expansion factor for jobs    |
|                                      | completed. A job's XFactor           |
|                                      | (expansion factor) is calculated by  |
|                                      | the following formula: (QueuedTime + |
|                                      | RunTime) / WallClockLimit.           |
+--------------------------------------+--------------------------------------+
| MaxXF\*                              | Highest expansion factor received by |
|                                      | jobs completed.                      |
+--------------------------------------+--------------------------------------+
| AvgQH\*                              | Average queue time (in hours) of     |
|                                      | jobs.                                |
+--------------------------------------+--------------------------------------+
| Effic                                | Average job efficiency. Job          |
|                                      | efficiency is calculated by dividing |
|                                      | the actual node-hours of CPU time    |
|                                      | used by the job by the node-hours    |
|                                      | allocated to the job.                |
+--------------------------------------+--------------------------------------+
| WCAcc\*                              | Average wall clock accuracy for jobs |
|                                      | completed. Wall clock accuracy is    |
|                                      | calculated by dividing a job's       |
|                                      | actual run time by its specified     |
|                                      | wall clock limit.                    |
|                                      |                                      |
|                                      | +----------+------------------------ |
|                                      | ------------------------------------ |
|                                      | ------------------------------------ |
|                                      | ------------------------------------ |
|                                      | +                                    |
|                                      | | |Note|   | A job's wallclock accur |
|                                      | acy is capped at 100% so even if a j |
|                                      | ob exceeds its requested walltime it |
|                                      |  will report an accuracy of 100%.    |
|                                      | |                                    |
|                                      | +----------+------------------------ |
|                                      | ------------------------------------ |
|                                      | ------------------------------------ |
|                                      | ------------------------------------ |
|                                      | +                                    |
+--------------------------------------+--------------------------------------+

+----------+---------------------------------------------------------------------------+
| |Note|   | \* These fields are empty until a group has completed at least one job.   |
+----------+---------------------------------------------------------------------------+

**Example 3**


::

    > showstats -n
    node stats from Mon Jul 10 00:00:00 to Mon Jul 10 16:30:00

    node      CfgMem MinMem MaxMem AvgMem | CfgProcs MinLoad MaxLoad AvgLoad
    node01     58368      0  21122   5841       32    0.00   32.76   27.62
    node02    122880      0  19466    220       30    0.00   33.98   29.54
    node03     18432      0   9533   2135       24    0.00   25.10   18.64
    node04     60440      0  17531   4468       32    0.00   30.55   24.61
    node05     13312      0   2597   1189        8    0.00    9.85    8.45
    node06     13312      0   3800   1112        8    0.00    8.66    5.27
    node07     13312      0   2179   1210        8    0.00    9.62    8.27
    node08     13312      0   3243   1995        8    0.00   11.71    8.02
    node09     13312      0   2287   1943        8    0.00   10.26    7.58
    node10     13312      0   2183   1505        8    0.00   13.12    9.28
    node11     13312      0   3269   2448        8    0.00    8.93    6.71
    node12     13312      0  10114   6900        8    0.00   13.13    8.44
    node13     13312      0   2616   2501        8    0.00    9.24    8.21
    node14     13312      0   3888    869        8    0.00    8.10    3.85
    node15     13312      0   3788    308        8    0.00    8.40    4.67
    node16     13312      0   4386   2191        7    0.00   18.37    8.36
    node17     13312      0   3158   1870        8    0.00    8.95    5.91
    node18     13312      0   5022   2397        8    0.00   19.25    8.19
    node19     13312      0   2437   1371        8    0.00    8.98    7.09
    node20     13312      0   4474   2486        8    0.00    8.51    7.11
    node21     13312      0   4111   2056        8    0.00    8.93    6.68
    node22     13312      0   5136   2313        8    0.00    8.61    5.75
    node23     13312      0   1850   1752        8    0.00    8.39    5.71
    node24     13312      0   3850   2539        8    0.00    8.94    7.80
    node25     13312      0   3789   3702        8    0.00   21.22   12.83
    node26     13312      0   3809   1653        8    0.00    9.34    4.91
    node27     13312      0   5637     70        4    0.00   17.97    2.46
    node28     13312      0   3076   2864        8    0.00   22.91   10.33


**Example 4**


::

    > showstats -v

    current scheduler time: Sat Aug 18 18:23:02 2007

    moab active for      00:00:01   started on Wed Dec 31 17:00:00
    statistics for iteration     0  initialized on Sat Aug 11 23:55:25

    Eligible/Idle Jobs:                 6/8      (75.000%)
    Active Jobs:                       13
    Successful/Completed Jobs:        167/167    (100.000%)
    Preempt Jobs:                       0
    Avg/Max QTime (Hours):           0.34/2.07
    Avg/Max XFactor:                 1.165/3.26
    Avg/Max Bypass:                  0.40/8.00

    Dedicated/Total ProcHours:      4.46K/4.47K  (99.789%)
    Preempt/Dedicated ProcHours:     0.00/4.46K  (0.000%)

    Current Active/Total Procs:        32/32     (100.0%)
    Current Active/Total Nodes:        16/16      (100.0%)

    Avg WallClock Accuracy:          64.919%
    Avg Job Proc Efficiency:         99.683%
    Min System Utilization:          87.323% (on iteration 46)
    Est/Avg Backlog:                02:14:06/03:02:567


This example shows a concise summary of the system scheduling state.
Note that ``showstats`` and ``showstats -s`` are equivalent.

The first line of output indicates the number of scheduling iterations
performed by the current scheduling process, followed by the time the
scheduler started. The second line indicates the amount of time the Moab
Scheduler has been scheduling in HH:MM:SS notation followed by the
statistics initialization time.

The fields are as follows:

+-----------------------+----------------------------------------------------------------------------------------------+
| Column                | Description                                                                                  |
+=======================+==============================================================================================+
| Active Jobs           | Number of jobs currently active (Running or Starting).                                       |
+-----------------------+----------------------------------------------------------------------------------------------+
| Eligible Jobs         | Number of jobs in the system queue (jobs that are considered when scheduling).               |
+-----------------------+----------------------------------------------------------------------------------------------+
| Idle Jobs             | Number of jobs both in and out of the system queue that are in the LoadLeveler Idle state.   |
+-----------------------+----------------------------------------------------------------------------------------------+
| Completed Jobs        | Number of jobs completed since statistics were initialized.                                  |
+-----------------------+----------------------------------------------------------------------------------------------+
| Successful Jobs       | Jobs that completed successfully without abnormal termination.                               |
+-----------------------+----------------------------------------------------------------------------------------------+
| XFactor               | Average expansion factor of all completed jobs.                                              |
+-----------------------+----------------------------------------------------------------------------------------------+
| Max XFactor           | Maximum expansion factor of completed jobs.                                                  |
+-----------------------+----------------------------------------------------------------------------------------------+
| Max Bypass            | Maximum bypass of completed jobs.                                                            |
+-----------------------+----------------------------------------------------------------------------------------------+
| Available ProcHours   | Total proc-hours available to the scheduler.                                                 |
+-----------------------+----------------------------------------------------------------------------------------------+
| Dedicated ProcHours   | Total proc-hours made available to jobs.                                                     |
+-----------------------+----------------------------------------------------------------------------------------------+
| Effic                 | Scheduling efficiency (DedicatedProcHours / Available ProcHours).                            |
+-----------------------+----------------------------------------------------------------------------------------------+
| Min Efficiency        | Minimum scheduling efficiency obtained since scheduler was started.                          |
+-----------------------+----------------------------------------------------------------------------------------------+
| Iteration             | Iteration on which the minimum scheduling efficiency occurred.                               |
+-----------------------+----------------------------------------------------------------------------------------------+
| Available Procs       | Number of procs currently available.                                                         |
+-----------------------+----------------------------------------------------------------------------------------------+
| Busy Procs            | Number of procs currently busy.                                                              |
+-----------------------+----------------------------------------------------------------------------------------------+
| Effic                 | Current system efficiency (BusyProcs/AvailableProcs).                                        |
+-----------------------+----------------------------------------------------------------------------------------------+
| WallClock Accuracy    | Average wall clock accuracy of completed jobs (job-weighted average).                        |
+-----------------------+----------------------------------------------------------------------------------------------+
| Job Efficiency        | Average job efficiency (UtilizedTime / DedicatedTime).                                       |
+-----------------------+----------------------------------------------------------------------------------------------+
| Est Backlog           | Estimated backlog of queued work in hours.                                                   |
+-----------------------+----------------------------------------------------------------------------------------------+
| Avg Backlog           | Average backlog of queued work in hours.                                                     |
+-----------------------+----------------------------------------------------------------------------------------------+

**Example 5**


::

    > showstats -u

    User Statistics Initialized Tue Aug 26 14:32:39

                  |----- Running ------|--------------------------------- Completed ----------------------------------|
     UserName  UID Jobs Procs ProcHours Jobs    %   PHReq    %    PHDed    %   FSTgt  AvgXF  MaxXF  AvgQH  Effic  WCAcc
      moorejt 2617    1    16     58.80    2   0.34   221   0.54 1896.6  11.25 -----   1.02   1.04   0.14  99.52 100.00
        zhong 1767    3    24    220.72   20   3.42  2306   5.65 1511.3   8.96 -----   0.71   0.96   0.49  99.37  67.48
          lui 2467    0     0      0.00   16   2.74  1970   4.82 1505.1   8.93 -----   1.02   6.33   0.25  98.96  57.72
        evans 3092    0     0      0.00   62  10.60  4960  12.14 1464.3   8.69   5.0   0.62   1.64   5.04  87.64  30.62
       wengel 2430    2    64    824.90    1   0.17   767   1.88  630.3   3.74 -----   0.18   0.18   4.26  99.63   0.40
        mukho 2961    2    16     71.06    6   1.03   776   1.90  563.5   3.34 -----   0.31   0.82   0.20  93.15  30.28
      jimenez 1449    1    16    302.29    2   0.34   768   1.88  458.3   2.72 -----   0.80   0.98   2.31  97.99  70.30
         neff 3194    0     0      0.00   74  12.65   669   1.64  352.5   2.09  10.0   0.50   1.93   0.51  96.03  32.60
       cholik 1303    0     0      0.00    2   0.34   552   1.35  281.9   1.67 -----   1.72   3.07  25.35  99.69  66.70
     jshoemak 2508    1    24    572.22    1   0.17   576   1.41  229.1   1.36 -----   0.55   0.55   3.74  99.20  39.20
         kudo 2324    1     8    163.35    6   1.03  1152   2.82  211.1   1.25 -----   0.12   0.34   1.54  96.77   5.67
       xztang 1835    1     8     18.99 ---- ------ ----- ------  176.3   1.05  10.0 ------ ------ ------  99.62 ------
       feller 1880    0     0      0.00   17   2.91   170   0.42  148.1   0.88 -----   0.95   1.83   0.14  97.59  84.31
        maxia 2936    0     0      0.00    1   0.17   191   0.47  129.1   0.77   7.5   0.88   0.88   4.49  99.84  69.10
     ktgnov71 2838    0     0      0.00    1   0.17   192   0.47   95.5   0.57 -----   0.53   0.53   0.34  90.07  51.20


This example shows a statistical listing of all active users. The top
line (User Statistics Initialized...) of the output indicates the
timeframe covered by the displayed statistics.

The statistical output is divided into two statistics categories,
Running and Completed. Running statistics include information about jobs
that are currently running. Completed statistics are compiled using
historical information from both running and completed jobs.

The fields are as follows:

+--------------------------------------+--------------------------------------+
| Column                               | Description                          |
+======================================+======================================+
| UserName                             | Name of user.                        |
+--------------------------------------+--------------------------------------+
| UID                                  | User ID of user.                     |
+--------------------------------------+--------------------------------------+
| Jobs                                 | Number of running jobs.              |
+--------------------------------------+--------------------------------------+
| Procs                                | Number of procs allocated to running |
|                                      | jobs.                                |
+--------------------------------------+--------------------------------------+
| ProcHours                            | Number of proc-hours required to     |
|                                      | complete running jobs.               |
+--------------------------------------+--------------------------------------+
| Jobs\*                               | Number of jobs completed.            |
+--------------------------------------+--------------------------------------+
| %                                    | Percentage of total jobs that were   |
|                                      | completed by user.                   |
+--------------------------------------+--------------------------------------+
| PHReq\*                              | Total proc-hours requested by        |
|                                      | completed jobs.                      |
+--------------------------------------+--------------------------------------+
| %                                    | Percentage of total proc-hours       |
|                                      | requested by completed jobs that     |
|                                      | were requested by user.              |
+--------------------------------------+--------------------------------------+
| PHDed                                | Total proc-hours dedicated to active |
|                                      | and completed jobs. The proc-hours   |
|                                      | dedicated to a job are calculated by |
|                                      | multiplying the number of allocated  |
|                                      | procs by the length of time the      |
|                                      | procs were allocated, regardless of  |
|                                      | the job's CPU usage.                 |
+--------------------------------------+--------------------------------------+
| %                                    | Percentage of total prochours        |
|                                      | dedicated that were dedicated by     |
|                                      | user.                                |
+--------------------------------------+--------------------------------------+
| FSTgt                                | Fairshare target. A user's fairshare |
|                                      | target is specified in the           |
|                                      | ``fs.cfg`` file. This value should   |
|                                      | be compared to the user's node-hour  |
|                                      | dedicated percentage to determine if |
|                                      | the target is being met.             |
+--------------------------------------+--------------------------------------+
| AvgXF\*                              | Average expansion factor for jobs    |
|                                      | completed. A job's XFactor           |
|                                      | (expansion factor) is calculated by  |
|                                      | the following formula: (QueuedTime + |
|                                      | RunTime) / WallClockLimit.           |
+--------------------------------------+--------------------------------------+
| MaxXF\*                              | Highest expansion factor received by |
|                                      | jobs completed.                      |
+--------------------------------------+--------------------------------------+
| AvgQH\*                              | Average queue time (in hours) of     |
|                                      | jobs.                                |
+--------------------------------------+--------------------------------------+
| Effic                                | Average job efficiency. Job          |
|                                      | efficiency is calculated by dividing |
|                                      | the actual node-hours of CPU time    |
|                                      | used by the job by the node-hours    |
|                                      | allocated to the job.                |
+--------------------------------------+--------------------------------------+
| WCAcc\*                              | Average wallclock accuracy for jobs  |
|                                      | completed. Wallclock accuracy is     |
|                                      | calculated by dividing a job's       |
|                                      | actual run time by its specified     |
|                                      | wallclock limit.                     |
|                                      |                                      |
|                                      | +----------+------------------------ |
|                                      | ------------------------------------ |
|                                      | ------------------------------------ |
|                                      | ------------------------------------ |
|                                      | +                                    |
|                                      | | |Note|   | A job's wallclock accur |
|                                      | acy is capped at 100% so even if a j |
|                                      | ob exceeds its requested walltime it |
|                                      |  will report an accuracy of 100%.    |
|                                      | |                                    |
|                                      | +----------+------------------------ |
|                                      | ------------------------------------ |
|                                      | ------------------------------------ |
|                                      | ------------------------------------ |
|                                      | +                                    |
+--------------------------------------+--------------------------------------+

+----------+--------------------------------------------------------------------------+
| |Note|   | \* These fields are empty until a user has completed at least one job.   |
+----------+--------------------------------------------------------------------------+

**Example 6**


::

    > showstats -T
    statistics initialized Mon Jul 10 15:29:41

                  |-------- Active ---------|---------------------------------- Completed -----------------------------------|
    user           Jobs Procs ProcHours  Mem Jobs    %    PHReq    %    PHDed    %   FSTgt   AvgXF  MaxXF  AvgQH  Effic  WCAcc
    root              0     0      0.00    0   56 100.00  2.47K 100.00  1.58K  48.87 -----    1.22   0.00   0.24 100.00  58.84
     l1.1             0     0      0.00    0   25  44.64 845.77  34.31 730.25  22.54 -----    1.97   0.00   0.20 100.00  65.50
      Administrati    0     0      0.00    0   10  17.86 433.57  17.59 197.17   6.09 -----    3.67   0.00   0.25 100.00  62.74
      Engineering     0     0      0.00    0   15  26.79 412.20  16.72 533.08  16.45 -----    0.83   0.00   0.17 100.00  67.35
     l1.2             0     0      0.00    0   31  55.36  1.62K  65.69 853.00  26.33 -----    0.62   0.00   0.27 100.00  53.46
      Shared          0     0      0.00    0    3   5.36  97.17   3.94  44.92   1.39 -----    0.58   0.00   0.56 100.00  31.73
      Test            0     0      0.00    0    3   5.36  14.44   0.59  14.58   0.45 -----    0.43   0.00   0.17 100.00  30.57
      Research        0     0      0.00    0   25  44.64  1.51K  61.16 793.50  24.49 -----    0.65   0.00   0.24 100.00  58.82


    > showstats -T 2
    statistics initialized Mon Jul 10 15:29:41

                  |-------- Active ---------|---------------------------------- Completed -----------------------------------|
    user           Jobs Procs ProcHours  Mem Jobs    %    PHReq    %    PHDed    %   FSTgt   AvgXF  MaxXF  AvgQH  Effic  WCAcc
      Test            0     0      0.00    0   22   4.99 271.27   0.55 167.42   0.19 -----    3.86   0.00   2.89 100.00  60.76
      Shared          0     0      0.00    0   59  13.38 12.30K  24.75  4.46K   5.16 -----    6.24   0.00  10.73 100.00  49.87
      Research        0     0      0.00    0  140  31.75  9.54K  19.19  5.40K   6.25 -----    2.84   0.00   5.52 100.00  57.86
      Administrati    0     0      0.00    0   84  19.05  7.94K  15.96  4.24K   4.91 -----    4.77   0.00   0.34 100.00  62.31
      Engineering     0     0      0.00    0  136  30.84 19.67K  39.56 28.77K  33.27 -----    3.01   0.00   3.66 100.00  63.70


    > showstats -T l1.1
    statistics initialized Mon Jul 10 15:29:41

                  |-------- Active ---------|---------------------------------- Completed -----------------------------------|
    user           Jobs Procs ProcHours  Mem Jobs    %    PHReq    %    PHDed    %   FSTgt   AvgXF  MaxXF  AvgQH  Effic  WCAcc
     l1.1             0     0      0.00    0  220  49.89 27.60K  55.52 33.01K  38.17 -----    3.68   0.00   2.39 100.00  63.17
      Administrati    0     0      0.00    0   84  19.05  7.94K  15.96  4.24K   4.91 -----    4.77   0.00   0.34 100.00  62.31
      Engineering     0     0      0.00    0  136  30.84 19.67K  39.56 28.77K  33.27 -----    3.01   0.00   3.66 100.00  63.70


.. rubric:: See Also
   :name: see-also

-  `Moab Client Installation <../2.2installation.html#client>`__ -
   explains how to distribute this command to client nodes
-  `resetstats <resetstats.html>`__ command - re-initialize statistics
-  `showstats -f <showstatsf.html>`__ command - display full matrix
   statistics

