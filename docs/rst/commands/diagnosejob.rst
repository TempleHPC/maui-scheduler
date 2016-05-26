

.. rubric:: diagnose -j (diagnose job)
   :name: diagnose--j-diagnose-job

| 
| **Overview:**

The 'diagnose -q' command is used to present information about user
records maintained by Maui. The information presented includes user
name, UID, scheduling priority, default job flags, default QOS level,
List of accessible QOS levels, and list of accessible partitions.

**Usage:**

diagnose -u [<USERNAME>]

**Example:**

``> diagnose -u``\ ````

| ``Diagnosing Users``
| `` Name UID Priority Flags DefaultQOS QOSList PartitionList``\ ````

| `` jacksond 160 2223 [NONE] 4 31:63 [NONE]``
| `` steve 345 0 [NONE] -1 [NONE] [NONE]``
| `` tom 346 0 [NONE] -1 [NONE] [NONE]``
| `` susam 347 0 [NONE] -1 [NONE] [NONE]``
| `` studnt01 351 10 [NONE] -1 [NONE] [NONE]``
| `` studnt04 354 10 [NONE] -1 [NONE] [NONE]``
| `` studnt05 355 10 [NONE] -1 [NONE] [NONE]``

Note that only users with have jobs which are currently queued or have
been queued since Maui was most recently started are listed. For user
statistics, see the `showstats <showstats.html>`__ command.

