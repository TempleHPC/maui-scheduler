
.. rubric:: mdiag -c
   :name: mdiag--c

**(Moab Class Diagnostics)**
**Synopsis**

::

    mdiag -c [-v] [classid] [--format=xml]

**Overview**

The **mdiag -c** command provides detailed information about the classes
Moab is currently tracking. This command also allows an administrator to
verify correct throttling policies and access provided to and from other
credentials.

+----------+-------------------------------------------------------------------------------------------------------------------------------------------------------------+
| |Note|   | The term `class <../3.2environment.html#classdefinition>`__ is used interchangeably with the term *queue* and generally refers to resource manager queue.   |
+----------+-------------------------------------------------------------------------------------------------------------------------------------------------------------+

**`XML Output <>`__**

**mdiag -c** information can be reported as XML as well. This is done
with the command "mdiag -c <CLASS\_ID> --format=xml". XML based class
information will be written to STDOUT in the following format:


::

    <Data>
      <class <ATTR>="<VAL>" ... >
        <stats <ATTR>="<VAL>" ... >
          <Profile <ATTR>="<VAL>" ... >
          </Profile>
        </stats>
      </class>
    <Data>

      ...
    </Data>


In addition to the attributes listed below, mdiag -c's XML children
describe its general statistics information (stats XML Element) and
profile based statistics (`Profile <../xml/Profile.html>`__ XML
element).

**XML Attributes**

+--------------------+--------------------------------------------------------------------------------------------------+
| Name               | Description                                                                                      |
+====================+==================================================================================================+
| ADEF               | Accounts a class has access to.                                                                  |
+--------------------+--------------------------------------------------------------------------------------------------+
| CAPACITY           | Number of procs available to the class.                                                          |
+--------------------+--------------------------------------------------------------------------------------------------+
| DEFAULT.ATTR       | Default attributes attached to a job.                                                            |
+--------------------+--------------------------------------------------------------------------------------------------+
| DEFAULT.DISK       | Default required disk attached to a job.                                                         |
+--------------------+--------------------------------------------------------------------------------------------------+
| DEFAULT.FEATURES   | Default required node features attached to a job.                                                |
+--------------------+--------------------------------------------------------------------------------------------------+
| DEFAULT.GRES       | Default generic resources attached to a job.                                                     |
+--------------------+--------------------------------------------------------------------------------------------------+
| DEFAULT.MEM        | Default required memeory attached to a job.                                                      |
+--------------------+--------------------------------------------------------------------------------------------------+
| DEFAULT.NODESET    | Default specified nodeset attached to a job.                                                     |
+--------------------+--------------------------------------------------------------------------------------------------+
| DEFAULT.WCLIMIT    | Default wallclock limit attached to a job.                                                       |
+--------------------+--------------------------------------------------------------------------------------------------+
| EXCL.FEATURES      | List of excluded (disallowed) node features.                                                     |
+--------------------+--------------------------------------------------------------------------------------------------+
| EXCL.FLAGS         | List of excluded (disallowed) job flags.                                                         |
+--------------------+--------------------------------------------------------------------------------------------------+
| FSTARGET           | The class' fairshare target.                                                                     |
+--------------------+--------------------------------------------------------------------------------------------------+
| HOLD               | If TRUE this credential has a hold on it, FALSE otherwise.                                       |
+--------------------+--------------------------------------------------------------------------------------------------+
| HOSTLIST           | The list of hosts in this class.                                                                 |
+--------------------+--------------------------------------------------------------------------------------------------+
| JOBEPILOG          | Scheduler level job epilog to be run after job is completed by resource manager (script path).   |
+--------------------+--------------------------------------------------------------------------------------------------+
| JOBFLAGS           | Default flags attached to jobs in the class.                                                     |
+--------------------+--------------------------------------------------------------------------------------------------+
| JOBPROLOG          | Scheduler level job prolog to be run before job is started by resource manager (script path).    |
+--------------------+--------------------------------------------------------------------------------------------------+
| ID                 | The unique ID of this class.                                                                     |
+--------------------+--------------------------------------------------------------------------------------------------+
| LOGLEVEL           | The log level attached to jobs in the class.                                                     |
+--------------------+--------------------------------------------------------------------------------------------------+
| MAX.PROC           | The max processors per job in the class.                                                         |
+--------------------+--------------------------------------------------------------------------------------------------+
| MAX.PS             | The max processor-seconds per job in the class.                                                  |
+--------------------+--------------------------------------------------------------------------------------------------+
| MAX.WCLIMIT        | The max wallclock limit per job in the class.                                                    |
+--------------------+--------------------------------------------------------------------------------------------------+
| MAXIJOB            | The max idle jobs in the class.                                                                  |
+--------------------+--------------------------------------------------------------------------------------------------+
| MAXIPROC           | The max idle processors in the class.                                                            |
+--------------------+--------------------------------------------------------------------------------------------------+
| MAXJOBPERUSER      | The max jobs per user.                                                                           |
+--------------------+--------------------------------------------------------------------------------------------------+
| MAXNODEPERJOB      | The max nodes per job.                                                                           |
+--------------------+--------------------------------------------------------------------------------------------------+
| MAXNODEPERUSER     | The max nodes per user.                                                                          |
+--------------------+--------------------------------------------------------------------------------------------------+
| MAXPROCPERJOB      | The max processors per job.                                                                      |
+--------------------+--------------------------------------------------------------------------------------------------+
| MAXPROCPERNODE     | The max processors per node.                                                                     |
+--------------------+--------------------------------------------------------------------------------------------------+
| MAXPROCPERUSER     | The max processors per user.                                                                     |
+--------------------+--------------------------------------------------------------------------------------------------+
| MIN.NODE           | The minimum nodes per job in the class.                                                          |
+--------------------+--------------------------------------------------------------------------------------------------+
| MIN.PROC           | The minimum processors per job in the class.                                                     |
+--------------------+--------------------------------------------------------------------------------------------------+
| MIN.WCLIMIT        | The minimum wallclock limit per job in the class.                                                |
+--------------------+--------------------------------------------------------------------------------------------------+
| NODEACCESSPOLICY   | The node access policy associated with jobs in the class.                                        |
+--------------------+--------------------------------------------------------------------------------------------------+
| OCDPROCFACTOR      | Dedicated processor factor.                                                                      |
+--------------------+--------------------------------------------------------------------------------------------------+
| OCNODE             | Overcommit node.                                                                                 |
+--------------------+--------------------------------------------------------------------------------------------------+
| PRIORITY           | The class' associated priority.                                                                  |
+--------------------+--------------------------------------------------------------------------------------------------+
| PRIORITYF          | Priority calculation function.                                                                   |
+--------------------+--------------------------------------------------------------------------------------------------+
| REQ.FEATURES       | Required features for a job to be considered in the class.                                       |
+--------------------+--------------------------------------------------------------------------------------------------+
| REQ.FLAGS          | Required flags for a job to be considered in the class.                                          |
+--------------------+--------------------------------------------------------------------------------------------------+
| REQ.IMAGE          | Required image for a job to be considered in the class.                                          |
+--------------------+--------------------------------------------------------------------------------------------------+
| REQUIREDUSERLIST   | The list of users who have access to the class.                                                  |
+--------------------+--------------------------------------------------------------------------------------------------+
| RM                 | The resouce manager reporting the class.                                                         |
+--------------------+--------------------------------------------------------------------------------------------------+
| RMLIST             | The list of resource managers who have access to the class.                                      |
+--------------------+--------------------------------------------------------------------------------------------------+
| STATE              | The class' state.                                                                                |
+--------------------+--------------------------------------------------------------------------------------------------+
| WCOVERRUN          | Tolerated amount of time beyond the specified wall clock limit.                                  |
+--------------------+--------------------------------------------------------------------------------------------------+

**Example 1**


::

    > mdiag -c

    Class/Queue Status

    ClassID        Priority Flags        QDef              QOSList* PartitionList        Target Limits

    DEFAULT               0 ---          ---                   ---  ---                   0.00  ---
    batch                 1 ---          ---                   ---  [A][B]               70.00  MAXJOB=33:200,250
      MAX.WCLIMIT=10:00:00  MAXPROCPERJOB=128
    long                  1 ---          low                   low  [A]                  10.00  MAXJOB=3:100,200
      MAX.WCLIMIT=1:00:00:00  MAXPROCPERJOB=128
    fast                100 ---          high                 high  [B]                  10.00  MAXJOB=8:100,150
      MAX.WCLIMIT=00:30:00  MAXPROCPERJOB=128
    bigmem                1 ---          low,high              low  ---                  10.00  MAXJOB=1:100,200
      MAXPROCPERJOB=128


+--------------------------------------+--------------------------------------+
| |Note|                               | The **Limits** column will display   |
|                                      | limits in the following format:      |
|                                      | <USAGE>:<HARDLIMIT>[,<SOFTLIMIT>]    |
+--------------------------------------+--------------------------------------+

In the example above, class ``fast`` has **MAXJOB** soft and hard limits
of 100 and 150 respectively and is currently running 8 jobs.

.. rubric:: See Also
   :name: see-also

-  `showstats <showstats.html>`__ command - display general statistics

