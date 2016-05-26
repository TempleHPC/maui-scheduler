

.. rubric:: showgrid
   :name: showgrid

**showgrid *STATISTICTYPE* [-h]**

**Purpose**

Shows table of various scheduler statistics.

**Permissions**

This command can be run by any Maui Scheduler Administrator.

**Parameters**

*STATISTICTYPE*
Values for this parameter:
AVGBYPASS
Average bypass count. Includes summary of job-weighted expansion bypass
and total samples.
AVGQTIME
Average queue time. Includes summary of job-weighted queue time and
total samples.
AVGXFACTOR
Average expansion factor. Includes summary of job-weighted expansion
factor, node-weighted expansion factor, node-second-weighted expansion
factor, and total number of samples.
BFCOUNT
Number of jobs backfilled. Includes summary of job-weighted backfill job
percent and total samples.
BFNHRUN
Number of node-hours backfilled. Includes summary of job-weighted
backfill node-second percentage and total samples.
JOBCOUNT
Number of jobs. Includes summary of total jobs and total samples.
JOBEFFICIENCY
Job efficiency. Includes summary of job-weighted job efficiency percent
and total samples.
MAXBYPASS
Maximum bypass count. Includes summary of overall maximum bypass and
total samples.
MAXXFACTOR
Maximum expansion factor. Includes summary of overall maximum expansion
factor and total samples.
NHREQUEST
Node-hours requested. Includes summary of total node-hours requested and
total samples.
NHRUN
Node-hours run. Includes summary of total node-hours run and total
samples.
QOSDELIVERED
Quality of service delivered. Includes summary of job-weighted quality
of service success rate and total samples.
WCACCURACY
Wall clock accuracy. Includes summary of overall wall clock accuracy and
total samples.

    **NOTE**
    The *STATISTICTYPE* parameter value must be entered in uppercase
    characters.

**Flags**

+------+--------------------------+
| -h   | Help for this command.   |
+------+--------------------------+

**Description**

This command displays a table of the selected Maui Scheduler statistics,
such as expansion factor, bypass count, jobs, node-hours, wall clock
accuracy, and backfill information.

**Example**

``% showgrid AVGXFACTOR``

::

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

The ``showgrid`` command returns a table with data for the specified
*STASTICTYPE* parameter. The left-most column shows the maximum number
of nodes required by the jobs shown in the other columns. The column
heads indicate the maximum wall clock time (in HH:MM:SS notation)
requested by the jobs shown in the columns. The data returned in the
table varies by the *STATISTICTYPE* requested. For table entries with
one number, it is of the data requested. For table entries with two
numbers, the left number is the data requested and the right number is
the number of jobs used to calculate the average. Table entries that
contain only dashes (-------) indicate no job has completed that matches
the profile associated for this inquiry. The bottom row shows the totals
for each column. Following each table is a summary, which varies by the
*STATISTICTYPE* requested.

This particular example shows the average expansion factor grid. Each
table entry indicates two pieces of information -- the average expansion
factor for all jobs that meet this slot's profile and the number of jobs
that were used to calculate this average. For example, the XFactors of
two jobs were averaged to obtain an average XFactor of 1.24 for jobs
requiring over 2 hours 8 minutes, but not more than 4 hours 16 minutes
and between 5 and 8 nodes. Totals along the bottom provide overall
XFactor averages weighted by job, node, and node-seconds.

**Related Commands**

None.

**Default File Location**

``/u/loadl/maui/bin/showgrid``

**Notes**

None.

© Copyright 1998, Maui High Performance Computing Center. All rights
reserved.

