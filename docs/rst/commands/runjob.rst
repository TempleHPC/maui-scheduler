
.. rubric:: runjob
   :name: runjob

**(Run Job)**
**Caution**: This command is deprecated. Use `mjobctl
-x <mjobctl.html#execute>`__ instead.

.. rubric:: Synopsis
   :name: synopsis

::

    runjob [-c|-f|-n nodelist|-p partition|-s|-x] jobid

.. rubric:: Overview
   :name: overview

This command will attempt to immediately start the specified job.
runjob is a deprecated command, replaced by `mjobctl <mjobctl.html>`__

.. rubric:: Access
   :name: access

By default, this command can be run by any Moab administrator.
**Parameters**

+-------------+---------------------------+
| **JOBID**   | Name of the job to run.   |
+-------------+---------------------------+

**** 
+----------------------+-------------------------------------------------------------------------------------------------------------------+
| **Args**             | **Description**                                                                                                   |
+----------------------+-------------------------------------------------------------------------------------------------------------------+
| **-c**               | *Clear* job parameters from previous runs (used to clear PBS neednodes attribute after PBS job launch failure)    |
+----------------------+-------------------------------------------------------------------------------------------------------------------+
| **-f**               | Attempt to *force* the job to run, ignoring throttling policies                                                   |
+----------------------+-------------------------------------------------------------------------------------------------------------------+
| **-n <NODELIST>**    | Attempt to start the job using the specified *nodelist* where nodenames are comma or colon delimited              |
+----------------------+-------------------------------------------------------------------------------------------------------------------+
| **-p <PARTITION>**   | Attempt to start the job in the specified *partition*                                                             |
+----------------------+-------------------------------------------------------------------------------------------------------------------+
| **-s**               | Attempt to *suspend* the job                                                                                      |
+----------------------+-------------------------------------------------------------------------------------------------------------------+
| **-x**               | Attempt to force the job to run, ignoring throttling policies, QoS constraints, and reservations                  |
+----------------------+-------------------------------------------------------------------------------------------------------------------+

.. rubric:: Example
   :name: example

   This example attempts to run job cluster.231.

::

    > runjob cluster.231

    job cluster.231 successfully started

.. rubric:: See Also:
   :name: see-also

-  `mjobctl <mjobctl.html>`__
-  `canceljob <canceljob.html>`__ - cancel a job.
-  `checkjob <checkjob.html>`__ - show detailed status of a job.
-  `showq <showq.html>`__ - list queued jobs.

