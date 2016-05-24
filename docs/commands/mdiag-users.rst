
.. rubric:: mdiag -u
   :name: mdiag--u

**(Moab User Diagnostics)**
**Synopsis**

::

     
    mdiag -u [userid]

**Overview**

The **mdiag -u** command is used to present information about user
records maintained by Moab. The information presented includes user
name, UID, scheduling priority, default job flags, default QOS level,
List of accessible QOS levels, and list of accessible partitions.
**Example 1**


::

    > mdiag -u
    evaluating user information
    Name         Priority        Flags         QDef      QOSList*        PartitionList Target  Limits
     
    jvella              0       [NONE]       [NONE]       [NONE]                [NONE]   0.00  [NONE]
      ALIST=Engineering
      Message:  profiling enabled (597 of 3000 samples/00:15:00 interval)
    [NONE]              0       [NONE]       [NONE]       [NONE]                [NONE]   0.00  [NONE]
    reynolds            0       [NONE]       [NONE]       [NONE]                [NONE]   0.00  [NONE]
      ALIST=Administration
      Message:  profiling enabled (597 of 3000 samples/00:15:00 interval)
    mshaw               0       [NONE]       [NONE]       [NONE]                [NONE]   0.00  [NONE]
      ALIST=Test
      Message:  profiling enabled (584 of 3000 samples/00:15:00 interval)
    kforbes             0       [NONE]       [NONE]       [NONE]                [NONE]   0.00  [NONE]
      ALIST=Shared
      Message:  profiling enabled (597 of 3000 samples/00:15:00 interval)
    gastor              0       [NONE]       [NONE]       [NONE]                [NONE]   0.00  [NONE]
      ALIST=Engineering
      Message:  profiling enabled (597 of 3000 samples/00:15:00 interval)


Note that only users which have jobs which are currently queued or have
been queued since Moab was most recently started are listed.

.. rubric:: See Also
   :name: see-also

-  `showstats <showstats.html>`__ command (display user statistics)

