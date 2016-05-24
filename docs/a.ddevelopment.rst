Appendix D: Structure Limits
############################

Maui is distributed in a configuration capable of supporting multiple
architectures and systems ranging from a few processors to several
thousand processors. However, in spite of its flexibility, it still
contains a number of static structures defined in header files. These
structures limit the default number of jobs, reservations, nodes, etc,
which Maui can handle and are set to values which provide a reasonable
compromise between capability and memory consumption for most sites.
However, many sites desire to increase some of these settings to extend
functionality, or decrease them to save consumed memory. The most common
parameters are listed below and can be adjusted by simply modifying the
appropriate **#define** and rebuilding Maui.

The documented limits have been well tested however caution is advised
that changes to some limits may result in undesirable side-affects.

+----------------------------+-------------------+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **Parameter**              | **Location**      | **Limit**   | **Description**                                                                                                                                                                 |
+----------------------------+-------------------+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **MMAX\_ATTR**             | moab.h            | 128         | total number of distinct node attributes (PBS node attributes/LL node features) which can be tracked                                                                            |
+----------------------------+-------------------+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **MMAX\_CLASS**            | moab.h            | 12          | total number of distinct job classes/queues available                                                                                                                           |
+----------------------------+-------------------+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **MMAX\_CLIENT**           | moab.h            | 4           | total number of simultaneous client connections allowed                                                                                                                         |
+----------------------------+-------------------+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **MMAX\_FSDEPTH**          | moab.h            | 24          | number of active fairshare windows                                                                                                                                              |
+----------------------------+-------------------+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **MMAX\_JOB**              | moab.h            | 4096        | maximum total number of simultaneous idle/active jobs allowed. **NOTE**: on some releases of Maui, **MAX\_MJOB** may also need to be set and synchronized with **MMAX\_JOB**.   |
+----------------------------+-------------------+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **MMAX\_NODE**             | moab.h            | 5120        | maximum number of compute nodes supported                                                                                                                                       |
+----------------------------+-------------------+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **MMAX\_NODE\_PER\_JOB**   | msched-common.h   | 1024        | maximum number of compute nodes which can be allocated to a single job                                                                                                          |
+----------------------------+-------------------+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **MAX\_MPAR**              | moab.h            | 4           | maximum number of partitions supported                                                                                                                                          |
+----------------------------+-------------------+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **MAX\_MQOS**              | moab.h            | 128         | total number of distinct QOS objects available to jobs                                                                                                                          |
+----------------------------+-------------------+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **MMAX\_RES\_DEPTH**       | moab.h            | 256         | total number of distinct reservations allowed per node                                                                                                                          |
+----------------------------+-------------------+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **MMAX\_SRES**             | moab.h            | 128         | total number of distinct standing reservations available                                                                                                                        |
+----------------------------+-------------------+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **MMAX\_TASK**             | moab.h            | 1560        | total number of tasks allowed per job                                                                                                                                           |
+----------------------------+-------------------+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

Maui currently possesses hooks to allow sites to create local algorithms
for handling site specific needs in several areas. The 'contrib'
directory contains a number of sample 'local' algorithms for various
purposes. The 'Local.c' module incorporates the algorithm of interest
into the main code. The following scheduling areas are currently handled
via the 'Local.c' hooks.

| **Local Job Attributes**
| **Local Node Allocation Policies**
| **Local Job Priorities**
| **Local Fairness Policies**

**Overview of Major Structures** (Under Construction)

| Nodes
| mnode\_t

| Jobs
| mjob\_t

| Reservations
| mres\_t

| Partitions
| mpar\_t

| QOS
| mqos\_t
