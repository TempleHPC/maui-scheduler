
.. rubric:: mdiag -j
   :name: mdiag--j

**(Moab Job Diagnostics)**
**\ Synopsis**

::

    mdiag -j [jobid] [-t <partition>] [-v] [-w] [--flags=policy] [--format=xml]

**Overview**

The **mdiag -j** command provides detailed information about the state
of jobs Moab is currently tracking. This command also performs a large
number of sanity and state checks. The job configuration and status
information, as well as the results of the various checks, are presented
by this command. If the ``-v`` (verbose) flag is specified, additional
information about less common job attributes is displayed. If
``--flags=policy`` is specified, information about job templates is
displayed.

If used with the ``-t <partition>`` option on a running job, the only
thing **mdiag -j** shows is if the job is running on the specified
partition. If used on job that is not running, it shows if the job is
able to run on the specified partition.

The ``-w`` flag enables you to select only jobs associated with a given
credential (user, acct, class, group, qos). For example:


::

    mdiag -j -w user=david


Output from the preceding example displays only the user named David's
jobs.

**\ XML Output**

If XML output is requested (via the `--format=xml <#synopsis>`__
argument), XML based node information will be written to STDOUT in the
following format:


::

    <Data>
      <job ATTR="VALUE" ... > </job>
      ...
    </Data>


For information about legal attributes, refer to the `XML
Attributes <mjobctl.html#xml>`__ table.

.. rubric:: See Also
   :name: see-also

-  `checkjob <checkjob.html>`__
-  `mdiag <mdiag.html>`__

