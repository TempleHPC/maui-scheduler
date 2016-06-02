
.. rubric:: showbf
   :name: showbf

**(Show Available Resources)**
**Synopsis**

::

    showbf [-A] [-a account] [-c class] [-d duration] [-D] [-f features] [-g group] [-L] 
           [-m [==|>|>=|<|<=] memory] [-n nodecount] [-p partition] [-q qos] [-u user] [-v]

**Overview**

Shows what resources are available for immediate use.
This command can be used by any user to find out how many processors are
available for immediate use on the system. It is anticipated that users
will use this information to submit jobs that meet these criteria and
thus obtain quick job turnaround times. This command incorporates down
time, reservations, and node state information in determining the
available backfill window.

+----------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| |Note|   | If specific information is not specified, showbf will return information for the user and group running but with global access for other credentials. For example, if '-q qos' is not specified, Moab will return resource availability information for a job as if it were entitled to access *all* QOS based resources (i.e., resources covered by reservations with a QOS based ACL), if '-c class' is not specified, the command will return information for resources accessible by any class.   |
+----------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

+----------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| |Note|   | The **showbf** command incorporates node configuration, node utilization, node state, and node reservation information into the results it reports. This command does **not** incorporate constraints imposed by credential based fairness policies on the results it reports.   |
+----------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

**Access**

By default, this command can be used by any user or administrator.
**Parameters**

+---------------+--------------------------------------------------------------------------------------------------------+
| Parameter     | Description                                                                                            |
+===============+========================================================================================================+
| ACCOUNT       | Account name.                                                                                          |
+---------------+--------------------------------------------------------------------------------------------------------+
| CLASS         | Class/queue required.                                                                                  |
+---------------+--------------------------------------------------------------------------------------------------------+
| DURATION      | Time duration specified as the number of seconds or in [DD:]HH:MM:SS notation.                         |
+---------------+--------------------------------------------------------------------------------------------------------+
| FEATURELIST   | Colon separated list of node features required.                                                        |
+---------------+--------------------------------------------------------------------------------------------------------+
| GROUP         | Specify particular group.                                                                              |
+---------------+--------------------------------------------------------------------------------------------------------+
| MEMCMP        | Memory comparison used with the -m flag. Valid signs are >, >=, ==, <=, and <.                         |
+---------------+--------------------------------------------------------------------------------------------------------+
| MEMORY        | Specifies the amount of required real memory configured on the node, (in MB), used with the -m flag.   |
+---------------+--------------------------------------------------------------------------------------------------------+
| NODECOUNT     | Specify number of nodes for inquiry with -n flag.                                                      |
+---------------+--------------------------------------------------------------------------------------------------------+
| PARTITION     | Specify partition to check with -p flag.                                                               |
+---------------+--------------------------------------------------------------------------------------------------------+
| QOS           | Specify QOS to check with -q flag.                                                                     |
+---------------+--------------------------------------------------------------------------------------------------------+
| USER          | Specify particular user to check with -u flag.                                                         |
+---------------+--------------------------------------------------------------------------------------------------------+

**Flags**

+--------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Flag   | Description                                                                                                                                                                                                                                                                                                                                    |
+========+================================================================================================================================================================================================================================================================================================================================================+
| -A     | Show resource availability information for all users, groups, and accounts. By default, **showbf** uses the default user, group, and account ID of the user issuing the command.                                                                                                                                                               |
+--------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| -a     | Show resource availability information only for specified account.                                                                                                                                                                                                                                                                             |
+--------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| -d     | Show resource availability information for specified duration.                                                                                                                                                                                                                                                                                 |
+--------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| -D     | Display current and future resource availability notation.                                                                                                                                                                                                                                                                                     |
+--------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| -g     | Show resource availability information only for specified group.                                                                                                                                                                                                                                                                               |
+--------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| -h     | Help for this command.                                                                                                                                                                                                                                                                                                                         |
+--------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| -L     | Enforce Hard limits when showing available resources.                                                                                                                                                                                                                                                                                          |
+--------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| -m     | Allows user to specify the memory requirements for the backfill nodes of interest. It is important to note that if the optional *MEMCMP* and *MEMORY* parameters are used, they **MUST** be enclosed in single ticks (') to avoid interpretation by the shell. For example, enter ``showbf -m '==256'`` to request nodes with 256 MB memory.   |
+--------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| -n     | Show resource availability information for a specified number of nodes. That is, this flag can be used to force ``showbf`` to display only blocks of resources with at least this many nodes available.                                                                                                                                        |
+--------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| -p     | Show resource availability information for the specified partition.                                                                                                                                                                                                                                                                            |
+--------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| -q     | Show information for the specified QOS.                                                                                                                                                                                                                                                                                                        |
+--------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| -u     | Show resource availability information only for specified user.                                                                                                                                                                                                                                                                                |
+--------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

**Example 1**

In this example, a job requiring up to 2 processors could be submitted
for immediate execution in partition **ClusterA** for any duration.
Additionally, a job requiring 1 processor could be submitted for
immediate execution in partition **ClusterB**. Note that by default,
each task is tracked and reported as a request for a single processor.


::

    > showbf

    Partition     Tasks  Nodes   StartOffset      Duration       StartDate
    ---------     -----  -----  ------------  ------------  --------------
    ALL               3      3      00:00:00      INFINITY  11:32:38_08/19
    ReqID=0
    ClusterA          1      1      00:00:00      INFINITY  11:32:38_08/19
    ReqID=0
    ClusterB          2      2      00:00:00      INFINITY  11:32:38_08/19
    ReqID=0


**Example 2**

In this example, the output verifies that a backfill window exists for
jobs requiring a 3 hour runtime and at least 16 processors. Specifying
job duration is of value when time based access is assigned to
reservations (i.e., using the **SRCFG** **TIMELIMIT** ACL)


::

    > showbf -r 16 -d 3:00:00

    backFill window (user: 'john' group: 'staff' partition: ALL) Mon Feb 16 08:28:54

    partition ALL:
      33 procs available with no time limit


**Example 3**

In this example, a resource availability window is requested for
processors located only on nodes with at least 512 MB of memory. In the
example above, the command output reports that no processors are
available for immediate use which meet this constraint.


::

    > showbf -m ' =512'

    backfill window (user: 'john' group: 'staff' partition: ALL) Thu Jun 18 16:03:04

    no procs available


.. rubric:: See Also
   :name: see-also

-  `Moab Client Installation <../2.2installation.html#client>`__ -
   explains how to distribute this command to client nodes
-  `showq <showq.html>`__
-  `mdiag -t <mdiag.html>`__

