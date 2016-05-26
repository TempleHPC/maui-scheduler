
.. rubric:: mdiag -S
   :name: mdiag--s

**(Moab Scheduler Diagnostics)**
**Synopsis**

::

     
    mdiag -S [-v]

**Overview**

The '**mdiag -S**' command is used to present information about the
status of the scheduler and grid interface.
This command will report on the following aspects of scheduling:

-  General Scheduler Configuration

   -  Reports short and long term scheduler load
   -  Reports detected overflows of node, job, reservation, partition,
      and other scheduler object tables

-  High Availability

   -  Configuration
   -  Reports health of HA primary
   -  Reports health of HA backup

-  Scheduling Status

   -  Reports if scheduling is paused
   -  Reports if scheduling is stopped

-  System Reservation Status

   -  Reports if global system reservation is active

-  Message Profiling/Statistics Status

**Example 1**


::

    > mdiag -S

    Moab Server running on orion-1:43225  (Mode: NORMAL)
      Load(5m)  Sched: 12.27%  RMAction: 1.16%  RMQuery: 75.30%  User: 0.29%  Idle: 10.98%
      Load(24h) Sched: 10.14%  RMAction: 0.93%  RMQuery: 74.02%  User: 0.11%  Idle: 13.80%

      HA Fallback Server:  orion-2:43225  (Fallback is Ready)

      Note:  system reservation blocking all nodes

      Message:  profiling enabled (531 of 600 samples/5:00 interval)


