
.. rubric:: mdiag -p
   :name: mdiag--p

**(Moab Priority Diagnostics)**
.. rubric:: Synopsis
   :name: synopsis

::

    mdiag -p [-t partition] [-v]

.. rubric:: Overview
   :name: overview

   The '**mdiag -p**' command is used to display *at a glance*
information about the job priority configuration and its effects on the
current eligible jobs.  The information presented by this command
includes priority weights, priority components, and the percentage
contribution of each component to the total job priority.
   The command hides information about priority components which have
been deactivated (ie, by setting the corresponding component priority
weight to 0).  For each displayed priority component, this command gives
a small amount of context sensitive information.  The following table
documents this information.  In all cases, the output is of the form
<PERCENT>(<CONTEXT INFO>) where <PERCENT> is the percentage contribution
of the associated priority component to the job's total priority.

**Note**: By default, this command only shows information for jobs which
are eligible for immediate execution.  Jobs which violate soft or hard
policies, or have holds, job dependencies, or other job constraints in
place will not be displayed.  If priority information is needed for any
of these jobs, use the '`-v <#verbose>`__ flag or the
`checkjob <checkjob.html>`__ command.

.. rubric:: Format
   :name: format

+--------------+--------------+--------------+--------------+--------------+--------------+
| **Flag**     | **Name**     | **Format**   | **Default**  | **Descriptio | **Example**  |
|              |              |              |              | n**          |              |
+--------------+--------------+--------------+--------------+--------------+--------------+
| -v           | \ **V | ---          | ---          | display      | mdiag -p -v  |
|              | ERBOSE**     |              |              | verbose      | ::           |
|              |              |              |              | priority     |              |
|              |              |              |              | information. |     > mdiag  |
|              |              |              |              |  If          | -p -v        |
|              |              |              |              | specified,   |              |
|              |              |              |              | display      | display      |
|              |              |              |              | priority     | priority     |
|              |              |              |              | breakdown    | summary      |
|              |              |              |              | information  | information  |
|              |              |              |              | for blocked, | for blocked, |
|              |              |              |              | eligible,    | eligible,    |
|              |              |              |              | and active   | and active   |
|              |              |              |              | jobs.        | jobs         |
|              |              |              |              |  **Note**:   |              |
|              |              |              |              | By default,  |              |
|              |              |              |              | only         |              |
|              |              |              |              | information  |              |
|              |              |              |              | for eligible |              |
|              |              |              |              | jobs is      |              |
|              |              |              |              | displayed.   |              |
+--------------+--------------+--------------+--------------+--------------+--------------+

.. rubric:: Output
   :name: output

+--------------------------+--------------------------------------------+-----------------------------------------------------------------------------------------+
| **Priority Component**   | **Format**                                 | **Description**                                                                         |
+--------------------------+--------------------------------------------+-----------------------------------------------------------------------------------------+
| Target                   | <PERCENT>()                                |                                                                                         |
+--------------------------+--------------------------------------------+-----------------------------------------------------------------------------------------+
| QOS                      | <PERCENT>(<QOS>:<QOSPRI>)                  | QOS:         QOS associated with job                                                    |
|                          |                                            | QOSPRI:  Priority assigned to the QOS                                                   |
+--------------------------+--------------------------------------------+-----------------------------------------------------------------------------------------+
| FairShare                | <PERCENT>(<USR>:<GRP>:<ACC>:<QOS>:<CLS>)   | USR:          user fs usage - user fs target                                            |
|                          |                                            | GRP:          group fs usage - group fs target                                          |
|                          |                                            | ACC:         account fs usage - account fs target                                       |
|                          |                                            | QOS:         QOS fs usage - QOS fs target                                               |
|                          |                                            | CLS:         class fs usage - class fs target                                           |
+--------------------------+--------------------------------------------+-----------------------------------------------------------------------------------------+
| Service                  | <PERCENT>(<QT>:<XF>:<Byp>)                 | QTime:          job queue time which is applicable towards priority (in minutes)        |
|                          |                                            | XF:          current theoretical minimum XFactor is job were to start immediately       |
|                          |                                            | Byp:          number of times job was bypassed by lower priority jobs via backfill      |
+--------------------------+--------------------------------------------+-----------------------------------------------------------------------------------------+
| Resource                 | <PERCENT>(<NDE>:<PE>:<PRC>:<MEM>)          | NDE:         nodes requested by job                                                     |
|                          |                                            | PE:             Processor Equivalents as calculated by all resources requested by job   |
|                          |                                            | PRC:          processors requested by job                                               |
|                          |                                            | MEM:       real memory requested by job                                                 |
+--------------------------+--------------------------------------------+-----------------------------------------------------------------------------------------+

.. rubric:: Example 1
   :name: example-1

mdiag -p
::

    diagnosing job priority information (partition: ALL)
     
    Job                    PRIORITY*   Cred(  QOS)    FS(Accnt)  Serv(QTime)
                 Weights   --------       1(    1)     1(    1)     1(    1)
     
    13678                      1321*    7.6(100.0)   0.2(  2.7)  92.2(1218.)
    13698                       235*   42.6(100.0)   1.1(  2.7)  56.3(132.3)
    13019                      8699     0.6( 50.0)   0.3( 25.4)  99.1(8674.)
    13030                      8699     0.6( 50.0)   0.3( 25.4)  99.1(8674.)
    13099                      8537     0.6( 50.0)   0.3( 25.4)  99.1(8512.)
    13141                      8438     0.6( 50.0)   0.2( 17.6)  99.2(8370.)
    13146                      8428     0.6( 50.0)   0.2( 17.6)  99.2(8360.)
    13153                      8360     0.0(  1.0)   0.1( 11.6)  99.8(8347.)
    13177                      8216     0.0(  1.0)   0.1( 11.6)  99.8(8203.)
    13203                      8127     0.6( 50.0)   0.3( 25.4)  99.1(8102.)
    13211                      8098     0.0(  1.0)   0.1( 11.6)  99.8(8085.)
    ...
    13703                       137    36.6( 50.0)  12.8( 17.6)  50.6( 69.2)
    13702                        79     1.3(  1.0)   5.7(  4.5)  93.0( 73.4)
     
    Percent Contribution   --------     0.9(  0.9)   0.4(  0.4)  98.7( 98.7)
     
    * indicates system prio set on job

   The **mdiag -p** command only displays information for priority
components actually utilized.  In the above example, QOS, Account
Fairshare, and QueueTime components are utilized in determining a job's
priority.  Other components, such as Service Targets, and Bypass are not
used and thus are not displayed.  (See the '`Priority
Overview <../5.1.1priorityoverview.html>`__' for more information)  The
output consists of a header, a job by job analysis of jobs, and a
summary section.

   The header provides column labeling and provides configured priority
component and subcomponent weights.  In the above example, QOSWEIGHT is
set to 1000 and FSWEIGHT is set to 100.  When configuring fairshare, a
site also has the option of weighting the individual components of a
job's overall fairshare, including its user, group, and account
fairshare components.  In this output, the user, group, and account
fairshare weights are set to 5, 1, and 1 respectively.

   The job by job analysis displays a job's total priority and the
percentage contribution to that priority of each of the priority
components.  In this example, job ``13019`` has a total priority of
8699.  Both QOS and Fairshare contribute to the job's total priority
although these factors are quite small, contributing 0.6% and 0.3%
respectively with the fairshare factor being contributed by an account
fairshare target.  For this job, the dominant factor is the *service*
subcomponent *qtime* which is contributing 99.1% of the total priority
since the job has been in the queue for approximately 8600 minutes.

   At the end of the job by job description, a 'Totals' line is
displayed which documents the average percentage contributions of each
priority component to the current idle jobs.  In this example, the QOS,
Fairshare, and Service components contributed an average of 0.9%, 0.4%,
and 98.7% to the jobs' total priorities.

.. rubric:: See Also
   :name: see-also

-  `Job Priority Overview <../5.1jobprioritization.html>`__
-  `Moab Cluster Manager - Priority
   Manager <http://www.adaptivecomputing.com/mcm>`__

