
.. rubric:: mdiag -q
   :name: mdiag--q

**(Moab QoS Diagnostics)**
**Synopsis**

::

    mdiag -q [qosid]

**Overview**

The '**mdiag -q**' command is used to present information about each QOS
maintained by Moab. The information presented includes QOS name,
membership, scheduling priority, weights and flags.
**Example 1: Standard QOS Diagnostics**


::

    > mdiag -q
    QOS Status

    System QOS Settings:  QList: DEFAULT (Def: DEFAULT)  Flags: 0

    Name                * Priority QTWeight QTTarget XFWeight XFTarget     QFlags   JobFlags Limits

    DEFAULT                      1        1        3        1     5.00  PREEMPTEE     [NONE] [NONE]
      Accounts:  it research
      Classes:  batch
    [ALL]                        0        0        0        0     0.00     [NONE]     [NONE] [NONE]
    high                      1000        1        2        1    10.00  PREEMPTOR     [NONE] [NONE]
      Accounts:  engineering it development research
      Classes:  fast
    urgent                   10000        1        1        1     7.00  PREEMPTOR     [NONE] [NONE]
      Accounts:  engineering it development
    low                        100        1        5        1     1.00  PREEMPTEE     [NONE] [NONE]
      Accounts:  engineering marketing it development research
      Classes:  long bigmem


