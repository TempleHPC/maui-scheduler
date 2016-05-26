.. rubric:: PBS Integration Guide - RM Access Control
   :name: pbs-integration-guide---rm-access-control

.. rubric:: Server Configuration
   :name: server-configuration

In PBS qmgr,
set server managers += <MAUIADMIN>@\*.<YOURDOMAIN>
set server operators += <MAUIADMIN>@\*.<YOURDOMAIN>
i.e.,
::

    > qmgr
    Qmgr: set server managers += staff@*.ucsd.edu
    Qmgr: set operators += staff@*.ucsd.edu
    Qmgr: quit

NOTE: if different, you may want to replace "\*.<YOURDOMAIN>" with
"<MAUISERVERHOSTNAME>".
.. rubric:: Mom Configuration
   :name: mom-configuration

In the mom\_priv/config file on each node where pbs\_mom runs, set the
following:
$restricted \*.<YOURDOMAIN>
$clienthost <MAUISERVERHOSTNAME>
