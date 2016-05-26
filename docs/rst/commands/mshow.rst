
.. rubric:: mshow
   :name: mshow

**(Moab Show)**
**Synopsis**

::

    mshow [-a]  [-q jobqueue] 

**Overview**

The mshow command displays various diagnostic messages about the system
and job queues.
**Arguments**

Flag
Description
`-a <mshowa.html>`__
**`AVAILABLE RESOURCES <mshowa.html>`__**
-q [<QUEUENAME>]
**JOB QUEUE**
**Format**

**AVAILABLE RESOURCES**
Format:
Can be combined with --flags=[tid\|verbose\|future] --format=xml and/or
-w
Default:
---
Description:
Display available resources.
Example:


::

    > mshow -a -w user=john --flags=tid --format=xml


Show resources available to john in XML format with a transaction id.
See `mshow -a <mshowa.html>`__ for details
 
 
**JOB QUEUE**
Format:
---
Default:
---
Description:
Show the job queue.
Example:


::

    > mshow -q
        [information about all queues]
    ...


 
 
.. rubric:: See Also
   :name: see-also

-  `Moab Client Installation <../2.2installation.html#client>`__ -
   explains how to distribute this command to client nodes
-  `mshow -a <mshowa.html>`__ command to show available resources

