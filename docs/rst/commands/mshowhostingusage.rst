
.. rubric:: mshow -a
   :name: mshow--a

**(Moab Show Available Resources)**
.. rubric:: Basic Current and Future Requests
   :name: basic-current-and-future-requests

The **mshow** command can report information on many aspects of the
scheduling environment. To request information on available resources,
the '**-a**' flag should be used. By default, the **mshow** command
resource availability query only reports resources that are immediately
available. To request information on specific resources, the type of
resources required can be specified using the '**-w**' flag as in the
following example:

::

    > mshow -a -w taskmem=1500,duration=600
    ...

To view current and future resource availability, the '**future** flag
should be set as in the following example:

::

    > mshow -a -w taskmem=1500,duration=600 --flags=future
    ...

.. rubric:: Co-allocation Resources Queries
   :name: co-allocation-resources-queries

In many cases, a particular request will need simultaneous access to
resources of different types. The **mshow** command supports a
*co-allocation* request specified by using multiple '**-w**' arguments.
For example, to request 16 nodes with feature ``fastcpu`` and 2 nodes
with feature ``fastio``, the following request might be used:

::

    > mshow -a -w minprocs=16,duration=1:00:00,nodefeature=fastcpu -w minprocs=2,nodefeature=fastio,duration=1:00:00 --flags=future

    Partition     Procs  Nodes   StartOffset      Duration       StartDate
    ---------     -----  -----  ------------  ------------  --------------
    ALL              16      8      00:00:00       1:00:00  13:00:18_08/25  ReqID=0
    ALL               2      1      00:00:00       1:00:00  13:00:18_08/25  ReqID=1

The `mshow -a <../commands/mshow.html>`__ documentation contains a list
of the different resources that may be queried as well as examples on
using **mshow**.

.. rubric:: Using Transaction IDs
   :name: using-transaction-ids

By default, the **mshow** command reports simply when and where the
requested resources are available. However, when the '**tid**' flag is
specified, the **mshow** command returns both resource availability
information and a *handle* to these resources called a *Transaction ID*
as in the following example:

::

    > mshow -a -w minprocs=16,nodefeature=fastcpu,duration=2:00:00 --flags=future,tid

    Partition     Procs  Nodes   StartOffset      Duration       StartDate
    ---------     -----  -----  ------------  ------------  --------------
    ALL              16     16      00:00:00       2:00:00  13:00:18_08/25  TID=26 ReqID=0

In the preceding example, the returned transaction id (**TID**) may then
be used to reserve the available resources using the `mrsvctl -c
-R <../commands/mrsvctl.html>`__ command:

::

    > mrsvctl -c -R 26

    reservation system.1 successfully created

Any TID can be printed out using the `mschedctl -l
trans <../commands/mschedctl.html>`__ command:

::

    > mschedctl -l trans 26

    TID[26]  A1='node01'  A2='600'  A3='1093465728'  A4='ADVRES'  A5='fastio'

Where **A1** is the hostlist, **A2** is the duration, **A3** is the
starttime, **A4** are any flags, and **A5** are any features.

.. rubric:: Using Reservation Profiles
   :name: using-reservation-profiles

Reservation profiles (`RSVPROFILE <../a.fparameters.html#rsvprofile>`__)
stand as templates against which reservations can be created. They can
contain a hostlist, startime, endtime, duration, access-control list,
flags, triggers, variables, and most other attributes of an
Administrative Reservation. The following example illustrates how to
create a reservation with the exact same trigger-set.

.. raw:: html

   <div class="hscroll">

::

    -----
    # moab.cfg
    -----

    RSVPROFILE[test1] TRIGGER=Sets=$Var1.$Var2.$Var3.!Net,EType=start,AType=exec,Action=/tmp/host/triggers/Net.sh,Timeout=1:00:00
    RSVPROFILE[test1] TRIGGER=Requires=$Var1.$Var2.$Var3,Sets=$Var4.$Var5,EType=start,AType=exec,Action=/tmp/host/triggers/FS.sh+$Var1:$Var2:$Var3,Timeout=20:00
    RSVPROFILE[test1] TRIGGER=Requires=$Var1.$Var2.$Var3.$Var4.$Var5,Sets=!NOOSinit.OSinit,Etype=start,AType=exec,Action=/tmp/host/triggers/OS.sh+$Var1:$Var2:$Var3:$Var4:$Var5
    RSVPROFILE[test1] TRIGGER=Requires=NOOSini,AType=cancel,EType=start
    RSVPROFILE[test1] TRIGGER=EType=start,Requires=OSinit,AType=exec,Action=/tmp/host/triggers/success.sh
    ...
    -----


To create a reservation with this profile the `mrsvctl -c
-P <../commands/mrsvctl.html>`__ command is used:

::

    > mrsvctl -c -P test1


    reservation system.1 successfully created

.. rubric:: Using Reservation Groups
   :name: using-reservation-groups

Reservation groups are a way for Moab to tie reservations together. When
a reservation is created using multiple Transaction IDs, these
transactions and their resulting reservations are tied together into one
group.

::

    > mrsvctl -c -R 34,35,36

    reservation system.99 successfully created
    reservation system.100 successfully created
    reservation system.101 successfully created

In the preceding example, these three reservations would be tied
together into a single group. The `mdiag
-r <../commands/mdiag-reservations.html>`__ command can be used to see
which group a reservation belongs to. The `mrsvctl -q diag
-g <../commands/mrsvctl.html>`__ command can also be used to print out a
specific group of reservations. The `mrsvctl -c
-g <../commands/mrsvctl.html>`__ command can also be used to release a
group of reservations.

.. rubric:: See Also
   :name: see-also

-  `mshow <mshow.html>`__

