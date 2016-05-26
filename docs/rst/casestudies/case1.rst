Case Study: Mixed Parallel/Serial Homogeneous Cluster
#####################################################

**Overview**

A multi-user site wishes to control the distribution of compute cycles
while minimizing job turnaround time and maximizing overall system
utilization. 

**Resources**

+-----------------------+---------------------------------------------------------------------------+
| Compute Nodes:        | 64 2-way SMP Linux® nodes with 512 MB RAM and 16 GB local scratch space   |
+-----------------------+---------------------------------------------------------------------------+
| Resource Manager:     | OpenPBS 2.3                                                               |
+-----------------------+---------------------------------------------------------------------------+
| Network:              | 100 MB switched Ethernet                                                  |
+-----------------------+---------------------------------------------------------------------------+

**Workload**

+-----------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Job Size:       | Range in size from 1 to 32 processors with approximate quartile job frequency distribution of:                                                                                                                                                                                                                                                                                                                                                                                 |
|                 | 1 - 2, 3 - 8, 9 - 24, and 25 - 32 nodes                                                                                                                                                                                                                                                                                                                                                                                                                                        |
+-----------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Job Length:     | Jobs range in length from 1 to 24 hours                                                                                                                                                                                                                                                                                                                                                                                                                                        |
+-----------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Job Owners:     | Six major groups consisting of about 50 users in total                                                                                                                                                                                                                                                                                                                                                                                                                         |
+-----------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Notes:          | During prime time hours, the majority of jobs submitted are smaller, short running development jobs where users are testing out new code and new data sets.  The owners of these jobs are often unable to proceed with their work until a job they have submitted completes.  Many of these jobs are interactive in nature.  Throughout the day, large, longer running production workload is also submitted but these jobs do not have comparable turnaround time pressure.   |
+-----------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

**Goals**

The groups 'Meteorology' and 'Statistics' must receive approximately 45%
and 35% of the total computing resources respectively.  Nodes cannot be
shared among tasks from different jobs. 

The system should attempt to minimize turnaround time during prime hours
(Mon - Fri, 8:00 AM to 5:00 PM) and maximize system utilization during
all other times.  System maintenance should be scheduled efficiently
within these constraints. 

**Analysis**

The network topology is flat and nodes are homogeneous.  This makes life
significantly simpler.  The focus for this site is controlling
distribution of compute cycles without negatively impacting overall
system turnaround and utilization.  Currently, the best mechanism for
doing this is `Fairshare <../6.3fairshare.html>`__. 

Fairshare can be used to adjust the priority of jobs to favor or
disfavor jobs based on fairshare targets and historical usage.  In
essence, this feature improves the turnaround time of the jobs not
meeting their fairshare target at the expense of those that are. 
Depending on the criticality of the resource distribution constraints,
an allocation bank such as
`Gold <http://www.adaptivecomputing.com/gold>`__, which enables more
stringent control over the amount of resources which can be delivered to
various users, may be desirable. 

To manage the prime time job turnaround constraints, a `standing
reservation <../7.1.3standingreservations.html>`__ would probably be the
best approach.  A standing reservation can be used to set aside a subset
of the nodes for quick turnaround jobs.  This reservation can be
configured with a time based access point to allow only jobs which will
complete within some time X to utilize these resources.  The reservation
has advantages over a typical queue based solution in this case in that
these quick turnaround jobs can be run anywhere resources are available,
either inside, or outside the reservation, or even crossing reservation
boundaries.  The site does not have any hard constraints about what is
acceptable turnaround time so the best approach would probably be to
analyze the site's workload under a number of configurations using the
`simulator <../16.3.0simulations.html>`__ and observe the corresponding
scheduling behavior.

For general optimization, there are a number of scheduling aspects to
consider, scheduling algorithm, reservation policies, node allocation
policies, and job prioritization.  It is almost always a good idea to
utilize the scheduler's `backfill <../8.2backfill.html>`__ capability
since this has a tendency to increase average system utilization and
decrease average turnaround time in a surprisingly fair manner.  It does
tend to favor somewhat small and short jobs over others which is exactly
what this site desires.  Reservation policies are often best left alone
unless rare starvation issues arise or quality of service policies are
desired.  Node allocation policies are effectively meaningless since the
system is homogeneous.  The final scheduling aspect, job prioritization,
can play a significant role in meeting site goals. 

To maximize overall system utilization, maintaining a significant
`Resource <../5.1.4prioritystrategies.html>`__ priority factor will
favor large resource (processor) jobs, pushing them to the front of the
queue.  Large jobs, though often only a small portion of a site's job
count, regularly account for the majority of a site's delivered compute
cycles.  To minimize job turnaround, the
`XFactor <../5.1.2priorityfactors.html>`__ priority factor will favor
short running jobs.  Finally, in order for fairshare to be effective, a
significant `Fairshare <../6.3fairshare.html>`__ priority factor must be
included. 

**Configuration**

For this scenario, a resource manager configuration consisting of a
single, global queue/class with no constraints would allow Moab the
maximum flexibility and opportunities for optimization. 

The following Moab configuration would be a good initial stab:

::

    # reserve 16 processors during prime time for jobs requiring less than 2 hours to complete
    SRNAME[0]          fast
    SRTASKCOUNT[0]     16
    SRDAYS[0]          MON TUE WED THU FRI
    SRSTARTTIME[0]     8:00:00
    SRENDTIME[0]       17:00:00
    SRMAXTIME[0]       2:00:00

    # prioritize jobs for Fairshare, XFactor, and Resources
    RESOURCEWEIGHT     20
    XFACTORWEIGHT      100
    FAIRSHAREWEIGHT    100

    # disable SMP node sharing
    NODEACCESSPOLICY   DEDICATED

::

    Group:Meteorology  FSTARGET=45
    Group:Statistics   FSTARGET=35


**Monitoring**

The `mdiag -f <../commands/mdiag-fairshare.html>`__ command will allow
you to monitor the effectiveness of the fairshare component of your job
prioritization.  Adjusting the Fairshare priority factor will make
fairshare more or less effective.  A trade off must occur between
fairshare and other goals managed via job prioritization. 
The `mdiag -p <../commands/mdiag-priority.html>`__ command will help you
analyze the priority distributions of the currently idle jobs.  The
`showstats -f AVGXFACTOR <../commands/showstatsf.html>`__ command will
provide a good indication of average job turnaround while the
`showstats <../commands/showstats.html>`__ command will give an
excellent analysis of longer term historical performance statistics. 

**Conclusions**

Any priority configuration will need to be tuned over time because the
effect of priority weights is highly dependent upon the site specific
workload.  Additionally, the priority weights themselves are part of a
feedback loop which adjust the site workload.  However, most sites
quickly stabilize and significant priority tuning is unnecessary after a
few days.
