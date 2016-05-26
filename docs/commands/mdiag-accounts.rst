
.. rubric:: mdiag -a
   :name: mdiag--a

**(Moab Account Diagnostics)**
**Synopsis**

::

    mdiag -a [accountid]

**Overview**

   The **mdiag -a** command provides detailed information about the
`accounts <../3.5credoverview.html#account>`__ (aka projects) Moab is
currently tracking.  This command also allows an administrator to verify
correct throttling policies and access provided to and from other
credentials.
**Example 1**


::

    > mdiag -a
    evaluating acct information
    Name         Priority        Flags         QDef      QOSList*                 PartitionList Target  Limits

    engineering       100            -         high      high,urgent,low             [A][B]      30.00  MAXJOB=50,75  MAXPROC=400,500 
    marketing           1            -          low      low                         [A]          5.00  MAXJOB=100,110  MAXPS=54000,54500
    it                 10            -      DEFAULT      DEFAULT,high,urgent,low     [A]        100.00  MAXPROC=100,1250 MAXPS=12000,12500
      FSWEIGHT=1000
    development       100            -        high       high,urgent,low             [A][B]      30.00  MAXJOB=50,75 MAXNODE=100,120 
    research          100            -        high       DEFAULT,high,low            [A][B]      30.00  MAXNODE=400,500 MAXPS=900000,1000000
    DEFAULT             0            -           -       -                           -            0.00  - 


.. rubric:: See Also
   :name: see-also

-  `Account <../3.5credoverview.html#account>`__ credential

.. raw:: html

   <div class="navIcons bottomIcons">

|Home| |Up|


.. raw:: html

   <div class="docsearch">


