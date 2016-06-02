
.. rubric:: showstats -f
   :name: showstats--f

**(showstats -f)**
**Synopsis**

::

    showstats -f statistictype

**Overview**

Shows table of various scheduler statistics.
This command displays a table of the selected Moab Scheduler statistics,
such as expansion factor, bypass count, jobs, proc-hours, wall clock
accuracy, and backfill information.

**Access**

This command can be run by any Moab Scheduler Administrator.
**Parameters**

+-----------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Parameter                   | Description                                                                                                                                                                                |
+=============================+============================================================================================================================================================================================+
| **AVGBYPASS**               | Average bypass count. Includes summary of job-weighted expansion bypass and total samples.                                                                                                 |
+-----------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| \ **AVGQTIME**       | Average queue time. Includes summary of job-weighted queue time and total samples.                                                                                                         |
+-----------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **AVGXFACTOR**              | Average expansion factor. Includes summary of job-weighted expansion factor, processor-weighted expansion factor, processor-hour-weighted expansion factor, and total number of samples.   |
+-----------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **BFCOUNT**                 | Number of jobs backfilled. Includes summary of job-weighted backfill job percent and total samples.                                                                                        |
+-----------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **BFPHRUN**                 | Number of proc-hours backfilled. Includes summary of job-weighted backfill proc-hour percentage and total samples.                                                                         |
+-----------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| \ **ESTSTARTTIME**   | Job start time estimate for jobs meeting specified processor/duration criteria. This estimate is based on the `reservation start time analysis <showstart.html#rsv>`__ algorithm.          |
+-----------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **JOBCOUNT**                | Number of jobs. Includes summary of total jobs and total samples.                                                                                                                          |
+-----------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **JOBEFFICIENCY**           | Job efficiency. Includes summary of job-weighted job efficiency percent and total samples.                                                                                                 |
+-----------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **MAXBYPASS**               | Maximum bypass count. Includes summary of overall maximum bypass and total samples.                                                                                                        |
+-----------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **MAXXFACTOR**              | Maximum expansion factor. Includes summary of overall maximum expansion factor and total samples.                                                                                          |
+-----------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **PHREQUEST**               | proc-hours requested. Includes summary of total proc-hours requested and total samples.                                                                                                    |
+-----------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **PHRUN**                   | proc-hours run. Includes summary of total proc-hours run and total samples.                                                                                                                |
+-----------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **QOSDELIVERED**            | Quality of service delivered. Includes summary of job-weighted quality of service success rate and total samples.                                                                          |
+-----------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **WCACCURACY**              | Wall clock accuracy. Includes summary of overall wall clock accuracy and total samples.                                                                                                    |
+-----------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

**Example 1**


::

    > showstats -f AVGXFACTOR

    Average XFactor Grid

    [ NODES ][ 00:02:00 ][ 00:04:00 ][ 00:08:00 ][ 00:16:00 ][ 00:32:00 ][ 01:04:00 ][ 02:08:00 ][ 04:16:00 ][ 08:32:00 ][ 17:04:00 ][ 34:08:00 ][   TOTAL  ]
    [    1  ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ]
    [    2  ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ]
    [    4  ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][  1.00   1][ -------- ][  1.12   2][ -------- ][ -------- ][  1.10   3]
    [    8  ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][  1.00   2][  1.24   2][ -------- ][ -------- ][ -------- ][  1.15   4]
    [   16  ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][  1.01   2][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][  1.01   2]
    [   32  ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ]
    [   64  ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ]
    [  128  ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ]
    [  256  ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ]
    [ T TOT ][ -------- ][ -------- ][ -------- ][ -------- ][ -------- ][  1.01   2][  1.00   3][  1.24   2][  1.12   2][ -------- ][ -------- ]
    Job Weighted X Factor:     1.0888
    Node Weighted X Factor:    1.1147
    NS Weighted X Factor:      1.1900
    Total Samples:                9


The **showstats -f** command returns a table with data for the specified
*STATISTICTYPE* parameter. The left-most column shows the maximum number
of processors required by the jobs shown in the other columns. The
column headers indicate the maximum wall clock time (in HH:MM:SS
notation) requested by the jobs shown in the columns. The data returned
in the table varies by the *STATISTICTYPE* requested. For table entries
with one number, it is of the data requested. For table entries with two
numbers, the left number is the data requested and the right number is
the number of jobs used to calculate the average. Table entries that
contain only dashes (-------) indicate no job has completed that matches
the profile associated for this inquiry. The bottom row shows the totals
for each column. Following each table is a summary, which varies by the
*STATISTICTYPE* requested.

+----------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| |Note|   | The column and row break down can be adjusted using the `STATPROC\* <../a.fparameters.html#statprocmin>`__ and `STATTIME\* <../a.fparameters.html%0A#stattimemin>`__ parameters respectively.   |
+----------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

This particular example shows the average expansion factor grid. Each
table entry indicates two pieces of information -- the average expansion
factor for all jobs that meet this slot's profile and the number of jobs
that were used to calculate this average. For example, the XFactors of
two jobs were averaged to obtain an average XFactor of 1.24 for jobs
requiring over 2 hours 8 minutes, but not more than 4 hours 16 minutes
and between 5 and 8 processors. Totals along the bottom provide overall
XFactor averages weighted by job, processors, and processor-hours.

.. rubric:: See Also
   :name: see-also

-  `Moab Client Installation <../2.2installation.html#client>`__ -
   explains how to distribute this command to client nodes
-  `resetstats <resetstats.html>`__ command
-  `showstats <showstats.html>`__ command
-  `STATPROCMIN <../a.fparameters.html#statprocmin>`__ parameter
-  `STATPROCSTEPCOUNT <../a.fparameters.html#statprocstepcount>`__
   parameter
-  `STATPROCSTEPSIZE <../a.fparameters.html#statprocstepsize>`__
   parameter
-  `STATTIMEMIN <../a.fparameters.html#stattimemin>`__ parameter
-  `STATTIMESTEPCOUNT <../a.fparameters.html#stattimestepcount>`__
   parameter
-  `STATTIMESTEPSIZE <../a.fparameters.html#stattimestepsize>`__
   parameter

