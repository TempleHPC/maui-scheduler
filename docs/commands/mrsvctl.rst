
.. rubric:: mrsvctl
   :name: mrsvctl

**(Moab Reservation Control)**
**Synopsis**

::

    mrsvctl -c [-a acl] [-b subtype] [-d duration] [-D description] [-e endtime] [-E]
               [-f features] [-F flags] [-g rsvgroup] [-h hostexp] 
               [-I {cancel|end|failure|start}] [-n name] [-p partition] [-P profile]
               [-R resources] [-s starttime] [-S setvalue] [-t tasks]
               [-T trigger] [-V variable] [-x joblist]
    mrsvctl -l [{reservationid | -i index}]
    mrsvctl -C [-g standing_reservationid]
    mrsvctl -m {reservationid | -i index} [-d duration] [-e endtime] [-h hostexp]
               [-s starttime] [--flags=force]
    mrsvctl -q {reservationid | -i index} 
    mrsvctl -r {reservationid | -i index}
    mrsvctl -C {reservationid}

**Overview**

**mrsvctl** controls the creation, modification, querying, and releasing
of reservations.
The timeframe covered by the reservation can be specified on either an
absolute or relative basis. Only jobs with credentials listed in the
reservation's **access control list** can utilize the reserved
resources. However, these jobs still have the freedom to utilize
resources outside of the reservation. The reservation will be assigned a
name derived from the ACL specified. If no reservation ACL is specified,
the reservation is created as a *system* reservation and no jobs will be
allowed access to the resources during the specified timeframe (valuable
for system maintenance, etc). See the `Reservation
Overview <../7.1.1resoverview.html>`__ for more information.

Reservations can be viewed using the **-q** flag and can be released
using the **-r** flag.

+----------+------------------------------------------------------------------------------------------------------------------------------------------------------------+
| |Note|   | By default, reservations are not exclusive and may overlap with other reservations and jobs. Use the '`-E <#EXCLUSIVE>`__' flag to adjust this behavior.   |
+----------+------------------------------------------------------------------------------------------------------------------------------------------------------------+

**Access**

By default, this command can be run by level 1 and level 2 Moab
administrators (see `ADMINCFG <../a.fparameters.html#admincfg>`__).

**Format**

-a
Name:
\ **ACL**
Format:
<TYPE>==<VAL>[,<TYPE>==<VAL>]...
Where <TYPE> is one of the following:
    **ACCT**,
    **CLASS**,
    **GROUP**,
    **JATTR**,
    **QOS**,
    **RSV**, or
    **USER**
Default:
---
Description:
List of credentials with access to the reserved resources (See also:
`ACL Modifiers <../7.1.5managingreservations.html#aclmodifiers>`__)
Example:


::

    > mrsvctl -c -h node01 -a USER==john+,CLASS==batch-


Moab will make a reservation on node01 allowing access to user john and
restricting access from class batch when other resources are available
to class batch


::

    > mrsvctl -m -a USER-=john system.1


Moab will remove user john from the ``system.1`` reservation
Notes:

-  There are three different assignment operators that can be used for
   modifying credentials in the ACL. The operator '==' will reassess the
   list for that particular credential type. The '+=' operator will
   append to the list for that credential type, and '-=' will remove
   from the list.
-  To add multiple credentials of the same type with one command, use a
   colon to separate them. To separate lists of different credential
   types, use commas. For example, to reassign the user list to consist
   of users Joe and Bob, and to append the group MyGroup to the groups
   list on the system.1 reservation, you could use the command "mrsvctl
   -m -a USER==Joe:Bob,GROUP+=MyGroup system.1".
-  Any of the ACL modifiers may be used. When using them, it is often
   useful to put single quotes on either side of the assignment command.
   For example, "mrsvctl -m -a **'**\ USER==&Joe\ **'** system.1".
-  Some flags are mutually exclusive. For example, the ! modifier means
   that the credential is blocked from the reservation and the &
   modifier means that the credential **must** run on that reservation.
   Moab will take the most recently parsed modifier. Modifiers may be
   placed on either the left or the right of the argument, so
   "USER==&JOE" and "USER==JOE&" are equivalent. Moab parses each
   argument starting from right to left on the right side of the
   argument, then from left to right on the left side. So, if the
   command was "USER==&!Joe&", Moab would keep the equivalent of
   "USER==!Joe" because the ! would be the last one parsed.
-  You can set a reservation to have a time limit for submitted jobs
   using DURATION and the \* modifier. For example, "mrsvctl -m -a
   'DURATION<=\*1:00:00' system.1" would cause the system.1 reservation
   to not accept any jobs with a walltime greater than one hour.
-  You can verify the ACL of a reservation using the "mdiag -r" command.


::

    mrsvctl -m -a 'USER==Joe:Bob,GROUP-=BadGroup,ACCT+=GoodAccount,DURATION<=*1:00:00' system.1


Moab will reassign the USER list to be Joe and Bob, will remove BadGroup
from the GROUP list, append GoodAccount to the ACCT list, and only allow
jobs that have a submitted walltime of an hour or less on the
``system.1`` reservation.


::

    mrsvctl -m -a 'USER==Joe,USER==Bob' system.1


Moab will assign the USER list to Joe, and then reassign it again to
Bob. The final result will be that the USER list will just be Bob. To
add Joe and Bob, use "mrsvctl -m -a USER==Joe:Bob system.1" or "mrsvctl
-m -a USER==Joe,USER+=Bob system.1".
 
 
-b
Name:
\ **SUBTYPE**
Format:
One of the `node category <../5.4nodeavailability.html#nodecat>`__
values or node category shortcuts.
Default:
---
Description:
Add subtype to reservation.
Example:


::

    > mrsvctl -c -b swmain -t ALL


Moab will associate the reserved nodes with the `node
category <../5.4nodeavailability.html#nodecat>`__ **swmain**.
 
 
-c
Name:
\ **CREATE**
Format:
<ARGUMENTS>
Default:
---
Description:
Creates a reservation.

Note: The -x flag, when used with -F ignjobrsv, lets users create
reservations but exclude certain nodes from being part of the
reservation because they are running specific jobs. The -F flag
instructs mrsvctl to still consider nodes with current running jobs.

Examples:


::

    > mrsvctl -c -t ALL


Moab will create a reservation across all system resources.


::

    > mrsvctl -c -t 5 -F ignjobrsv -x moab.5,moab.6


Moab will create the reservation while assigning the nodes. Nodes
running jobs moab5 and moab6 will not be assigned to the reservation.

 
 
-C
Name:
\ **CLEAR**
Format:
<RSVID> \| -g <SRSVID>
Default:
---
Description:
Clears any disabled time slots from standing reservations and allows the
recreation of disabled reservations
Example:


::

    > mrsvctl -C -g testing


Moab will clear any disabled timeslots from the standing reservation
*testing*.
 
 
-d
Name:
\ **DURATION**
Format:
[[[DD:]HH:]MM:]SS
Default:
INFINITY
Description:
Duration of the reservation (not needed if ENDTIME is specified)
Example:


::

    > mrsvctl -c -h node01 -d 5:00:00


Moab will create a reservation on ``node01`` lasting 5 hours.
 
 
-D
Name:
\ **DESCRIPTION**
Format:
<STRING>
Default:
---
Description:
Human-readable description of reservation or purpose
Example:


::

    > mrsvctl -c -h node01 -d 5:00:00 -D 'system maintenance to test network'


Moab will create a reservation on node01 lasting 5 hours.
 
 
-e
Name:
\ **ENDTIME**
Format:
[HH[:MM[:SS]]][\_MO[/DD[/YY]]] 
or 
+[[[DD:]HH:]MM:]SS
Default:
INFINITY
Description:
Absolute or relative time reservation will end (not required if Duration
specified). ENDTIME also supports an epoch timestamp.
Example:


::

    > mrsvctl -c -h node01 -e +3:00:00


Moab will create a reservation on node01 ending in 3 hours.
 
 
-E
Name:
\ **EXCLUSIVE**
Format:
N/A
Default:
---
Description:
When specified, Moab will only create a reservation if there are no
other reservations (exclusive or otherwise) which would conflict with
the time and space constraints of this reservation.  If exceptions are
desired, the `rsvaccesslist <#rsvaccesslist>`__ attribute can be set or
the `ignrsv <../7.1.5managingreservations.html#ignrsv>`__ flag can be
used.
Example:


::

    > mrsvctl -c -h node01 -E


Moab will only create a reservation on ``node01`` if no conflicting
reservations are found.

+----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| |Note|   | This flag is only used at the time of reservation creation. Once the reservation is created, Moab allows jobs into the reservation based on the ACL. Also, once the exclusive reservation is created, it is possible that Moab will overlap it with jobs that match the ACL.   |
+----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

 
 
-f
Name:
\ **FEATURES**
Format:
<STRING>[:<STRING>]...
Default:
---
Description:
List of node features which must be possessed by the reserved resources
Example:


::

    > mrsvctl -c -h node[0-9] -f fast


Moab will create a reservation on nodes matching the expression and
which also have the feature ``fast.``
 
 
-F
Name:
\ **FLAGS**
Format:
<flag>[[,<flag>]...]
Default:
---
Description:
Comma-delimited list of flags to set for the reservation (see `Managing
Reservations <../7.1.5managingreservations.html#flagoverview>`__ for
flags).
Example:


::

    > mrsvctl -c -h node01 -F ignstate


Moab will create a reservation on ``node01`` ignoring any conflicting
node states.
 
 
-g
Name:
\ **RSVGROUP**
Format:
<STRING>
Default:
---
Description:
For a **create** operation, create a reservation in this reservation
group. For list and modify operations, take actions on all reservations
in the specified reservation group. The **-g** option can also be used
in conjunction with the `**-r** <#RELEASE>`__ option to release a
reservation associated with a specified group. See `Reservation
Group <../7.1.1resoverview.html#rsvgroup>`__ for more information.
Example:


::

    > mrsvctl -c -g staff -h 'node0[1-9]'


Moab will create a reservation on nodes matching the expression given
and assign it to the reservation group ``staff.``
 
 
-h
Name:
\ **HOSTLIST**
Format:
<STRING>
or
**ALL**
Default:
---
Description:
Host list (comma delimited), host regular expression, host range, or
class mapping indicating the nodes which the reservation will allocate.

+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| |Caution|   | The **HOSTLIST** attribute is always treated as a regular expression. foo10 will map to foo10, foo101, foo1006, etc. To request an exact host match, the expression can be bounded by the carat and dollar op expression markers as in ^foo10$.   |
+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

Example:


::

    > mrsvctl -c -h 'node0[1-9]'


Moab will create a reservation on nodes matching the expression given.


::

    > mrsvctl -c -h class:batch


Moab will create a reservation on all nodes which support class/queue
``batch.``
 
 
-i
Name:
\ **INDEX**
Format:
<STRING>
Default:
---
Description:
Use the reservation index instead of full reservation ID.
Example:


::

    > mrsvctl -m -i 1 starttime=+5:00


Moab will modify the reservation with the index of 1 to start in 5
minutes.
 
 
-I
Name:
\ **SIGNAL**
Format:
{cancel\|end\|failure\|start}
Default:
---
Description:
Send signals under the specified event conditions.
Example:


::

    > mrsvctl -m -I start starttime=+5:00


Moab will send a signal when the start event occurs.
 
 
-l
Name:
\ **LIST**
Format:
<RSV\_ID> or ALL
RSV\_ID can the name of a reservation or a regular expression.
Default:
ALL
Description:
List reservation(s).
Example:


::

    > mrsvctl -l system*


Moab will list all of the reservations whose names start with
``system``.
 
 
-m
Name:
\ **MODIFY**
Format:
<ATTR>=<VAL>[-m <ATTR2>=<VAL2>]...
Where <ATTR> is one of the following:
+-----------------------------------------------------------+-----------------------------------------------------+
| **`flags <../7.1.5managingreservations.html#flags>`__**   |                                                     |
+-----------------------------------------------------------+-----------------------------------------------------+
| **duration**                                              | duration{+=\|-=\|=}<RELTIME>                        |
+-----------------------------------------------------------+-----------------------------------------------------+
| **endtime**                                               | endtime{+=\|-=}<RELTIME> or endtime=<ABSTIME>       |
+-----------------------------------------------------------+-----------------------------------------------------+
| **hostexp**                                               | hostexp[+=\|-=]<node>[,<node>]                      |
+-----------------------------------------------------------+-----------------------------------------------------+
| **label**                                                 | label=<LABEL>                                       |
+-----------------------------------------------------------+-----------------------------------------------------+
| **reqtaskcount**                                          | reqtaskcount{+=\|-=\|=}<TASKCOUNT>                  |
+-----------------------------------------------------------+-----------------------------------------------------+
| **rsvgroup**                                              |                                                     |
+-----------------------------------------------------------+-----------------------------------------------------+
| **starttime**                                             | starttime{+=\|-=}<RELTIME> or starttime=<ABSTIME>   |
+-----------------------------------------------------------+-----------------------------------------------------+

Default:
---
Description:
Modify aspects of a reservation.
Example:


::

    > mrsvctl -m duration=2:00:00 system.1


Moab sets the duration of reservation ``system.1`` to be exactly two
hours, thus modifying the endtime of the reservation.


::

    > mrsvctl -m starttime+=5:00:00 system.1


Moab advances the starttime of system.1 five hours from its current
starttime (without modifying the duration of the reservation).


::

    > mrsvctl -m endtime-=5:00:00 system.1


Moab moves the endtime of reservation system.1 ahead five hours from its
current endtime (without modifying the starttime; thus, this action is
equivalent to modifying the duration of the reservation).


::

    > mrsvctl -m -s 15:00:00_7/6/08 system.1


Moab sets the starttime of reservation system.1 to 3:00 p.m. on July 6,
2008.


::

    > mrsvctl -m -s -=5:00:00 system.1


Moab moves the starttime of reservation system.1 ahead five hours.


::

    > mrsvctl -m -s +5:00:00 system.1


Moab moves the starttime of reservation system.1 five hours from the
current time.


::

    > mrsvctl -m -d +=5:00:00 system.1


Moab extends the duration of system.1 by five hours.


::

    > mrsvctl -m flags+=ADVRES system.1


Moab adds the flag ``ADVRES`` to reservation ``system.1``.
Notes:

-  When the label is assigned for a reservation, the reservation can
   then be referenced by that label as well as by the reservation name.
   The reservation name cannot be modified.
-  The starttime of a reservation can be modified by using **starttime**
   or **-s**. Modifing the starttime does not change the duration of the
   reservation, so the endtime changes as well. The starttime can be
   changed to be before the current time, but if the change causes the
   endtime to be before the current time, the change is not allowed.
-  The endtime of a reservation can be modified by using **endtime** or
   **-e**. Modifying the endtime changes the duration of the reservation
   as well (and vice versa). An endtime **cannot** be placed before the
   starttime or before the current time.
-  The duration can be changed by using **duration** or **-d**. Duration
   cannot be negative.
-  The += and -= operators operate on the time of the reservation
   (starttime+=5 adds five seconds to the current reservation
   starttime), while + and - operate on the current time (starttime+5
   sets the starttime to five seconds from now). The + and - operators
   can be used on -s, and + can be used on -e as well.
-  If the starttime or endtime specified is before the current time
   without a date specified, it is set to the next time that fits the
   command. To force the date, add the date as well. For the following
   examples, assume that the current time is 9:00 a.m. on March 1, 2007.


::

    > mrsvctl -m -s 8:00:00_3/1/07 system.1


Moab moves system.1's starttime to 8:00 a.m., March 1.


::

    > mrsvctl -m -s 8:00:00 system.1


Moab moves system.1's starttime to 8:00 a.m., March 2.


::

    > mrsvctl -m -e 7:00:00 system.1


Moab moves system.1's endtime to 7:00 a.m., March 3. This happens
because the endtime must also be after the starttime, so Moab continues
searching until it has found a valid time that is in the future and
after the starttime.


::

    > mrsvctl -m -e 7:00:00_3/2/07 system.1


Moab will return an error because the endtime cannot be before the
starttime.
 
 
-n
Name:
\ **NAME**
Format:
<STRING>
Default:
---
Description:
Name for new reservation.

+----------+--------------------------------------------------------------------------------------------------------------------------+
| |Note|   | If no name is specified, the reservation name is set to first name listed in ACL or ``SYSTEM`` if no ACL is specified.   |
+----------+--------------------------------------------------------------------------------------------------------------------------+

+----------+-------------------------------------------------+
| |Note|   | Reservation names may not contain whitespace.   |
+----------+-------------------------------------------------+

Example:


::

    mrsvctl -c -h node01 -n John


Moab will create a reservation on node01 with the name ``John``.
 
 
-p
Name:
\ **PARTITION**
Format:
<STRING>
Default:
---
Description:
Only allocate resources from the specified partition
Example:


::

    mrsvctl -c -p switchB -t 14


Moab will allocate 14 tasks from the ``switchB`` partition.
 
 
-P
Name:
\ **PROFILE**
Format:
<STRING>
Default:
---
Description:
Indicates the `reservation profile <../a.fparameters.html#rsvprofile>`__
to load when creating this reservation
Example:


::

    mrsvctl -c -P testing2 -t 14


Moab will allocate 14 tasks to a reservation defined by the ``testing2``
reservation profile.
 
 
-q
Name:
\ **QUERY**
Format:
<RSV\_ID> where <RSV\_ID> is treated as a regular expression
or
ALL --flags=COMPLETED
Default:
---
Description:
Get diagnostic information or list all completed reservations.
Example:


::

    mrsvctl -q ALL --flags=COMPLETED


Moab will query completed reservations.


::

    mrsvctl -q system.1


Moab will query the reservation ``system.1``.
 
 
-r
Name:
\ **RELEASE**
Format:
<RSV\_ID> (treated as a regular expression)
Default:
---
Description:
Releases the specified reservation.
Example:


::

    > mrsvctl -r system.1


Moab will release reservation ``system.1``.


::

    > mrsvctl -r -g idle


Moab will release all idle job reservations.
 
 
-R
Name:
\ **RESOURCES**
Format:
<tid> or
<RES>=<VAL>[{,\|+\|;}<RES>=<VAL>]...
Where <RES> is one of the following:
    **PROCS**,
    **MEM**,
    **DISK**,
    **SWAP**
    **GRES**
Default:
PROCS=-1
Description:
Specifies the resources to be reserved per task. (-1 indicates all
resources on node)

+----------+---------------------------------------------------------------------------------+
| |Note|   | For **GRES** resources, <VAL> is specified in the format <GRESNAME>[:<COUNT>]   |
+----------+---------------------------------------------------------------------------------+

Example:


::

    > mrsvctl -c -R MEM=100;PROCS=2 -t 2


Moab will create a reservation for two tasks with the specified
resources
 
 
-s
Name:
\ **STARTTIME**
Format:
[HH[:MM[:SS]]][\_MO[/DD[/YY]]] 
or 
+[[[DD:]HH:]MM:]SS
Default:
[NOW]
Description:
Absolute or relative time reservation will start. STARTTIME also
supports an epoch timestamp.
Example:


::

    > mrsvctl -c -t ALL -s 3:00:00_4/4/04


Moab will create a reservation on all system resources at 3:00 am on
April 4, 2004


::

    > mrsvctl -c -h node01 -s +5:00


Moab will create a reservation in 5 minutes on node01
 
 
-S
Name:
\ **SET ATTRIBUTE**
Format:
<ATTR>=<VALUE> where <ATTR> is one of
**aaccount** (accountable account),
**agroup** (accountable group),
**aqos** (accountable QoS),
**auser** (accountable user),
**reqarch** (required architecture),
**reqmemory** (required node memory - in MB),
**reqnetwork** (required network),
**reqos** (required operating system), or
**\ rsvaccesslist** (comma delimited list of reservations or
reservation groups which can be accessed by this reservation request)
Default:
---
Description:
Specifies a reservation attribute will be used to create this
reservation
Example:


::

    > mrsvctl -c -h node01 -S aqos=high


Moab will create a reservation on ``node01`` and will use the QOS
``high`` as the accountable credential
 
 
-t
Name:
\ **TASKS**
Format:
**<INTEGER>[-<INTEGER>]** 
Default:
---
Description:
Specifies the number of tasks to reserve. ``ALL`` indicates all
resources available should be reserved.

+----------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| |Note|   | If the task value is set to ``ALL``, Moab applies the reservation regardless of existing reservations and exclusive issues. If an integer is used, Moab only allocates accessible resources. If a range is specified Moab attempts to reserve the maximum number of tasks, or at least the minimum.   |
+----------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

Example:


::

    > mrsvctl -c -t ALL


Moab will create a reservation on all resources.


::

    > mrsvctl -c -t 3


Moab will create a reservation for three tasks.


::

    > mrsvctl -c -t 3-10 -E


Moab will attempt to reserve 10 tasks but will fail if it cannot get at
least three.
 
 
-T
Name:
\ **TRIGGER**
Format:
<STRING>
Default:
N/A
Description:
Comma-delimited reservation trigger list following format described in
the trigger format section of the reservation configuration overview.
See `Trigger Creation <../19.1triggers.html>`__ for more information.
Example:


::

    > mrsvctl -c -h node01 -T offset=200,etype=start,atype=exec,action=/opt/moab/tools/support.diag.pl


Moab will create a reservation on node01 and fire the script
``/tmp/email.sh`` 200 seconds after it starts
 
 
-V
Name:
\ **VARIABLE**
Format:
<name>[=<value>][[;<name>[=<value>]]...]
Default:
N/A
Description:
Semicolon-delimited list of variables that will be set when the
reservation is created (see `17.5.4 Trigger
Variables <../20.1triggers.html>`__). Names with no values will simply
be set to TRUE.
Example:


::

    > mrsvctl -c -h node01 -V $T1=mac;var2=18.19


Moab will create a reservation on node01 and set $T1 to mac and var2 to
18.19.
-x
Name:
\ **JOBLIST**
Format:
-x <jobs to be excluded>
Default:
N/A
Description:
The -x flag, when used with -F ignjobrsv, lets users create reservations
but exclude certain nodes that are running the listed jobs.The -F flag
instructs mrsvctl to still consider nodes with current running jobs. The
nodes are not listed directly.
Example:


::

    > mrsvctl -c -t 5 -F ignjobrsv -x moab.5,moab.6


Moab will create the reservation while assigning the nodes. Nodes
running jobs moab5 and moab6 will not be assigned to the reservation.
**Parameters**

\ **RESERVATION ID**
Format:
<STRING>
Default:
---
Description:
The name of a reservation or a regular expression for several
reservations.
Example:


::

    system*


Specifies all reservations starting with 'system'.
**Resource Allocation Details**

When allocating resources, the following rules apply:

-  When specifying tasks, a each task defaults to *one full compute
   node* unless otherwise specified using the `-R <#RESOURCES>`__
   specification
-  When specifying tasks, the reservation will not be created unless all
   requested resources can be allocated. (This behavior can be changed
   by specifying '**`-F <#FLAGS>`__ besteffort**')
-  When specifying tasks or hosts, only nodes in an idle or running
   state will be considered. (This behavior can be changed by specifying
   '**`-F <#FLAGS>`__ ignstate**')

**\ Reservation Timeframe Modification**

Moab supports dynamically modifying the timeframe of existing
reservations. This can be accomplished using the `mrsvctl
-m <#MODIFY>`__ flag. By default, Moab will perform advanced boundary
and resource access to verify that the modification does not result in
an invalid scheduler state. However, in certain circumstances
administrators may wish to *FORCE* the modification in spite of any
access violations. This can be done using the switch *mrsvctl -m
--flags=force* which forces Moab to bypass any access verification and
force the change through.
**Extending a reservation by modifying the endtime**

The following increases the endtime of a reservation using the "+=" tag:


::

    $> showres

    ReservationID       Type S       Start         End    Duration    N/P    StartTime
    system.1            User -    11:35:57  1:11:35:57  1:00:00:00    1/2    Sat Nov 18 00:00:00

    1 reservation located

    $> mrsvctl -m endtime+=24:00:00 system.1
    endtime for rsv 'system.1' changed

    $> showres

    ReservationID       Type S       Start         End    Duration    N/P    StartTime
    system.1            User -    11:35:22  2:11:35:22  2:00:00:00    1/2    Sat Nov 18 00:00:00

    1 reservation located


The following increases the endtime of a reservation by setting the
endtime to an absolute time:


::

    $> showres

    ReservationID       Type S       Start         End    Duration    N/P    StartTime
    system.1            User -    11:33:18  1:11:33:18  1:00:00:00    1/2    Sat Nov 18 00:00:00

    1 reservation located

    $> mrsvctl -m endtime=0_11/20 system.1
    endtime for rsv 'system.1' changed

    $> showres

    ReservationID       Type S       Start         End    Duration    N/P    StartTime
    system.1            User -    11:33:05  2:11:33:05  2:00:00:00    1/2    Sat Nov 18 00:00:00

    1 reservation located


**Extending a reservation by modifying the duration**

The following increases the duration of a reservation using the "+="
tag:


::

    $> showres

    ReservationID       Type S       Start         End    Duration    N/P    StartTime
    system.1            User -    11:28:46  1:11:28:46  1:00:00:00    1/2    Sat Nov 18 00:00:00

    1 reservation located

    $> mrsvctl -m duration+=24:00:00 system.1
    duration for rsv 'system.1' changed

    >$ showres

    ReservationID       Type S       Start         End    Duration    N/P    StartTime
    system.1            User -    11:28:42  2:11:28:42  2:00:00:00    1/2    Sat Nov 18 00:00:00

    1 reservation located


The following increases the duration of a reservation by setting the
duration to an absolute time:


::

    $> showres

    ReservationID       Type S       Start         End    Duration    N/P    StartTime
    system.1            User -    11:26:41  1:11:26:41  1:00:00:00    1/2    Sat Nov 18 00:00:00

    1 reservation located

    $> mrsvctl -m duration=48:00:00 system.1
    duration for rsv 'system.1' changed

    $> showres

    ReservationID       Type S       Start         End    Duration    N/P    StartTime
    system.1            User -    11:26:33  2:11:26:33  2:00:00:00    1/2    Sat Nov 18 00:00:00

    1 reservation located


**Shortening a reservation by modifying the endtime**

The following modifies the endtime of a reservation using the "-=" tag:


::

    $> showres

    ReservationID       Type S       Start         End    Duration    N/P    StartTime
    system.1            User -    11:15:51  2:11:15:51  2:00:00:00    1/2    Sat Nov 18 00:00:00

    1 reservation located

    $> mrsvctl -m endtime-=24:00:00 system.1
    endtime for rsv 'system.1' changed

    $> showres

    ReservationID       Type S       Start         End    Duration    N/P    StartTime
    system.1            User -    11:15:48  1:11:15:48  1:00:00:00    1/2    Sat Nov 18 00:00:00

    1 reservation located


The following modifies the endtime of a reservation by setting the
endtime to an absolute time:


::

    $ showres

    ReservationID       Type S       Start         End    Duration    N/P    StartTime
    system.1            User -    11:14:00  2:11:14:00  2:00:00:00    1/2    Sat Nov 18 00:00:00

    1 reservation located

    $> mrsvctl -m endtime=0_11/19 system.1
    endtime for rsv 'system.1' changed

    $> showres

    ReservationID       Type S       Start         End    Duration    N/P    StartTime
    system.1            User -    11:13:48  1:11:13:48  1:00:00:00    1/2    Sat Nov 18 00:00:00

    1 reservation located


**Shortening a reservation by modifying the duration**

The following modifies the duration of a reservation using the "-=" tag:


::

    $> showres

    ReservationID       Type S       Start         End    Duration    N/P    StartTime
    system.1            User -    11:12:20  2:11:12:20  2:00:00:00    1/2    Sat Nov 18 00:00:00

    1 reservation located

    $> mrsvctl -m duration-=24:00:00 system.1
    duration for rsv 'system.1' changed

    $> showres

    ReservationID       Type S       Start         End    Duration    N/P    StartTime
    system.1            User -    11:12:07  1:11:12:07  1:00:00:00    1/2    Sat Nov 18 00:00:00

    1 reservation located


The following modifies the duration of a reservation by setting the
duration to an absolute time:


::

    $> showres

    ReservationID       Type S       Start         End    Duration    N/P    StartTime
    system.1            User -    11:10:57  2:11:10:57  2:00:00:00    1/2    Sat Nov 18 00:00:00

    1 reservation located

    $> mrsvctl -m duration=24:00:00 system.1
    duration for rsv 'system.1' changed

    $> showres

    ReservationID       Type S       Start         End    Duration    N/P    StartTime
    system.1            User -    11:10:50  1:11:10:50  1:00:00:00    1/2    Sat Nov 18 00:00:00

    1 reservation located


**Modifying the starttime of a reservation**

The following increases the starttime of a reservation using the "+="
tag:


::

    $> showres

    ReservationID       Type S       Start         End    Duration    N/P    StartTime
    system.1            User -    11:08:30  2:11:08:30  2:00:00:00    1/2    Sat Nov 18 00:00:00

    1 reservation located

    $> mrsvctl -m starttime+=24:00:00 system.1
    starttime for rsv 'system.1' changed

    $> showres

    ReservationID       Type S       Start         End    Duration    N/P    StartTime
    system.1            User -  1:11:08:22  3:11:08:22  2:00:00:00    1/2    Sun Nov 19 00:00:00

    1 reservation located


The following decreases the starttime of a reservation using the "-="
tag:


::

    $> showres

    ReservationID       Type S       Start         End    Duration    N/P    StartTime
    system.1            User -    11:07:04  2:11:07:04  2:00:00:00    1/2    Sat Nov 18 00:00:00

    1 reservation located

    $> mrsvctl -m starttime-=24:00:00 system.1
    starttime for rsv 'system.1' changed

    $> showres

    ReservationID       Type S       Start         End    Duration    N/P    StartTime
    system.1            User -   -12:53:04  1:11:06:56  2:00:00:00    1/2    Fri Nov 17 00:00:00

    1 reservation located


The following modifies the starttime of a reservation using an absolute
time:


::

    $> showres

    ReservationID       Type S       Start         End    Duration    N/P    StartTime
    system.1            User -    11:05:31  2:11:05:31  2:00:00:00    1/2    Sat Nov 18 00:00:00

    1 reservation located

    $> mrsvctl -m starttime=0_11/19 system.1
    starttime for rsv 'system.1' changed

    $> showres

    ReservationID       Type S       Start         End    Duration    N/P    StartTime
    system.1            User -  1:11:05:18  3:11:05:18  2:00:00:00    1/2    Sun Nov 19 00:00:00

    1 reservation located


The following modifies the starttime of a reservation using an absolute
time:


::

    $> showres

    ReservationID       Type S       Start         End    Duration    N/P    StartTime
    system.1            User -    11:04:04  2:11:04:04  2:00:00:00    1/2    Sat Nov 18 00:00:00

    1 reservation located

    $> mrsvctl -m starttime=0_11/17 system.1
    starttime for rsv 'system.1' changed

    $> showres

    ReservationID       Type S       Start         End    Duration    N/P    StartTime
    system.1            User -   -12:56:02  1:11:03:58  2:00:00:00    1/2    Fri Nov 17 00:00:00

    1 reservation located


**Examples**

-  `Example 1 <#example1>`__: Basic Reservation
-  `Example 2 <#example2>`__: System Maintenance Reservation
-  `Example 3 <#example3>`__: Explicit Task Description
-  `Example 4 <#example4>`__: Dynamic Reservation Modification
-  `Example 5 <#example5>`__: Adding a Reservation Trigger
-  `Example 6 <#example6>`__: Index-based Reservation Release
-  `Example 7 <#example7>`__: Reservation Modification
-  `Example 8 <#example8>`__: Allocating Reserved Resources
-  `Example 9 <#example9>`__: Modifying an Existing Reservation

**\ Example 1: Basic Reservation**

Reserve two nodes for use by users john and mary for a period of 8 hours
starting in 24 hours


::

    > mrsvctl -c -a USER=john,USER=mary -s +24:00:00 -d 8:00:00 -t 2

    reservation 'system.1' created


**\ Example 2: System Maintenance Reservation**

Schedule a system wide reservation to allow a system maintenance on Jun
20, 8:00 AM until Jun 22, 5:00 PM.


::

    % mrsvctl -c -s 8:00:00_06/20 -e 17:00:00_06/22 -h ALL

    reservation 'system.1' created


**\ Example 3: Explicit Task Description**

Reserve one processor and 512 MB of memory on nodes node003 through node
006 for members of the group staff and jobs in the interactive class


::

    > mrsvctl -c -R PROCS=1,MEM=512 -a GROUP=staff,CLASS=interactive -h 'node00[3-6]'

    reservation 'system.1' created


**\ Example 4: Dynamic Reservation Modification**

Modify reservation john.1 to start in 2 hours, run for 2 hours, and
include node02 in the hostlist.


::

    > mrsvctl -m starttime=+2:00:00,duration=2:00:00,HostExp+=node02

    Note:  hosts added to rsv system.3


**\ Example 5: Adding a Reservation Trigger**

Add a trigger to reservation system.1


::

    > mrsvctl -m TRIGGER=X

    Note:  trigger added to rsv system.1


**\ Example 6: Index-based Reservation Release**

Release reservation system.1 using its index.


::

    > mrsvctl -r -i 1

    reservation 'system.1' successfully released


**\ Example 7: Reservation Modification**

Remove user John's access to reservation system.1


::

    > mrsvctl -m -a USER=John system.1 --flags=unset

    successfully changed ACL for rsv system.1


**\ Example 8: Allocating Reserved Resources**

Allocate resources for group ``dev`` which are
`exclusive <#EXCLUSIVE>`__ except for resources found within
reservations myrinet.3 or john.6


::

    > mrsvctl -c -E -a group=dev,rsv=myrinet.3,rsv=john.6 -h 'node00[3-6]'

    reservation 'dev.14' created


Create exclusive ``network`` reservation on racks 3 and 4


::

    > mrsvctl -c -E -a group=ops -g network -f rack3 -h ALL 

    reservation 'ops.1' created

    > mrsvctl -c -E -a group=ops -g network -f rack4 -h ALL

    reservation 'ops.2' created


Allocate 64 nodes for 2 hours to new reservation and grant access to
reservation ``system.3`` and all reservations in the reservation group
``network``


::

    > mrsvctl -c -E -d 2:00:00 -a group=dev -t 64 -S rsvaccesslist=system.3,network 

    reservation 'system.23' created


Allocate 4 nodes for 1 hour to new reservation and grant access to idle
job reservations


::

    > mrsvctl -c -E -d 1:00:00 -t 4 -S rsvaccesslist=idle

    reservation 'system.24' created


**\ Example 9: Modifying an Existing Reservation**

Remove user ``john`` from reservation ACL


::

    > mrsvctl -m -a USER=john system.1 --flags=unset

    successfully changed ACL for rsv system.1


Change reservation group


::

    > mrsvctl -m RSVGROUP=network ops.4

    successfully changed RSVGROUP for rsv ops.4


.. rubric:: See Also
   :name: see-also

-  `Moab Client Installation <../2.2installation.html#client>`__ -
   explains how to distribute this command to client nodes
-  `Admin Reservation Overview <../7.1.2adminreservations.html>`__
-  `showres <showres.html>`__
-  `mdiag -r <mdiag-reservations.html>`__
-  `mshow -a <mshowa.html>`__ command to identify available resources
-  `job to rsv
   binding <../7.1.5managingreservations.html#aclmodifiers>`__
.. |Caution| image:: /resources/docs/images/caution.png

