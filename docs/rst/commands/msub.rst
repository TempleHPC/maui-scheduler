.. raw:: html

   <div class="sright">

.. raw:: html

   <div class="sub-content-head">

Moab Workload Manager


.. raw:: html

   <div id="sub-content-rpt" class="sub-content-rpt">

.. raw:: html

   <div id="tab-container" class="tab-container docs">

.. raw:: html

   <div class="topNav">

.. raw:: html

   <div class="docsSearch">


.. raw:: html

   <div class="navIcons topIcons">

|Home| |Up|
**Moab Workload Manager\ :sup:`®`**

--------------

+--------------------------------------+--------------------------------------+
| .. rubric:: msub                     | **(Moab Job Submit)**                |
|    :name: msub                       |                                      |
+--------------------------------------+--------------------------------------+

**Synopsis**

::

    msub  [-a datetime][-A account][-c interval][-C directive_prefix][-d path]
          [-e path][-E][-h][-I][-j join][-k keep][-K][-l resourcelist][-m mailoptions]
          [-M user_list][-N name][-o path][-p priority][-q destination][-r]
          [-S pathlist][-u userlist][-v variablelist][-V][-W additionalattributes]
          [-z][script]

**Overview**

**msub** allows users to submit jobs directly to Moab. When a job is
submitted directly to a resource manager (such as TORQUE), it is
constrained to run on only those nodes that the resource manager is
directly monitoring. In many instances, a site may be controlling
multiple resource managers. When a job is submitted to Moab rather than
to a specific resource manager, it is not constrained as to what nodes
it is executed on. **msub** can accept command line arguments (with the
same syntax as
`qsub <http://www.adaptivecomputing.com/products/torque/docs/commands/qsub.html>`__),
job scripts (in either PBS or LoadLeveler syntax), or the SSS Job XML
specification.

Submitted jobs can then be viewed and controlled via the
`mjobctl <mjobctl.html>`__ command.

+----------+--------------------------------------------------------------------------------------------------+
| |Note|   | Flags specified in the following table are not necessarily supported by all resource managers.   |
+----------+--------------------------------------------------------------------------------------------------+

**Access**

When Moab is configured to run as root, any user may submit jobs via
msub.
**Flags**

\ **-a**
Name:
\ **Eligible Date**
Format:
[[[[CC]YY]MM]DD]hhmm[.SS]
Default:
---
Description:
Declares the time after which the job is eligible for execution.
Example:


::

    > msub -a 12041300 cmd.pbs


Moab will not schedule the job until 1:00 pm on December 4, of the
current year.
 
 
\ **-A**
Name:
\ **Account**
Format:
<ACCOUNT NAME>
Default:
---
Description:
Defines the account associated with the job.
Example:


::

    > msub -A research cmd.pbs


Moab will associate this job with account research.
 
 
\ **-c**
Name:
\ **Checkpoint Interval**
Format:
[n\|s\|c\|c=<minutes>]
Default:
---
Description:
Checkpoint of the will occur at the specified interval.
**n** — No Checkpoint is to be performed.
**s** — Checkpointing is to be performed only when the server executing
the job is shut down.
**c** — Checkpoint is to be performed at the default minimum time for
the server executing the job.
**c=<minutes>** — Checkpoint is to be performed at an interval of
minutes.
Example:


::

    > msub -c c=12 cmd.pbs


The job will be checkpointed every 12 minutes.
 
 
\ **-C**
Name:
\ **Directive Prefix**
Format:
'<PREFIX NAME>'
Default:
First known prefix (#PBS, #@, #BSUB, #!, #MOAB, #MSUB)
Description:
Specifies which directive prefix should be used from a job script.

-  It is best to submit with single quotes. '#PBS'
-  An empty prefix will cause Moab to not search for any prefix. -C ''
-  Command line arguments have precedence over script arguments.
-  Custom prefixes can be used with the -C flag. -C '#MYPREFIX'
-  Custom directive prefixes must use PBS syntax.
-  If the -C flag is not given, Moab will take the first default prefix
   found. Once a directive is found, others are ignored.

Example:


::

    > msub -C '#MYPREFIX' cmd.pbs

    #MYPREFIX -l walltime=5:00:00 (in cmd.pbs)


Moab will use the #MYPREFIX directive specified in cmd.pbs, setting the
wallclock limit to five hours.
 
 
\ **-d**
Name:
\ **Execution Directory**
Format:
<path>
Default:
Depends on the RM being used. If using TORQUE, the default is $HOME. If
using SLURM, the default is the submission directory.
Description:
Specifies which directory the job should execute in.
Example:


::

    > msub -d /home/test/job12 cmd.pbs


The job will begin execution in the ``/home/test/job12`` directory.
 
 
\ **-e**
Name:
\ **Error Path**
Format:
[<hostname>:]<path>
Default:
$SUBMISSIONDIR/$JOBNAME.e$JOBID
Description:
Defines the path to be used for the standard error stream of the batch
job.
Example:


::

    > msub -e test12/stderr.txt


The STDERR stream of the job will be placed in the relative (to
execution) directory specified.
 
 
\ **-E**
Name:
\ **Environment Variables**
Format:
---
Default:
---
Description:
Moab adds the following variables, if populated, to the job's
environment:

-  **MOAB\_ACCOUNT**: Account name.
-  **MOAB\_BATCH**: Set if a batch job (non-interactive).
-  **MOAB\_CLASS**: Class name.
-  **MOAB\_DEPEND**: Job dependency string.
-  **MOAB\_GROUP**: Group name.
-  **MOAB\_JOBID**: Job ID. If submitted from the grid, grid jobid.
-  **MOAB\_JOBNAME**: Job name.
-  **MOAB\_MACHINE**: Name of the machine (ie. Destination RM) that the
   job is running on.
-  **MOAB\_NODECOUNT**: Number of nodes allocated to job.
-  **MOAB\_NODELIST**: Comma-separated list of nodes (listed singly with
   no ppn info).
-  **MOAB\_PARTITION**: Partition name the job is running in. If grid
   job, cluster scheduler's name.
-  **MOAB\_PROCCOUNT**: Number of processors allocated to job.
-  **MOAB\_QOS**: QOS name.
-  **MOAB\_TASKMAP**: Node list with procs per node listed.
   <nodename>.<procs>
-  **MOAB\_USER**: User name.

In SLURM environments, not all variables will be populated since the
variables are added at submission (such as NODELIST). With TORQUE/PBS,
the variables are added just before the job is started.

This feature only works with SLURM and TORQUE/PBS.

Example:


::

    > msub -E mySim.cmd


The job *mySim* will be submitted with extra environment variables.
 
 
\ **-h**
Name:
\ **Hold**
Format:
N/A
Default:
---
Description:
Specifies that a user hold be applied to the job at submission time.
Example:


::

    > msub -h cmd.ll


The job will be submitted with a user hold on it.
 
 
\ **-I**
Name:
\ **Interactive**
Format:
N/A
Default:
---
Description:
Declares the the job is to be run interactively.
Example:


::

    > msub -I job117.sh


The job will be submitted in interactive mode.
 
 
\ **-j**
Name:
\ **Join**
Format:
[oe\|n]
Default:
n (not merged)
Description:
Declares if the standard error stream of the job will be merged with the
standard output stream of the job. If "oe" is specified, the error and
output streams will be merged into the output stream.
Example:


::

    > msub -j oe cmd.sh


STDOUT and STDERR will be merged into one file.
 
 
\ **-k**
Name:
\ **Keep**
Format:
[e\|o\|eo\|oe\|n]
Default:
n (not retained)
Description:
Defines which (if either) of output and error streams will be retained
on the execution host (overrides path for stream).
Example:


::

    > msub -k oe myjob.sh


STDOUT and STDERR for the job will be retained on the execution host.
 
 
\ **-K**
Name:
\ **Continue Running**
Format:
N/A
Default:
---
Description:
Tells the client to continue running until the submitted job is
completed. The client will query the status of the job every 5 seconds.
The time interval between queries can be specified or disabled via
`MSUBQUERYINTERVAL <../a.fparameters.html#msubqueryinterval>`__.

+----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| |Note|   | Use the -K option SPARINGLY (if at all) as it slows down the Moab scheduler with frequent queries. Running ten jobs with the -K option creates an additional fifty queries per minute for the scheduler.   |
+----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

Example:


::

    > msub -K newjob.sh

    3
    Job 3 completed*


\*Only shows up after job completion.
 
 
\ **-l**
Name:
\ **Resource List**
Format:
::

    <STRING>

(either `standard PBS/TORQUE
options <http://www.adaptivecomputing.com/resources/torque/index.html20/2.1jobsubmission.html#resources>`__
or `resource manager extensions <../13.3rmextensions.html>`__)
Default:
---
Description:
Defines the resources that are required by the job and establishes a
limit to the amount of resource that can be consumed. Either resources
native to the resource manager (see `PBS/TORQUE
resources <http://www.adaptivecomputing.com/resources/torque/index.html20/2.1jobsubmission.html#resources>`__)
or scheduler `resource manager extensions <../13.3rmextensions.html>`__
may be specified. Note that resource lists are dependent on the resource
manager in use.

Example:


::

    > msub -l nodes=32:ppn=2,pmem=1800mb,walltime=3600,VAR=testvar:myvalue cmd.sh


The job requires 32 nodes with 2 processors each, 1800 MB per task, a
walltime of 3600 seconds, and a variable named testvar with a value of
myvalue.

 

 
 
\ **-m**
Name:
\ **Mail Options**
Format:
[[n]\|[a][b][e]]
Default:
---
Description:
Defines the set of conditions (abort,begin,end) when the server will
send a mail message about the job to the user.
Example:


::

    > msub -m be cmd.sh


Mail notifications will be sent when the job begins and ends.
 
 
\ **-M**
Name:
\ **Mail List**
Format:
<user>[@<host>][,<user>[@<host>],...]
Default:
$JOBOWNER
Description:
Specifies the list of users to whom mail is sent by the execution
server.
Example:


::

    > msub -M jon@node01,bill@node01,jill@node02 cmd.sh


Mail will be sent to the specified users if the job is aborted.
 
 
\ **-N**
Name:
\ **Name**
Format:
<STRING>
Default:
STDIN or name of job script
Description:
Specifies the user-specified job name attribute.
Example:


::

    > msub -N chemjob3 cmd.sh


Job will be associated with the name chemjob3.
 
 
\ **-o**
Name:
\ **Output Path**
Format:
[<hostname>:]<path>
Default:
$SUBMISSIONDIR/$JOBNAME.o$JOBID
Description:
Defines the path to be used for the standard output stream of the batch
job.
Example:


::

    > msub -o test12/stdout.txt


The STDOUT stream of the job will be placed in the relative (to
execution) directory specified.
 
 
\ **-p**
Name:
\ **Priority**
Format:
<INTEGER> (between -1024 and 0)
Default:
0
Description:
Defines the priority of the job.
To enable priority range from -1024 to +1023, see
`**ENABLEPOSUSERPRIORITY** <../a.fparameters.html#enableposuserpriority>`__.
Example:


::

    > msub -p 25 cmd.sh


The job will have a user priority of 25.
 
 
\ **-q**
Name:
\ **Destination Queue (Class)**
Format:
[<queue>][@<server>]
Default:
[<DEFAULT>]
Description:
Defines the destination of the job.
Example:


::

    > msub -q priority cmd.sh


The job will be submitted to the priority queue.
 
 
\ **-r**
Name:
\ **Rerunable**
Format:
[y\|n]
Default:
n
Description:
Declares whether the job is rerunable.
Example:


::

    > msub -r n cmd.sh


The job cannot be rerun.
 
 
\ **-S**
Name:
\ **Shell Path**
Format:
<path>[@<host>][,<path>[@<host>],...]
Default:
$SHELL
Description:
Declares the shell that interprets the job script.
Example:


::

    > msub -S /bin/bash


The job script will be interpreted by the ``/bin/bash`` shell.
 
 
\ **-u**
Name:
\ **User List**
Format:
<user>[@<host>[,<user>[@<host>],...]
Default:
UID of msub command
Description:
Defines the user name under which the job is to run on the execution
system.
Example:


::

    > msub -u bill@node01 cmd.sh


On node01 the job will run under Bill's UID, if permitted.
 
 
\ **-v**
Name:
\ **Variable List**
Format:
<string>[,<string>,...]
Default:
---
Description:
Expands the list the environment variables that are exported to the job
(taken from the msub command environment).
Example:


::

    > msub -v DEBUG cmd.sh


The DEBUG environment variable will be defined for the job.
 
 
\ **-V**
Name:
\ **All Variables**
Format:
N/A
Default:
N/A
Description:
Declares that all environment variables in the msub environment are
exported to the batch job
Example:


::

    > msub -V cmd.sh


All environment variables will be exported to the job.
 
 
\ **-W**
Name:
\ **Additional Attributes**
Format:
<string>
Default:
---
Description:
Allows the for the specification of additional job attributes (See
`Resource Manager Extension <../13.3rmextensions.html>`__)
Example:


::

    > msub -W x=GRES:matlab:1 cmd.sh


The job requires one resource of "matlab".
 
 
\ **-z**
Name:
\ **Silent Mode**
Format:
N/A
Default:
N/A
Description:
The job's identifier will not be printed to stdout upon submission.
Example:


::

    > msub -z cmd.sh


No job identifier will be printout the stdout upon successful
submission.
**\ Job Script**

   The **msub** command supports job scripts written in any one of the
following languages:

Language
Notes
`PBS/TORQUE Job Submission
Language <http://www.adaptivecomputing.com/resources/torque/index.html/commands/qsub.html>`__
---
`LoadLeveler Job Submission
Language <http://publib.boulder.ibm.com/infocenter/clresctr/index.jsp?topic=/com.ibm.cluster.loadl.doc/loadl33/am2ug30223.html>`__
Use the `INSTANTSTAGE <../a.fparameters.html#instantstage>`__ parameter
as only a subset of the command file keywords are interpreted by Moab.
`SSS XML Job Object Specification <../SSSJobObject_3.0.9.pdf>`__
---
LSF Job Submission Language
enabled in Moab 4.2.4 and higher
**/etc/msubrc**

Sites that wish to automatically add parameters to every job submission
can populate the file '/etc/msubrc' with global parameters that every
job submission will inherit.

For example, if a site wished every job to request a particular generic
resource they could use the following /etc/msubrc:


::

    -W x=GRES:matlab:2


**Usage Notes**

**msub** is designed to be as flexible as possible, allowing users
accustomed to PBS, LSF, or LoadLeveler syntax, to continue submitting
jobs as they normally would. It is not recommended that different styles
be mixed together in the same **msub** command.

When only one resource manager is configured inside of Moab, all jobs
are immediately staged to the only resource manager available. However,
when multiple resource managers are configured Moab will determine which
resource manager can run the job soonest. Once this has been determined,
Moab will stage the job to the resource manager.

It is possible to have Moab take a "best effort" approach at submission
time using the ***forward*** flag. When this flag is specified, Moab
will do a quick check and make an intelligent guess as to which resource
manager can run the job soonest and then immediately stage the job.

Moab can be configured to instantly stage a job to the underlying
resource manager (like TORQUE/LOADLEVELER) through the parameter
`INSTANTSTAGE <../a.fparameters.html#instantstage>`__. When set inside
*moab.cfg*, Moab will migrate the job instantly to an appropriate
resource manager. Once migrated, Moab will destroy all knowledge of the
job and refresh itself based on the information given to it from the
underlying resource manager.

In most instances Moab can determine what syntax style the job belongs
to (PBS or LoadLeveler); if Moab is unable to make a guess, it will
default the style to whatever resource manager was configured at compile
time. If LoadLeveler and PBS were both compiled then LoadLeveler takes
precedence.

Moab can translate a subset of job attributes from one syntax to
another. It is therefore possible to submit a PBS style job to a
LoadLeveler resource manager, and vice versa, though not all job
attributes will be translated.

**Example 1**


::

    > msub -l nodes=3:ppn=2,walltime=1:00:00,pmem=100kb script2.pbs.cmd

    4364.orion


**Example 2**

Example 2 is the XML-formatted version of Example 1. See `Submitting
Jobs via msub in XML <../msub_xml_format.html>`__ for more information.


::

    <job>
      <InitialWorkingDirectory>/home/user/test/perlAPI</InitialWorkingDirectory>
      <Executable>/home/user/test/perlAPI/script2.pbs.cmd</Executable>
      <SubmitLanguage>PBS</SubmitLanguage>
      <Requested>
        <Feature>ppn2</Feature>
        <Processors>3</Processors>
        <WallclockDuration>3600</WallclockDuration>
      </Requested>
    </job>


**See Also**

-  `Moab Client Installation <../2.2installation.html#client>`__ -
   explains how to distribute this command to client nodes
-  `mjobctl <mjobctl.html>`__ command to view, modify, and cancel jobs
-  `checkjob <checkjob.html>`__ command to view detailed information
   about the job
-  `mshow <mshow.html>`__ command to view all jobs in the queue
-  `DEFAULTSUBMITLANGUAGE <../a.fparameters.html#defaultsubmitlanguage>`__
   parameter
-  `MSUBQUERYINTERVAL <../a.fparameters.html#msubqueryinterval>`__
   parameter
-  `SUBMITFILTER <../a.fparameters.html#submitfilter>`__ parameter
-  `Applying the msub Submit Filter <../msub_submitfilter.html>`__ for
   job script sample

.. raw:: html

   <div class="navIcons bottomIcons">

|Home| |Up|





.. raw:: html

   <div class="sub-content-btm">




.. |Home| image:: /resources/docs/images/home.png
   :target: ../index.html
.. |Up| image:: /resources/docs/images/upArrow.png
   :target: ../a.gcommandoverview.html
.. |Note| image:: /resources/docs/images/note.png
.. |Home| image:: /resources/docs/images/home.png
   :target: index.html
