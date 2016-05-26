
.. rubric:: setqos
   :name: setqos

**(Set QoS)**
**Caution**: This command is deprecated. Use `mjobctl
-m <mjobctl.html#modify>`__ instead.

.. rubric:: Synopsis
   :name: synopsis

::

    setqos qosid jobid

.. rubric:: Overview
   :name: overview

Set Quality Of Service for a specified job.
This command allows users to change the QOS of their own jobs.

.. rubric:: Access
   :name: access

This command can be run by any user.
**Parameters**

+-----------+-------------+
| *JOBID*   | Job name.   |
+-----------+-------------+
| *QOSID*   | QOS name.   |
+-----------+-------------+

.. rubric:: Example 1
   :name: example-1

   This example sets the Quality Of Service to a value of
*high\_priority* for job *moab.3*.

::

    > setqos high_priority moab.3

    Job QOS Adjusted

