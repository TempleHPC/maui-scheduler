
.. rubric:: showres
   :name: showres

**(Show Reservation)**
**Synopsis**

::

    showres [-f] [-n [-g]] [-o] [-r] [reservationid]

**Overview**

This command displays all reservations currently in place within Moab.
The default behavior is to display reservations on a
reservation-by-reservation basis.
**Access**

By default, this command can be run by any Moab administrator, or by any
valid user if the parameter
`RSVCTLPOLICY <../a.fparameters.html#rsvctlpolicy>`__ is set to **ANY**.

Flag
Description
**-f**
show *free* (unreserved) resources rather than reserved resources.  The
'**-f**' flag cannot be used in conjunction with the any other flag
**-g**
when used with the '-n' flag, shows '*grep*'-able output with nodename
on every line
**-n**
display information regarding all *nodes* reserved by <RSVID>
\ **-o**
display all reservations which *overlap* <RSVID> (in time and space)
 **Note**: not supported with '-n' flag
\ **-r**
display reservation timeframes in *relative* time mode
**-v**
show *verbose* output. If used with the '**-n**' flag, the command will
display all reservations found on nodes contained in <RSVID>. Otherwise,
it will show long reservation start dates including the reservation
year.

+-------------+--------------------------------------------+
| Parameter   | Description                                |
+=============+============================================+
| **RSVID**   | ID of reservation of interest - optional   |
+-------------+--------------------------------------------+

**Example 1**


::

    > showres
     
    ReservationID       Type S       Start         End    Duration    N/P    StartTime
     
    12941                Job R   -00:05:01     2:41:39     2:46:40   13/25   Thu Sep  1 15:02:50
    12944                Job R   -00:05:01     6:34:59     6:40:00   16/16   Thu Sep  1 15:02:50
    12946                Job R   -00:05:01  1:05:54:59  1:06:00:00    1/2    Thu Sep  1 15:02:50
    12954                Job R   -00:04:59     2:55:01     3:00:00    2/4    Thu Sep  1 15:02:52
    12956                Job I  1:05:54:59  1:12:34:59     6:40:00   16/32   Fri Sep  2 21:02:50
    12969                Job I     6:34:59    13:14:59     6:40:00    4/4    Thu Sep  1 21:42:50
     
    6 reservations located
     


The above example shows all reservations on the system. The fields are
as follows:

+---------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Column              | Description                                                                                                                                                                                                                                                     |
+=====================+=================================================================================================================================================================================================================================================================+
| **Type**            | Reservation Type. This will be one of the following: Job, User, Group, Account, or System.                                                                                                                                                                      |
+---------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **ReservationID**   | This is the name of the reservation. Job reservation names are identical to the job name. User, Group, or Account reservations are the user, group, or account name followed by a number. System reservations are given the name SYSTEM followed by a number.   |
+---------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **S**               | State. This field is valid only for job reservations. It indicates whether the job is (S)tarting, (R)unning, or (I)dle.                                                                                                                                         |
+---------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **Start**           | Relative start time of the reservation. Time is displayed in HH:MM:SS notation and is relative to the present time.                                                                                                                                             |
+---------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **End**             | Relative end time of the reservation. Time is displayed in HH:MM:SS notation and is relative to the present time. Reservation that will not complete in 1,000 hours are marked with the keyword INFINITY.                                                       |
+---------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **Duration**        | Duration of the reservation in HH:MM:SS notation. Reservations lasting more than 1,000 hours are marked with the keyword INFINITY.                                                                                                                              |
+---------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **Nodes**           | Number of nodes involved in reservation.                                                                                                                                                                                                                        |
+---------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **StartTime**       | Time Reservation became active.                                                                                                                                                                                                                                 |
+---------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

**Example 2**


::

    > showres -n
    reservations on Thu Sep  1 16:49:59
     
    NodeName        Type      ReservationID   JobState Task       Start    Duration  StartTime
     
    G5-001           Job              12946    Running    2    -1:47:09  1:06:00:00  Thu Sep  1 15:02:50
    G5-001           Job              12956       Idle    2  1:04:12:51     6:40:00  Fri Sep  2 21:02:50
    G5-002           Job              12956       Idle    2  1:04:12:51     6:40:00  Fri Sep  2 21:02:50
    G5-002           Job              12953    Running    2   -00:29:37     4:26:40  Thu Sep  1 16:20:22
    G5-003           Job              12956       Idle    2  1:04:12:51     6:40:00  Fri Sep  2 21:02:50
    G5-003           Job              12953    Running    2   -00:29:37     4:26:40  Thu Sep  1 16:20:22
    G5-004           Job              12956       Idle    2  1:04:12:51     6:40:00  Fri Sep  2 21:02:50
    G5-004           Job              12953    Running    2   -00:29:37     4:26:40  Thu Sep  1 16:20:22
    G5-005           Job              12956       Idle    2  1:04:12:51     6:40:00  Fri Sep  2 21:02:50
    G5-005           Job              12953    Running    2   -00:29:37     4:26:40  Thu Sep  1 16:20:22
    G5-006           Job              12956       Idle    2  1:04:12:51     6:40:00  Fri Sep  2 21:02:50
    G5-006           Job              12953    Running    2   -00:29:37     4:26:40  Thu Sep  1 16:20:22
    G5-007           Job              12956       Idle    2  1:04:12:51     6:40:00  Fri Sep  2 21:02:50
    G5-007           Job              12939    Running    2   -00:29:37     3:00:00  Thu Sep  1 16:20:22
    G5-008           Job              12956       Idle    2  1:04:12:51     6:40:00  Fri Sep  2 21:02:50
    G5-008           Job              12939    Running    2   -00:29:37     3:00:00  Thu Sep  1 16:20:22
    G5-009           Job              12956       Idle    2  1:04:12:51     6:40:00  Fri Sep  2 21:02:50
    G5-009           Job              12939    Running    2   -00:29:37     3:00:00  Thu Sep  1 16:20:22
    G5-010           Job              12956       Idle    2  1:04:12:51     6:40:00  Fri Sep  2 21:02:50
    G5-010           Job              12939    Running    2   -00:29:37     3:00:00  Thu Sep  1 16:20:22
    G5-011           Job              12956       Idle    2  1:04:12:51     6:40:00  Fri Sep  2 21:02:50
    G5-011           Job              12939    Running    2   -00:29:37     3:00:00  Thu Sep  1 16:20:22
    G5-012           Job              12956       Idle    2  1:04:12:51     6:40:00  Fri Sep  2 21:02:50
    G5-012           Job              12939    Running    2   -00:29:37     3:00:00  Thu Sep  1 16:20:22
    G5-013           Job              12956       Idle    2  1:04:12:51     6:40:00  Fri Sep  2 21:02:50
    G5-013           Job              12939    Running    2   -00:29:37     3:00:00  Thu Sep  1 16:20:22
    G5-014           Job              12956       Idle    2  1:04:12:51     6:40:00  Fri Sep  2 21:02:50
    G5-014           Job              12939    Running    2   -00:29:37     3:00:00  Thu Sep  1 16:20:22
    G5-015           Job              12956       Idle    2  1:04:12:51     6:40:00  Fri Sep  2 21:02:50
    G5-015           Job              12949    Running    2   -00:08:57     3:00:00  Thu Sep  1 16:41:02
    G5-016           Job              12956       Idle    2  1:04:12:51     6:40:00  Fri Sep  2 21:02:50
    G5-016           Job              12947    Running    2   -00:08:57     3:00:00  Thu Sep  1 16:41:02
    P690-001         Job              12944    Running    1    -1:47:09     6:40:00  Thu Sep  1 15:02:50
    P690-002         Job              12944    Running    1    -1:47:09     6:40:00  Thu Sep  1 15:02:50
    P690-003         Job              12944    Running    1    -1:47:09     6:40:00  Thu Sep  1 15:02:50
    P690-004         Job              12944    Running    1    -1:47:09     6:40:00  Thu Sep  1 15:02:50
    P690-005         Job              12944    Running    1    -1:47:09     6:40:00  Thu Sep  1 15:02:50
    P690-006         Job              12944    Running    1    -1:47:09     6:40:00  Thu Sep  1 15:02:50
    P690-007         Job              12944    Running    1    -1:47:09     6:40:00  Thu Sep  1 15:02:50
    P690-008         Job              12944    Running    1    -1:47:09     6:40:00  Thu Sep  1 15:02:50
    P690-009         Job              12944    Running    1    -1:47:09     6:40:00  Thu Sep  1 15:02:50
    P690-010         Job              12944    Running    1    -1:47:09     6:40:00  Thu Sep  1 15:02:50
    P690-011         Job              12944    Running    1    -1:47:09     6:40:00  Thu Sep  1 15:02:50
    P690-012         Job              12944    Running    1    -1:47:09     6:40:00  Thu Sep  1 15:02:50
    P690-013         Job              12944    Running    1    -1:47:09     6:40:00  Thu Sep  1 15:02:50
    P690-013         Job              12969       Idle    1     4:52:51     6:40:00  Thu Sep  1 21:42:50
    P690-014         Job              12944    Running    1    -1:47:09     6:40:00  Thu Sep  1 15:02:50
    P690-014         Job              12969       Idle    1     4:52:51     6:40:00  Thu Sep  1 21:42:50
    P690-015         Job              12944    Running    1    -1:47:09     6:40:00  Thu Sep  1 15:02:50
    P690-015         Job              12969       Idle    1     4:52:51     6:40:00  Thu Sep  1 21:42:50
    P690-016         Job              12944    Running    1    -1:47:09     6:40:00  Thu Sep  1 15:02:50
    P690-016         Job              12969       Idle    1     4:52:51     6:40:00  Thu Sep  1 21:42:50
     
    52 nodes reserved


This example shows reservations for nodes. The fields are as follows:

+---------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Column              | Description                                                                                                                                                                                                                                                     |
+=====================+=================================================================================================================================================================================================================================================================+
| **NodeName**        | Node on which reservation is placed.                                                                                                                                                                                                                            |
+---------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **Type**            | Reservation Type. This will be one of the following: Job, User, Group, Account, or System.                                                                                                                                                                      |
+---------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **ReservationID**   | This is the name of the reservation. Job reservation names are identical to the job name. User, Group, or Account reservations are the user, group, or account name followed by a number. System reservations are given the name SYSTEM followed by a number.   |
+---------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **JobState**        | This field is valid only for job reservations. It indicates the state of the job associated with the reservation.                                                                                                                                               |
+---------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **Start**           | Relative start time of the reservation. Time is displayed in HH:MM:SS notation and is relative to the present time.                                                                                                                                             |
+---------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **Duration**        | Duration of the reservation in HH:MM:SS notation. Reservations lasting more than 1000 hours are marked with the keyword INFINITY.                                                                                                                               |
+---------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **StartTime**       | Time Reservation became active.                                                                                                                                                                                                                                 |
+---------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

**Example 3**


::

    > showres 12956
     
    ReservationID       Type S       Start         End    Duration    N/P    StartTime
     
    12956                Job I  1:04:09:32  1:10:49:32     6:40:00   16/32   Fri Sep  2 21:02:50
     
    1 reservation located
     


In this example, information for a specific reservation (job) is
displayed.

.. rubric:: See Also:
   :name: see-also

-  `Moab Client Installation <../2.2installation.html#client>`__ -
   explains how to distribute this command to client nodes
-  `mrsvctl -c <mrsvctl.html>`__ - create new reservations.
-  `mrsvctl -r <mrsvctl.html>`__ - release existing reservations.
-  `mdiag -r <mdiag.html>`__ - diagnose/view the state of existing
   reservations.
-  `Reservation Overview <../7.1.1resoverview.html>`__ - description of
   reservations and their use.

