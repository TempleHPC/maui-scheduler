
.. rubric:: mdiag -R
   :name: mdiag--r

**(Moab Resource Manager Diagnostics)**
**Synopsis**

::

     
    mdiag -R [-v] [-V job] [resourcemanagerid]

**Overview**

The 'mdiag -R' command is used to present information about configured
resource managers. The information presented includes name, host, port,
state, type, performance statistics and failure notifications.
**Example 1**


::

    > mdiag -R -v

    RM[base]  Type: PBS  State: Active  ResourceType: COMPUTE
      Version:            '1.2.0p6-snap.1124480497'
      Nodes Reported:     4
      Flags:              executionServer,noTaskOrdering,typeIsExplicit
      Partition:          base
      Event Management:   EPORT=15004
      Note:  SSS protocol enabled
      Submit Policy:      NODECENTRIC
      DefaultClass:       batch
      Variables:          DefaultApp=MPICHG2,GridNet=Infiniband
      RM Performance:     AvgTime=0.00s  MaxTime=1.03s  (1330 samples)

    RM[base] Failures:
      Mon May  3 09:15:16  clusterquery     'cannot get node info (rm is unavailable)'
      Mon May  3 10:25:46  workloadquery    'cannot get job info (request timed out)'

    RM[Boeing]  Type: NATIVE  State: Active  ResourceType: LICENSE
      Cluster Query URL:  file://$HOME/lic.dat
      Licenses Reported:  3 types (3 of 6 available)
      Partition:          SHARED
      License Stats:      Avg License Avail:   0.00  (438 iterations)
      Iteration Summary:  Idle: 0.00  Active: 100.00  Busy: 0.00
      RM Performance:     AvgTime=0.00s  MaxTime=0.00s  (877 samples)

    RM[GM]  Type: NATIVE  State: Active  ResourceType: COMPUTE
      FlushPeriod:        HOUR
      Charge URL:         ChargeURL=exec:///$HOME/tools/charge.pl
      Delete URL:         DeleteURL=exec:///$HOME/tools/delete.pl
      Quote URL:          QuoteURL=exec:///$HOME/tools/quote.pl
      Reserve URL:        ReserveURL=exec:///$HOME/tools/reserve.pl
      Create URL:         CreateURL=exec:///$HOME/tools/create.pl
      Cluster Query URL:  file:///tmp/gm.dat
      Nodes Reported:     2
      Partition:          GM
      RM Performance:     AvgTime=0.00s  MaxTime=0.00s  (877 samples)

    Note:  use 'mrmctl -f -r ' to clear stats/failures


