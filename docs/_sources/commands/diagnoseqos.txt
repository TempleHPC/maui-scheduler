

.. rubric:: diagnose -q
   :name: diagnose--q

**(Maui QOS diagnostic)**
.. rubric:: Synopsis:
   :name: synopsis

diagnose -q
.. rubric:: Overview:
   :name: overview

The 'diagnose -q' command is used to present information about the QOS
settings. Information includes weights, flags, and limits, as well as
which groups, accounts, and classes are assigned to it.

**Example:**

::

    > diagnose -Q
    QOS Status

    System QOS Settings:  QList: DEFAULT (Def: DEFAULT)  Flags: 0

    Name                * Priority QTWeight QTTarget XFWeight XFTarget     QFlags   JobFlags Limits

    DEFAULT                      1        1        3        1     5.00  PREEMPTEE     [NONE] [NONE]
      Groups:    sanjose
      Accounts:  it research
      Classes:  batch
    [ALL]                        0        0        0        0     0.00     [NONE]     [NONE] [NONE]
    urgent                   10000        1        1        1     7.00  PREEMPTOR     [NONE] [NONE]
      Groups:    dallas austin boston
      Accounts:  engineering it development
    low                        100        1        5        1     1.00  PREEMPTEE     [NONE] [NONE]
      Groups:    sanjose
      Accounts:  engineering marketing it development research
      Classes:  long bigmem
    high                         0        0        0        0     0.00     [NONE]     [NONE] [NONE]
      Groups:    dallas austin boston
      Accounts:  engineering it development research
      Classes:  fast
    1                            0        0        0        0     0.00     [NONE]     [NONE] [NONE]
    5                            0        0        0        0     0.00     [NONE]     [NONE] [NONE]

