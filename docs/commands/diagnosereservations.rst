

.. rubric:: diagnose -r
   :name: diagnose--r

**(Maui Reservation Diagnostics)**
.. rubric:: Synopsis
   :name: synopsis

diagnose -r [reservationid]
.. rubric:: Overview
   :name: overview

The **diagnose -r** command allows administrators to look at detailed
reservation information. It provides the name, type, partition,
starttime and endtime, proc and node counts, as well as actual
utilization figures. It also provides detailed information about which
resources are being used, how many nodes, how much memory, swap, and
processors are being associated with each task. Administrators can also
view the Access Control Lists for each reservation as well as any flags
that may be active in the reservation.
.. rubric:: Example
   :name: example

::

    diagnose -r
    Diagnosing Reservations
    RsvID                      Type Par   StartTime     EndTime     Duration Node Task Proc
    -----                      ---- ---   ---------     -------     -------- ---- ---- ----
    engineer.0.1               User   A    -6:29:00    INFINITY     INFINITY    0    0    7
        Flags: STANDINGRSV IGNSTATE OWNERPREEMPT
        ACL:   CLASS==batch+:==long+:==fast+:==bigmem+ QOS==low-:==high+ JATTR==PREEMPTEE+
        CL:    RSV==engineer.0.1
        Task Resources: PROCS: [ALL]
        Attributes (HostExp='fr10n01 fr10n03 fr10n05 fr10n07 fr10n09 fr10n11 fr10n13 fr10n15')
        Active PH: 43.77/45.44 (96.31%)
        SRAttributes (TaskCount: 0  StartTime: 00:00:00  EndTime: 1:00:00:00  Days: ALL)

    research.0.2               User   A    -6:29:00    INFINITY     INFINITY    0    0    8
        Flags: STANDINGRSV IGNSTATE OWNERPREEMPT
        ACL:   CLASS==batch+:==long+:==fast+:==bigmem+ QOS==high+:==low- JATTR==PREEMPTEE+
        CL:    RSV==research.0.2
        Task Resources: PROCS: [ALL]
        Attributes (HostExp='fr3n01 fr3n03 fr3n05 fr3n07 fr3n07 fr3n09 fr3n11 fr3n13 fr3n15')
        Active PH: 51.60/51.93 (99.36%)
        SRAttributes (TaskCount: 0  StartTime: 00:00:00  EndTime: 1:00:00:00  Days: ALL)

    fast.0.3                   User   A    00:14:05     5:14:05      5:00:00    0    0   16
        Flags: STANDINGRSV IGNSTATE OWNERPREEMPT
        ACL:   CLASS==fast+ QOS==high+:==low+:==urgent+:==DEFAULT+ JATTR==PREEMPTEE+
        CL:    RSV==fast.0.3
        Task Resources: PROCS: [ALL]
        Attributes (HostExp='fr12n01 fr12n02 fr12n03 fr12n04 fr12n05 fr12n06 fr12n07 fr12n08 fr12n09 fr12n10 fr12n11 fr12n12 fr12n13 fr12n14 fr12n15 fr12n16')
        SRAttributes (TaskCount: 0  StartTime: 00:00:00  EndTime: 5:00:00  Days: Mon,Tue,Wed,Thu,Fri)

    fast.1.4                   User   A  1:00:14:05  1:05:14:05      5:00:00    0    0   16
        Flags: STANDINGRSV IGNSTATE OWNERPREEMPT
        ACL:   CLASS==fast+ QOS==high+:==low+:==urgent+:==DEFAULT+ JATTR==PREEMPTEE+
        CL:    RSV==fast.1.4
        Task Resources: PROCS: [ALL]
        Attributes (HostExp='fr12n01 fr12n02 fr12n03 fr12n04 fr12n05 fr12n06 fr12n07 fr12n08 fr12n09 fr12n10 fr12n11 fr12n12 fr12n13 fr12n14 fr12n15 fr12n16')
        SRAttributes (TaskCount: 0  StartTime: 00:00:00  EndTime: 5:00:00  Days: Mon,Tue,Wed,Thu,Fri)

    job2411                     Job   A   -00:01:00    00:06:30     00:07:30    0    0    6
        ACL:   JOB==job2411=
        CL:    JOB==job2411 USER==jimf GROUP==newyork ACCT==it CLASS==bigmem QOS==low JATTR==PREEMPTEE DURATION==00:07:30 PROC==6 PS==2700

    job1292                     Job   A    00:00:00    00:07:30     00:07:30    0    0    4
        ACL:   JOB==job1292=
        CL:    JOB==job1292 USER==jimf GROUP==newyork ACCT==it CLASS==batch QOS==DEFAULT JATTR==PREEMPTEE DURATION==00:07:30 PROC==4 PS==1800

