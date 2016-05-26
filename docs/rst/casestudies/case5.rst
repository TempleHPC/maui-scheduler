Case Study: Multi-Queue Cluster with QOS and Charge Rates
#########################################################

**Overview**

A 160 node, single processor Linux® cluster is to be used to support
various organizations within an enterprise.  The system should allow a
user to request improved job turnaround time in exchange for a higher
charge rate.  A portion of the system must be reserved for small
development jobs at all times. 

**Resources**

+-----------------------+------------------------------------------------------------+
| Compute Nodes:        | 128 800MHz single processor Linux® nodes with 512 MB RAM   |
|                       | 32 1.2GHz single processor Linux® nodes with 512 MB RAM    |
+-----------------------+------------------------------------------------------------+
| Resource Manager:     | OpenPBS 2.3                                                |
+-----------------------+------------------------------------------------------------+
| Network:              | 100 MB switched Ethernet                                   |
+-----------------------+------------------------------------------------------------+

**Workload**

+-----------------+----------------------------------------------------+
| Job Size:       | Range in size from 1 to 80 processors              |
+-----------------+----------------------------------------------------+
| Job Length:     | Jobs range in length from 15 minutes to 24 hours   |
+-----------------+----------------------------------------------------+
| Job Owners:     | Various                                            |
+-----------------+----------------------------------------------------+

**Goals**

Management desires the following queue structure:

::

    QueueName      Nodes        MaxWallTime   Priority     ChargeRate
    -----------------------------------------------------------------
    Test            <=16            0:30:00        100             1x
    Serial             1            2:00:00         10             1x
    Serial-Long        1           24:00:00         10             2x
    Short           2-16            4:00:00         10             1x
    Short-Long      2-16           24:00:00         10             2x
    Med            17-64            8:00:00         20             1x
    Med-Long       17-64           24:00:00         20             2x
    Large          65-80           24:00:00         50             2x
    LargeMem           1            8:00:00         10             4x

For charging, management has decided to charge by job walltime since the
nodes will not be shared.  Management has also dictated that 16 of the
single processor nodes should be dedicated to running small jobs
requiring 16 or fewer nodes.  Management has also decided that it would
like to allow only serial jobs to run on the large memory nodes and
would like to charge these jobs at a rate of 4x.  There are no
constraints on the remaining nodes. 

This site has goals which are focused more on a supplying a
straightforward queue environment to the end users than on maximizing
the scheduling performance of the system.  The Moab configuration has
the primary purpose of faithfully reproducing the queue constraints
above while maintaining reasonable scheduling performance in the
process. 

**Analysis**

Since we are using PBS as the resource manager, this should be a pretty
straightforward process.  It will involve setting up an allocations
manager to handle charging, configuring queue priorities, and creating a
system reservation to manage the 16 processors dedicated to small jobs,
and another for managing the large memory nodes. 

**Configuration**

First, the queue structure must be configured.  The best place to handle
this is via the PBS configuration.  The ``qmgr`` command can be used to
set up the nine queues described above.  PBS supports the node and
walltime constraints as well as the queue priorities.  Moab will honor
queue priorities configured within PBS.  Alternatively, these priorities
can be specified directly within the Moab ``fs.cfg`` file for resource
managers which do not support this capability. 

We will be using QBank to handle all allocations and so, will want to
configure the the "per class charge rates" there.  Note that QBank 2.9
or higher is required for per class charge rate support. 

Now, two reservations are needed.  The first reservation will be for the
16 small memory nodes.  It should only allow node access to jobs
requesting up to 16 processors.  In this environment, this is probably
most easily accomplished with a reservation class ACL containing the
queues which allow 1 - 16 node jobs. 

**Monitoring**

The commands `mdiag -s <../commands/mdiag.html>`__ and `mdiag
-r <../commands/mdiag-reservations.html>`__ can be used to examine the
standing reservations and all reservations in more detail.  The `mdiag
-c <../commands/mdiag-class.html>`__ command can be used to examine and
verify class/queue information.  The `mshow <../commands/mshow.html>`__
command may also be used to display information about a class/queue,
job, and reservation. 

**Conclusions:**

Moab supports intelligent scheduling across a wide variety of existing
queue types.  This intelligence allows administrators to charge for
access to priority resources in a very flexible manner.  Moab will work
seamlessly with other software products to implement the desired
behavior.
