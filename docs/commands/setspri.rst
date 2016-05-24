
.. rubric:: setspri
   :name: setspri

**(Set System Priorities)**
**Caution**: This command is deprecated. Use `mjobctl
-p <mjobctl.html#priority>`__ instead.

.. rubric:: Synopsis
   :name: synopsis

::

    setspri [-r] priority jobid

.. rubric:: Overview
   :name: overview

(This command is deprecated by the `mjobctl command <mjobctl.html>`__)
Set or remove absolute or relative system priorities for a specified
job.

This command allows you to set or remove a system priority level for a
specified job. Any job with a system priority level set is guaranteed a
higher priority than jobs without a system priority. Jobs with higher
system priority settings have priority over jobs with lower system
priority settings.

.. rubric:: Access
   :name: access

This command can be run by any Moab Scheduler Administrator.
**Parameters**

+--------------------------------------+--------------------------------------+
| *JOB*                                | Name of job.                         |
+--------------------------------------+--------------------------------------+
| *`PRIORITY <>`__*                    | System priority level. By default,   |
|                                      | this priority is an absolute         |
|                                      | priority overriding the policy       |
|                                      | generated priority value. Range is 0 |
|                                      | to clear, 1 for lowest, 1000 for     |
|                                      | highest. The given value is added    |
|                                      | onto the system priority (see 32-bit |
|                                      | and 64-bit values below), except for |
|                                      | a given value of zero. If the '-r'   |
|                                      | flag is specified, the system        |
|                                      | priority is relative, adding or      |
|                                      | subtracting the specified value from |
|                                      | the policy generated priority.       |
|                                      |                                      |
|                                      | If a relative priority is specified, |
|                                      | any value in the range +/-           |
|                                      | 1,000,000,000 is acceptable.         |
+--------------------------------------+--------------------------------------+

**Flags**

+------+----------------------------------------+
| -r   | Set relative system priority on job.   |
+------+----------------------------------------+

.. rubric:: Example 1
   :name: example-1

   In this example, a system priority of 10 is set for job orion.4752

::

    > setspri 10 orion.4752 

    job system priority adjusted

.. rubric:: Example 2
   :name: example-2

   In this example, system priority is cleared for job clusterB.1102

::

    > setspri 0 clusterB.1102

    job system priority adjusted

.. rubric:: Example 3
   :name: example-3

   In this example, the job's priority will be increased by 100000 over
the value determine by configured priority policy.

::

    > setspri -r 100000 job.00001

    job system priority adjusted

**Note**: This command is deprecated.  Use `mjobctl <mjobctl.html>`__
instead.

