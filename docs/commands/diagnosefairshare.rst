+--------------------------------------------------------------------------+
| .. rubric:: diagnose -f                                                  |
|    :name: diagnose--f                                                    |
|                                                                          |
| **Maui Fairshare Diagnostics**                                           |
+--------------------------------------------------------------------------+

.. rubric:: Synopsis
   :name: synopsis

::

    diagnose -f

|
| **Overview:**

| The 'diagnose -f' command is used to display 'at a glance' information
  about the fairshare configuration. The affect of this fairshare
  information is determined by the fairshare priority weights as
  described in the '`Job Prioritization
  Overview <../5.1.1priorityoverview.html>`__'.

**Examples:**

::

    > diagnose -f
    FairShare Information

    Depth: 8 intervals   Interval Length: 12:00:00   Decay Rate: 1.00

    FS Policy: [NONE]
    System FS Settings:  Target Usage: 0.00    Flags: 0

    FSInterval        %     Target
    FSWeight       ------- -------
    TotalUsage      100.00 -------

    GROUP
    -------------
    dallas            0.00  15.00
    sanjose           0.00  15.00
    seattle           0.00  15.00
    austin            0.00  15.00
    boston            0.00  15.00
    orlando           0.00  15.00
    newyork           0.00  15.00

    ACCT
    -------------
    marketing         0.00   5.00
    it                0.00   5.00
    development       0.00  30.00
    research          0.00  30.00

    QOS
    -------------
    urgent            0.00   5.00
    low               0.00   5.00
    high              0.00  50.00
    1                 0.00  50.00
    5                 0.00  50.00

    CLASS
    -------------
    batch             0.00  70.00
    long              0.00  10.00
    fast              0.00  10.00
    bigmem            0.00  10.00
