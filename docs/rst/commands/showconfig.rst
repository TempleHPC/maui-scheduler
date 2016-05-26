
.. rubric:: showconfig
   :name: showconfig

**(Show Configuration)**
**Caution**: This command is deprecated. Use `mschedctl
-l <mschedctl.html#LIST>`__ instead.

.. rubric:: Synopsis
   :name: synopsis

::

    showconfig [-v]

.. rubric:: Overview
   :name: overview

View the current configurable parameters of the Moab Scheduler.
   The showconfig command shows the current scheduler version and the
settings of all 'in memory' parameters.  These parameters are set via
internal defaults, command line arguments, environment variable
settings, parameters in the moab.cfg file, and via the `mschedctl
-m <mschedctl.html>`__ command.  Because of the many sources of
configuration settings, the output may differ from the contents of the
moab.cfg file.  The output is such that it can be saved and used as the
contents of the moab.cfg file if desired.

.. rubric:: Access
   :name: access

This command can be run by a level 1, 2, or 3 Moab administrator.
**Flags**

+------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| -h   | Help for this command.                                                                                                                                                                                                                                     |
+------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| -v   | This optional flag turns on verbose mode, which shows all possible Moab Scheduler parameters and their current settings. If this flag is not used, this command operates in context-sensitive terse mode, which shows only relevant parameter settings.    |
+------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

.. rubric:: Example 1
   :name: example-1

showconfig
::

    > showconfig

    # moab scheduler version 4.2.4 (PID: 11080)

    BACKFILLPOLICY                  FIRSTFIT
    BACKFILLMETRIC                  NODES

    ALLOCATIONPOLICY                MINRESOURCE
    RESERVATIONPOLICY               CURRENTHIGHEST
    ...

**IMPORTANT Note**:  The showconfig flag without the '-v' flag does not
show the settings of all parameters.  It does show all major parameters
and all parameters which are in effect and have been set to non-default
values.  However, it hides other rarely used parameters and those which
currently have no effect or are set to default values.  To show the
settings of all parameters, use the '-v' (verbose) flag.  This will
provide an extended output.  This output is often best used in
conjunction with the 'grep' command as the output can be voluminous.

.. rubric:: See Also:
   :name: see-also

-  Use the ``mschedctl -m`` command to change the various Moab Scheduler
   parameters.
-  See the `Parameters <../a.fparameters.html>`__ document for details
   about configurable parameters.

