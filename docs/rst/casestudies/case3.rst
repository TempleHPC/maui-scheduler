Case Study: Background Load

**Overview**

A 64 proc Altix system needs to be scheduled with a significant
'background' load.

**Resources**

+-----------------------+--------------------------------------------+
| Compute Nodes:        | 64 processor Altix system with 32 GB RAM   |
+-----------------------+--------------------------------------------+
| Resource Manager:     | OpenPBS 2.3                                |
+-----------------------+--------------------------------------------+
| Network:              | InternalSGI                                |
+-----------------------+--------------------------------------------+

**Workload**

+-----------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Job Size:       | Range in size from 1 to 32 processors                                                                                                                                                                                                                                              |
+-----------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Job Length:     | Jobs range in length from 15 minutes to 48 hours                                                                                                                                                                                                                                   |
+-----------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Job Owners:     | Various                                                                                                                                                                                                                                                                            |
+-----------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Notes:          | This is a login/development machine.  At any given time, there may be a significant load from jobs and processes outside of the resource manager's view or control.  The major impact of this load related to scheduling is in the area of cpu load and real memory consumption.   |
+-----------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

**Goals**

The scheduler must run the machine at maximum capacity without
overcommitting either memory or processors.  A significant, variable
background load exists from jobs submitted outside of the resource
manager's view or control.  The scheduler must track and account for
this load and allow space for some variability and growth of this load
over time.  The scheduler should also kill any job which violates its
requested resource allocation and notify the associated user of this
violation.

The scheduler should maximize the throughput associated with the queued
jobs while avoiding starvation as a secondary concern. 

**Analysis**

The background load causes many problems in any mixed batch/interactive
environment.  One problem is that a situation may arise in which the
highest priority batch job cannot run.  Moab can make a reservation for
this highest priority job.  But because there are no constraints on the
background load, Moab cannot determine when it will drop enough to allow
this job to run.  By default, it optimistically attempts a reservation
for the next scheduling iteration, perhaps 1 minute out. 

The problem is that this reservation now exists one minute out and when
Moab attempts to backfill, it can only consider jobs which request less
than one minute or which can fit "beside" this high priority job. 
During the next iteration, Moab still cannot run the job because the
background load has not dropped and again creates a new reservation for
one minute into the future. 

The background load has basically turned batch scheduling into an
exercise in "resource scavenging."  If the priority job reservation was
not there, other smaller queued jobs might be able to run.  Therefore,
to maximize the "scavenging" effect, the scheduler should be configured
to allow this high priority job the first opportunity to use available
resources but prevent it from reserving these resources if it cannot run
immediately. 

**Configuration**

The configuration needs to accomplish several main objectives including:

-  track the background load to prevent oversubscription
-  favor small, short jobs to maximize job turnaround
-  prevent blocked high priority jobs from creating reservations
-  work with an allocation manager to charge for utilized CPU time
-  cancel jobs which exceed specified resource limits
-  notify users when a job is canceled due to resource usage limit
   violations

::

    # allow jobs to share node
    NODEACCESSPOLICY           SHARED

    # track background load
    NODELOADPOLICY             ADJUSTPROCS
    NODEUNTRACKEDLOADFACTOR    1.2

    # favor short jobs, disfavor large jobs
    QUEUETIMEWEIGHT            0
    RESOURCEWEIGHT             -10
    PROCWEIGHT                 128
    MEMWEIGHT                  1
    XFACTOR                    1000

    # disable priority reservations for the default QOS
    QOSFLAGS[0]                NORESERVATION

    # debit by CPU
    BANKTYPE                   QBANK
    BANKSERVER                 develop1
    BANKPORT                   2334
    BANKCHARGEMODE             DEBITSUCCESSFULLCPU

    # kill resource hogs
    RESOURCELIMITPOLICY        WALLTIME:ALWAYS:CANCEL

    # notify user of job events
    NOTIFYSCRIPT               tools/notify.pl

**Monitoring**

The most difficult aspects of this environment are properly reserving
space for the untracked background load.  Since this load is outside the
control of the scheduler and resource manager, it has no constraints. 
It could grow suddenly and overwhelm the machine, or just as easily
disappear.  The NODEUNTRACKEDLOADFACTOR parameter provides slack for
this background load to grow and shrink.  However, since there is no
control over the load, the effectiveness of this parameter will depend
on the statistical behavior of the load.  The greater the value, the
more slack provided, and the less likely the system is to be
overcommitted.  However, a larger value also means more resources are in
this reserve which are not available for scheduling. 

The second aspect of this environment which must be monitored is the
trade-off between high job throughput and job starvation.  The 'locally
greedy' approach of favoring the smallest, shortest jobs will have a
negative effect on larger and longer jobs.  The large, long jobs which
have been queued for some time can be pushed to the front of the queue
by increasing the
`QUEUETIMEWEIGHT <../a.fparameters.html#queuetimeweight>`__ factor until
a satisfactory balance is achieved. 

**Conclusions**

The right solution is to migrate the users over to the batch system or
provide them with a constrained resource 'box' to play in, either
through a processor partition, standing reservation, or a logical
software system.  The value in this is that it prevents this
unpredictable background load from wreaking havoc with a dedicated
resource reservation system. 

Moab can reserve resources for jobs according to all currently available
information.  However, the unpredictable nature of the background load
means that those resources may not be available when they should be,
which result in canceled reservations and hinder enforcement of site
policies and priorities. 
