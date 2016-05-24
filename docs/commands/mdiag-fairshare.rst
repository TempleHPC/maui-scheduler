
.. rubric:: mdiag -f
   :name: mdiag--f

**(Moab Fairshare Diagnostics)**
**Synopsis**

::

    mdiag -f [-o user|group|acct|qos|class] [--flags=relative] [-w par=<PARTITIONID>]

**Overview:**

The **mdiag -f** command is used to display *at a glance* information
about the fairshare configuration and historic resource utilization. The
fairshare usage may impact job prioritization, job eligibility, or both
based on the credential **FSTARGET** and **FSCAP** attributes and by the
fairshare priority weights as described in the `Job Prioritization
Overview <../5.1jobprioritization.html>`__. The information presented by
this command includes fairshare configuration and credential fairshare
usage over time.
The command hides information about credentials which have no fairshare
target and no fairshare cap.

If an object type (<OTYPE>) is specified, then only information for that
credential type (**user**, **group**, **acct**, **class**, or **qos**)
will be displayed. If the **relative** flag is set, then per user
fairshare usage will be displayed relative to each non-user credential
(see Example 2 below). **Note**: Relative output is only displayed for
credentials which have user mappings. For example, if there is no
association between classes and users, no relative *per user* fairshare
usage class breakdown will be provided.

**Example 1**

Standard Fairshare Output


::

    > mdiag -f

    FairShare Information

    Depth: 6 intervals   Interval Length: 00:20:00   Decay Rate: 0.50

    FS Policy: SDEDICATEDPES
    System FS Settings:  Target Usage: 0.00    Flags: 0

    FSInterval        %     Target       0       1       2       3       4       5
    FSWeight       ------- -------  1.0000  0.5000  0.2500  0.1250  0.0625  0.0312
    TotalUsage      100.00 -------    85.3   476.1   478.9   478.5   475.5   482.8

    USER
    -------------
    mattp             2.51 -------    2.20    2.69    2.21    2.65    2.65    3.01
    jsmith           12.82 -------   12.66   15.36   10.96    8.74    8.15   13.85
    kyliem            3.44 -------    3.93    2.78    4.36    3.11    3.94    4.25
    tgh               4.94 -------    4.44    5.12    5.52    3.95    4.66    4.76
    walex             1.51 -------    3.14    1.15    1.05    1.61    1.22    1.60
    jimf              4.73 -------    4.67    4.31    5.67    4.49    4.93    4.92
    poy               4.64 -------    4.43    4.61    4.58    4.76    5.36    4.90
    mjackson          0.66 -------    0.35    0.78    0.67    0.77    0.55    0.43
    tfw              17.44 -------   16.45   15.59   19.93   19.72   21.38   15.68
    gjohn             2.81 -------    1.66    3.00    3.16    3.06    2.41    3.33
    ljill            10.85 -------   18.09    7.23   13.28    9.24   14.76    6.67
    kbill            11.10 -------    7.31   14.94    4.70   15.49    5.42   16.61
    stevei            1.58 -------    1.41    1.34    2.09    0.75    3.30    2.15
    gms               1.54 -------    1.15    1.74    1.63    1.40    1.38    0.90
    patw              5.11 -------    5.22    5.11    4.85    5.20    5.28    5.78
    wer               6.65 -------    5.04    7.03    7.52    6.80    6.43    2.83
    anna              1.97 -------    2.29    1.68    2.27    1.80    2.37    2.17
    susieb            5.69 -------    5.58    5.55    5.57    6.48    5.83    6.16

    GROUP
    -------------
    dallas           13.25  15.00    14.61   12.41   13.19   13.29   15.37   15.09
    sanjose*          8.86  15.00     6.54    9.55    9.81    8.97    8.35    4.16
    seattle          10.05  15.00     9.66   10.23   10.37    9.15    9.94   10.54
    austin*          30.26  15.00    29.10   30.95   30.89   28.45   29.53   29.54
    boston*           3.44  15.00     3.93    2.78    4.36    3.11    3.94    4.25
    orlando*         26.59  15.00    29.83   26.77   22.56   29.49   25.53   28.18
    newyork*          7.54  15.00     6.33    7.31    8.83    7.54    7.34    8.24

    ACCT
    -------------
    engineering      31.76  30.00    32.25   32.10   31.94   30.07   30.74   31.14
    marketing         8.86   5.00     6.54    9.55    9.81    8.97    8.35    4.16
    it                9.12   5.00     7.74    8.65   10.92    8.29   10.64   10.40
    development*     24.86  30.00    24.15   24.76   25.00   24.84   26.15   26.78
    research         25.40  30.00    29.32   24.94   22.33   27.84   24.11   27.53

    QOS
    -------------
    DEFAULT*          0.00  50.00  ------- ------- ------- ------- ------- -------
    high*            83.69  90.00    86.76   83.20   81.71   84.35   83.19   88.02
    urgent            0.00   5.00  ------- ------- ------- ------- ------- -------
    low*             12.00   5.00     7.34   12.70   14.02   12.51   12.86    7.48

    CLASS
    -------------
    batch*           51.69  70.00    53.87   52.01   50.80   50.38   48.67   52.65
    long*            18.75  10.00    16.54   18.36   20.89   18.36   21.53   16.28
    fast*            15.29  10.00    18.41   14.98   12.58   16.80   15.15   18.21
    bigmem           14.27  10.00    11.17   14.65   15.73   14.46   14.65   12.87


**Example 2**

Grouping User Output by Account


mdiag -f -o acct --flags=relative
::

    > mdiag -f -o acct --flags=relative

    FairShare Information

    Depth: 6 intervals   Interval Length: 00:20:00   Decay Rate: 0.50

    FS Policy: SDEDICATEDPES
    System FS Settings:  Target Usage: 0.00    Flags: 0

    FSInterval        %     Target       0       1       2       3       4       5
    FSWeight       ------- -------  1.0000  0.5000  0.2500  0.1250  0.0625  0.0312
    TotalUsage      100.00 -------    23.8   476.1   478.9   478.5   475.5   482.8

    ACCOUNT
    -------------
    dallas           13.12  15.00    15.42   12.41   13.19   13.29   15.37   15.09
      mattp          19.47 -------   15.00   21.66   16.75   19.93   17.26   19.95
      walex           9.93 -------   20.91    9.28    7.97   12.14    7.91   10.59
      stevei         12.19 -------    9.09   10.78   15.85    5.64   21.46   14.28
      anna           14.77 -------   16.36   13.54   17.18   13.55   15.44   14.37
      susieb         43.64 -------   38.64   44.74   42.25   48.74   37.92   40.81
    sanjose*          9.26  15.00     8.69    9.55    9.81    8.97    8.35    4.16
      mjackson        7.71 -------    6.45    8.14    6.81    8.62    6.54   10.29
      gms            17.61 -------   21.77   18.25   16.57   15.58   16.51   21.74
      wer            74.68 -------   71.77   73.61   76.62   75.80   76.95   67.97
    seattle          10.12  15.00    10.16   10.23   10.37    9.15    9.94   10.54
      tgh            49.56 -------   46.21   50.05   53.26   43.14   46.91   45.13
      patw           50.44 -------   53.79   49.95   46.74   56.86   53.09   54.87
    austin*          30.23  15.00    25.58   30.95   30.89   28.45   29.53   29.54
      jsmith         42.44 -------   48.77   49.62   35.47   30.70   27.59   46.90
      tfw            57.56 -------   51.23   50.38   64.53   69.30   72.41   53.10
    boston*           3.38  15.00     3.78    2.78    4.36    3.11    3.94    4.25
      kyliem        100.00 -------  100.00  100.00  100.00  100.00  100.00  100.00
    orlando*         26.20  15.00    30.13   26.77   22.56   29.49   25.53   28.18
      poy            17.90 -------   16.28   17.22   20.30   16.15   20.98   17.39
      ljill          37.85 -------   58.60   26.99   58.87   31.33   57.79   23.67
      kbill          44.25 -------   25.12   55.79   20.83   52.52   21.23   58.94
    newyork*          7.69  15.00     6.24    7.31    8.83    7.54    7.34    8.24
      jimf           61.42 -------   69.66   58.94   64.20   59.46   67.21   59.64
      gjohn          38.58 -------   30.34   41.06   35.80   40.54   32.79   40.36


.. rubric:: See Also:
   :name: see-also

-  `Fairshare Overview <../6.3fairshare.html>`__

