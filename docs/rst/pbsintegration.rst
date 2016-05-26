.. rubric:: Maui - PBS Integration Guide
   :name: maui---pbs-integration-guide

--------------

.. rubric:: Overview
   :name: overview

Maui can be used as an external scheduler for the PBS resource
management system. In this configuration, PBS manages the job queue and
the compute resources while Maui queries the PBS Server and the PBS
MOM's to obtain up to date job and node information. Using this
information, Maui directs PBS to manage jobs in accordance with
specified Maui policies, priorities, and reservations.

.. rubric:: Steps
   :name: steps

Maui drives PBS via the PBS scheduling API. To enable Maui scheduling,
the following steps must be taken:

.. rubric:: 1. Install PBS
   :name: install-pbs

|image0| Keep track of the PBS target directory, **$PBSTARGDIR**.

--------------

.. rubric:: 2. Install Maui
   :name: install-maui

-  **untar** Maui distribution file.
-  **cd** into the maui-<VERSION> directory
-  run **./configure**
-  specify the PBS target directory when queried by configure (Maui
   requires a few PBS libraries and include files to enable the PBS
   scheduling interface)

|image1| If you have a non-standard PBS installation, You may need to
modify **src/Makefile** and change **PBSIP** and **PBSLP** values and
references as necessary for your local site configuration.

|image2| The **configure** script will automatically setup Maui so that
the user running configure will become the default *Primary Maui
Administrator*, $MAUIADMIN. This can be changed by modifying the
'**ADMIN <USERNAME>**' line in the **maui.cfg** file. The primary
administrator is the first user listed after the
**`ADMIN1 <a.fparameters.html#admin1>`__** parameter and is the ID under
which the Maui daemon will run.

|image3| Some Tru64 and IRIX systems have a local libnet library which
conflicts with PBS's libnet library. To resolve this, try setting PBSLIB
to '${PBSLIBDIR}/libnet.a -lpbs' in the Maui Makefile.

|image4| Maui is 64 bit compatible. If PBS is compiled in 64 bit mode,
Maui will likewise need to be compiled in this manner in order to
utilize the PBS scheduling API. (i.e., for IRIX compilers, add '-64' to
**OSCCFLAGS** and **OSLDFLAGS** variables in the Makefile).

--------------

.. rubric:: 3. Configure PBS
   :name: configure-pbs

|image5| Make sure $MAUIADMIN a PBS admin.
Maui communicates with both pbs\_server and pbs\_mom daemons. The
$MAUIADMIN should be authorized to talk to both PBS daemons.
(`suggestions <pbsaccess.html>`__)

|image6| For security purposes, sites may want to run Maui under a
non-root user id, the mom\_priv/config files must be world-readable and
contain the line '$restricted \*.<YOURDOMAIN> (i.e.,
'``$restricted *.uconn.edu``').

-  Set default PBS queue nodecount and walltime attributes.
   (`suggestions <pbsdefault.html>`__)
-  (OPTIONAL) Set PBS default queue (ie, in **qmgr** 'set system
   default\_queue = <QUEUENAME>)

|image7| PBS nodes can be configured as **time shared** **or** **space
shared** according to local needs.

|image8| Maui utilizes PBS's scheduling port to obtain real-time event
information from PBS regarding job and node transitions. Leaving the
default qmgr setting of '**``set server scheduling=True``**' will allow
Maui to receive and process this real-time information.

|image9| Do not start the TORQUE's ``pbs_sched`` daemon. This is the
default scheduler for TORQUE and Maui/Moabwill provide this service.

|image10| Maui's user interface port set using the parameter
**SCHEDCFG** or **SERVERPORT** is used for user-scheduler communication.
This port must be different from the PBS scheduler port used for
resource manager-scheduler communication.

|image11| PBS supports the concept of **virtual nodes**. Using this
feature, Maui can individually schedule processors on SMP nodes. The PBS
Administrator's Guide explains how to set up the
'$PBS\_HOME/server\_priv/nodes' file to enable this capability. (i.e.,
<NODENAME> np=<VIRTUAL NODE COUNT>).

--------------

.. rubric:: 4. Configure Maui
   :name: configure-maui

Next, specify PBS as the resource manager. This should be taken care of
by '**configure**', but if not, the following parameter must be
specified in the **maui.cfg** file:

``RMCFG[base]TYPE=PBS``
If a non-standard PBS configuration is being used, additional Maui
parameters may be required to point Maui to the right location:

``RMCFG[base] HOST=$PBSSERVERHOST PORT=$PBSSERVERPORT``
(See the `Resource Manager Overview <13.1rmoverview.html>`__ for more
information.)

.. rubric:: Known Issues
   :name: known-issues

**PBS features not supported by Maui:**

-  Maui 3.0 and earlier only supports the following node specifications:

   ::

       nodes=+<HostList>[:ppn=<X>][+<HostList>[:PPN=<X>]]...
       nodes=<NodeCount>[:PPN=<X>][:<FEATURE>][:<FEATURE>]...

-  Maui 3.2 supports basic scheduling of all PBS node specifications but
   will provide only limited optimization services for these jobs.
   |image12| Maui is by default very liberal in its interpretation of
   <NODECOUNT>:PPN=<X>. In its standard configuration, Maui interprets
   this as 'give the job <NODECOUNT>\*<X> tasks with AT LEAST <X> tasks
   per node'. Set the
   `JOBNODEMATCHPOLICY <a.fparameters.html#jobnodematchpolicy>`__
   parameter to **EXACTNODE** to have Maui support PBS's default
   allocation behavior of <NODECOUNT> nodes with exactly <X> tasks per
   node.

**Maui features not supported by PBS:**

-  PBS does not support the concept of a job QOS or other extended
   scheduling features by default. This can be fixed using the
   techniques described `here <13.3.1pbsrmextensions.html>`__. See the
   `RM Extensions Overview <13.3rmextensions.html>`__ for more
   information.
-  Earlier versions of PBS do not maintain job completion information.
   Thus, an external scheduler cannot determine if the job completed
   successfully or if internal PBS problems occurred preventing the job
   from being properly updated. This problem will not in any way affect
   proper scheduling of jobs but may, potentially, affect scheduler
   statistics. If your site is prone to frequent PBS *hangs*, you may
   want to set the Maui `PURGETIME <a.fparameters.html#purgetime>`__
   parameter to allow Maui to hold job information in memory for a
   period of time until PBS recovers.

--------------

.. rubric:: Trouble-shooting
   :name: trouble-shooting

Common Problems:

-  PBS versions prior to 2.4 hang when MOM's have troubles.
-  On TRU64 systems, the PBS 'libpbs' library does not properly export a
   number of symbols required by Maui. This can be worked around by
   modifying the Maui Makefile to link the PBS 'rm.o' object file
   directly into Maui.

**See also:**

`Maui Administrators Guide <mauiadmin.html>`__
`Maui Users Guide <mauiusers.html>`__
