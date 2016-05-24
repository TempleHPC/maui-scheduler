
.. rubric:: mdiag
   :name: mdiag

**(Moab Diagnostics)**
**Synopsis**

::

    mdiag -a [accountid]
    mdiag -b [-l policylevel] [-t partition]
    mdiag -c [classid]
    mdiag -C [configfile]  // diagnose config file syntax
    mdiag -f [-o user|group|acct|qos|class] [-v]
    mdiag -g [groupid]
    mdiag -G [Green]
    mdiag -j [jobid] [-t <partition>] [-v]
    mdiag -L [-v]  // diagnose usage limits
    mdiag -m [rackid]
    mdiag -n [-A <creds>] [-t partition] [nodeid] [-v]
    mdiag -p [-t partition] [-v] // diagnose job priority
    mdiag -q [qosid]
    mdiag -r [reservationid] [-v] [-w type=<type>]
    mdiag -R [resourcemanagername] [-v]
    mdiag -s [standingreservationid]
    mdiag -S [-v] // diagnose scheduler
    mdiag -t [-v] // diagnose partitions
    mdiag -T [triggerid] [-v]
    mdiag -u [userid]
    mdiag -x
    mdiag [--format=xml]

**Overview**

The **mdiag** command is used to display information about various
aspects of the cluster and the results of internal diagnostic tests.  In
summary, it provides the following:

-  current object health and state information
-  current object configuration (resources, policies, attributes, etc)
-  current and historical performance/utilization information
-  reports on recent failure
-  object messages

**Arguments**

Argument
Description
`-a [accountid] <mdiag-accounts.html>`__
display `account <../3.5credoverview.html#account>`__ information
`-b <mdiag-blockedjobs.html>`__
display information on jobs blocked by policies, holds, or other
factors.

+----------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| |Note|   | If blocked job diagnostics are specified, the '**-t**' option is also available to constrain the report to analysis of particular partition.  Also, with blocked job diagnosis, the '**-l**' option can be used to specify the analysis policy level.   |
+----------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

`-c [classid] <mdiag-class.html>`__
display `class <../3.5credoverview.html#class>`__ information
`-C <>`__ [file]
analyze configuration file for errors including use of invalid
parameters, deprecated parameters, and illegal values. (If you start
Moab with the -e flag, Moab evaluates the configuration file at startup
and quits if an error exists.)
`-f <mdiag-fairshare.html>`__
display `fairshare <../6.3fairshare.html>`__ information
`-g [groupid] <mdiag-groups.html>`__
display `group <../3.5credoverview.html#group>`__ information
-G [Green]
display `power management <../18.2enablinggreen.html>`__ information
`-j [jobid] <mdiag-jobs.html>`__
display job information
-L
display limits
-m rackid
display rack/frame information
`-n [nodeid] <mdiag-node.html>`__
display nodes.

+----------+-------------------------------------------------------------------------------------------------------------------------------+
| |Note|   | If node diagnostics are specified, the '**-t**' option is also available to constrain the report to a particular partition.   |
+----------+-------------------------------------------------------------------------------------------------------------------------------+

-o <OTYPE>[:<OID>]
organize information by specified object type
`-p <mdiag-priority.html>`__
display `job priority <../5.1jobprioritization.html>`__.

+----------+-----------------------------------------------------------------------------------------------------------------------------------+
| |Note|   | If priority diagnostics are specified, the '**-t**' option is also available to constrain the report to a particular partition.   |
+----------+-----------------------------------------------------------------------------------------------------------------------------------+

`-q [qosid] <mdiag-qos.html>`__
display `qos <../3.5credoverview.html#qos>`__ information
`-r [reservationid] <mdiag-reservations.html>`__
display reservation information
`-R [rmid] <mdiag-rm.html>`__
display resource manager information
-s [srsv]
display `standing reservation <../7.1.3standingreservations.html>`__
information
`-S <mdiag-sched.html>`__
display general scheduler information
`-T [triggerid] <mdiag-triggers.html>`__
display trigger information
`-u [userid] <mdiag-users.html>`__
display `user <../3.5credoverview.html#user>`__ information
-x
display advanced system information
--format=xml
display output in XML format
.. rubric:: See Also
   :name: see-also

-  `Moab Client Installation <../2.2installation.html#client>`__ -
   explains how to distribute this command to client nodes
-  `checkjob <checkjob.html>`__
-  `checknode <checknode.html>`__

