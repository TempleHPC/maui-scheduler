
.. rubric:: mdiag -n
   :name: mdiag--n

**(Moab Node Diagnostics)**
**\ Synopsis**

::

    mdiag -n [-t partitionid] [-A creds] [-v] [--format=xml] [nodeid]

**Overview**

The **mdiag -n** command provides detailed information about the state
of nodes Moab is currently tracking. This command also performs a large
number of sanity and state checks. The node configuration and status
information as well as the results of the various checks are presented
by this command.
**Arguments**

Flag
Argument
Description
[-A]
{user\|group\|account\|qos\|class\|job}:<OBJECTID>
report if each node is accessible by requested job or credential
[-t]
<partitionid>
report only nodes from specified partition
[-v]
-
show verbose output (do not truncate columns and add columns for
additional node attributes)
**Output**

This command presents detailed node information in whitespace delineated
fields.
The output of this command can be extensive and the values for a number
of fields may be truncated. If truncated, the '**-v** flag can be used
to display full field content.

Column
Format
Name
<NODE NAME
State
<NODE STATE
Procs
<AVAILABLE PROCS>:<CONFIGURED PROCS>
Memory
<AVAILABLE MEMORY>:<CONFIGURED MEMORY>
Disk
<AVAILABLE DISK>:<CONFIGURED DISK>
Swap
<AVAILABLE SWAP>:<CONFIGURED SWAP>
Speed
<RELATIVE MACHINE SPEED>
Opsys
<NODE OPERATING SYSTEM>
Arch
<NODE HARDWARE ARCHITECTURE>
Par
<PARTITION NODE IS ASSIGNED TO>
Load
<CURRENT 1 MINUTE BSD LOAD>
Rsv
<NUMBER OF RESERVATIONS ON NODE>
Classes
<CLASS NAME><CLASS INSTANCES AVAILABLE>:<CLASS INSTANCES CONFIGURED>...
Network
<NETWORK NAME>...
Features
<NODE FEATURE>...
**Example 1**


::

    > mdiag -n

    compute node summary
    Name                    State   Procs      Memory         Opsys
     
    opt-001                  Busy    0:2      2048:2048        SuSE
    opt-002                  Busy    0:2      2048:2048        SuSE
    opt-003                  Busy    0:2      2048:2048        SuSE
    opt-004                  Busy    0:2      2048:2048        SuSE
    opt-005                  Busy    0:2      2048:2048        SuSE
    opt-006                  Busy    0:2      2048:2048        SuSE
    WARNING:   swap is low on node opt-006
    opt-007                  Busy    0:2      2048:2048        SuSE
    opt-008                  Busy    0:2      2048:2048        SuSE
    opt-009                  Busy    0:2      2048:2048        SuSE
    opt-010                  Busy    0:2      2048:2048        SuSE
    opt-011                  Busy    0:2      2048:2048        SuSE
    opt-012                  Busy    0:2      2048:2048        SuSE
    opt-013                  Busy    0:2      2048:2048        SuSE
    opt-014                  Busy    0:2      2048:2048        SuSE
    opt-015                  Busy    0:2      2048:2048        SuSE
    opt-016                  Busy    0:2      2048:2048        SuSE
    x86-001                  Busy    0:1       512:512       Redhat
    x86-002                  Busy    0:1       512:512       Redhat
    x86-003                  Busy    0:1       512:512       Redhat
    x86-004                  Busy    0:1       512:512       Redhat
    x86-005                  Idle    1:1       512:512       Redhat
    x86-006                  Idle    1:1       512:512       Redhat
    x86-007                  Idle    1:1       512:512       Redhat
    x86-008                  Busy    0:1       512:512       Redhat
    x86-009                  Down    1:1       512:512       Redhat
    x86-010                  Busy    0:1       512:512       Redhat
    x86-011                  Busy    0:1       512:512       Redhat
    x86-012                  Busy    0:1       512:512       Redhat
    x86-013                  Busy    0:1       512:512       Redhat
    x86-014                  Busy    0:1       512:512       Redhat
    x86-015                  Busy    0:1       512:512       Redhat
    x86-016                  Busy    0:1       512:512       Redhat
    P690-001                 Busy    0:1     16384:16384        AIX
    P690-002                 Busy    0:1     16384:16384        AIX
    P690-003                 Busy    0:1     16384:16384        AIX
    P690-004                 Busy    0:1     16384:16384        AIX
    P690-005                 Busy    0:1     16384:16384        AIX
    P690-006                 Busy    0:1     16384:16384        AIX
    P690-007                 Idle    1:1     16384:16384        AIX
    P690-008                 Idle    1:1     16384:16384        AIX
    WARNING:   node P690-008 is missing ethernet adapter
    P690-009                 Busy    0:1     16384:16384        AIX
    P690-010                 Busy    0:1     16384:16384        AIX
    P690-011                 Busy    0:1     16384:16384        AIX
    P690-012                 Busy    0:1     16384:16384        AIX
    P690-013                 Busy    0:1     16384:16384        AIX
    P690-014                 Busy    0:1     16384:16384        AIX
    P690-015                 Busy    0:1     16384:16384        AIX
    P690-016                 Busy    0:1     16384:16384        AIX
    -----                     ---    6:64   745472:745472     -----
     
    Total Nodes: 36  (Active: 30  Idle: 5  Down: 1)


**Note:** Warning messages are interspersed with the node configuration
information with all warnings preceded by the keyword '**WARNING**'.

**\ XML Output**

If XML output is requested (via the `--format=xml <#synopsis>`__
argument), XML based node information will be written to STDOUT in the
following format:


mdiag -n --format=xml
::

    <Data>
      <node> <ATTR>="<VAL>" ... </node>
      ...
    </Data>


In addition to the attributes listed below, mdiag -n's node element XML
has children that describe a node's messages
(`Messages <../xml/Messages.html>`__ XML element).

**XML Attributes**

Name
Description
AGRES
Available generic resources
ALLOCRES
Special allocated resources (like vlans)
ARCH
The node's processor architecture.
AVLETIME
Time when the node will no longer be availble (used in Utility centers)
AVLSTIME
Time when the node will be available (used in Utility centers)
CFGCLASS
Classes configured on the node
GRES
generic resources on the node
ENABLEPROFILING
If true, a node's state and usage is tracked over time.
FEATURES
A list of comma separated custom features describing a node.
GMETRIC
A list of comma separated consumable resources associated with a node.
HOPCOUNT
How many hops the node took to reach this Moab (used in hierarchical
grids)
ISDELETED
Node has been deleted
ISDYNAMIC
Node is dynamic (used in Utility centers)
JOBLIST
The list of jobs currently running on a node.
LOAD
current load as reported by the resource manager
LOADWEIGHT
load weight used when calculating node priority
MAXJOB
See `Node Policies <12.3nodepolicies.html>`__ for details.
MAXJOBPERUSER
See `Node Policies <12.3nodepolicies.html>`__ for details.
MAXLOAD
See `Node Policies <12.3nodepolicies.html>`__ for details.
MAXPROC
See `Node Policies <12.3nodepolicies.html>`__ for details.
MAXPROCPERUSER
See `Node Policies <12.3nodepolicies.html>`__ for details.
NETWORK
The ability to specify which networks are available to a given node is
limited to only a few resource manager. Using the **NETWORK** attribute,
administrators can establish this node to network connection directly
through the scheduler. The `NODECFG <a.fparameters.html#nodecfg>`__
parameter allows this list to be specified in a comma delimited list.
NODEID
The unique identifier for a node.
NODESTATE
The state of a node.
OS
A node's operating system.
OSLIST
Operating systems the node can run
OSMODACTION
URL for changing the operating system
OWNER
Credential type and name of owner
PARTITION
The partition a node belongs to. See `Node
Location <12.1nodelocation.html>`__ for details.
POWER
The state of the node's power. Either ON or OFF.
PRIORITY
The fixed node priority relative to other nodes.
PROCSPEED
A node's processsor speed information specified in MHz.
RACK
The rack associated with a node's physical location.
RADISK
The total available disk on a node.
RAMEM
The total available memory available on a node.<
RAPROC
The total number of processors available on a node.
RASWAP
The total available swap on a node.
RCMEM
The total configured memory on a node.
RCPROC
The total configured processors on a node.
RCSWAP
The total configured swap on a node.
RESCOUNT
Number of reservations on the node
RSVLIST
List of reservations on the node
RESOURCES
Deprecated (use GRES)
RMACCESSLIST
A comma separated list of resource managers who have access to a node.
SIZE
The number of slots or size units consumed by the node.
SLOT
The first slot in the rack associated with the node's physical location.
SPEED
A node's relative speed.
SPEEDWEIGHT
speed weight used to calculate node's priority
STATACTIVETIME
Time node was active
STATMODIFYTIME
Time node's state was modified
STATTOTALTIME
Time node has been monitored
STATUPTIME
Time node has been up
TASKCOUNT
The number of tasks on a node.
.. rubric:: See Also
   :name: see-also

-  `checknode <checknode.html>`__

