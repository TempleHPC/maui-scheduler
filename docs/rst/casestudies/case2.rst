Case Study: Partitioned Heterogeneous Cluster
#############################################

**Overview**

A site possessing a mixture of uniprocessor and dual processor nodes
desires to dedicate a subset of nodes to time-sharing serial jobs, a
subset to parallel batch jobs, and provide a set of nodes to be used as
overflow.

**Resources**

+---------------------+--------------------------------------------------------------------------------------------+
| Compute Nodes:      | *Group A*: 16 single processor Linux® nodes with 128 MB RAM and 1 GB local scratch space   |
|                     | *Group B*: 8 2-way SMP Linux® nodes with 256 MB RAM and 4 GB local scratch space           |
|                     | *Group C*: 8 single processor Linux® nodes with 192 MB RAM and 2 GB local scratch space    |
+---------------------+--------------------------------------------------------------------------------------------+
| Resource Manager:   | OpenPBS 2.3                                                                                |
+---------------------+--------------------------------------------------------------------------------------------+
| Network:            | 100 MB switched Ethernet                                                                   |
+---------------------+--------------------------------------------------------------------------------------------+

**Workload**

+---------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Job Size:     | Range in size from 1 to 32 processors with approximate quartile job frequency distribution of:                                                                                                                                                                                                                                                                                                                                                                              |
|               | 1 - 2, 3 - 8, 9 - 24, and 25 - 32 nodes                                                                                                                                                                                                                                                                                                                                                                                                                                     |
+---------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Job Length:   | Jobs range in length from 1 to 24 hours                                                                                                                                                                                                                                                                                                                                                                                                                                     |
+---------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Job Owners:   | Six major groups consisting of about 50 users in total                                                                                                                                                                                                                                                                                                                                                                                                                      |
+---------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Notes:        | During prime time hours, the majority of jobs submitted are smaller, short running development jobs where users are testing out new code and new data sets. The owners of these jobs are often unable to proceed with their work until a job they have submitted completes. Many of these jobs are interactive in nature. Throughout the day, large, longer running production workload is also submitted but these jobs do not have comparable turnaround time pressure.   |
+---------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

**Goals**

Nodes in Group A must run only parallel jobs. Nodes in Group B must only
run serial jobs, with up to 4 serial jobs per node. Nodes in Group C
must not be used unless a job cannot locate resources elsewhere. The
scheduler should attempt to intelligently load balance the timesharing
nodes.

**Analysis**

The network topology is flat and and nodes are homogeneous within each
group. The easiest way to configure the overflow group is to create two
PBS queues, *serial* and *parallel*, with appropriate min and max node
counts as desired.

By default, Moab interprets the PBS *exclusive hostlist* queue attribute
as constraining jobs in the queue to run only on the nodes contained in
the hostlist. We can take advantage of this behavior to assign nodes in
Group A and Group C to the queue *parallel* while the nodes in Group B
and Group C are assigned to the queue *serial*. The same can be done
with classes if using Loadleveler. Moab will incorporate this queue
information when making scheduling decisions.

The next step is to make the scheduler use the overflow nodes of group C
only as a last resort. This can be accomplished using a negative
affinity `standing reservation <../7.1.3standingreservations.html>`__.
This configuration will tell the scheduler that these nodes can be used,
but should only be used if it cannot find compute resources elsewhere.

The final step, load balancing, is accomplished in two parts. First, the
nodes in group B must be configured to allow up to 4 serial jobs to run
at a time. This is best accomplished using the PBS *virtual nodes*
capability. To load balance, simply select the
`CPULOAD <../5.2nodeallocation.html>`__ allocation algorithm in Moab.
This algorithm will instruct Moab to schedule the job based on which
node has the most idle CPU time.

**Configuration**

This site requires both resource manager and scheduler configuration.
The following Moab configuration would be needed:

::

    # reserve overflow processors
    SRNAME[0]         overflow
    SRHOSTLIST[0]     cn0[1-8]          # hostname regular expression
    SRCLASSLIST[0]    parallel- batch-  # use minus sign to indicate negative affinity

    # allow SMP node sharing
    ALLOCATIONPOLICY  CPULOAD
    NODEACCESSPOLICY  SHARED

::

    set queue serial resources_max.nodeccount=1
    set queue serial acl_hosts=an01+an02+...an16+cn01+cn02+...cn08
    set queue serial acl_host_enable=true

    set queue parallel resources_min.nodecount=2
    set queue parallel acl_hosts=bn01+bn02+...bn08+cn01+cn02+...cn08
    set queue parallel acl_host_enable=true

::

    bn01 np=4
    bn01 np=4

**Monitoring**

The commands `mdiag -s <../commands/mdiag.html>`__ and `mdiag
-r <../commands/mdiag-reservations.html>`__ can be used to examine the
standing reservations and all reservations in more detail. The `mdiag
-c <../commands/mdiag-class.html>`__ command can be used to examine and
verify class/queue information. The `mshow <../commands/mshow.html>`__
command may also be used to display information about a class/queue,
job, and reservation.

**Conclusions**

Moab's policy engine supports an incredible range of capabilities.
Configuring different groups of nodes to provide different types of
service is a straightforward process.
