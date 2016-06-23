**Fairshare Config File**
The fairshare config file, **fs.cfg**, is used to manage fairshare and
QOS parameters. While still supported, its use is largely superseded by
the **\*CFG** suite of parameters in later versions of Maui (Maui 3.0.7
and higher). The file uses the following format:

<OBJECT> <ATTR>=<VALUE> [<ATTR>=<VALUE>]...

The following object types may be specified:

| USER:<USER>
| GROUP:<GROUP>
| ACCOUNT:<ACCOUNT>
| QOS:<QOSNAME>
| SYSTEM

**NOTE:** The keyword 'DEFAULT' may be used in place of an actual user,
group, account, or QOS name to specify the default fairshare
configuration for objects not explicitly specified, i.e. USER:DEFAULT
FSTARGET=5.0

The following attributes may be specified:

| QDEF (default QOS)
| QLIST (QOS access list)
| PDEF (default partition)
| PLIST (partition access list)
| JOBFLAGS (special job attributes)
| FSTARGET (target percent system utilization)

QDEF

| DESCRIPTION: specifies default QOS value for jobs
| FORMAT: <INTEGER>
| DEFAULT: [NONE]
| EXAMPLE: QDEF=3
| DETAILS: Default QOS values are assigned to a job in the following
  precedence order: User -> Group -> Account -> System
| (i.e., User QOS defaults overrule all others). If no default values
  are specified, the job will be assigned QOS 0.

QLIST

| DESCRIPTION: specifies list of QOS values which jobs have access to
| FORMAT: <QOSINDEX>[,<QOSINDEX>]...
| DEFAULT: [NONE]
| EXAMPLE: QLIST=2,4-8
| DETAILS: If the QLIST value is followed by an ampersand, '&', QLIST
  values are considered an 'AND' list rather than an 'OR' list.

PDEF

| DESCRIPTION: specifies default partition in which jobs will run
| FORMAT: <PARTITIONNAME>
| DEFAULT: [ANY]
| EXAMPLE: PDEF=OldSP
| DETAILS: Default partition values are assigned to a job in the
  following precedence order: User -> Group -> Account -> System. If no
  default partition is specified, the job will be assigned to any
  partition.

PLIST

| DESCRIPTION: specifies list of partitions which jobs have access to
| FORMAT: <PARTITION>[:<PARTITION>]...
| DEFAULT: [ALL]
| EXAMPLE: PLIST=OldSP:NewSP:O2K
| DETAILS: PLIST values are or'd together to determine the partitions a
  job may access.

JOBFLAGS

| DESCRIPTION: specifies default job flags
| FORMAT: <FLAG>[:<FLAG>]...
| where flag is one of the following:
| BENCHMARK // maintain maximum locality
| SPAN // allow job resources to cross partition boundaries
| ADVRES // allocate only reserved resources
| SHAREDNODE // share resources with other jobs
| NOQUEUE // cancel job if resources not immediately available
| DEFAULT: [NONE]
| EXAMPLE: JOBFLAGS=ADVRES:SPAN

FSTARGET

| DESCRIPTION: specifies target fairshare utilization in percent (see
  FairShare.doc)
| FORMAT: <DOUBLE>[+-^]
| DEFAULT: [NONE]
| EXAMPLE: FSTARGET=10.0 // FairShare target is 10.0%
| EXAMPLE: FSTARGET=25.5- // FairShare cap is 25.5%
| EXAMPLE: FSTARGET=5+ // FairShare floor is 5%

Sample 'fs.cfg' file

| ---------------
| SYSTEM PLIST=OldSP PDEF=OldSP QLIST=0 QDEF=0
| USER:DEFAULTFSTARGET=5.0-
| USER:steve QLIST=2,3,4 QDEF=2
| USER:bob QDEF=2 FSFLAGS=ADVRES
| USER:charles FSTARGET=15+ QLIST=4,5,6 QDEF=4
| GROUP:staff FSTARGET=10.0-
| ACCOUNT:system FSTARGET=35.5+ PLIST=DevSP:OldSP PDEF=DevSP
| QOS:3 FSTARGET=75.0
| ---------------

The above sample config file does the following:

- default jobs will be granted access to the partition OldSP and QOS '0'
- jobs submitted by default will have only limited access to compute
resources via a default 'per user' fairshare target cap of 5.0%
- user steve will have access to QOS's '2', '3', and '4' and his jobs
will use QOS '2' unless he explicitly requested a different QOS level.
- jobs submitted by user bob will default to using QOS '2' and all of
bob's jobs may only run on reserved nodes.
- user charles has a fairshare floor of 15% of delivered resources, and
has access to QOS's '4', '5', and '6' with his jobs defaulting to QOS
'4'.
- jobs submitted by members of the group staff are given a fairshare
target cap of 10.0%.
- jobs submitted under the system account ID are given extensive access
to resources via a fairshare target floor of 35.5%
- jobs running under QOS '3' will be given a fairshare target of 75.0%
