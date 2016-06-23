.. rubric:: Maui-SGE Integration Guide
   :name: maui-sge-integration-guide

**Overview:**

| Maui can be used as an external scheduler for the Sun Grid Engine
  (SGE) resource management system (requires SGE v5.3 source
  distribution). In this configuration, SGE manages the job queue and
  the compute resources while Maui queries the SGE Server to obtain up
  to date job, node, and configuration information. Using this
  information, Maui directs SGE to manage jobs in accordance with
  specified Maui policies, priorities, and reservations.

**Steps:**

Maui drives SGE via the SGE scheduling API. To enable Maui scheduling,
the following steps must be taken:

**1)** **Install SGE**

`SGE Installation
HowTo <http://gridengine.sunsource.net/unbranded-source/browse/~checkout~/gridengine/doc/INSTALL>`__

`SGE Build
README <http://gridengine.sunsource.net/unbranded-source/browse/~checkout~/gridengine/source/README.BUILD>`__

--------------

**2)** **Install Maui**
| - **untar** Maui distribution file.
| - **cd** into the maui-<X> directory
| - run **./configure**
| |image0| The **configure** script will automatically setup Maui so
  that the user running configure will become the default *Primary Maui
  Administrator*, $MAUIADMIN. This can be changed by modifying the
  '**ADMIN <USERNAME>**' line in the **maui.cfg** file. The primary
  administrator is the first user listed after the
  **`ADMIN1 <a.fparameters.html#admin1>`__** parameter and is the ID
  under which the Maui daemon will run.

--------------

**3) Configure SGE**

-  Set default PE (i.e.,**qconf -ap default** and add line '**-pe
   default 1**' to file ``$SGE_ROOT/default/common/sge_request``, See
   SGE man page **sge\_pe.5** for more information)
-  Optional - Set default walltime for jobs (add line '**-l
   h\_rt=<limit>**' to file ``$SGE_ROOT/default/common/sge_request``)
-  disable default SGE scheduler (edit
   ``$SGE_ROOT/default/common/rcsge and comment out the line starting sge_schedd.``)
-  If not the same user, add Maui's *Primary Admin User* as SGE
   *operator* (use '**qconf -am <user>**')

|image1| All jobs submitted to Maui must be assigned a PE. Jobs without
an assigned PE will have a batch hold placed upon them.

--------------

**4) Configure Maui**
- specify SGE as the resource manager:

This should be taken care of by '**configure**', but if not, the
following parameter must be specified in the **maui.cfg** file:

**``RMCFG[base] TYPE=SGE``**

If you have a non-standard SGE configuration, you may need to specify
additional Maui parameters to point Maui to the right location:

| **``RMCFG[base] HOST=$SGESERVERHOST``**
| **``RMCFG[base] PORT=$SGESERVERPORT``**

(See the `Resource Manager Overview <13.1rmoverview.html>`__ for more
information)

--------------

**5) Start Daemons**

#. start SGE

   -  issue command '**$SGE\_ROOT/default/common/rcsge**' as user root

#. start Maui

   -  source ``$SGE_ROOT/default/common/settings.sh``
   -  issue command '**maui**' as *Primary Maui Admin* user

--------------

**Current Issues:**

**<N/A>**

--------------

**Trouble-shooting:**
**<N/A>**

**See also:**

`Maui Administrators Guide <mauiadmin.html>`__
`Maui Users Guide <mauiusers.html>`__
