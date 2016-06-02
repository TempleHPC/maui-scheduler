
.. rubric:: releasehold
   :name: releasehold

**(Release Hold)**
**Caution**: This command is deprecated. Use `mjobctl
-u <mjobctl.html#unhold>`__ instead.

.. rubric:: Synopsis
   :name: synopsis

::

    releasehold [-a|-b] jobexp

.. rubric:: Overview
   :name: overview

Release hold on specified job(s).
This command allows you to release batch holds or all holds (system,
user, and batch) on specified jobs. Any number of jobs may be released
with this command.

.. rubric:: Access
   :name: access

By default, this command can be run by any Moab Scheduler Administrator.
**Parameters**

+----------+----------------------------------------+
| JOBEXP   | Job expression of job(s) to release.   |
+----------+----------------------------------------+

**Flags**

+-------------+--------------------------------------------------------------------------+
| -a   | Release all types of holds (user, system, batch) for specified job(s).   |
+-------------+--------------------------------------------------------------------------+
| -b   | Release batch hold from specified job(s).                                |
+-------------+--------------------------------------------------------------------------+
| -h          | Help for this command.                                                   |
+-------------+--------------------------------------------------------------------------+

.. rubric:: Example 1
   :name: example-1

releasehold -b
::

    > releasehold -b 6443

    batch hold released for job 6443

   In this example, a batch hold was released from this one job.

.. rubric:: Example 2
   :name: example-2

releasehold -a
::

    > releasehold -a "81[1-6]"

    holds modified for job 811
    holds modified for job 812
    holds modified for job 813
    holds modified for job 814
    holds modified for job 815
    holds modified for job 816

   In this example, all holds were released from the specified jobs.

.. rubric:: See Also
   :name: see-also

-  `sethold <sethold.html>`__
-  `mjobctl <mjobctl.html>`__

