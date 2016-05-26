
.. rubric:: setres
   :name: setres

**(Set Reservation)**

+----------+---------------------------------------------------------------------------------+
| |Note|   | This command is deprecated. Use `mrsvctl -c <mrsvctl.html#CREATE>`__ instead.   |
+----------+---------------------------------------------------------------------------------+

.. rubric:: Synopsis
   :name: synopsis

::

    setres [arguments] resourceexpression

.. rubric:: Overview
   :name: overview

Reserve resources for use by jobs with particular credentials or
attributes.
**ARGUMENTS:**

| **                [ -a <ACCOUNT\_LIST ]**
| **                [ -b <SUBTYPE ]**
| **                [ -c <CHARGE\_SPEC> ]**
| **                [ -d <DURATION> ]**
| **                [ -e <ENDTIME> ]**
| **                [ -E ]   // EXCLUSIVE**
| **                [ -f <FEATURE\_LIST> ]**
| **                [ -g <GROUP\_LIST> ]**
| **                [ -n <NAME> ]**
| **                [ -o <OWNER> ]**
| **                [ -p <PARTITION> ]**
| **                [ -q <QUEUE\_LIST> ]   // (ie CLASS\_LIST)**
| **                [ -Q <QOSLIST> ]**
| **                [ -r <RESOURCE\_DESCRIPTION> ]**
| **                [ -R <RESERVATION\_PROFILE> ]**
| **                [ -s <STARTTIME> ]**
| **                [ -T <TRIGGER> ]**
| **                [ -u <USER\_LIST> ]**
| **                [ -x <FLAGS> ]**

.. rubric:: Access
   :name: access

   This command can be run by level 1 and level 2 Moab administrators.
.. rubric:: Parameters
   :name: parameters

**Name**
**Format**
**Default**
**Description**
**ACCOUNT\_LIST**
<STRING>[:<STRING>]...
---
list of accounts that will be allowed access to the reserved resources
**SUBTYPE**
<STRING>
---
specify the subtype for a reservation
**CHARGE\_SPEC**
<ACCOUNT>[,<GROUP>[,<USER>]]
---
specifies which credentials will be accountable for unused resources
dedicated to the reservation
**CLASS\_LIST**
<STRING>[:<STRING>]...
---
list of classes that will be allowed access to the reserved resource
**DURATION**
[[[DD:]HH:]MM:]SS
INFINITY
duration of the reservation (not needed if ENDTIME is specified)
**ENDTIME**
[HH[:MM[:SS]]][\_MO[/DD[/YY]]] 
or 
+[[[DD:]HH:]MM:]SS
INFINITY
absolute or relative time reservation will end (not required if Duration
specified)
**EXCLUSIVE**
N/A
N/A
requests exclusive access to resources
**FEATURE\_LIST**
<STRING>[:<STRING>]...
---
list of node features which must be possessed by the reserved resources
**FLAGS**
<STRING>[:<STRING>]...
---
list of reservation flags (See `Managing
Reservations <../7.1.5managingreservations.html>`__ for details)
**GROUP\_LIST**
<STRING>[:<STRING>]...
---
list of groups that will be allowed access to the reserved resources
**NAME**
<STRING>
name set to first name listed in ACL or ``SYSTEM`` if no ACL specified
name for new reservation
**OWNER**
<CREDTYPE>:<CREDID> where CREDTYPE is one of user, group, acct, class,
or qos
N/A
specifies which credential is granted reservation ownership privileges
**PARTITION**
<STRING>
[ANY]
partition in which resources must be located
**QOS\_LIST**
<STRING>[:<STRING>]...
---
list of QOS's that will be allowed access to the reserved resource
**RESERVATION\_**
**PROFILE**
existing reservation profile ID
N/A
requests that default reservation attributes be loaded from the
specified reservation profile (see
`RSVPROFILE <../a.fparameters.html#rsvprofile>`__)
**RESOURCE\_**
**DESCRIPTION**
colon delimited list of zero or more of the following <ATTR>=<VALUE>
pairs
**PROCS**\ =<INTEGER>
**MEM**\ =<INTEGER>
**DISK**\ =<INTEGER>
**SWAP**\ =<INTEGER> **GRES**\ =<STRING>
PROCS=-1
specifies the resources to be reserved per task.  (-1 indicates all
resources on node)
**RESOURCE\_**
**EXPRESSION**
**ALL **
or
**TASKS**\ {**==**\ \|>=}<TASKCOUNT> 
or 
<HOST\_REGEX>
Required Field.  No Default
specifies the tasks to reserve.  **ALL** indicates all resources
available should be reserved.
**Note**:   If **ALL** or a host expression is specified, Moab will
apply the reservation regardless of existing reservations and exclusive
issues. If **TASKS** is used, Moab will only allocate accessible
resources.
**STARTTIME**
[HH[:MM[:SS]]][\_MO[/DD[/YY]]] 
or 
+[[[DD:]HH:]MM:]SS
NOW
absolute or relative time reservation will start
**TRIGGER**
<STRING>
N/A
comma delimited reservation trigger list following format described in
the `trigger format <../7.1.5managingreservations.html#TRIGGER>`__
section of the reservation configuration overview.
**USER\_LIST**
<STRING>[:<STRING>]...
---
list of users that will be allowed access to the reserved resources
.. rubric:: Description
   :name: description

   The **setres** command allows an arbitrary block of resources to be
reserved for use by jobs which meet the specified access constraints. 
The timeframe covered by the reservation can be specified on either an
absolute or relative basis.  Only jobs with credentials listed in the
reservation ACL (i.e., **USERLIST**, **GROUPLIST**,...) can utilize the
reserved resources.  However, these jobs still have the freedom to
utilize resources outside of the reservation.  The reservation will be
assigned a name derived from the ACL specified.  If no reservation ACL
is specified, the reservation is created as a *system* reservation and
no jobs will be allowed access to the resources during the specified
timeframe (valuable for system maintenance, etc).  See the `Reservation
Overview <../7.1.1resoverview.html>`__ for more information.
   Reservations can be viewed using the `showres <showres.html>`__
command and can be released using the `releaseres <releaseres.html>`__
command.

.. rubric:: Example 1
   :name: example-1

   Reserve two nodes for use by users john and mary for a period of 8
hours starting in 24 hours

::

    > setres -u john:mary -s +24:00:00 -d 8:00:00 TASKS==2

    reservation 'john.1' created on 2 nodes (2 tasks)

    node001:1
    node005:1

.. rubric:: Example 2
   :name: example-2

   Schedule a system wide reservation to allow a system maintenance on
Jun 20, 8:00 AM until Jun 22, 5:00 PM.

::

    > setres -s 8:00:00_06/20 -e 17:00:00_06/22 ALL

    reservation 'system.1' created on 8 nodes (8 tasks)

    node001:1
    node002:1
    node003:1
    node004:1
    node005:1
    node006:1
    node007:1
    node008:1

.. rubric:: Example 3
   :name: example-3

   Reserve one processor and 512 MB of memory on nodes node003 through
node 006 for members of the group staff and jobs in the interactive
class.

::

    > setres -r PROCS=1:MEM=512 -g staff -l interactive 'node00[3-6]'

    reservation 'staff.1' created on 4 nodes (4 tasks)

    node003:1
    node004:1
    node005:1
    node006:1

.. |Note| image:: /resources/docs/images/caution.png

