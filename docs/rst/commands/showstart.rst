
.. rubric:: showstart
   :name: showstart

**(Show Start Estimate)**
**Synopsis**

::

    showstart {jobid|proccount[@duration]|s3jobspec} [-e {all|hist|prio|rsv}]
              [-f] [-g [peer]] [-l qos=<QOS>] [--format=xml]

**Overview**

This command displays the estimated start time of a job based a number
of analysis types. This analysis may include information based on
historical usage, earliest available reservable resources, and priority
based backlog analysis. Each type of analysis will provide somewhat
different estimates base on current cluster environmental conditions. By
default, only reservation based analysis is performed.
\ **Historical** analysis utilizes historical queue times for
jobs which match a similiar processor count and job duration profile.
This information is updated on a sliding window which is configurable
within ``moab.cfg``

\ **Reservation** based start time estimation incorporates
information regarding current administrative, user, and job reservations
to determine the earliest time the specified job could allocate the
needed resources and start running. In essence, this estimate will
indicate the earliest time the job would start *assuming this job was
the highest priority job in the queue*.

\ **Priority** based job start analysis determines when the
queried job would fit in the queue and determines the estimated amount
of time required to complete the jobs which are currently running or
scheduled to run before this job can start.

In all cases, if the job is running, this command will return the time
the job started. If the job already has a reservation, this command will
return the start time of the reservation.

**Access**

By default, this command can be run by any user.
**Parameters**

Parameter
Description
\ **DURATION**
duration of pseudo-job to be checked in format [[[DD:]HH:]MM:]SS
(default duration is 1 second)
\ **-e**
estimate method. By default, Moab will use the reservation based
estimation method.
\ **-f**
use *feedback*. If specified, Moab will apply historical accuracy
information to improve the quality of the estimate. See
`ENABLESTARTESTIMATESTATS <../a.fparameters.html#enablestartestimatestats>`__
for more information.
\ **-g**
grid mode. Obtain showstart information from remote resource managers.
If -g is not used and Moab determines that job is already migrated, Moab
obtains showstart information form the remote Moab where the job was
migrated to. All resource managers can be queried by using the keyword
"all" which returns all information in a table.


::

    $ showstart -g all head.1

    Estimated Start Times

    [ Remote RM ] [ Reservation ] [ Priority ] [ Historical ]
    [ c1 ] [ 00:15:35 ] [ ] [ ]
    [ c2 ] [ 3:15:38 ] [ ] [ ]


\ **-l qos=<QOS>**
Specifies what QOS the job must start under, using the same syntax as
the `**msub** <msub.html>`__ command. Currently, no other resource
manager extensions are supported. This flag only applies to hypothetical
jobs by using the ``proccount[@duration]`` syntax.
\ **JOBID**
job to be checked
\ **PROCCOUNT**
number of processors in pseudo-job to be checked
\ **S3JOBSPEC**
XML describing the job according to the Dept. of Energy `Scalable
Systems Software <../SSSWireProtocol_3.0.3.html>`__/S3 job
specification.
**Example 1**


::

    > showstart orion.13762

    job orion.13762 requires 2 procs for 0:33:20

    Estimated Rsv based start in                 1:04:55 on Fri Jul 15 12:53:40
    Estimated Rsv based completion in            2:44:55 on Fri Jul 15 14:33:40

    Estimated Priority based start in            5:14:55 on Fri Jul 15 17:03:40
    Estimated Priority based completion in       6:54:55 on Fri Jul 15 18:43:40

    Estimated Historical based start in         00:00:00 on Fri Jul 15 11:48:45
    Estimated Historical based completion in     1:40:00 on Fri Jul 15 13:28:45

    Best Partition: fast


**Example 2**


::

    > showstart 12@3600

    job 12@3600 requires 12 procs for 1:00:00
    Earliest start in         00:01:39 on Wed Aug 31 16:30:45
    Earliest completion in     1:01:39 on Wed Aug 31 17:30:45
    Best Partition: 32Bit


+----------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| |Note|   | You cannot specify job flags when running **showstart**, and since a job by default can only run on one partition, **showstart** fails when querying for a job requiring more nodes than the largest partition available.   |
+----------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

**Additional Information**

For reservation based estimates, the information provided by this
command is more highly accurate if the job is highest priority, if the
job has a reservation, or if the majority of the jobs which are of
higher priority have reservations.  Consequently, sites wishing to make
decisions based on this information may want to consider using the
`RESERVATIONDEPTH <../a.fparameters.html#reservationdepth>`__ parameter
to increase the number of priority based reservations.  This can be set
so that most, or even all idle jobs receive priority reservations and
make the results of this command generally useful.  The only caution of
this approach is that increasing the **RESERVATIONDEPTH** parameter more
tightly constrains the decisions of the scheduler and may resulting in
slightly lower system utilization (typically less than 8% reduction). 
.. rubric:: See Also
   :name: see-also

-  `Moab Client Installation <../2.2installation.html#client>`__ -
   explains how to distribute this command to client nodes
-  `checkjob <checkjob.html>`__
-  `showres <showres.html>`__
-  `showstats -f eststarttime <showstatsf.html#eststarttime>`__
-  `showstats -f avgqtime <showstatsf.html#avgqtime>`__
-  `Job Start Estimates <../15.3jobstarttimeestimates.html>`__

