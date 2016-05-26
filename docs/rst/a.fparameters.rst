Appendix F: Maui Parameters
###########################

See the `Parameters Overview <3.4configure.html>`__ in the Maui Admin
Manual for further information about specifying parameters.
**ACCOUNTCFG[<ACCOUNTID>]**
Format:
list of zero or more space delimited <ATTR>=<VALUE> pairs where <ATTR>
is one of the following:
**PRIORITY**, **FSTARGET**, **QLIST**, **QDEF**, **PLIST** , **PDEF**,
**FLAGS**, or a `fairness policy <6.2throttlingpolicies.html>`__
specification.
Default:
[NONE]
Details:
specifies account specific attributes. See the `flag
overview <jobflagoverview.html>`__ for a description of legal flag
values.
Example:

::

    ACCOUNTCFG[projectX] MAXJOB=50 QDEF=highprio


(up to 50 jobs submitted under the account ID ``projectX`` will be
allowed to execute simultaneously and will be assigned the QOS
`` highprio`` by default.)

 
 
**AFSWEIGHT**
Format:
<INTEGER>
Default:
0
Details:
specifies the priority weight to be applied to the account fairshare
factor. (See `Fairshare Priority Factor <6.3fairshare.html>`__ )
Example:


::

    AFSWEIGHT 10

 
 
**ACCOUNTWEIGHT**
Format:
<INTEGER>
Default:
0
Details:
specifies the priority weight to be applied to the specified account
priority. (See `Credential Priority
Factor <5.1.2priorityfactors.html#cred>`__ )
Example:

::

    ACCOUNTWEIGHT 100

 
 
**ADMIN1**
Format:
space delimited list of user names
Default:
root
Details:
users listed under the parameter ADMIN1 are allowed to perform any
scheduling function. They have full control over the scheduler and
access to all data. The first user listed in the ADMIN1 user list is
considered to be the 'primary admin' and is the ID under which maui must
be started and run. Valid values include user names or the keyword
'ALL'.
Example:

::

    ADMIN1 mauiuser steve scott jenny


(all users listed have full access to maui control commands and maui
data. Maui must be started by and run under the 'mauiuser' user id since
mauiuser is the primary admin.

 
 
**ADMIN2**
Format:
space delimited list of user names
Default:
[NONE]
Details:
users listed under the parameter ADMIN2 are allowed to change all job
attributes and are granted access to all informational Maui commands.
Valid values include user names or the keyword 'ALL'.
Example:

::

    ADMIN2 jack karen


(jack and karen can modify jobs, i.e., 'canceljob, setqos, setspri,
etc.) and can run all Maui information commands).

 
 
**ADMIN3**
Format:
space delimited list of user names
Default:
[NONE]
Details:
users listed under the parameter ADMIN3 are allowed access to all
informational maui commands. They cannot change scheduler or job
attributes. Valid values include user names or the keyword 'ALL'.
Example:

::

    ADMIN3 ops

(user ``ops`` can run all informational command such as 'checkjob' or
checknode')

 
 
**AMCFG**
Format:
one or more **key-value** pairs as described in the `Allocation Manager
Configuration Overview <6.4allocationmanagement.html#config>`__
Default:
N/A
Details:
specifies the interface and policy configuration for the
scheduler-allocation manager interface. Described in detail in the
`Allocation Manager Configuration
Overview <6.4allocationmanagement.html#config>`__
Example:


::

    AMCFG[bank] TYPE=QBANK HOST=supercluster.org PORT=7111 DEFERJOBONFAILURE=FALSE


(the QBank server will be contacted at port 7111 on host
supercluster.org)

 
 
**BACKFILLDEPTH**
Format:
<INTEGER>
Default:
0 (no limit)
Details:
specifies the number idle jobs to evaluate for backfill. The backfill
algorithm will evaluate the top <X> priority jobs for scheduling. By
default, all jobs are evaluated.
Example:

::

    BACKFILLDEPTH 128


(evaluate only the top 128 highest priority idle jobs for consideration
for backfill)

 
 
**BACKFILLMETRIC**
Format:
one of the following **PROCS**, **PROCSECONDS**, **SECONDS** , **PE**,
or **PESECONDS**
Default:
**PROCS**
Details:
specifies the criteria used by the backfill algorithm to determine the
'best' jobs to backfill. Only applicable when using BESTFIT or GREEDY
backfill algorithms
Example:

::

    BACKFILLMETRIC PROCSECONDS

 
 
**BACKFILLPOLICY**
Format:
one of the following: **FIRSTFIT**, **BESTFIT**, **GREEDY** , or
**NONE**
Default:
**FIRSTFIT**
Details:
specifies what backfill algorithm will be used
Example:

::

    BACKFILLPOLICY BESTFIT

 
 
**BFCHUNKDURATION**
Format:
[[[DD:]HH:]MM:]SS
Default:
0 (chunking disabled)
Details:
specifies the duration during which freed resources will be aggregated
for use by larger jobs. Used in conjunction with **BFCHUNKSIZE**. See
`Configuring Backfill <8.2backfill.html#config>`__ for more information.
Example:

::

    BFCHUNKDURATION 00:05:00
    BFCHUNKSIZE     4


(aggregate backfillable resources for up to 5 minutes, making resources
available only to jobs of size 4 or larger)

 
 
**BFCHUNKSIZE**
Format:
<INTEGER>
Default:
0 (chunking disabled)
Details:
specifies the minimum job size which can utilize *chunked* resources.
Used in conjunction with **BFCHUNKDURATION**. See `Configuring
Backfill <8.2backfill.html#config>`__ for more information
Example:

::

    BFCHUNKDURATION 00:05:00
    BFCHUNKSIZE     4

(aggregate backfillable resources for up to 5 minutes, making resources
available only to jobs of size 4 or larger)

 
 
**BFPRIORITYPOLICY**
Format:
one of **RANDOM**, **DURATION**, or **HWDURATION**
Default:
NONE
Details:
specifies policy to use when prioritizing backfill jobs for preemption
Example:

::

    BFPRIORITYPOLICY  DURATION


(use length of job in determining which backfill job to preempt)

 
**BYPASSWEIGHT**
Format:
<INTEGER>
Default:
0
Details:
specifies the weight to be applied to a job's backfill bypass count when
determining a job's priority
Example:

::

    BYPASSWEIGHT 5000

 
 
**CHECKPOINTEXPIRATIONTIME**
Format:
[[[DD:]HH:]MM:]SS
Default:
INFINITY
Details:
specifies how 'stale' checkpoint data can be before it is ignored and
purged.
Example:

::

    CHECKPOINTEXPIRATIONTIME 1:00:00:00

(Expire checkpoint data which has been stale for over one day)

 
 
**CHECKPOINTFILE**
Format:
<STRING>
Default:
maui.ck
Details:
name (absolute or relative) of the Maui checkpoint file.
Example:



::

    CHECKPOINTFILE /var/adm/maui/maui.ck


(Maintain the Maui checkpoint file in the file specified)

 
 
**CHECKPOINTINTERVAL**
Format:
[[[DD:]HH:]MM:]SS
Default:
00:05:00
Details:
time between automatic Maui checkpoints
Example:



::

    CHECKPOINTINTERVAL 00:15:00


(Maui should checkpoint state information every 15 minutes)

 
 
**CLASSCFG[<CLASSID>]**
Format:
list of zero or more space delimited <ATTR>=<VALUE> pairs where <ATTR>
is one of the following: **PRIORITY**, **FSTARGET**, **QLIST**,
**QDEF**, **PLIST** , **PDEF**, **FLAGS**, or a `fairness
policy <6.2throttlingpolicies.html>`__ specification.
Default:
[NONE]
Details:
specifies class specific attributes. See the `flag
overview <jobflagoverview.html>`__ for a description of legal flag
values.
Example:



::

    CLASSCFG[batch] MAXJOB=50 QDEF=highprio


(up to 50 jobs submitted to the class ``batch`` will be allowed to
execute simultaneously and will be assigned the QOS ``highprio`` by
default.)

 
 
**CLASSWEIGHT**
Format:
<INTEGER>
Default:
0
Details:
specifies the weight to be applied to the class priority of each job
(See `Cred Factor <5.1.2priorityfactors.html#cred>`__ )
Example:



::

    CLASSWEIGHT 10


 
 
**CLIENTCFG[<X>]**
Format:
one or more of the following: **CSALGO** or **CSKEY**
Default:
[NONE]
Details:
specifies the shared secret key and encryption algorithm which Maui will
use to communicate with the named peer daemon. **NOTE:** this parameter
may only be specified in the **maui-private.cfg** config file)
Example:



::

    CLIENTCFG[silverB] CSKEY=apple7


(Maui will use the session key ``apple7`` for encrypting and decrypting
messages sent from ``silverB``)

 
 
**CLIENTTIMEOUT**
Format:
[[[DD:]HH:]MM:]SS
Default:
00:00:30
Details:
time which Maui client commands will wait for a response from the Maui
server (NOTE: may also be specified as an environment variable)
Example:



::

    CLIENTTIMEOUT 00:15:00


(Maui clients will wait up to 15 minutes for a response from the server
before timing out)

 
 
**CREDWEIGHT**
Format:
<INTEGER>
Default:
1
Details:
specifies the `credential component <5.1.2priorityfactors.html#cred>`__
weight
Example:



::

    CREDWEIGHT 2


 
 
**DEFAULTCLASSLIST**
Format:
space delimited list of one or more <STRING>'s
Default:
[NONE]
Details:
specifies the default classes supported on each node for RM systems
which do not provide this information
Example:



::

    DEFAULTCLASSLIST serial parallel


 
 
**DEFERCOUNT**
Format:
<INTEGER>
Default:
24
Details:
specifies the number of times a job can be deferred before it will be
placed in batch hold.
Example:



::

    DEFERCOUNT 12


 
 
**DEFERSTARTCOUNT**
Format:
<INTEGER>
Default:
1
Details:
specifies number of time a job will be allowed to fail in its start
attempts before being deferred.
Example:



::

    DEFERSTARTCOUNT 3


 
 
**DEFERTIME**
Format:
[[[DD:]HH:]MM:]SS
Default:
1:00:00
Details:
specifies amount of time a job will be held in the deferred state before
being released back to the Idle job queue
Example:



::

    DEFERTIME 0:05:00


 
 
**DISKWEIGHT**
Format:
<INTEGER>
Default:
0
Details:
specifies the priority weight to be applied to the amount of dedicated
disk space required per task by a job (in MB)
Example:



::

    RESWEIGHT 10




DISKWEIGHT 100


(if a job requires 12 tasks and 512 MB per task of dedicated local disk
space, Maui will increase the job's priority by 10 \* 100 \* 12 \* 512)

 
 
**DISPLAYFLAGS**
Format:
one or more of the following values (space delimited)
NODECENTRIC

Default:
[NONE]
Details:
specifies flags which control how maui client commands will display
various information
Example:



::

    DISPLAYFLAGS NODECENTRIC


 
 
**DOWNNODEDELAYTIME\***
Format:
[[[DD:]HH:]MM:]SS
Default:
24:00:00
Details:
default time an unavailable node (Down or Drain) is marked unavailable
Example:



::

    DOWNNODEDELAYTIME 1:00:00


(Maui will assume 'down' nodes will be available 1 hour after they go
down unless a system reservation is placed on the node)

 
 
**ENABLEMULTINODEJOBS**
Format:
<BOOLEAN>
Default:
TRUE
Details:
specifies whether or not the scheduler will allow jobs to span more than
one node
Example:



::

    ENABLEMULTINODEJOBS FALSE


 
 
**ENABLEMULTIREQJOBS**
Format:
<BOOLEAN>
Default:
FALSE
Details:
specifies whether or not the scheduler will allow jobs to specify
multiple independent resource requests (i.e., pbs jobs with resource
specifications such as '-l nodes=3:fast+1:io')
Example:



::

    ENABLEMULTIREQJOBS TRUE


 
 
**ENABLENEGJOBPRIORITY[X]**
Format:
<BOOLEAN>
Default:
FALSE
Details:
if set to TRUE, the scheduler will allow job priority value to range
from -INFINITY to MMAX\_PRIO, otherwise, job priority values are given a
lower bound of '1'. (see `REJECTNEGPRIOJOBS <#rejectnegpriojobs>`__)
Example:



::

    ENABLENEGJOBPRIORITY TRUE


(Job priority may range from -INFINITY to MMAX\_PRIO.)

 
 
**FEATURENODETYPEHEADER**
Format:
<STRING>
Default:
[NONE]
Details:
specifies the header used to specify node type via node features (ie, LL
features or PBS node attributes).
Example:



::

    FEATURENODETYPEHEADER xnt


(Maui will interpret all node features with the leading string `` xnt``
as a nodetype specification - as used by QBank and other allocation
managers, and assign the associated value to the node. i.e., xntFast)

 
 
**FEATUREPARTITIONHEADER**
Format:
<STRING>
Default:
[NONE]
Details:
specifies the header used to specify node partition via node features
(ie, LL features or PBS node attributes).
Example:



::

    FEATUREPARTITIONHEADER xpt


(Maui will interpret all node features with the leading string `` xpt``
as a partition specification and assign the associated value to the
node. i.e., xptGold)

 
 
**FEATUREPROCSPEEDHEADER**
Format:
<STRING>
Default:
[NONE]
Details:
specifies the header used to extract node processor speed via node
features (i.e., LL features or PBS node attributes). NOTE: Adding a
trailing '$' character will specifies that only features with a trailing
number be interpreted. For example, the header 'sp$' will match 'sp450'
but not 'sport'
Example:



::

    FEATUREPROCSPEEDHEADER xps


(Maui will interpret all node features with the leading string `` xps``
as a processor speed specification and assign the associated value to
the node. i.e., xps950)

 
 
**FEEDBACKPROGRAM**
Format:
<STRING>
Default:
[NONE]
Details:
specifies the name of the program to be run at the completion of each
job. If not fully qualified, Maui will attempt to locate this program in
the 'tools' subdirectory.
Example:



::

    FEEDBACKPROGRAM /var/maui/fb.pl


(Maui will run the specified program at the completion of each job.)

 
 
**FSACCOUNTWEIGHT**
Format:
<INTEGER>
Default:
0
Details:
specifies the weight assigned to the account subcomponent of the
fairshare component of priority
Example:



::

    FSACCOUNTWEIGHT 10


 
 
**FSCAP**
Format:
<DOUBLE>
Default:
0 (NO CAP)
Details:
specifies the maximum allowed value for a job's total pre-weighted
fairshare component
Example:



::

    FSCAP 10.0


(Maui will not allow a job's pre-weighted fairshare component to exceed
10.0,



ie, Priority = FSWEIGHT \* MIN(FSCAP,FSFACTOR) + ...)
 
 
**FSCONFIGFILE**
Format:
<STRING>
Default:
fs.cfg
Details:
Example:
 
 
**FSDECAY**
Format:
<DOUBLE>
Default:
1.0
Details:
Example:
 
 
**FSDEPTH**
Format:
<INTEGER>
Default:
7
Details:
NOTE: The number of available fairshare windows is bounded by the
**MAX\_FSDEPTH** value (24 in Maui 3.2.6 and earlier, 32 in Maui 3.2.7
and later)
Example:



::

    FSDEPTH 12


 
 
**FSGROUPWEIGHT**
Format:
<INTEGER>
Default:
0
Details:
Example:



::

    FSGROUPWEIGHT 4


 
 
**FSINTERVAL**
Format:
[[[DD:]HH:]MM:]SS
Default:
24:00:00
Details:
specifies the length of each fairshare '`window <6.3fairshare.html>`__ '
Example:



::

    FSINTERVAL 12:00:00




(track fairshare usage in 12 hour blocks)
 
 
**FSPOLICY**
Format:
one of the following: **DEDICATEDPS**, **DEDICATEDPES**
Format:
[NONE]
Details:
specifies the unit of tracking fairshare usage. **DEDICATEDPS** tracks
dedicated processor seconds. **DEDICATEDPES** tracks dedicated
processor-equivalent seconds
Example:



::

    FSPOLICY DEDICATEDPES


(Maui will track fairshare usage by dedicated process-equivalent
seconds)

 
 
**FSQOSWEIGHT**
Format:
<INTEGER>
Default:
0
Details:
specifies the priority weight assigned to the QOS fairshare subcomponent
Example:
 
 
**FSUSERWEIGHT**
Format:
<INTEGER>
Default:
0
Details:
specifies the priority weight assigned to the user fairshare subfactor.
Example:



::

    FSUSERWEIGHT 8


 
 
**FSWEIGHT**
Format:
<INTEGER>
Default:
0
Details:
specifies the priority weight assigned to the summation of the fairshare
subfactors
Example:



::

    FSWEIGHT 500


 
 
**GROUPCFG[<GROUPID>]**
Format:
list of zero or more space delimited <ATTR>=<VALUE> pairs where <ATTR>
is one of the following:
**PRIORITY**, **FSTARGET**, **QLIST**, **QDEF**, **PLIST** , **PDEF**,
**FLAGS**, or a `fairness policy <6.2throttlingpolicies.html>`__
specification.
Example:
[NONE]
Details:
specifies group specific attributes. See the `flag
overview <jobflagoverview.html>`__ for a description of legal flag
values.
Example:



::

    GROUPCFG[staff] MAXJOB=50 QDEF=highprio


(up to 50 jobs submitted by members of the group ``staff`` will be
allowed to execute simultaneously and will be assigned the QOS
`` highprio`` by default.)

 
 
**GROUPWEIGHT**
Format:
<INTEGER>
Default:
0
Details:
specifies the priority weight assigned to the specified group priority
(See `Cred Factor <5.1.2priorityfactors.html#cred>`__)
Example:



::

    GROUPWEIGHT 20


 
 
**JOBAGGREGATIONTIME**
Format:
[[[DD:]HH:]MM:]SS
Default:
0
Details:
specifies the minimum amount of time the scheduler should wait after
receiving a job event until it should process that event. This parameter
allows sites with *bursty* job submissions to process job events in
groups decreasing total job scheduling cycles and allowing the scheduler
to make more intelligent choices by aggregating job submissions and
choosing between the jobs. (See `Considerations for Large
Clusters <a.ilargeclusters.html>`__ )
Example:



::

    JOBAGGREGATIONTIME 00:00:04




::

    RMPOLLINTERVAL 00:00:30


The scheduler will wait 4 seconds between scheduling cycles when job
events have been received and will wait 30 seconds between scheduling
cycles otherwise

 
 
**JOBMAXSTARTTIME**
Format:
[[[DD:]HH:]MM:]SS
Default:
-1 (NO LIMIT)
Details:
length of time a job is allowed to remain in a 'starting' state. If a
'started' job does not transition to a running state within this amount
of time, the scheduler will cancel the job, believing a system failure
has occurred.
Example:



::

    JOBMAXSTARTTIME 2:00:00


(jobs may attempt to start for up to 2 hours before being cancelled by
the scheduler)

 
 
**JOBMAXOVERRUN**
Format:
[[[DD:]HH:]MM:]SS
Default:
0
Details:
amount of time Maui will allow a job to exceed its wallclock limit
before it is terminated
Example:



::

    JOBMAXOVERRUN 1:00:00


(allow jobs to exceed their wallclock limit by up to 1 hour)

 
 
**JOBNODEMATCHPOLICY**
Format:
zero or more of the following: **EXACTNODE** or **EXACTPROC**
Format:
[NONE]
Details:
specifies additional constraints on how compute nodes are to be
selected. **EXACTNODE** indicates that Maui should select as many nodes
as requested even if it could pack multiple tasks onto the same node.
**EXACTPROC** indicates that Maui should select only nodes with exactly
the number of processors configured as are requested per node even if
nodes with excess processors are available.
Example:



::

    JOBNODEMATCHPOLICY EXACTNODE


(In a PBS job with resource specification 'nodes=<x>:ppn=<y>', Maui will
allocate exactly <y> task on each of <x> distinct nodes.)

 
 
**JOBPRIOACCRUALPOLICY**
Format:
one of the following: **ALWAYS**, **FULLPOLICY**, **QUEUEPOLICY**
Default:
**QUEUEPOLICY**
Details:
specifies how the dynamic aspects of a job's priority will be adjusted.
**ALWAYS** indicates that the job will accrue queuetime based priority
from the time it is submitted. **FULLPOLICY** indicates that it will
accrue priority only when it meets all queue AND run policies.
**QUEUEPOLICY** indicates that it will accrue priority so long as it
satisfies various queue policies, i.e. MAXJOBQUEUED.
Example:



::

    JOBPRIOACCRUALPOLICY QUEUEPOLICY


(Maui will adjust the job's dynamic priority subcomponents, i.e.,
QUEUETIME, XFACTOR, and TARGETQUEUETIME, etc. each iteration that the
job satisfies the associated 'QUEUE' policies such as MAXJOBQUEUED.)

 
 
**JOBSIZEPOLICY**
Format:
<N/A>
Default:
[NONE]
Details:
<N/A>
Example:
<N/A>
 
 
**JOBSYNCTIME**
Format:
[[[DD:]HH:]MM:]:SS
Default:
00:10:00
Details:
specifies the length of time after which Maui will sync up a job's
expected state with an unexpected reported state. IMPORTANT NOTE: Maui
will not allow a job to run as long as its expected state does not match
the state reported by the resource manager. NOTE: this parameter is
named JOBSYNCDEADLINE in Maui 3.0.5 and earlier
Example:



::

    JOBSYNCTIME 00:01:00


 
 
**LOGDIR**
Format:
<STRING>
Default:
log
Details:
specifies the directory in which log files will be maintained. If
specified as a relative path, LOGDIR will be relative to $(MAUIHOMEDIR)
(see `Logging Overview <14.2logging.html>`__ )
Example:



::

    LOGDIR /tmp


(Maui will record its log files directly into the ``/tmp`` directory)

 
 
**LOGFACILITY**
Format:
colon delimited list of one or more of the following: **fCORE**,
**fSCHED**, **fSOCK**, **fUI**, **fLL**, **fSDR** , **fCONFIG**,
**fSTAT**, **fSIM**, **fSTRUCT**, **fFS**, **fCKPT**, **fBANK**,
**fRM**, **fPBS**, **fWIKI**, **fALL**
Default:
**fALL**
Details:
specifies which types of events to log (see `Logging
Overview <14.2logging.html>`__ )
Example:



::

    LOGFACILITY fRM:fPBS


(Maui will log only events involving general resource manager or PBS
interface activities.)

 
 
**LOGFILE**
Format:
<STRING>
Default:
maui.log
Details:
name of the maui log file. This file is maintained in the directory
pointed to by <LOGDIR> unless <LOGFILE> is an absolute path (see
`Logging Overview <14.2logging.html>`__ )
Example:



::

    LOGFILE maui.test.log


(Log information will be written to the file ``maui.test.log`` located
in the directory pointed to by the `LOGDIR <#logdir>`__ parameter)

 
 
**LOGFILEMAXSIZE**
Format:
<INTEGER>
Default:
10000000
Details:
maximum allowed size (in bytes) the log file before it will be 'rolled'
(see `Logging Overview <14.2logging.html>`__ )
Example:



::

    LOGFILEMAXSIZE 50000000


(Log files will be rolled when they reach 50 MB in size)

 
 
**LOGFILEROLLDEPTH**
Format:
<INTEGER>
Default:
2
Details:
number of old log files to maintain (i.e., when full, maui.log will be
renamed maui.log.1, maui.log.1 will be renamed maui.log.2, ... (see
`Logging Overview <14.2logging.html>`__ )
Example:



::

    LOGFILEROLLDEPTH 5


(Maui will maintain the last 5 log files.)

 
 
**LOGLEVEL**
Format:
<INTEGER> (0-9)
Default:
0
Details:
specifies the verbosity of Maui logging where 9 is the most verbose
(NOTE: each logging level is approximately an order of magnitude more
verbose than the previous level) (see `Logging
Overview <14.2logging.html>`__ )
Example:



::

    LOGLEVEL 4


(Maui will write all Maui log messages with a threshold of 4 or lower to
the 'maui.log' file)

 
 
**MAXJOBPERUSERCOUNT**
Format:
<INTEGER>[,<INTEGER>]
Default:
0 (No Limit)
Details:
maximum number of active jobs allowed at any given time. (NOTE: This
parameter is deprecated, see `note <policynote-3.0.html>`__).
Example:
 
 
**MAXJOBQUEUEDPERUSERCOUNT**
Format:
<INTEGER>[,<INTEGER>]
Default:
0 (No Limit)
Details:
maximum number of idle jobs which can be considered for scheduling and
which can acquire 'system queue time' for increasing job priority.
(NOTE: This parameter is deprecated, see
`note <policynote-3.0.html>`__).
Example:
 
 
**MAXNODEPERUSERCOUNT**
Format:
<INTEGER>[,<INTEGER>]
Default:
0 (No Limit)
Details:
maximum allowed total PE count which can be dedicated at any given time.
(NOTE: This parameter is deprecated, see
`note <policynote-3.0.html>`__).
Example:
 
 
**MAXPEPERUSERCOUNT**
Format:
<INTEGER>[,<INTEGER>]
Default:
0 (No Limit)
Details:
maximum allowed total PE count which can be dedicated at any given time.
(NOTE: This parameter is deprecated, see
`note <policynote-3.0.html>`__).
Example:
 
 
**MAXPROCPERUSERCOUNT**
Format:
<INTEGER>[,<INTEGER>]
Default:
0 (No Limit)
Details:
maximum allowed total processors which can be dedicated at any give
time. (NOTE: This parameter is deprecated, see
`note <policynote-3.0.html>`__).
Example:
 
 
**MAXPSPERUSER**
Format:
<INTEGER>[,<INTEGER>]
Default:
0 (No Limit)
Details:
maximum allowed sum of outstanding dedicated processor-second
obligations of all active jobs. (NOTE: This parameter is deprecated, see
`note <policynote-3.0.html>`__).
Example:
 
 
**MAXWCPERUSER**
Format:
[[[DD:]HH:]MM:]SS[,[[[DD:]HH:]MM:]SS]
Default:
0 (No Limit)
Details:
maximum allowed sum of outstanding walltime limits of all active jobs.
NOTE: only available in Maui 3.2 and higher.
Example:
 
 
**MEMWEIGHT**\ [X]
Format:
<INTEGER>
Default:
0
Details:
specifies the coefficient to be multiplied by a job's MEM (dedicated
memory in MB) factor
Example:



::

    RESWEIGHT[0] 10




::

    MEMWEIGHT[0] 1000


(each job's priority will be increased by 10 \* 1000 \* its MEM factor)

 
 
**NODEACCESSPOLICY**
Format:
one of the following:\ **SHARED**, **SINGLEJOB**, **SINGLETASK** , or
**SINGLEUSER**
Default:
**SHARED**
Details:
specifies how node resources will be shared by various tasks (See the
'`Node Access Overview <5.3nodeaccess.html>`__ ' for more information)
Example:



::

    NODEACCESSPOLICY SINGLEUSER


(Maui will allow resources on a node to be used by more than one job
provided that the job's are all owned by the same user)

 
 
**NODEALLOCATIONPOLICY**
Format:
one of the following: **FIRSTAVAILABLE**, **LASTAVAILABLE**,
**MINRESOURCE**, **CPULOAD**, **PRIORITY, LOCAL, CONTIGUOUS, MAXBALANCE,
or FASTEST**
Default:
**LASTAVAILABLE**
Details:
specifies how Maui should allocate available resources to jobs. (See the
`Node Allocation <5.2nodeallocation.html>`__ section of the Admin manual
for more information)
Example:



::

    NODEALLOCATIONPOLICY MINRESOURCE


(Maui will apply the node allocation policy 'MINRESOURCE' to all jobs by
default)

 
 
**NODEAVAILABILITYPOLICY**
Format:
<POLICY>[:<RESOURCETPYE>] ...
where <POLICY> is one of **COMBINED**, **DEDICATED**, or **UTILIZED**
and <RESOURCETYPE> is one of **PROC**, **MEM**, **SWAP**, or **DISK**

Default:
**COMBINED**
Details:
specifies how Maui will evaluate node availability on a per resource
basis. (See the `Node Availability <5.4nodeavailability.html>`__ section
of the Admin manual for more information)
Example:



::

    NODEAVAILABILITYPOLICY DEDICATED:PROCS COMBINED:MEM


(Maui will ignore resource utilization information in locating available
processors for jobs but will use both dedicated and utilized memory
information in determining memory availability)

 
 
**NODECFG[X]**
Format:
list of space delimited <ATTR>=<VALUE> pairs where <ATTR> is one of the
following: **ACCESS, MAXJOB**, **MAXJOBPERUSER**, **MAXLOAD**, **FRAME**
, **SLOT**, **SPEED**, **PROCSPEED**, **PARTITION**, **NODETYPE** ,
**FEATURES**
Default:
[NONE]
Details:
specifies node-specific attributes for the node indicated in the array
field. See the `Node Configuration Overview <5.2nodeallocation.html>`__
for more information.
Example:



::

    NODECFG[nodeA] MAXJOB=2 SPEED=1.2


(Maui will only only two simultaneous jobs to run on node '``nodeA`` '
and will assign a relative machine speed of 1.2 to this node.)

 
 
**NODEDOWNSTATEDELAYTIME**
Format:
[[[DD:]HH:]MM:]SS
Default:
0:00:00
Details:
length of time Maui will assume down, drained (offline), or corrupt
nodes will remain unavailable for scheduling if a system reservation is
not explicitly created for the node. **NOTE**: This parameter is enabled
in Maui 3.0.7 and higher.
Example:



::

    NODEDOWNSTATEDELAYTIME 0:30:00


(Maui will assume down, drained, and corrupt nodes are not available for
scheduling for at least 30 minutes from the current time. Thus, these
nodes will never be allocated to starting jobs. Also, these nodes will
only be available for reservations starting more than 30 minutes in the
future.)

 
 
**NODELOADPOLICY**
Format:
one of the following: **ADJUSTSTATE** or **ADJUSTPROCS**
Default:
**ADJUSTSTATE**
Details:
specifies if a node's load affects its state or its available
processors. ADJUSTSTATE tells Maui to mark the node busy when MAXLOAD is
reached. ADJUSTPROCS causes the node's available procs to be equivalent
to MIN(ConfiguredProcs - DedicatedProcs,MaxLoad - CurrentLoad) NOTE:
NODELOADPOLICY only affects a node if MAXLOAD has been set.
Example:



::

    NODELOADPOLICY ADJUSTSTATE


(Maui will mark a node busy if its measured load exceeds its MAXLOAD
setting)

 
 
**NODEMAXLOAD**
Format:
<DOUBLE>
Default:
0.0
Details:
specifies that maximum load on a idle of running node. If the node's
load reaches or exceeds this value, Maui will mark the node 'busy'
Example:



::

    NODEMAXLOAD 0.75


(Maui will adjust the state of all Idle and Running nodes with a load >=
.75 to the state 'Busy')

 
 
**NODEPOLLFREQUENCY**
Format:
<INTEGER>
Default:
0 (Poll Always)
Details:
specifies the number of scheduling iterations between scheduler
initiated node manager queries.
Example:



::

    NODEPOLLFREQUENCY 5


(Maui will update node manager based information every 5 scheduling
iterations)

 
 
**NODESETATTRIBUTE**
Format:
one of **FEATURE**, **MEMORY**, or **PROCSPEED**
Default:
[NONE]
Details:
specifies the type of node attribute by which node set boundaries will
be established. **NOTE**: enabled in Maui 3.0.7 and higher. (See `Node
Set Overview <8.3nodesetoverview.html>`__ )
Example:



::

    NODESETATTRIBUTE PROCSPEED


(Maui will create node sets containing nodes with common processor
speeds)

 
 
**NODESETDELAY**
Format:
[[[DD:]HH:]MM:]SS
Default:
0:00:00
Details:
specifies the length of time Maui will delay a job if adequate idle
resources are available but not adequate resources within node set
constraints. **NOTE**: in Maui 3.2 and higher, setting NODESETDELAY to
any non-zero value will force Maui to always use nodesets. A value of
zero will cause Maui to use nodesets on a best effort basis. (See `Node
Set Overview <8.3nodesetoverview.html>`__)
Example:



::

    NODESETDELAY 0:00:00


(Maui will create node sets containing nodes with common processor
speeds)

 
 
**NODESETLIST**
Format:
<STRING>
Default:
[NONE]
Details:
specifies the list of node attribute values which will be considered for
establishing node sets. **NOTE**: enabled in Maui 3.0.7 and higher. (See
`Node Set Overview <8.3nodesetoverview.html>`__ )
Example:



::

    NODESETPOLICY ONEOF




::

    NODESETATTRIBUTE FEATURE




::

    NODESETLIST switchA switchB


(Maui will allocate nodes to jobs either using only nodes with the
'switchA' feature or using only nodes with the 'switchB' feature.)

 
 
**NODESETPOLICY**
Format:
one of **ONEOF**, **FIRSTOF**, or **ANYOF**
Format:
[NONE]
Details:
specifies how nodes will be allocated to the job from the various node
set generated. **NOTE**: enabled in Maui 3.0.7 and higher. (See `Node
Set Overview <8.3nodesetoverview.html>`__ )
Example:



::

    NODESETPOLICY ONEOF




::

    NODESETATTRIBUTE NETWORK


(Maui will create node sets containing nodes with common network
interfaces)

 
 
**NODESETPRIORITYTYPE**
Format:
one of **BESTFIT**, **WORSTFIT**, **BESTRESOURCE**, or **MINLOSS**
Default:
MINLOSS
Details:
specifies how resource sets will be selected when more than one feasible
resource can can be found. **NOTE**: This parameter is available in Maui
3.0.7 and higher. (See `Node Set Overview <8.3nodesetoverview.html>`__ )
Example:



::

    NODESETPRIORITYTYPE BESTRESOURCE




::

    NODESETATTRIBUTE PROCSPEED


(Maui will select the resource set containing the fastest nodes
available)

 
 
**NODESETTOLERANCE**
Format:
<FLOAT>
Default:
0.0 (Exact match only)
Details:
specifies the tolerance for selection of mixed processor speed nodes. A
tolerance of **X** allows a range of processors to be selected subject
to the constraint
(Speed.Max - Speed.Min) / Speed.Min <= X

| **NOTE**: Tolerances are only applicable when NODESETFEATURE is set to
  PROCSPEED. This parameter is available in Maui 3.0.7 and higher.
| (See `Node Set Overview <8.3nodesetoverview.html>`__ )

Example:



::

    NODESETATTRIBUTE PROCSPEED




::

    NODESETTOLERANCE 0.5


(Maui will only allocate nodes with up to a 50% procspeed difference.)

 
 
**NODESYNCTIME**
Format:
[[[DD:]HH:]MM:]SS
Default:
00:10:00
Details:
specifies the length of time after which Maui will sync up a node's
expected state with an unexpected reported state. IMPORTANT NOTE: Maui
will not start new jobs on a node with an expected state which does not
match the state reported by the resource manager. NOTE: this parameter
is named **NODESYNCDEADLINE** in Maui 3.0.5 and earlier.
Example:



::

    NODESYNCTIME 1:00:00


 
 
**NODEWEIGHT**
Format:
<INTEGER>
Default:
0
Details:
specifies the weight which will be applied to a job's requested node
count before this value is added to the job's cumulative priority.
**NOTE** : this weight currently only applies when a nodecount is
specified by the user job. If the job only specifies tasks or
processors, no node factor will be applied to the job's total priority.
(This will be rectified in future versions.)
Example:



::

    NODEWEIGHT 1000


 
 
**NOTIFICATIONPROGRAM**
Format:
<STRING>
Default:
[NONE]
Details:
specifies the name of the program to handle all notification call-outs
Example:



::

    NOTIFICATIONPROGRAM tools/notifyme.pl


 
 
**PEWEIGHT[X]**
Format:
<INTEGER>
Default:
0
Details:
specifies the coefficient to be multiplied by a job's PE (processor
equivalent) priority factor
Example:



::

    RESWEIGHT[0] 10




::

    PEWEIGHT[0] 100


(each job's priority will be increased by 10 \* 100 \* its PE factor)

 
 
**PLOTMAXPROC**
Format:
<INTEGER>
Default:
512
Details:
specifies the maximum number of processors requested by jobs to be
displayed in matrix outputs (as displayed by the
`showgrid <commands/showgrid.html>`__ or
`profiler <commands/profiler.html>`__ commands)
Example:



::

    PLOTMINPROC 1




::

    PLOTMAXPROC 1024


(each matrix output will display data in rows for jobs requesting
between 1 and 1024 processors)

 
 
**PLOTMAXTIME**
Format:
[[[DD:]HH:]MM:]SS
Default:
68:00:00
Details:
specifies the maximum duration of jobs to be displayed in matrix outputs
(as displayed by the `showgrid <commands/showgrid.html>`__ or
`profiler <commands/profiler.html>`__ commands)
Example:



::

    PLOTMINTIME 1:00:00




::

    PLOTMAXTIME 64:00:00


(each matrix output will display data in columns for jobs requesting
between 1 and 64 hours of run time)

 
 
**PLOTMINPROC**
Format:
<INTEGER>
Default:
1
Details:
specifies the minimum number of processors requested by jobs to be
displayed in matrix outputs (as displayed by the
`showgrid <commands/showgrid.html>`__ or
`profiler <commands/profiler.html>`__ commands)
Example:



::

    PLOTMINPROC 1




::

    PLOTMAXPROC 1024


(each matrix output will display data in rows for jobs requesting
between 1 and 1024 processors)

 
 
**PLOTMINTIME**
Format:
[[[DD:]HH:]MM:]SS
Default:
00:02:00
Details:
specifies the minimum duration of jobs to be displayed in matrix outputs
(as displayed by the `showgrid <commands/showgrid.html>`__ or
`profiler <commands/profiler.html>`__ commands)
Example:



::

    PLOTMINTIME 1:00:00




::

    PLOTMAXTIME 64:00:00


(each matrix output will display data in columns for jobs requesting
between 1 and 64 hours of run time)

 
 
**PLOTPROCSCALE**
Format:
<INTEGER>
Default:
9
Details:
specifies the number of rows into which the range of processors
requested per job will be divided when displayed in matrix outputs (as
displayed by the `showgrid <commands/showgrid.html>`__ or
`profiler <commands/profiler.html>`__ commands)
Example:



::

    PLOTMINPROC 1




::

    PLOTMAXPROC 1024




::

    PLOTPROCSCALE 10


(each matrix output will display job data divided into 10 rows which are
evenly spaced geometrically covering the range of jobs requesting
between 1 and 1024 processors)

 
 
**PLOTTIMESCALE**
Format:
<INTEGER>
Default:
11
Details:
specifies the number of columns into which the range of job durations
will be divided when displayed in matrix outputs (as displayed by the
`showgrid <commands/showgrid.html>`__ or
`profiler <commands/profiler.html>`__ commands)
Example:



::

    PLOTMINTIME 2:00:00




::

    PLOTMAXTIME 32:00:00




::

    PLOTTIMESCALE 5


(each matrix output will display job data divided into 5 columns which
are evenly spaced geometrically covering the range of jobs requesting
between 2 and 32 hours, i.e., display columns for 2, 4, 8, 16, and 32
hours of walltime)

 
 
**PREEMPTPOLICY**
Format:
one of the following:
**REQUEUE**, **SUSPEND**, **CHECKPOINT**
Format:
**REQUEUE**
Details:
specifies how preemptible jobs will be preempted (Available in Maui
3.2.2 and higher)
Example:



::

    PREEMPTPOLICY CHECKPOINT


(jobs that are to be preempted will be checkpointed and restarted at a
later time)

 
 
**PROCWEIGHT[X]**
Format:
<INTEGER>
Default:
0
Details:
specifies the coefficient to be multiplied by a job's requested
processor count priority factor
Example:



::

    PROCWEIGHT 2500


 
 
**PURGETIME**
Format:
[[[DD:]HH:]MM:]SS
Default:
0
Details:
The amount of time Maui will keep a job or node record for an object no
longer reported by the resource manager. Useful when using a resource
manager which 'drops' information about a node or job due to internal
failures. **NOTE**: In Maui 3.2.0 an higher, this parameter is
superseded by **JOBPURGETIME** and **NODEPURGETIME**
Example:



::

    PURGETIME 00:05:00


(Maui will maintain a job or node record for 5 minutes after the last
update regarding that object received from the resource manager.)

 
 
**QOSCFG[<QOSID>]**
Format:
list of zero or more space delimited <ATTR>=<VALUE> pairs where <ATTR>
is one of the following: **PRIORITY**, **FSTARGET**, **QTWEIGHT**,
**QTTARGET**, **XFWEIGHT**, **XFTARGET**, **PLIST**, **PDEF**,
**QFLAGS**, or a `fairness policy <6.2throttlingpolicies.html>`__
specification.
Default:
[NONE]
Details:
specifies QOS specific attributes. See the `flag
overview <jobflagoverview.html>`__ for a description of legal flag
values. **NOTE:** Available in Maui 3.0.6 and higher. **QOSCFG**
supersedes **QOSNAME**, **QOSPRIORITY**, **QOSFLAGS**, and other
'**QOS\***' parameters.
Example:



::

    QOSCFG[commercial] PRIORITY=1000 MAXJOB=4 MAXPROCS=80


(The scheduler will increase the priority of jobs using QOS commercial,
and will allow up to 4 simultaneous QOS commercial jobs with up to 80
total allocated processors.)

 
 
**QOSFEATURES[X]**
Format:
one or more node feature values or [ANY]
Default:
[ANY]
Details:
specifies which node features must be present on resources allocated to
jobs of the associated QOS. This parameter takes a QOS name as an array
index.
Example:



::

    QOSFEATURES[2] wide interactive


(jobs with a QOS value of 2 may only run on nodes with the feature
'wide' AND the feature 'interactive' set)

 
 
**QOSFLAGS[X]**
Format:
one or more of the following (space delimited)
IGNJOBPERUSER, IGNPROCPERUSER, IGNNODEPERUSER, IGNPSPERUSER,
IGNJOBQUEUEDPERUSER, IGNJOBPERGROUP, IGNPROCPERGROUP, IGNPSPERGROUP,
IGNJOBQUEUEDPERGROUP, IGNJOBPERACCOUNT, IGNPROCPERACCOUNT,
IGNPSPERACCOUNT, IGNJOBQUEUEDPERACCOUNT, IGNSYSMAXPROC, IGNSYSMAXTIME,
IGNSYSMAXPS, IGNSRMAXTIME, IGNUSER, IGNGROUP, IGNACCOUNT, IGNSYSTEM,
IGNALL, PREEMPT, DEDICATED, RESERVEALWAYS, USERESERVED, NOBF,
NORESERVATION, RESTARTPREEMPT
Default:
[NONE]
Details:
specifies the attributes of the corresponding QOS value See the Admin
Manual `QOS Overview <7.3qos.html>`__ section for details (NOTE: some
flags are only supported under Maui 3.1 and later)
Example:



::

    QOSFLAGS[1] ADVRES IGNMAXJOBPERUSER


(jobs with a QOS value of 1 must run in an advance reservation and can
ignore the MAXJOBPERUSER policy)

 
 
**QOSPRIORITY[X]**
Format:
<INTEGER>
Default:
0
Details:
specifies the priority associated with this QOS (NOTE: only used in Maui
3.0.x)
Example:



::

    QOSPRIORITY[2] 1000


(set the priority of QOS 2 to 1000)

 
 
**QOSQTTARGET[X]**
Format:
[[[DD:]HH:]MM:]SS
Default:
[NONE]
Details:
specifies the target job queuetime associated with this QOS
Example:



::

    QOSQTTARGET 2:00:00


 
 
**QOSQTWEIGHT[X]**
Format:
<INTEGER>
Default:
0
Details:
specifies the 'per QOS' queue time priority weight
Example:



::

    QOSQTWEIGHT 5


 
 
**QOSXFTARGET[X]**
Format:
<DOUBLE>
Default:
[NONE]
Details:
specifies the expansion factor target used in a job's 'Target Factor'
priority calculation
Example:



::

    QOSWEIGHT[3] 10




::

    QOSXFTARGET[3] 5.0


(jobs requesting a QOS of 3 will have their priority grow exponentially
as the job's minimum expansion factor approaches 5.0)

 
 
**QOSXFWEIGHT[X]**
Format:
<INTEGER>
Default:
0
Details:
specifies the weight which will be added to the base XFWEIGHT for all
jobs using QOS 'X'
Example:



::

    XFACTORWEIGHT[0] 100




::

    QOSXFWEIGHT[2] 1000


(jobs using QOS '2' will have an effective XFACTORWEIGHT of 1100 while
jobs using other QOS's will have an XFACTORWEIGHT of 100)

 
 
**QUEUETIMECAP[X]**
Format:
<DOUBLE>
Default:
0 (NO CAP)
Details:
specifies the maximum allowed pre-weighted queuetime priority factor.
Example:



::

    QUEUETIMECAP[0] 10000




::

    QUEUETIMEWEIGHT[0] 10


(a job that has been queued for 40 minutes will have its queuetime
priority factor calculated as 'Priority = QUEUETIMEWEIGHT \*
MIN(10000,40)')

 
 
**QUEUETIMEWEIGHT[X]**
Format:
<INTEGER>
Default:
1
Details:
specifies multiplier applied to a job's queue time (in minutes) to
determine the job's queuetime priority factor
Example:



::

    QUEUETIMEWEIGHT[0] 20


(a job that has been queued for 4:20:00 will have a queuetime priority
factor of 20 \* 260)

 
 
**REJECTNEGPRIOJOBS[X]**
Format:
<BOOLEAN>
Default:
TRUE
Details:
if enabled, the scheduler will refuse to start any job with a negative
priority. (see `ENABLENEGJOBPRIORITY <#enablenegjobpriority>`__)
Example:



::

    ENABLENEGJOBPRIORITY TRUE
    REJECTNEGPRIOJOBS    TRUE


(Any job with a priority less than 0 will be rejected)

 
 
**RESCTLPOLICY**
Format:
one of the following:
**ADMINONLY**, **ANY**
Format:
**ADMINONLY**
Details:
specifies who can create admin reservations (Available in Maui 3.2 and
higher)
Example:



::

    RESCTLPOLICY ANY


(any valid user can create an arbitrary admin reservation)

 
 
**RESDEPTH**
Format:
<INTEGER>
Default:
24
Details:
specifies the maximum number of reservations which can be on any single
node. IMPORTANT NOTE: on large way SMP systems, this value often must be
increased. To be on the safe side, this value should be approximately
twice the average sum of admin, standing, and job reservations present.
Example:



::

    RESDEPTH 64


 
 
**RESERVATIONDEPTH[X]**
Format:
<INTEGER>
Default:
1
Details:
specifies how many priority reservations are allowed in the associated
reservation stack
Example:



::

    RESERVATIONDEPTH[0] 4




::

    RESERVATIONQOSLIST[0] 1 3 5


(jobs with QOS values of 1, 3, or 5 can have a cumulative total of up to
4 priority reservations)

 
 
**RESERVATIONPOLICY**
Format:
one of the following: **CURRENTHIGHEST**, **HIGHEST**, **NEVER**
Default:
**CURRENTHIGHEST**
Details:
specifies how Maui reservations will be handled. (See also
`RESERVATIONDEPTH <#reservationdepth>`__ )
Example:



::

    RESERVATIONPOLICY CURRENTHIGHEST




::

    RESERVATIONDEPTH 2


(Maui will maintain reservations for only the two currently highest
priority jobs)

 
 
**RESERVATIONQOSLIST[X]**
Format:
one or more QOS values or [ALL]
Default:
[ALL]
Details:
specifies which QOS levels have access to the associated reservation
stack
Example:



::

    RESERVATIONDEPTH[0] 4




::

    RESERVATIONQOSLIST[0] 1 3 5


(jobs with QOS values of 1, 3, or 5 can have a cumulative total of up to
4 priority reservations)

 
 
**RESERVATIONRETRYTIME[X]**
Format:
[[[DD:]HH:]MM:]SS
Default:
0
Details:
Period of time Maui will continue to attempt to start a job in a
reservation when job start failures are detected due to resource manager
corruption
Example:
 
 
**RESCAP[X]**
Format:
<DOUBLE>
Default:
0 (NO CAP)
Details:
specifies the maximum allowed pre-weighted job resource priority factor
Example:



::

    RESCAP[0] 1000


(The total resource priority factor component of a job's priority will
not be allowed to exceed 1000, i.e., 'Priority = RESWEIGHT \*
MIN(RESCAP,<RESOURCEFACTOR>) + ...)

 
 
**RESOURCELIMITPOLICY**
Format:
<RESOURCE>:<POLICY>:<ACTION> [:<VIOLATIONTIME>]...
where **RESOURCE** is one of **PROC**, **DISK**, **SWAP**, or **MEM**,
where **POLICY** is one of **ALWAYS** or **EXTENDEDVIOLATION** and where
**ACTION** is one of **CANCEL**, **REQUEUE**, or **SUSPEND**

Default:
no limit enforcement
Details:
specifies how the scheduler should handle jobs which utilize more
resources than they request. **NOTE**: Only available in Maui 3.2 and
higher.
Example:



::

    RESOURCELIMITPOLICY MEM:ALWAYS:CANCEL


(Maui will cancel all jobs which exceed their requested memory limits.)

 
 
**RESWEIGHT[X]**
Format:
<INTEGER>
Default:
0
Details:
all resource priority components are multiplied by this value before
being added to the total job priority.
Example:



::

    RESWEIGHT[0] 5




::

    MEMORYWEIGHT[0] 10




::

    PROCWEIGHT[0] 100




::

    SWAPWEIGHT[0] 0




::

    RESCAP[0] 2000


(the job priority resource factor will be calculated as MIN(2000,5 \*
(10 \* JobMemory + 100 \* JobProc)))

 
 
**RMAUTHTYPE[X]**
Format:
one of **CHECKSUM**, **PKI**, or **SECUREPORT**
Default:
**CHECKSUM**
Details:
specifies the security protocol to be used in scheduler-resource manager
communication. NOTE: deprecated in Maui 3.2 - use **RMCFG**
Example:



::

    RMAUTHTYPE[0] CHECKSUM


(The scheduler will require a secure checksum associated with each
resource manager message)

 
 
**RMCFG**
Format:
one or more **key-value** pairs as described in the `Resource Manager
Configuration Overview <13.2rmconfiguration.html>`__
Default:
N/A
Details:
specifies the interface and policy configuration for the
scheduler-resource manager interface. Described in detail in the
`Resource Manager Configuration Overview <13.2rmconfiguration.html>`__
Example:



::

    RMCFG[bank] TYPE=PBS


(the PBS server will be used for resource management)

 
 
**RMNMPORT[X]**
Format:
<INTEGER>
Default:
(any valid port number)
Details:
specifies a non-default RM node manager through which extended node
attribute information may be obtained. NOTE: deprecated in Maui 3.2 -
use **RMCFG**
Example:



::

    RMNMPORT[0] 13001


(Maui will contact the node manager located on each compute node at port
13001)

 
 
**RMPOLLINTERVAL**
Format:
[[[DD:]HH:]MM:]SS
Default:
00:01:00
Details:
specifies interval between RM polls
Example:



::

    RMPOLLINTERVAL 60


(Maui will refresh its resource manager information every 60 seconds.
NOTE: this parameter specifies the global poll interval for all resource
managers)

 
 
**RMPORT[X]**
Format:
<INTEGER>
Default:
0
Details:
specifies the port on which Maui should contact the associated resource
manager. The value '0' specifies to use the appropriate default port for
the resource manager type selected. NOTE: deprecated in Maui 3.2 - use
**RMCFG**
Example:



::

    RMTYPE[0] PBS




::

    RMHOST[0] cws




::

    RMPORT[0] 20001


(Maui will attempt to contact the PBS server daemon on host cws, port
20001)

 
 
**RMSERVER[X]**
Format:
<HOSTNAME>
Default:
[NONE]
Details:
specifies the host on which Maui should contact the associated resource
manager. An empty value specifies to use the default hostname for the
resource manager selected. NOTE: this parameter is renamed **RMHOST** in
Maui 3.0.6 and higher. NOTE: deprecated in Maui 3.2 - use **RMCFG**
Example:



::

    RMTYPE[0] LL2




::

    RMHOST[0]




::

    RMPORT[0] 0


(Maui will attempt to contact the Loadleveler version 2 Negotiator
daemon on the default host and port, as specified in the LL config
files)

 
 
**RMTIMEOUT[X]**
Format:
<INTEGER>
Default:
15
Details:
seconds maui will wait for a response from the associated resource
manager. NOTE: deprecated in Maui 3.2 - use **RMCFG**
Example:



::

    RMTIMEOUT[1] 30


(The scheduler will wait 30 seconds to receive a response from resource
manager '1' before timing out and giving up. The scheduler will try
again on the next iteration.)

 
 
**RMTYPE[X]**
Format:
<RMTYPE>[:<RMSUBTYPE>] where <RMTYPE is one of the following: **LL**,
**PBS**, or **WIKI** and <RMSUBTYPE> is one of **RMS**
Default:
LL
Details:
specifies type of resource manager to be contacted by Maui. NOTE: for
**RMTYPE** **WIKI**, **RMAUTHTYPE** must be set to **CHECKSUM**. NOTE:
deprecated in Maui 3.2 - use **RMCFG**.
Example:



::

    RMTYPE[0] PBS




::

    RMHOST[0] cluster1




::

    RMPORT[0] 15003




::

    RMTYPE[1] PBS




::

    RMHOST[1] cluster2




::

    RMPORT[1] 15004


(Maui will interface to two different PBS resource managers, one located
on server cluster1 at port 15003 and one located on server cluster2 at
port 15004)

 
 
**SERVERHOST**
Format:
<HOSTNAME>
Default:
[NONE]
Details:
hostname of machine on which maui will run. NOTE: this parameter MUST be
specified.
Example:



::

    SERVERHOST geronimo.scc.edu


(Maui will execute on the host ``geronimo.scc.edu``)

 
 
**SERVERMODE**
Format:
one of the following:
**NORMAL**, **TEST**, or **SIMULATION**
Default:
NORMAL
Details:
specifies how Maui interacts with the outside world. See <Testing> for
more information
Example:



::

    SERVERMODE SIMULATION


 
 
**SERVERNAME**
Format:
<STRING>
Default:
<SERVERHOST>
Details:
Example:



::

    SERVERNAME mauiA


 
 
**SERVERPORT**
Format:
<INTEGER> (range: 1-64000)
Default:
40559
Details:
port on which maui will open its user interface socket
Example:



::

    SERVERPORT 30003


(Maui will listen for client socket connections on port 30003)

 
 
**SIMAUTOSHUTDOWN**
Format:
<BOOLEAN>
Default:
**TRUE**
Details:
if TRUE, the scheduler will end simulations when the active queue and
idle queue become empty
Example:



::

    SIMAUTOSHUTDOWN ON


(The scheduler simulation will end as soon as there are no jobs running
and no idle jobs which could run)

 
 
**SIMCPUSCALINGPERCENT**
Format:
<INTEGER>
Default:
100 (no scaling)
Details:
specifies whether to increase or decrease the runtime and wallclock
limit of each job in the workload trace file.
Example:
 
 
**SIMDEFAULTJOBFLAGS**
Format:
zero or more of the following:
**ADVRES**, **HOSTLIST**, **RESTARTABLE**, **PREEMPTEE** ,
**DEDICATED**, **PREEMPTOR**
Default:
[NONE]
Details:
cause Maui to force the specified job flags on all jobs supplied in the
workload trace file
Example:



::

    SIMDEFAULTJOBFLAGS DEDICATED


(Maui will set the 'DEDICATED' job flag on all jobs loaded from the
workload trace file)

 
 
**SIMEXITITERATION**
Format:
<INTEGER>
Default:
0 (no exit iteration)
Details:
iteration on which a Maui simulation will create a simulation summary
and exit.
Example:



::

    SIMEXITITERATION 36000


 
 
**SIMFLAGS**
Format:
zero or more of the following:
**IGNHOSTLIST**, **IGNCLASS**, **IGNQOS**, **IGNMODE**, **IGNFEATURES**
Default:
[NONE]
Details:
controls how Maui handles trace based information
Example:



::

    SIMFLAGS IGNHOSTLIST


(Maui will ignore hostlist information specified in the workload trace
file)

 
 
**SIMIGNOREJOBFLAGS**
Format:
zero or more of the following:
**ADVRES**, **HOSTLIST**, **RESTARTABLE**, **PREEMPTEE** ,
**DEDICATED**, **PREEMPTOR**
Format:
[NONE]
Details:
cause Maui to ignore specified job flags if supplied in the workload
trace file
Example:



::

    SIMIGNOREJOBFLAGS DEDICATED


(Maui will ignore the 'DEDICATED' job flag if specified in any job
trace)

 
 
**SIMINITIALQUEUEDEPTH**
Format:
<INTEGER>
Default:
16
Details:
specifies how many jobs the simulator will initially place in the idle
job queue
Example:



::

    SIMINITIALQUEUEDEPTH 64




::

    SIMJOBSUBMISSIONPOLICY CONSTANTJOBDEPTH


(Maui will initially place 64 idle jobs in the queue and, because of the
specified queue policy, will attempt to maintain this many jobs in the
idle queue throughout the duration of the simulation)

 
 
**SIMJOBSUBMISSIONPOLICY**
Format:
one of the following: **NORMAL**, **CONSTANTJOBDEPTH**, or
**CONSTANTPSDEPTH**
Default:
**CONSTANTJOBDEPTH**
Details:
specifies how the simulator will submit new jobs into the idle queue.
(NORMAL mode causes jobs to be submitted at the time recorded in the
workload trace file, **CONSTANTJOBDEPTH** and **CONSTANTPSDEPTH**
attempt to maintain an idle queue of <SIMINITIALQUEUEDEPTH> jobs and
procseconds respectively)
Example:



::

    SIMJOBSUBMISSIONPOLICY NORMAL


(Maui will submit jobs with the relative time distribution specified in
the workload trace file.)

 
 
**SIMNODECONFIGURATION**
Format:
one of the following:
**UNIFORM** or **NORMAL**
Default:
**NORMAL**
Details:
specifies whether or not maui will filter nodes based on resource
configuration while running a simulation
Example:
 
 
**SIMNODECOUNT**
Format:
<INTEGER>
Default:
0 (no limit)
Details:
specifies the maximum number of nodes maui will load from the simulation
resource file
Example:
 
 
**SIMRESOURCETRACEFILE**
Format:
<STRING>
Default:
traces/resource.trace
Details:
specifies the file from which maui will obtain node information when
running in simulation mode. Maui will attempt to locate the file
relative to <MAUIHOMEDIR> unless specified as an absolute path
Example:



::

    SIMRESOURCETRACEFILE traces/nodes.1


(Maui will obtain node traces when running in simulation mode from the
<MAUIHOMEDIR>``/traces/nodes.1`` file)

 
 
**SIMRMRANDOMDELAY**
Format:
<INTEGER>
Default:
0
Details:
specifies the random delay added to the RM command base delay
accumulated when making any resource manager call in simulation mode
Example:



::

    SIMRMRANDOMDELAY 5


(Maui will add a random delay of between 0 and 5 seconds to the
simulated time delay of all RM calls)

 
 
**SIMSTOPITERATION**
Format:
<INTEGER>
Default:
0 (no stop iteration)
Details:
specifies on which scheduling iteration a maui simulation will stop and
was for a command to resume scheduling
Example:



::

    SIMSTOPITERATION 1


(Maui should stop after the first iteration of simulated scheduling and
wait for admin commands)

 
 
**SIMTIMERATIO**
Format:
<INTEGER>
Default:
0 (no time ratio)
Details:
determines wall time speedup. Simulated Maui time will advance
<SIMTIMERATIO> \* faster than real wall time.
Example:



::

    SIMTIMERATIO 10


(Maui simulation time will advance 10 times faster than real world wall
time. For example, in 1 hour, Maui will process 10 hours of simulated
workload)

 
 
**SIMWORKLOADTRACEFILE**
Format:
<STRING>
Default:
traces/workload.trace
Details:
specifies the file from which maui will obtain job information when
running in simulation mode. Maui will attempt to locate the file
relative to <MAUIHOMEDIR> unless specified as an absolute path
Example:



::

    SIMWORKLOADTRACEFILE traces/jobs.2


(Maui will obtain job traces when running in simulation mode from the
<MAUIHOMEDIR>/traces/jobs.2 file)

 
 
**SRACCESS[X]**
Deprecated in Maui 3.2 and higher
Refer to `SRCFG <#SRCFG>`__
Format:
DEDICATED or SHARED
Default:
DEDICATED
Details:
If set to SHARED, allows a standing reservation to utilize resources
already allocated to other non-job reservations. Otherwise, these other
reservations will block resource access. (See `Managing
Reservations <7.1.5managingreservations.html>`__ )
Example:



::

    SRACCESS[test] SHARED


(Standing reservation 'test' may access resources allocated to existing
standing and administrative reservations)

 
 
**SRACCOUNTLIST[X]**
Deprecated in Maui 3.2 and higher
Refer to `SRCFG <#SRCFG>`__
Format:
list of valid account names
Default:
[NONE]
Details:
specifies that jobs with the associated accounts may use the resources
contained within this reservation
Example:



::

    SRACCOUNTLIST[1] ops staff


(jobs using the account ``ops`` or ``staff`` are granted access to the
resources in standing reservation '1')

 
 
**SRCHARGEACCOUNT[X]**
Deprecated in Maui 3.2 and higher
Refer to `SRCFG <#SRCFG>`__
Format:
any valid accountname
Default:
[NONE]
Details:
specifies the account to which maui will charge all idle cycles within
the reservation (via the allocation bank)
Example:



::

    SRCHARGEACCOUNT[test] steve


(The scheduler will charge all idle cycles within reservations
supporting standing reservation test to user 'steve')

 
 
**SRCFG[X]**
Format:
one or more of the following <ATTR>=<VALUE> pairs
**ACCESS**
**ACCOUNTLIST**
**CHARGEACCOUNT**
**CLASSLIST**
**DAYS**
**DEPTH**
**ENDTIME**
**FLAGS**
**GROUPLIST**
**HOSTLIST**
**JOBATTRLIST**
**NODEFEATURES**
**OWNER**
**PARTITION**
**PERIOD**
**PRIORITY**
**QOSLIST**
**RESOURCES**
**STARTTIME**
**TASKCOUNT**
**TIMELIMIT**
**TPN**
**TRIGGER**
**USERLIST**
NOTE: **HOSTLIST** and *ACL* list values must be comma delimited. (i.e.,
HOSTLIST=nodeA,nodeB)
Default:
[NONE]
Details:
specifies attributes of a standing reservation. Available in Maui 3.2
and higher. See `Managing
Reservations <7.1.5managingreservations.html>`__ for details.
Example:



::

    SRCFG[fast] STARTTIME=9:00:00 ENDTIME=15:00:00




::

    SRCFG[fast] HOSTLIST=node0[1-4]$




::

    SRCFG[fast] QOSLIST=high:low


(Maui will create a standing reservation running from 9:00 AM to 3:00 PM
on nodes 1 through 4 accessible by jobs with QOS high or low.)

 
 
**SRCLASSLIST[X]**
Deprecated in Maui 3.2
Refer to `SRCFG <#SRCFG>`__
Format:
list of valid class names
Default:
[NONE]
Details:
specifies that jobs requiring any of these classes may use the resources
contained within this reservation
Example:



::

    SRCLASSLIST[2] interactive


(maui will allow all jobs requiring any of the classes listed access to
the resources reserved by standing reservation '2')

 
 
**SRDAYS[X]**
Deprecated in Maui 3.2
Refer to `SRCFG <#SRCFG>`__
Format:
one or more of the following (space delimited)
**Mon,** **Tue,** **Wed**, **Thu,** **Fri,** **Sat,** **Sun**, or
**[ALL]**
Default:
**[ALL]**
Details:
specifies which days of the week the standing reservation will be active
Example:



::

    SRDAYS[1] Mon Tue Wed Thu Fri


(standing reservation '1' will be active on Monday thru Friday)

 
 
**SRDEPTH[X]**
Deprecated in Maui 3.2
Refer to `SRCFG <#SRCFG>`__
Format:
<INTEGER>
Default:
2
Details:
specifies the number of standing reservations which will be created (one
per day)
Example:



::

    SRDEPTH[1] 7


(specifies that standing reservations will be created for standing
reservation '1' for today, and the next 6 days)

 
 
**SRENDTIME[X]**
Deprecated in Maui 3.2
Refer to `SRCFG <#SRCFG>`__
Format:
[[HH:]MM:]SS
Default:
24:00:00
Details:
specifies the time of day the standing reservation becomes inactive
Example:



::

    SRSTARTTIME[2] 8:00:00




::

    SRENDTIME[2] 17:00:00


(standing reservation '2' is active from 8:00 AM until 5:00 PM)

 
 
**SRFEATURES[X]**
Deprecated in Maui 3.2
Refer to `SRCFG <#SRCFG>`__
Replaced with `NODEFEATURES <7.1.5managingreservations.html>`__
Format:
space delimited list of node features
Default:
[NONE]
Details:
specifies the required node features for nodes which will be part of the
standing reservation
Example:



::

    SRFEATURES[3] wide fddi


(all nodes used in the standing reservation must have both the 'wide'
and 'fddi' node attributes)

 
 
**SRFLAGS**
Deprecated in Maui 3.2
Refer to `SRCFG <#SRCFG>`__
Format:
colon delimited list of zero or more of the following flags:
**SINGLEUSE**\ \*
**BYNAME**
**PREEMPTEE**\ \*
**TIMEFLEX**\ \*
**FORCE**

Default:
[NONE]
Details:
specifies special reservation attributes. See `Managing
Reservations <7.1.5managingreservations.html>`__ for details.
Example:



::

    SRFLAGS[1] BYNAME


(Jobs may only access the resources within this reservation if they
explicitly request the reservation 'by name'

 
 
**SRGROUPLIST[X]**
Deprecated in Maui 3.2
Refer to `SRCFG <#SRCFG>`__
Format:
one or more space delimited group names
Default:
[ALL]
Details:
specifies the groups which will be allowed access to this standing
reservation
Example:



::

    SRGROUPLIST[1] staff ops special




::

    SRCLASSLIST[1] interactive


(Maui will allow jobs with the listed group ID's or which request the
job class 'interactive' to use the resources covered by standing
reservation 1.)

 
 
**SRHOSTLIST[X]**
Deprecated in Maui 3.2
Refer to `SRCFG <#SRCFG>`__
Format:
one or more space delimited host names
Default:
[ALL]
Details:
specifies the set of host from which Maui can search for resources to
satisfy the reservation. If SRTASKCOUNT is also specified, only
<SRTASKCOUNT> tasks will be reserved. Otherwise, all hosts listed will
be reserved.
Example:



::

    SRHOSTLIST[3] node001 node002 node003




::

    SRRESOURCES[3] PROCS=2;MEM=512




::

    SRTASKCOUNT[3] 2


(Maui will reserve 2 tasks - with 2 processors and 512 MB each, using
resources located on node001, node002, and/or node003)

 
 
**SRMAXTIME[X]**
Deprecated in Maui 3.2
Refer to `SRCFG <#SRCFG>`__
Replaced with `TIMELIMIT <7.1.5managingreservations.html>`__
Format:
[[[DD:]HH:]MM:]SS
Default:
-1 (no time based access)
Details:
specifies the maximum allowed overlap between a the standing reservation
and a job requesting resource access
Example:



::

    SRMAXTIME[6] 1:00:00


(Maui will allow jobs to access up to one hour of resources in standing
reservation 6)

 
 
**SRNAME[X]**
Deprecated in Maui 3.2
Refer to `SRCFG <#SRCFG>`__
SRNAME should no longer be used
Format:
<STRING>
Default:
[NONE]
Details:
specifies name of standing reservation <X>
Default:
Example:
 
 
**SRPARTITION[X]**
Deprecated in Maui 3.2
Refer to `SRCFG <#SRCFG>`__
Format:
<STRING>
Default:
[ALL]
Details:
specifies the partition in which the standing reservation should be
created
Example:



::

    SRPARTITION[0] OLD


(only select resource for standing reservation 0 in partition 'OLD')

 
 
**SRPERIOD[X]**
Deprecated in Maui 3.2
Refer to `SRCFG <#SRCFG>`__
Format:
one of **DAY**, **WEEK**, or **INFINITY**
Default:
**DAY**
Details:
specifies the periodicity of the standing reservation
Example:



::

    SRPERIOD[1] WEEK


(each standing reservation covers a one week period)

 
 
**SRQOSLIST[X]**
Deprecated in Maui 3.2
Refer to `SRCFG <#SRCFG>`__
Format:
zero or more valid QOS names
Default:
[NONE]
Details:
specifies that jobs with the listed QOS names can access the reserved
resources
Example:



::

    SRQOSLIST[1] 1 3 4 5


(maui will allow jobs using QOS 1, 3, 4, and 5 to use the reserved
resources)

 
 
**SRRESOURCES[X]**
Deprecated in Maui 3.2
Refer to `SRCFG <#SRCFG>`__
Format:
semicolon delimited <ATTR>=<VALUE> pairs
Default:
PROCS=-1 (All processors available on node)
Details:
specifies what resources constitute a single standing reservation task.
(each task must be able to obtain all of its resources as an atomic unit
on a single node) Supported resources currently include the following:
| PROCS (number of processors)
| MEM (real memory in MB)
| DISK (local disk in MB)
| SWAP (virtual memory in MB)

Example:



::

    SRRESOURCES[1] PROCS=1;MEM=512


(each standing reservation task will reserve one processor and 512 MB of
real memory)

 
 
**SRSTARTTIME[X]**
Deprecated in Maui 3.2
Refer to `SRCFG <#SRCFG>`__
Format:
[[HH:]MM:]SS
Default:
00:00:00
Details:
specifies the time of day the standing reservation becomes active
Example:



::

    SRSTARTTIME[1] 08:00:00




::

    SRENDTIME[1] 17:00:00


(standing reservation '1' is active from 8:00 AM until 5:00 PM)

 
 
**SRTASKCOUNT[X]**
Deprecated in Maui 3.2
Refer to `SRCFG <#SRCFG>`__
Format:
<INTEGER>
Default:
0
Details:
specifies how may tasks should be reserved for the reservation
Example:



::

    SRRESOURCES[2] PROCS=1;MEM=256




::

    SRTASKCOUNT[2] 16


(standing reservation '2' will reserve 16 tasks worth of resources, in
this case, 16 procs and 4 GB of real memory)

 
 
**SRTIMELOGIC[X]**
Deprecated in Maui 3.2
Refer to `SRCFG <#SRCFG>`__
Format:
**AND** or **OR**
Default:
**OR**
Details:
specifies how SRMAXTIME access status will be combined with other
standing reservation access methods to determine job access. If
SRTIMELOGIC is set to OR, a job is granted access to the reserved
resources if it meets the MAXTIME criteria or any other access criteria
(i.e., SRUSERLIST) If SRTIMELOGIC is set to AND, a job is granted access
to the reserved resources only if it meets the MAXTIME criteria and at
least on other access criteria
Example:



::

    SRMAXTIME[5] 1:00:00




::

    SRUSERLIST[5] carol charles




::

    SRTIMELOGIC[5] AND


(Maui will allow jobs from users carol and charles to use up to one hour
of resources in standing reservation 5)

 
 
**SRTPN[X]**
Deprecated in Maui 3.2
Refer to `SRCFG <#SRCFG>`__
Format:
<INTEGER>
Default:
0 (no TPN constraint)
Details:
specifies the minimum number of tasks per node which must be available
on eligible nodes.
Example:



::

    SRTPN[2] 4




::

    SRRESOURCES[2] PROCS=2;MEM=256


(Maui must locate at least 4 tasks on each node that is to be part of
the reservation. That is, each node included in standing reservation '2'
must have at least 8 processors and 1 GB of memory available)

 
 
**SRUSERLIST[X]**
Deprecated in Maui 3.2
Refer to `SRCFG <#SRCFG>`__
Format:
space delimited list of users
Default:
[NONE]
Details:
specifies which users have access to the resources reserved by this
reservation
Example:



::

    SRUSERLIST[1] bob joe mary


(users bob, joe and mary can all access the resources reserved within
this reservation)

 
 
**SRWENDTIME[X]**
Deprecated in Maui 3.2
Refer to `SRCFG <#SRCFG>`__
Format:
[[[DD:]HH:]MM:]SS
Default:
7:00:00:00
Details:
specifies the week offset at which the stand reservation should end
Example:



::

    SRSTARTTIME[1] 1:08:00:00




::

    SRENDTIME[1] 5:17:00:00


(standing reservation '1' will run from Monday 8:00 AM to Friday 5:00
PM)

 
 
**SRWSTARTTIME[X]**
Deprecated in Maui 3.2
Refer to `SRCFG <#SRCFG>`__
Format:
[[[DD:]HH:]MM:]SS
Default:
0:00:00:00
Details:
specifies the week offset at which the standing reservation should start
Example:



::

    SRSTARTTIME[1] 1:08:00:00




::

    SRENDTIME[1] 5:17:00:00


(standing reservation '1' will run from Monday 8:00 AM to Friday 5:00
PM)

 
 
**STATDIR**
Format:
<STRING>
Default:
stats
Details:
specifies the directory in which Maui statistics will be maintained
Example:



::

    STATDIR /var/adm/maui/stats


 
 
**SYSCFG**
Format:
list of zero or more space delimited <ATTR>=<VALUE> pairs where <ATTR>
is one of the following:
**PRIORITY**, **FSTARGET**, **QLIST**, **QDEF**, **PLIST** , **PDEF**,
**FLAGS**, or a `fairness policy <6.2throttlingpolicies.html>`__
specification.
Default:
[NONE]
Details:
specifies system-wide default attributes. See the `Attribute/Flag
Overview <jobflagoverview.html>`__ for more information.
Example:



::

    SYSCFG PLIST=Partition1 QDEF=highprio


(by default, all jobs will have access to partition ``Partition1`` and
will use the QOS ``highprio``)

 
 
**SWAPWEIGHT**
Format:
<INTEGER>
Default:
0
Details:
specifies the priority weight assigned to the virtual memory request of
a job
Example:



::

    SWAPWEIGHT 10


 
 
**SYSTEMDEFAULTJOBWALLTIME**
Format:
[[[DD:]HH:]MM:]SS
Default:
10:00:00:00
Details:
specifies the walltime for jobs which do not explicitly set this value
Example:



::

    SYSTEMDEFAULTJOBWALLTIME 1:00:00:00


(Maui will assign a wallclock limit of 1 day to jobs which do not
explicitly specify a wallclock limit)

 
 
**SYSTEMMAXPROCPERJOB**
Format:
<INTEGER>
Default:
-1 (NO LIMIT)
Details:
specifies the maximum number of processors that can be requested by any
single job
Example:



::

    SYSTEMMAXJOBPROC 256


(Maui will reject jobs requesting more than 256 processors)

 
 
**SYSTEMMAXPROCSECONDPERJOB**
Format:
<INTEGER>
Default:
-1 (NO LIMIT)
Details:
specifies the maximum number of proc-seconds that can be requested by
any single job
Example:



::

    SYSTEMMAXJOBPROCSECOND 86400


(Maui will reject jobs requesting more than 86400 procs seconds. i.e.,
64 processors \* 30 minutes will be rejected, while a 2 processor \* 12
hour job will be allowed to run)

 
 
**SYSTEMMAXJOBWALLTIME**
Format:
[[[DD:]HH:]MM:]SS
Default:
-1 (NO LIMIT)
Details:
specifies the maximum amount of wallclock time that can be requested by
any single job
Example:



::

    SYSTEMMAXJOBWALLTIME 1:00:00:00


(Maui will reject jobs requesting more than one day of walltime)

 
 
**TARGWEIGHT**
Format:
<INTEGER>
Default:
0
Details:
specifies the weight to be applied to a job's queuetime and expansion
factor target components
Example:



::

    TARGETWEIGHT 1000


 
 
**TASKDISTRIBUTIONPOLICY**
Format:
one of **DEFAULT** or **LOCAL**
Default:
**DEFAULT**
Details:
specifies how job tasks should be mapped to allocated resources.
Example:



::

    TASKDISTRIBUTIONPOLICY DEFAULT


(Maui should use standard task distribution algorithms)

 
 
**TRAPFUNCTION**
Format:
<STRING>
Default:
[NONE]
Details:
specifies the functions to be trapped
Example:



::

    TRAPFUNCTION UpdateNodeUtilization|GetNodeSResTime


 
 
**TRAPJOB**
Format:
<STRING>
Default:
[NONE]
Details:
specifies the jobs to be trapped
Example:



::

    TRAPJOB buffy.0023.0


 
 
**TRAPNODE**
Format:
<STRING>
Default:
[NONE]
Details:
specifies the nodes to be trapped
Example:



::

    TRAPNODE node001|node004|node005


 
 
**TRAPRES**
Format:
<STRING>
Default:
[NONE]
Details:
specifies the reservations to be trapped
Example:



::

    TRAPRES interactive.0.1


 
 
**USAGEWEIGHT**
Format:
<INTEGER>
Default:
0
Details:
specifies the weight assigned to the percent and total job usage
subfactors
Example:



::

    USAGEWEIGHT 100


 
 
**USAGEPERCENTWEIGHT**
Format:
<INTEGER>
Default:
Details:
Example:
 
 
**USEMACHINESPEED**
Format:
ON or OFF
Default:
OFF
Details:
specifies whether or not job wallclock limits should be scaled by the
machine speed of the node(s) they are running on.
Example:



::

    USEMACHINESPEED ON


(job <X> specifying a wallclock limit of 1:00:00 would be given only 40
minutes to run if started on a node with a machine speed of 1.5)

 
 
**USERCFG[<USERID>]**
Format:
list of zero or more space delimited <ATTR>=<VALUE> pairs where <ATTR>
is one of the following:
**PRIORITY**, **FSTARGET**, **QLIST**, **QDEF**, **PLIST** , **PDEF**,
**FLAGS**, or a `fairness policy <6.2throttlingpolicies.html>`__
specification.
Default:
[NONE]
Details:
specifies user specific attributes. See the `flag
overview <jobflagoverview.html>`__ for a description of legal flag
values.
Example:



::

    USERCFG[john] MAXJOB=50 QDEF=highprio


(up to 50 jobs submitted under the user ID ``john`` will be allowed to
execute simultaneously and will be assigned the QOS
``highprio by default.)                        ``
 
 
**USERWEIGHT**
Format:
<INTEGER>
Default:
0
Details:
specifies the weight assigned to the specified user priority (see
`Credential Priority Factor <5.1.2priorityfactors.html#cred>`__ )
Example:



::

    USERWEIGHT 100


 
 
**USESYSTEMQUEUETIME**
Format:
ON or OFF
Default:
OFF
Details:
specifies whether or not job prioritization should be based on the time
the job has been eligible to run, i.e., idle and meets all fairness
policies (ON) or the time the job has been idle (OFF). NOTE: In Maui
3.0.8 and higher, this parameter has been superseded by the
`JOBPRIOACCRUALPOLICY <#jobprioaccrualpolicy>`__ parameter.
Example:



::

    USESYSTEMQUEUETIME OFF


(the queuetime and expansion factor components of a job's priority will
be calculated based on the length of time the job has been in the idle
state.)



(See `QUEUETIMEFACTOR <5.1.2priorityfactors.html>`__ for more info)
 
 
**WCVIOLATIONACTION**
Format:
<one of CANCEL or PREEMPT>
Default:
CANCEL
Details:
specifies the action to take when a job exceeds its wallclock limit. If
set to cancel, the job will be terminated. If set to PREEMPT, the action
defined by PREEMPTPOLICY parameter will be taken.
Example:



::

    WCVIOLATIONACTION PREEMPT




::

    PREEMPTPOLICY REQUEUE


(Maui will requeue jobs which exceed their wallclock limit)

 
 
**XFACTORCAP**
Format:
<DOUBLE>
Default:
0 (NO CAP)
Details:
specifies the maximum total pre-weighted contribution to job priority
which can be contributed by the expansion factor component. This value
is specified as an absolute priority value, not as a percent.
Example:



::

    XFACTORCAP 10000


(Maui will not allow a job's pre-weighted XFactor priority component to
exceed the value 10000)

 
 
**XFMINWCLIMIT**
Format:
[[[DD:]HH:]MM:]SS
Default:
-1 (NO LIMIT)
Details:
specifies the minimum job wallclock limit that will be considered in job
expansion factor priority calculations
Example:



::

    XFMINWCLIMIT 0:01:00


(jobs requesting less than one minute of wallclock time will be treated
as if their wallclock limit was set to one minute when determining
expansion factor for priority calculations)

 
 
**XFACTORWEIGHT**
Format:
<INTEGER>
Default:
0
Details:
specifies the weight to be applied to a job's minimum expansion factor
before it is added to the job's cumulative priority
Example:



::

    XFACTORWEIGHT 1000


(Maui will multiply a job's XFactor value by 1000 and then add this
value to its total priority
