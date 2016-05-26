
.. rubric:: canceljob
   :name: canceljob

**(Cancel Job)**
**Caution**: This command is deprecated. Use `mjobctl
-c <mjobctl.html#cancel>`__ instead.

.. rubric:: Synopsis
   :name: synopsis

::

    canceljob jobid [jobid]...

.. rubric:: Overview
   :name: overview

The ``canceljob`` command is used to selectively cancel the specified
job(s) (active, idle, or non-queued) from the queue.
.. rubric:: Access
   :name: access

This command can be run by any Moab Administrator and by the owner of
the job (see `ADMINCFG <../a.fparameters.html#admincfg>`__).
+------------+-----------------------+--------------+---------------+-------------------------------------------------------+-------------------------------+
| **Flag**   | **Name**              | **Format**   | **Default**   | **Description**                                       | **Example**                   |
+------------+-----------------------+--------------+---------------+-------------------------------------------------------+-------------------------------+
| -h         | \ **HELP**     |              | N/A           | Display usage information                             | ``> canceljob -h``            |
+------------+-----------------------+--------------+---------------+-------------------------------------------------------+-------------------------------+
|            | \ **JOB ID**   | <STRING>     | ---           | a jobid, a job expression, or the keyword '**ALL**'   | ``> canceljob 13001 13003``   |
+------------+-----------------------+--------------+---------------+-------------------------------------------------------+-------------------------------+

.. rubric:: Example 1
   :name: example-1

  Cancel job 6397

::

    > canceljob 6397

