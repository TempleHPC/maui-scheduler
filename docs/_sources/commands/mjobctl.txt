
.. rubric:: mjobctl
   :name: mjobctl

**(Moab Job Control)**
**Synopsis**

::

    mjobctl -c jobexp
    mjobctl -c -w attr=val
    mjobctl -C jobexp
    mjobctl -h [User|System|Batch|Defer|All] jobexp
    mjobctl -m attr{+=|=|-=}val jobexp
    mjobctl -N [<SIGNO>] jobexp
    mjobctl -n <JOBNAME>
    mjobctl -p <PRIORITY> jobexp
    mjobctl -q {diag|starttime|hostlist} jobexp
    mjobctl -r jobexp
    mjobctl -R jobexp
    mjobctl -s jobexp 
    mjobctl -u [User|System|Batch|Defer|All] jobexp
    mjobctl -w attr{+=|=|-=}val jobexp
    mjobctl -x [-w flags=val] jobexp

**Overview**

The **mjobctl** command controls various aspects of jobs. It is used to
submit, cancel, execute, and checkpoint jobs. It can also display
diagnostic information about each job. The **mjobctl** command enables
the Moab administrator to control almost all aspects of job behavior.
See `11.0 General Job Administration <../11.0generaljobadmin.html>`__
for more details on jobs and their attributes.
**Format**

-c — \ **Cancel**
Format:
JOBID
Default:
---
Description:
Cancel a job.

+----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| |Note|   | Use `-w (following a -c flag) <http://www.adaptivecomputing.com/resources/docs/mwm/commands/mjobctl.html#cancelwhere>`__ to specify job cancellation according to given credentials or job attributes. See `-c -w <http://www.adaptivecomputing.com/resources/docs/mwm/commands/mjobctl.html#cancelwhere>`__ for more information.   |
+----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

Example:


::

    > mjobctl -c job1045


Cancel job job1045.
 
 
-c -w — \ **Cancel Where**
Format:
<ATTR>=<VALUE>
where <ATTR>=[ **user** \| **account** \| **qos** \| **class** \|
**reqreservation** (RsvName) \| **state** (JobState) \| **jobname**
(JobName, not job ID)] \| **partition**
Default:
---
Description:
Cancel a job based on a given credential or job attribute.
Use -w following a
`-c <http://www.adaptivecomputing.com/resources/docs/mwm/commands/mjobctl.html#cancel>`__
flag to specify job cancellation according to credentials or job
attributes. (See examples.)
Also, you can cancel jobs from given partitions using -w
partition=<PAR1>[<PAR2>...]]; however, you must also either use another
-w flag to specify a job or use the standard job expression.
Example:


::

    > mjobctl -c -w state=USERHOLD


Cancels all jobs that currently have a USERHOLD on them.


::

    > mjobctl -c -w user=user1 -w acct=acct1


Cancels all jobs assigned to ``user1`` or ``acct1``.
 
 
-C — \ **Checkpoint**
Format:
JOBID
Default:
---
Description:
Checkpoint a job
Example:


::

    > mjobctl -C job1045


Checkpoint job job1045.
 
 
-h — \ **Hold**
Format:
<HOLDTYPE> <JOBEXP>
 
<HOLDTYPE> = { **user** \| **batch** \| **system** \| **defer** \|
**ALL** }
Default:
**user**
Description:
Set or release a job hold
See `Section 11.1, Job
Holds <http://www.adaptivecomputing.com/resources/docs/mwm/11.1jobholds.html>`__
for more information
Example:


::

    > mjobctl -h user job1045


Set a user hold on job job1045.


::

    > mjobctl -u all job1045


Unset all holds on job job1045.
 
 
-m — \ **Modify**
Format:
<ATTR>{ **+=** \| **=** \| **-=** } <VAL>
  <ATTR>={ **account** \| **awduration** \| **class** \| **deadline** \|
**depend** \| **eeduration** \| **env** \| **features** \| **feature**
\| **flags** \| **gres** \| **group** \| **hold** \| **hostlist** \|
**jobdisk** \| **jobmem** \| **jobname** \| **jobswap** \| **loglevel**
\| **messages** \| **minstarttime** \| **nodecount** \|
**notificationaddress** \| **partition** \| **priority** \|
**proccount** \| **queue** \| **qos** \| **reqreservation** \|
**rmxstring** \| **reqawduration** \| **sysprio** \| **trig** \|
**trigvar** \| **userprio** \| **var** \| **wclimit** }
Default:
---
Description:
Modify a specific job attribute.
For priority, use the '`-p <#p>`__' flag.
Modification of the job dependency is also communicated to the resource
manager in the case of SLURM and PBS/Torque.
To define values for **awduration**, **eeduration**, **minstarttime**,
**reqawduration**, and **wclimit**, use the `time
spec <../timespec.html>`__ format.
A non-active job's partition list can be modified by adding or
subtracting partitions. Note, though, that when adding or subtracting
multiple partitions, each partition must have its own
``-m partition{+= | = | -=}name`` on the command line. (See example for
adding multiple partitions.)
Example:


::

    > mjobctl -m reqawduration+=600 1664


Add 10 minutes to the job walltime.


::

    > mjobctl -m eeduration=-1 1664


Reset job's effective queue time.


::

    > mjobctl -m var=Flag1=TRUE 1664


Set the job variable ``Flag1`` to ``TRUE.``


::

    > mjobctl -m notificationaddress="name@server.com"


Sets the notification e-mail address associated with a job to
name@server.com.


::

    > mjobctl -m partition+=p3 -m partition+=p4 Moab.5


Adds multiple partitions (p3 and p4) to job ``Moab.5``.
 
 
-N — \ **Notify**
Format:
[signal=]<SIGID> <JOBID>
Default:
---
Description:
Send a signal to all jobs matching the job expression.
Example:


::

    > mjobctl -N INT 1664


Send an ``interrupt`` signal to job 1664.


::

    > mjobctl -N 47 1664


Send signal 47 to job 1664.
 
 
-n — \ **Name**
Format:
 
Default:
---
Description:
Select jobs by job name.
Example:
 
 
 
-p — \ **Priority**
Format:
 
Default:
---
Description:
Modify a job's `system priority <setspri.html#priority>`__.
Example:


::

    > mjobctl -p +1000 job1045


Adds 1000 points to the max `system priority <setspri.html#priority>`__,
ensuring that this job will be higher priority than all normal jobs. The
new priority of job1045 is 1000001000.


::

    > mjobctl -p 1000 job1045 --flags=relative


Adds 1000 points to what the `priority <setspri.html#priority>`__ of the
job would be from normal calculation. The new priority for job1045 is
1250.
 
 
-q — \ **Query**
Format:
[ **diag** \| **hostlist** \| **starttime** ] <JOBEXP>
Default:
---
Description:
Query a job.
Example:


::

    > mjobctl -q diag job1045


Query job job1045.


::

    > mjobctl -q starttime job1045


Query starttime of job job1045.

+----------+--------------------------------------------------------------+
| |Note|   | --flags=completed will only work with the **diag** option.   |
+----------+--------------------------------------------------------------+

 
 
-r — \ **Resume**
Format:
JOBID
Default:
---
Description:
Resume a job.
Example:


::

    > mjobctl -r job1045


Resume job job1045.
 
 
-R — \ **Requeue**
Format:
JOBID
Default:
---
Description:
Requeue a job.
Example:


::

    > mjobctl -R job1045


Requeue job job1045.
 
 
-s — \ **Suspend**
Format:
JOBID
Default:
---
Description:
Suspend a job.
Example:


::

    > mjobctl -s job1045


Suspend job job1045.
 
 
-S — \ **Submit**
Format:
JOBID
Default:
---
Description:
Submit a job.
Example:


::

    > mjobctl -S job1045


Submit job job1045.
 
 
-u — \ **Unhold**
Format:
[<TYPE>[,<TYPE>]] <JOBEXP>
 
<TYPE> = [ **user** \| **system** \| **batch** \| **defer** \| **ALL** ]
Default:
ALL
Description:
Release a hold on a job
See `Section 11.1, Job
Holds <http://www.adaptivecomputing.com/resources/docs/mwm/11.1jobholds.html>`__
for more information.
Example:


::

    > mjobctl -u user,system scrib.1045


Release user and system holds on job scrib.1045.
 
 
-w — \ **Where**
Format:
[**CompletionTime** \| **StartTime**][\ **<=** \| **=** \|
**>=**]<EPOCH\_TIME>
Default:
---
Description:
Add a *where* constraint clause to the current command. As it pertains
to **CompletionTime** \| **StartTime**, the *where* constraint only
works for completed jobs. CompletionTime filters according to the
completed jobs' completion times; StartTime filters according to the
completed jobs' start times.
Example:


::

    > mjobctl -q diag ALL --flags=COMPLETED --format=xml 

    -w CompletionTime>=1246428000 -w CompletionTime<=1254376800


Prints all completed jobs still in memory that completed between July 1,
2009 and October 1, 2009.
 
 
-x — \ **Execute**
Format:
JOBID
Default:
---
Description:
Execute a job. The -w option allows flags to be set for the job.
Allowable flags are, **ignorepolicies**, **ignorenodestate**, and
**ignorersv**.
Example:


::

    > mjobctl -x job1045


Execute job job1045.


::

    > mjobctl -x -w flags=ignorepolicies job1046


Execute job job1046 and ignore policies, such as MaxJobPerUser.
**Parameters**

\ **JOB EXPRESSION**
Format:
<STRING>
Default:
---
Description:
The name of a job or a regular expression for several jobs.

+----------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| |Note|   | Moab uses regular expressions conforming to the POSIX 1003.2 standard. This standard is somewhat different than the regular expressions commonly used for filename matching in Unix® environments (see 'man 7 regex'). To interpret a job expression as a regular expression, either specify the expression using a designated expression or wildcard character (one of '[]\*?^$') or in the Moab configuration file (moab.cfg), set the parameter **USEJOBREGEX** to **TRUE** (and take note of the following caution).   |
+----------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

+-------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| |Caution|   | If you set **USEJOBREGEX** to **TRUE**, treat all **mjobctl** job expressions as regular expressions regardless of whether wildcards are specified. This should be used with extreme caution since there is high potential for unintended consequences. For example, specifying ``canceljob m.1`` will not only cancel ``m.1``, but also ``m.11``,\ ``m.12``,\ ``m13``, and so on.   |
+-------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

+----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| |Note|   | In most cases, it is necessary to *quote* the job expression (i.e. "job13[5-9]") to prevent the shell from intercepting and interpreting the special characters.   |
+----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------+

+----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| |Note|   | The **mjobctl** command accepts a comma delimited list of job expressions. Example usage might be ``mjobctl -c job[1-2],job4`` or ``mjobctl -c job1,job2,job4``.   |
+----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------+

Example:


::

    > mjobctl -c "80.*"

    job '802' cancelled
    job '803' cancelled
    job '804' cancelled
    job '805' cancelled
    job '806' cancelled
    job '807' cancelled
    job '808' cancelled
    job '809' cancelled


Cancel all jobs starting with '80'.


::

    > mjobctl -m priority+=200 "74[3-5]" 

    job '743' system priority modified
    job '744' system priority modified
    job '745' system priority modified


**\ XML Output**

**mjobctl** information can be reported as XML as well. This is done
with the command "mjobctl -q diag <JOB\_ID>". In addition to the
attributes listed below, mjobctl's XML children describe a job's
requirements (`req <../xml/req.html>`__ XML element) and messages
(`Messages <../xml/Messages.html>`__ XML element).
**XML Attributes**

+--------------------+------------------------------------------------------------------------------------+
| Name               | Description                                                                        |
+====================+====================================================================================+
| Account            | the account assigned to the job                                                    |
+--------------------+------------------------------------------------------------------------------------+
| AllocNodeList      | the nodes allocated to the job                                                     |
+--------------------+------------------------------------------------------------------------------------+
| Args               | the job's executable arguments                                                     |
+--------------------+------------------------------------------------------------------------------------+
| AWDuration         | the active wall time consumed                                                      |
+--------------------+------------------------------------------------------------------------------------+
| BlockReason        | the block message index for the reason the job is not eligible                     |
+--------------------+------------------------------------------------------------------------------------+
| Bypass             | Number of times the job has been bypassed by other jobs                            |
+--------------------+------------------------------------------------------------------------------------+
| Calendar           | the job's timeframe constraint calendar                                            |
+--------------------+------------------------------------------------------------------------------------+
| Class              | the class assigned to the job                                                      |
+--------------------+------------------------------------------------------------------------------------+
| CmdFile            | the command file path                                                              |
+--------------------+------------------------------------------------------------------------------------+
| CompletionCode     | the return code of the job as extracted from the RM                                |
+--------------------+------------------------------------------------------------------------------------+
| CompletionTime     | the time of the job's completion                                                   |
+--------------------+------------------------------------------------------------------------------------+
| Cost               | the cost of executing the job relative to an allocation manager                    |
+--------------------+------------------------------------------------------------------------------------+
| CPULimit           | the CPU limit for the job                                                          |
+--------------------+------------------------------------------------------------------------------------+
| Depend             | any dependencies on the status of other jobs                                       |
+--------------------+------------------------------------------------------------------------------------+
| DRM                | the master destination RM                                                          |
+--------------------+------------------------------------------------------------------------------------+
| DRMJID             | the master destination RM job ID                                                   |
+--------------------+------------------------------------------------------------------------------------+
| EEDuration         | the duration of time the job has been eligible for scheduling                      |
+--------------------+------------------------------------------------------------------------------------+
| EFile              | the stderr file                                                                    |
+--------------------+------------------------------------------------------------------------------------+
| Env                | the job's environment variables set for execution                                  |
+--------------------+------------------------------------------------------------------------------------+
| EnvOverride        | the job's overriding environment variables set for execution                       |
+--------------------+------------------------------------------------------------------------------------+
| EState             | the expected state of the job                                                      |
+--------------------+------------------------------------------------------------------------------------+
| EstHistStartTime   | the estimated historical start time                                                |
+--------------------+------------------------------------------------------------------------------------+
| EstPrioStartTime   | the estimated priority start time                                                  |
+--------------------+------------------------------------------------------------------------------------+
| EstRsvStartTime    | the estimated reservation start time                                               |
+--------------------+------------------------------------------------------------------------------------+
| EstWCTime          | the estimated walltime the job will execute                                        |
+--------------------+------------------------------------------------------------------------------------+
| ExcHList           | the excluded host list                                                             |
+--------------------+------------------------------------------------------------------------------------+
| Flags              | Command delimited list of Moab flags on the jo                                     |
+--------------------+------------------------------------------------------------------------------------+
| GAttr              | the requested generic attributes                                                   |
+--------------------+------------------------------------------------------------------------------------+
| GJID               | the global job ID                                                                  |
+--------------------+------------------------------------------------------------------------------------+
| Group              | the group assigned to the job                                                      |
+--------------------+------------------------------------------------------------------------------------+
| Hold               | the hold list                                                                      |
+--------------------+------------------------------------------------------------------------------------+
| Holdtime           | the time the job was put on hold                                                   |
+--------------------+------------------------------------------------------------------------------------+
| HopCount           | the hop count between the job's peers                                              |
+--------------------+------------------------------------------------------------------------------------+
| HostList           | the requested host list                                                            |
+--------------------+------------------------------------------------------------------------------------+
| IFlags             | the internal flags for the job                                                     |
+--------------------+------------------------------------------------------------------------------------+
| IsInteractive      | if set, the job is interactive                                                     |
+--------------------+------------------------------------------------------------------------------------+
| IsRestartable      | if set, the job is restartable                                                     |
+--------------------+------------------------------------------------------------------------------------+
| IsSuspendable      | if set, the job is suspendable                                                     |
+--------------------+------------------------------------------------------------------------------------+
| IWD                | the directory where the job is executed                                            |
+--------------------+------------------------------------------------------------------------------------+
| JobID              | the job's batch ID.                                                                |
+--------------------+------------------------------------------------------------------------------------+
| JobName            | the user-specifed name for the job                                                 |
+--------------------+------------------------------------------------------------------------------------+
| JobGroup           | the job ID relative to its group                                                   |
+--------------------+------------------------------------------------------------------------------------+
| LogLevel           | the individual log level for the job                                               |
+--------------------+------------------------------------------------------------------------------------+
| MasterHost         | the specified host to run primary tasks on                                         |
+--------------------+------------------------------------------------------------------------------------+
| Messages           | any messages reported by Moab regarding the job                                    |
+--------------------+------------------------------------------------------------------------------------+
| MinPreemptTime     | the minimum amount of time the job must run before being eligible for preemption   |
+--------------------+------------------------------------------------------------------------------------+
| Notification       | any events generated to notify the job's user                                      |
+--------------------+------------------------------------------------------------------------------------+
| OFile              | the stdout file                                                                    |
+--------------------+------------------------------------------------------------------------------------+
| OldMessages        | any messages reported by Moab in the old message style regarding the job           |
+--------------------+------------------------------------------------------------------------------------+
| OWCLimit           | the original wallclock limit                                                       |
+--------------------+------------------------------------------------------------------------------------+
| PAL                | the partition access list relative to the job                                      |
+--------------------+------------------------------------------------------------------------------------+
| QueueStatus        | the job's queue status as generated this iteration                                 |
+--------------------+------------------------------------------------------------------------------------+
| QOS                | the QOS assigned to the job                                                        |
+--------------------+------------------------------------------------------------------------------------+
| QOSReq             | the requested QOS for the job                                                      |
+--------------------+------------------------------------------------------------------------------------+
| ReqAWDuration      | the requested active walltime duration                                             |
+--------------------+------------------------------------------------------------------------------------+
| ReqCMaxTime        | the requested latest allowed completion time                                       |
+--------------------+------------------------------------------------------------------------------------+
| ReqMem             | the total memory requested/dedicated to the job                                    |
+--------------------+------------------------------------------------------------------------------------+
| ReqNodes           | the number of requested nodes for the job                                          |
+--------------------+------------------------------------------------------------------------------------+
| ReqProcs           | the number of requested procs for the job                                          |
+--------------------+------------------------------------------------------------------------------------+
| ReqReservation     | the required reservation for the job                                               |
+--------------------+------------------------------------------------------------------------------------+
| ReqRMType          | the required RM type                                                               |
+--------------------+------------------------------------------------------------------------------------+
| ReqSMinTime        | the requested earliest start time                                                  |
+--------------------+------------------------------------------------------------------------------------+
| RM                 | the master source resource manager                                                 |
+--------------------+------------------------------------------------------------------------------------+
| RMXString          | the resource manager extension string                                              |
+--------------------+------------------------------------------------------------------------------------+
| RsvAccess          | the list of reservations accessible by the job                                     |
+--------------------+------------------------------------------------------------------------------------+
| RsvStartTime       | the reservation start time                                                         |
+--------------------+------------------------------------------------------------------------------------+
| RunPriority        | the effective job priority                                                         |
+--------------------+------------------------------------------------------------------------------------+
| Shell              | the execution shell's output                                                       |
+--------------------+------------------------------------------------------------------------------------+
| SID                | the job's system ID (parent cluster)                                               |
+--------------------+------------------------------------------------------------------------------------+
| Size               | the job's computational size                                                       |
+--------------------+------------------------------------------------------------------------------------+
| STotCPU            | the average CPU load tracked across all nodes                                      |
+--------------------+------------------------------------------------------------------------------------+
| SMaxCPU            | the max CPU load tracked across all nodes                                          |
+--------------------+------------------------------------------------------------------------------------+
| STotMem            | the average memory usage tracked across all nodes                                  |
+--------------------+------------------------------------------------------------------------------------+
| SMaxMem            | the max memory usage tracked across all nodes                                      |
+--------------------+------------------------------------------------------------------------------------+
| SRMJID             | the source RM's ID for the job                                                     |
+--------------------+------------------------------------------------------------------------------------+
| StartCount         | the number of the times the job has tried to start                                 |
+--------------------+------------------------------------------------------------------------------------+
| StartPriority      | the effective job priority                                                         |
+--------------------+------------------------------------------------------------------------------------+
| StartTime          | the most recent time the job started executing                                     |
+--------------------+------------------------------------------------------------------------------------+
| State              | the state of the job as reported by Moab                                           |
+--------------------+------------------------------------------------------------------------------------+
| StatMSUtl          | the total number of memory seconds utilized                                        |
+--------------------+------------------------------------------------------------------------------------+
| StatPSDed          | the total number of processor seconds dedicated to the job                         |
+--------------------+------------------------------------------------------------------------------------+
| StatPSUtl          | the total number of processor seconds utilized by the job                          |
+--------------------+------------------------------------------------------------------------------------+
| StdErr             | the path to the stderr file                                                        |
+--------------------+------------------------------------------------------------------------------------+
| StdIn              | the path to the stdin file                                                         |
+--------------------+------------------------------------------------------------------------------------+
| StdOut             | the path to the stdout file                                                        |
+--------------------+------------------------------------------------------------------------------------+
| StepID             | StepID of the job (used with LoadLeveler systems)                                  |
+--------------------+------------------------------------------------------------------------------------+
| SubmitHost         | the host where the job was submitted                                               |
+--------------------+------------------------------------------------------------------------------------+
| SubmitLanguage     | the RM langauge that the submission request was performed                          |
+--------------------+------------------------------------------------------------------------------------+
| SubmitString       | the string containing the entire submission request                                |
+--------------------+------------------------------------------------------------------------------------+
| SubmissionTime     | the time the job was submitted                                                     |
+--------------------+------------------------------------------------------------------------------------+
| SuspendDuration    | the amount of time the job has been suspended                                      |
+--------------------+------------------------------------------------------------------------------------+
| SysPrio            | the admin specified job priority                                                   |
+--------------------+------------------------------------------------------------------------------------+
| SysSMinTime        | the system specified min. start time                                               |
+--------------------+------------------------------------------------------------------------------------+
| TaskMap            | the allocation taskmap for the job                                                 |
+--------------------+------------------------------------------------------------------------------------+
| TermTime           | the time the job was terminated                                                    |
+--------------------+------------------------------------------------------------------------------------+
| User               | the user assigned to the job                                                       |
+--------------------+------------------------------------------------------------------------------------+
| UserPrio           | the user specified job priority                                                    |
+--------------------+------------------------------------------------------------------------------------+
| UtlMem             | the utilized memory of the job                                                     |
+--------------------+------------------------------------------------------------------------------------+
| UtlProcs           | the number of utilized processors by the job                                       |
+--------------------+------------------------------------------------------------------------------------+
| Variable           |                                                                                    |
+--------------------+------------------------------------------------------------------------------------+
| VWCTime            | the virtual wallclock limit                                                        |
+--------------------+------------------------------------------------------------------------------------+

**Example 1**


::

    > mjobctl -q diag ALL --format=xml

    <Data><job AWDuration="346" Class="batch" CmdFile="jobsleep.sh" EEDuration="0" 
    EState="Running" Flags="RESTARTABLE" Group="test" IWD="/home/test" JobID="11578" QOS="high" 
    RMJID="11578.lolo.icluster.org" ReqAWDuration="00:10:00" ReqNodes="1" ReqProcs="1" StartCount="1" 
    StartPriority="1" StartTime="1083861225" StatMSUtl="903.570" StatPSDed="364.610" StatPSUtl="364.610" 
    State="Running" SubmissionTime="1083861225" SuspendDuration="0" SysPrio="0" SysSMinTime="00:00:00" 
    User="test"><req AllocNodeList="hana" AllocPartition="access" ReqNodeFeature="[NONE]" 
    ReqPartition="access"></req></job><job AWDuration="346" Class="batch" CmdFile="jobsleep.sh" 
    EEDuration="0" EState="Running" Flags="RESTARTABLE" Group="test" IWD="/home/test" JobID="11579" 
    QOS="high" RMJID="11579.lolo.icluster.org" ReqAWDuration="00:10:00" ReqNodes="1" ReqProcs="1" 
    StartCount="1" StartPriority="1" StartTime="1083861225" StatMSUtl="602.380" StatPSDed="364.610" 
    StatPSUtl="364.610" State="Running" SubmissionTime="1083861225" SuspendDuration="0" SysPrio="0" 
    SysSMinTime="00:00:00" User="test"><req AllocNodeList="lolo" AllocPartition="access" 
    ReqNodeFeature="[NONE]" ReqPartition="access"></req></job></Data>


See Also

-  `Moab Client Installation <../2.2installation.html#client>`__ -
   explains how to distribute this command to client nodes
-  `setspri <setspri.html>`__
-  `canceljob <canceljob.html>`__
-  `runjob <runjob.html>`__
.. |Caution| image:: /resources/docs/images/caution.png

