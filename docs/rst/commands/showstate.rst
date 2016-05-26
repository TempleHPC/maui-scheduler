
.. rubric:: showstate
   :name: showstate

**(Show State)**
**Synopsis**

::

    showstate

**Overview**

This command provides a summary of the state of the system. It displays
a list of all active jobs and a text-based map of the status of all
nodes and the jobs they are servicing. Basic diagnostic tests are also
performed and any problems found are reported.
**Access**

By default, this command can be run by any Moab Administrator.
**Example 1**


::

    > showstate

    cluster state summary for Wed Nov 23 12:00:21

        JobID              S      User    Group Procs   Remaining            StartTime
        ------------------ - --------- -------- ----- -----------  -------------------

    (A)      fr17n11.942.0 R     johns    staff    16    13:21:15      Nov 22 12:00:21
    (B)      fr17n11.942.0 S     johns    staff    32    13:07:11      Nov 22 12:00:21
    (C)      fr17n11.942.0 R     johns    staff     8    11:22:25      Nov 22 12:00:21
    (D)      fr17n11.942.0 S     johns    staff     8    10:43:43      Nov 22 12:01:21
    (E)      fr17n11.942.0 S     johns    staff     8     9:19:25      Nov 22 12:01:21
    (F)      fr17n11.942.0 R     johns    staff     8     9:01:16      Nov 22 12:01:21
    (G)      fr17n11.942.0 R     johns    staff     1     7:28:25      Nov 22 12:03:22
    (H)      fr17n11.942.0 R     johns    staff     1     3:05:17      Nov 22 12:04:22
    (I)      fr17n11.942.0 S     johns    staff    24     0:54:38      Nov 22 12:00:22

    Usage Summary:  9 Active Jobs  106 Active Nodes

                  [0][0][0][0][0][0][0][0][0][1][1][1][1][1][1][1]
                  [1][2][3][4][5][6][7][8][9][0][1][2][3][4][5][6]

    Frame      2: XXXXXXXXXXXXXXXXXXXXXXXX[ ][A][C][ ][A][C][C][A]
    Frame      3: [ ][ ][ ][ ][ ][ ][A][ ][I][ ][I][ ][ ][ ][ ][ ]
    Frame      4: [ ][I][ ][ ][ ][A][ ][I][ ][ ][ ][E][ ][I][ ][E]
    Frame      5: [F][ ][E][ ][ ][ ][F][F][F][I][ ][ ][E][ ][E][E]
    Frame      6: [ ][I][I][E][I][ ][I][I][ ][I][F][I][I][I][I][F]
    Frame      7: [ ]XXX[ ]XXX[ ]XXX[ ]XXX[b]XXX[ ]XXX[ ]XXX[#]XXX
    Frame      9: [ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][E][ ]
    Frame     11: [ ][ ][ ][ ][ ][ ][I][F][@][ ][A][I][ ][F][ ][A]
    Frame     12: [A][ ][ ][A][ ][ ][C][A][ ][C][A][A][ ][ ][ ][ ]
    Frame     13: [D]XXX[I]XXX[ ]XXX[ ]XXX[ ]XXX[ ]XXX[I]XXX[I]XXX
    Frame     14: [D]XXX[I]XXX[I]XXX[D]XXX[ ]XXX[H]XXX[I]XXX[ ]XXX
    Frame     15: [b]XXX[b]XXX[b]XXX[b]XXX[D]XXX[b]XXX[b]XXX[b]XXX
    Frame     16: [b]XXX[ ]XXX[b]XXX[ ]XXX[b]XXX[b]XXX[ ]XXX[b]XXX
    Frame     17: [ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ]
    Frame     21: [ ]XXX[b]XXX[b]XXX[ ]XXX[b]XXX[b]XXX[b]XXX[b]XXX
    Frame     22: [b]XXX[b]XXX[b]XXX[ ]XXX[b]XXX[ ]XXX[b]XXX[b]XXX
    Frame     27: [b]XXX[b]XXX[ ]XXX[b]XXX[b]XXX[b]XXX[b]XXX[b]XXX
    Frame     28: [G]XXX[ ]XXX[D]XXX[ ]XXX[D]XXX[D]XXX[D]XXX[ ]XXX
    Frame     29: [A][C][A][A][C][ ][A][C]XXXXXXXXXXXXXXXXXXXXXXXX

    Key:  XXX:Unknown [*]:Down w/Job [#]:Down [']:Idle w/Job [ ]:Idle [@]:Busy w/No Job [!]:Drained
    Key:  [a]:(Any lower case letter indicates an idle node that is assigned to a job)
     
    Check Memory on Node fr3n07
    Check Memory on Node fr4n06
    Check Memory on Node fr4n09


In this example, nine active jobs are running on the system. Each job
listed in the top of the output is associated with a letter. For
example, job fr17n11.942.0 is associated with the letter "A". This
letter can now be used to determine where the job is currently running.
By looking at the system "map," it can be found that job fr17n11.942.0
(job "A") is running on nodes fr2n10, fr2n13, fr2n16, fr3n06 ...

The key at the bottom of the system map can be used to determine unusual
node states. For example, fr7n15 is currently in the state down.

After the key, a series of warning messages may be displayed indicating
possible system problems. In this case, warning message indicate that
there are memory problems on three nodes, fr3n07, fr4n06, and fr4n09.
Also, warning messages indicate that job fr15n09.1097.0 is having
difficulty starting. Node fr11n08 is in state BUSY but has no job
assigned to it (it possibly has a runaway job running on it).

.. rubric:: See Also
   :name: see-also

-  `Moab Client Installation <../2.2installation.html#client>`__ -
   explains how to distribute this command to client nodes
-  `Specifying Node Rack/Slot
   Location <../12.1nodelocation.html#racks>`__

