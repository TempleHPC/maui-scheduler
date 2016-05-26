
.. rubric:: changeparam
   :name: changeparam

**(Change Parameter)**
**Caution**: This command is deprecated. Use `mschedctl
-m <mschedctl.html#MODIFY>`__ instead.

.. rubric:: Synopsis
   :name: synopsis

::

    changeparam parameter value

.. rubric:: Overview
   :name: overview

   The **changeparam** command is used to dynamically change the value
of any parameter which can be specified in the ``moab.cfg`` file. The
changes take affect at the beginning of the next scheduling iteration.
They are not persistent, only lasting until Moab is shutdown.
   **changeparam** is a compact command of `mschedctl
-m <mschedctl.html#MODIFY>`__.

.. rubric:: Access
   :name: access

   This command can be run by a level 1 Moab administrator.
.. rubric:: Format
   :name: format

+------------+-----------------+--------------+---------------+----------------------------------------------------+---------------+
| **Flag**   | **Name**        | **Format**   | **Default**   | **Description**                                    | **Example**   |
+------------+-----------------+--------------+---------------+----------------------------------------------------+---------------+
|            | **PARAMETER**   | <STRING>     | [NONE]        | The name a Moab configuration parameter            |               |
+------------+-----------------+--------------+---------------+----------------------------------------------------+---------------+
|            | **VALUE**       | <STRING>     | [NONE]        | any valid value for `<PARAMETER> <#PARAMETER>`__   |               |
+------------+-----------------+--------------+---------------+----------------------------------------------------+---------------+

.. rubric:: Example 1
   :name: example-1

   Set Moab's LOGLEVEL to 6 for the current run:

changeparam
::

    > changeparam LOGLEVEL 6

    parameters changed

.. rubric:: Example 2
   :name: example-2

   Set Moab's ADMIN1 userlist to sys, mike and peter

changeparam
::

    > changeparam ADMIN1 sys mike peter

    parameters changed

