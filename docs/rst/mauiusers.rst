.. rubric:: Maui Users Manual
   :name: maui-users-manual

**Maui Introduction**
The Maui Scheduler was designed to offer improved job management and
scheduling to users while allowing users to continue 'business as
usual'. In fact, users do not need to change anything in the way they
submit and track jobs when Maui is installed. However, if a user
chooses, there are many new features and commands which can be utilized
to improve the user's ability to run jobs when, where, and how they
want.

The Maui Scheduler, as its name suggests, is a scheduler. It is not a
resource manager. A resource manager, such as PBS, Loadleveler, or LSF,
manages the job queue and manages the compute nodes. A scheduler tells
the resource manager what to do, when to run jobs, and where. Users
typically submit jobs and query the state of the machine and jobs
through the resource manager. When Maui is running, users can continue
to issue the exact same resource manager commands as before. However,
Maui also offers commands which provide additional information and
capabilities.

| Maui capabilities include many internal mechanisms to improve overall
  scheduling performance, allowing users to run more jobs on the same
  system and get their results back more quickly. Additionally, Maui
  allows users to create resource reservations which guarantee resource
  availability at particular times. Quality of service features are also
  enabled which allow a user to request improved job turnaround time,
  access to additional resources, or exemptions to particular policies
  automatically. (The site administrator may choose to make some of
  these capabilities only available at a higher 'job cost')

--------------

**Maui Overview**
Maui is an advanced cluster scheduler capable of optimizing scheduling
and node allocation decisions. It allows site administrators extensive
control over which jobs are considered eligible for for scheduling, how
the jobs are prioritized, and where these jobs are run. Maui supports
advance reservations, QOS levels, backfill, and allocation management.
Each of these features, if enabled, may require some adjustment on the
part of the user to optimize system performance.

--------------

**Backfill**
Backfill is a scheduling approach which allows some jobs to be run 'out
of order' so long as they do not delay the highest priority jobs in the
queue. In order to determine whether or not a job will be delayed, each
job must supply an estimate of how long it will need to run. This
estimate, known as a wallclock limit, is an estimation of the wall time
(or elapsed time) from job start to job finish. It is often wise to
slightly overestimate this limit because the scheduler may be configured
to kill jobs which exceed their wallclock limits. However,
overestimating a job's wallclock time by too much will prevent the
scheduler from being able to optimize the job queue as much as possible.
The more accurate the wallclock limit, the more 'holes' Maui can find to
start your job early.

Maui also provides the command `showbf <commands/showbf.html>`__ to
allow users to see exactly what resources are available for immediate
use. This can allow users to configure a job that will be able to run a
soon as it is submitted by utilizing only available resources.

Backfill scheduling significantly improves the ability of the scheduler
to utilize the available resources. Consequently, using backfill
scheduling increases system utilization and throughput while decreasing
average job queue time. Fortunately, backfill is a very forgiving
algorithm, allowing even jobs with very poor wallclock estimates to
benefit from it. However, better estimates will increase the amount of
improvement backfill scheduling can provide for your jobs.

--------------

**Allocation Management**
Maui possesses interfaces to a number of allocation management systems
such as PNNL's QBank. These systems allow each user to be given a
portion of the total compute resources available on the system. These
systems work by associating each user with one or more accounts. When a
job is submitted, the user specifies which account should be charged for
the resources consumed by the job. Default accounts may be specified to
automate the account specification process in most cases. If such a
system is being used at your site, your system administrators will
inform you as to if and how accounts should be specified.

--------------

` <>`__\ **Advance Reservations**
| Advance reservations allow a site to set aside certain resources for
  specific uses over a given timeframe. Access to a given reservation is
  controlled by a reservation-specific access control list (ACL) which
  determines who or what can use the reserved resources. It is important
  to note that while reservation ACL's *allow* particular jobs to
  utilize reserved resources, they do not *force* the job to utilize
  these resources. Maui will attempt to locate the best possible
  combination of available resources whether these are reserved or
  unreserved. For example, in the figure below, note that job X, which
  meets access criteria for both reservation A and B, allocates a
  portion of its resources from each reservation and the remainder from
  resources outside of both reservations.
| |image0|
| While by default, reservations make resources available to jobs which
  meet particular criteria, Maui can be configured to constrain jobs to
  only run within accessible reservations. Specifically, jobs can be
  forced to run only within reserved resources on a job by job basis or
  by specifying special QoS constraints. (See the `Resource Manager
  Extensions <13.3rmextensions.html>`__ and `QoS
  Facilities <7.3qos.html>`__ sections of the Maui Admin Manual for more
  information.)

See Also:

`Advance Reservation Overview <7.1advancereservations.html>`__, Maui
Admin Manual

--------------

**Quality of Service (QOS)**
The Maui QOS features allow a site to grant special privileges to
particular users. These benefits can include access to additional
resources, exemptions from certain policies, access to special
capabilities, and improved job prioritization. Each site determines
which advantages are important to make available and to whom. If you are
granted special QOS access, you can specify the QOS to use for your job
using the **QOS** keyword.

--------------

**Statistics**
Maui tracks a large number of statistics to help users determine how
well and how often their jobs are running. The
`showstats <commands/showstats.html>`__ command provides detailed
statistics on a per user, per group, and per account basis.
Additionally, the command `showgrid <commands/showgrid.html>`__ can be
used to determine what types of jobs get the best scheduling performance
allowing users to 'tune' their jobs to obtain optimal turnaround time.

--------------

**Diagnosis**
Maui provides the `checkjob <commands/checkjob.html>`__ command to allow
users to view a detailed status of each job they have submitted. This
command show all job attribute and state information and also provides
an analysis of whether or not the job can run. If the job is unable to
run, this command will provide a breakdown of these reasons why. The
`showstart <commands/showstart.html>`__ command provides an estimate of
job start time beyond this.

If your job still will not start, contact your system administrator. He
will have access to additional commands and detailed Maui logs which
will reveal exactly why the job cannot run.

--------------

**Workload Information**
| Maui offers an extensive array of job prioritization options to allow
  sites to control exactly how jobs run through the job queue. If your
  site administrators have chosen to take advantage of this, the job
  ordering shown by your resource manager queue listing command (i.e.,
  llq, qstat) will not reflect this. Maui provides the
  `showq <commands/showq.html>`__ command to display a relevant listing
  of both active and idle jobs.
