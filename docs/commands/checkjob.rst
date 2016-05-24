
.. rubric:: checkjob
   :name: checkjob

**(Check Job)**
**Synopsis**

::

    checkjob [-A] [-l policylevel] [-n nodeid] [-q qosid] [-r reservationid] [-v] [--flags=future] jobid 

**Overview**

   **checkjob** displays detailed job
`state <3.2environment.html#jobstates>`__ information and diagnostic
output for a specified job.  Detailed information is available for
queued, blocked, active, and recently completed jobs.
**Access**

   This command can be run by level 1-3 Moab administrators for any
job.  Also, end users can use **checkjob** to view the status of their
own jobs.
**Arguments**

-A (Attribute-Value pair)
Format:
 
Default:
---
Description:
Provides output in the form of parsable Attribute-Value pairs.
Example:


::

    > checkjob -A 6235


Moab will display job information in the following format:
``<ATTRIBUTE>=<VALUE>;``.
 
 
--flags
Format:
--flags=future
Default:
---
Description:
Evaluates future eligibility of job (ignore current resource state and
usage limitations).
Example:


::

    > checkjob -v --flags=future 6235


Display reasons why idle job is blocked ignoring node state and current
node utilization constraints.

 
 
-l (Policy level)
Format:
<POLICYLEVEL>
**HARD**, **SOFT**, or **OFF**
Default:
---
Description:
Reports job start eligibility subject to specified throttling policy
level.
Example:


::

    > checkjob -l SOFT 6235
    > checkjob -l HARD 6235


 
 
-n (NodeID)
Format:
<NODEID>
Default:
---
Description:
Checks job access to specified node and `preemption <#preemption>`__
status with regards to jobs located on that node.
Example:


::

    > checkjob -n node113 6235


 
 
-q (QoS)
Format:
<QOSID>
Default:
---
Description:
Checks job access to specified QoS <QOSID>.
Example:


::

    > checkjob -q special 6235


 
 
-r (Reservation)
Format:
<RSVID>
Default:
---
Description:
Checks job access to specified reservation <RSVID>.
Example:


::

    > checkjob -r orion.1 6235


 
 
-v (Verbose)
Format:
 
Default:
N/A
Description:
Sets verbose mode.
Example:


::

    > checkjob -v 6235


**Details**

   This command allows any Moab administrator to check the detailed
status and resource requirements of a active, queued, or recently
`completed <../a.fparameters.html#jobcpurgetime>`__ job.  Additionally,
this command performs numerous diagnostic checks and determines if and
where the job could potentially run.  Diagnostic checks include
`policy <../6.2throttlingpolicies.html>`__ violations, reservation
constraints, preemption status, and job to resource mapping.  If a job
cannot run, a text reason is provided along with a summary of how many
nodes are and are not available.  If the **-v** flag is specified, a
node by node summary of resource availability will be displayed for idle
jobs.
**Job Eligibility**

|    If a job cannot run, a text reason is provided along with a summary
  of how many nodes are and are not available.  If the **-v** flag is
  specified, a node by node summary of resource availability will be
  displayed for idle jobs.  For job level eligibility issues, one of the
  following reasons will be given:
|  

+---------------------------------------+-----------------------------------------------------------------------------------+
| Reason                                | Description                                                                       |
+=======================================+===================================================================================+
| job has hold in place                 | one or more job holds are currently in place                                      |
+---------------------------------------+-----------------------------------------------------------------------------------+
| insufficient idle procs               | there are currently not adequate processor resources available to start the job   |
+---------------------------------------+-----------------------------------------------------------------------------------+
| idle procs do not meet requirements   | adequate idle processors are available but these do not meet job requirements     |
+---------------------------------------+-----------------------------------------------------------------------------------+
| start date not reached                | job has specified a minimum *start date* which is still in the future             |
+---------------------------------------+-----------------------------------------------------------------------------------+
| expected state is not idle            | job is in an unexpected state                                                     |
+---------------------------------------+-----------------------------------------------------------------------------------+
| state is not idle                     | job is not in the idle state                                                      |
+---------------------------------------+-----------------------------------------------------------------------------------+
| dependency is not met                 | job depends on another job reaching a certain state                               |
+---------------------------------------+-----------------------------------------------------------------------------------+
| rejected by policy                    | job start is prevented by a throttling policy                                     |
+---------------------------------------+-----------------------------------------------------------------------------------+

|    If a job cannot run on a particular node, one of the following 'per
  node' reasons will be given:
|  

+----------------+-----------------------------------------------------+
| **Class**      | Node does not allow required job class/queue        |
+----------------+-----------------------------------------------------+
| **CPU**        | Node does not possess required processors           |
+----------------+-----------------------------------------------------+
| **Disk**       | Node does not possess required local disk           |
+----------------+-----------------------------------------------------+
| **Features**   | Node does not possess required node features        |
+----------------+-----------------------------------------------------+
| **Memory**     | Node does not possess required real memory          |
+----------------+-----------------------------------------------------+
| **Network**    | Node does not possess required network interface    |
+----------------+-----------------------------------------------------+
| **State**      | Node is not Idle or Running                         |
+----------------+-----------------------------------------------------+

**Reservation Access**

   The `-r <#r>`__ flag can be used to provide detailed information
about job access to a specific reservation

\ **Preemption Status**

   If a job is marked as a `preemptor <../8.4preemption.html>`__ and the
`-v <#v>`__ and `-n <#n>`__ flags are specified, **checkjob** will
perform a job by job analysis for all jobs on the specified node to
determine if they can be preempted.

**Output**

   The **checkjob** command displays the following job attributes:
 
Attribute
Value
Description
Account
<STRING>
Name of account associated with job
Actual Run Time
[[[DD:]HH:]MM:]SS
Length of time job actually ran.

+----------+---------------------------------------------------+
| |Note|   | This info is only displayed in simulation mode.   |
+----------+---------------------------------------------------+

Allocated Nodes
Square bracket delimited list of node and processor ids
List of nodes and processors allocated to job
Arch
<STRING>
Node architecture required by job
Attr
square bracket delimited list of job attributes
Job Attributes (i.e. [BACKFILL][BENCHMARK][PREEMPTEE])
Average Utilized Procs\*
<FLOAT>
Average load balance for a job
Avg Util Resources Per Task\*
<FLOAT>
 
Bypass
<INTEGER>
Number of times a lower priority job with a later submit time ran before
the job
Class
[<CLASS NAME> <CLASS COUNT>]
Name of class/queue required by job and number of class initiators
required per task.
Dedicated Resources Per Task\*
<INTEGER>
 
Disk
<INTEGER>
Amount of local disk required by job (in MB)
Estimated Walltime
[[[DD:]HH:]MM:]SS
The scheduler's estimated walltime.

+----------+--------------------------------------------------+
| |Note|   | In simulation mode, it is the actual walltime.   |
+----------+--------------------------------------------------+

Exec Size\*
<INTEGER>
Size of job executable (in MB)
Executable
<STRING>
Name of command to run
Features
Square bracket delimited list of <STRING>s
Node features required by job
Flags
 
 
Group
<STRING>
Name of Unix® group associated with job
Holds
Zero or more of User, System, and Batch
Types of job holds currently applied to job
Image Size
<INTEGER>
Size of job data (in MB)
IWD (**I**\ nitial **W**\ orking **D**\ irectory)
<DIR>
Directory to run the executable in
Memory
<INTEGER>
Amount of real memory required per node (in MB)
Max Util Resources Per Task\*
<FLOAT>
 
Network
<STRING 
Type of network adapter required by job
NodeAccess\*
 
 
Nodecount
<INTEGER 
Number of nodes required by job
Opsys
<STRING 
Node operating system required by job
Partition Mask
ALL or colon delimited list of partitions
List of `partitions <../7.2partitions.html>`__ the job has access to
PE
<FLOAT>
Number of processor-equivalents requested by job
QOS
<STRING>
Quality of Service associated with job
Reservation
<RSVID  ( <TIME1  -  <TIME2> Duration: <TIME3>)
RESID specifies the reservation id, TIME1 is the relative start time,
TIME2 the relative end time, TIME3 the duration of the reservation
Req
[<INTEGER>] TaskCount: <INTEGER> Partition: <partition>
A `job requirement <../3.2environment.html#reqdefinition>`__ for a
single type of resource followed by the number of tasks instances
required and the appropriate `partition <../7.2partitions.html>`__
StartCount
<INTEGER>
Number of times job has been started by Moab
StartPriority
<INTEGER>
Start priority of job
StartTime
<TIME>
Time job was started by the resource management system
State
One of Idle, Starting, Running, etc
Current Job State
SubmitTime
<TIME>
Time job was submitted to resource management system
Swap
<INTEGER>
Amount of swap disk required by job (in MB)
Task Distribution\*
Square bracket delimited list of nodes
 
Time Queued
Total Nodes\*
<INTEGER>
Number of nodes requested by job
Total Tasks
<INTEGER>
Number of tasks requested by job
User
<STRING>
Name of user submitting job
Utilized Resources Per Task\*
<FLOAT>
 
WallTime
[[[DD:]HH:]MM:]SS of [[[DD:]HH:]MM:]SS
Length of time job has been running out of the specified limit
In the above table, fields marked with an asterisk (\*) are only
displayed when set or when the **-v** flag is specified.
 
**Example 1**

  Check job 717


::

    > checkjob 717
    job 717

    State: Idle
    Creds:  user:jacksond  group:jacksond  class:batch
    WallTime: 00:00:00 of 00:01:40
    SubmitTime: Mon Aug 15 20:49:41
      (Time Queued  Total: 3:12:23:13  Eligible: 3:12:23:11)

    TerminationDate:   INFINITY  Sat Oct 24 06:26:40
    Total Tasks: 1

    Req[0]  TaskCount: 1  Partition: ALL
    Network: ---  Memory >= 0  Disk >= 0  Swap >= 0
    Opsys: ---  Arch: ---  Features: ---


    IWD:            /home/jacksond/moab/moab-4.2.3
    Executable:     STDIN
    Flags:          RESTARTABLE,NORMSTART
    StartPriority:  5063
    Reservation '717' (  INFINITY ->   INFINITY  Duration: 00:01:40)
    Note:  job cannot run in partition base (idle procs do not meet requirements : 0 of 1 procs found)
    idle procs:   4  feasible procs:   0

    Rejection Reasons: [State        :    3][ReserveTime  :    1]

    cannot select job 717 for partition GM (partition GM does not support requested class batch)


+--------------------------------------+--------------------------------------+
| |Note|                               | The example job cannot be started    |
|                                      | for two different reasons.           |
|                                      |                                      |
|                                      | -  It is temporarily blocked from    |
|                                      |    partition ``base`` because of     |
|                                      |    node state and node reservation   |
|                                      |    conflicts.                        |
|                                      | -  It is permanently blocked from    |
|                                      |    partition ``GM`` because the      |
|                                      |    requested class ``batch`` is not  |
|                                      |    supported in that partition.      |
+--------------------------------------+--------------------------------------+

.. rubric:: See Also
   :name: see-also

-  `Moab Client Installation <../2.2installation.html#client>`__ -
   explains how to distribute this command to client nodes
-  `mdiag -j <mdiag-jobs.html>`__ command - display additional detailed
   information regarding jobs
-  `showq <showq.html>`__ command - showq high-level job summaries
-  `JOBCPURGETIME <../a.fparameters.html#jobcpurgetime>`__ parameter -
   specify how long information regarding completed jobs is maintained
-  diagnosing job `preemption <8.4preemption.html#testing>`__

