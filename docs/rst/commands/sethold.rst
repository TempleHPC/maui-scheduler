
.. rubric:: sethold
   :name: sethold

**(Set Hold)**
**Caution**: This command is deprecated. Use `mjobctl
-h <mjobctl.html#hold>`__ instead.

.. rubric:: Synopsis
   :name: synopsis

::

    sethold [-b] jobid [jobid...]

.. rubric:: Overview
   :name: overview

Set hold on specified job(s).
.. rubric:: Permissions
   :name: permissions

This command can be run by any Moab Scheduler Administrator.
**Parameters**

+---------+------------------------------+
| *JOB*   | Job number of job to hold.   |
+---------+------------------------------+

**Flags**

+------+---------------------------------------------------------------------------------------------------------------------------------------+
| -b   | Set a batch hold. Typically, only the scheduler places batch holds. This flag allows an administrator to manually set a batch hold.   |
+------+---------------------------------------------------------------------------------------------------------------------------------------+
| -h   | Help for this command.                                                                                                                |
+------+---------------------------------------------------------------------------------------------------------------------------------------+

.. rubric:: Example 1
   :name: example-1

   In this example, a batch hold is placed on job fr17n02.1072.0 and job
fr15n03.1017.0.

::

    > sethold -b fr17n02.1072.0 fr15n03.1017.0

    Batch Hold Placed on All Specified Jobs

