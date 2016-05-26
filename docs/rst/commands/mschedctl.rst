
.. rubric:: mschedctl
   :name: mschedctl

**(Moab Scheduler Control)**
**Synopsis**

::

    mschedctl -A '<MESSAGE>'
    mschedctl -c message messagestring [-o type:val]
    mschedctl -c trigger triggerid -o type:val
    mschedctl -d trigger triggerid
    mschedctl -d vpc:vpcid
    mschedctl -d message:index

    mschedctl -f {all|estimates|fairshare|usage}

    mschedctl -k
    mschedctl -l {config|message|trigger|trans|vpc|vpcprofile} [--flags=verbose] [--xml]
    mschedctl -L [LOGLEVEL]
    mschedctl -m config string [-e] [--flags=persistent]
    mschedctl -m trigger triggerid attr=val[,attr=val...]
    mschedctl -m vpc vpcid attr=val[,attr=val...]

    mschedctl -n
    mschedctl -p
    mschedctl -r [resumetime]
    mschedctl -R
    mschedctl -s [STOPITERATION]
    mschedctl -S [STEPITERATION]

**Overview**

The **mschedctl** command controls various aspects of scheduling
behavior. It is used to manage scheduling activity, shutdown the
scheduler, and create resource trace files.  It can also evaluate,
modify, and create parameters, triggers, and messages.

+----------+------------------------------------------------------------------------------------------------------------------------------------------+
| |Note|   | With many flags, the '--msg=<MSG>' option can be specified to annotate the action in the `event log <../14.2logging.html#logevent>`__.   |
+----------+------------------------------------------------------------------------------------------------------------------------------------------+

**Format**

-A — \ **ANNOTATE**
Format:
<STRING>
Default:
---
Description:
Report the specified parameter modification to the event log and
annotate it with the specified message.
Example:


::

    > mschedctl --flags=pers -A 'increase logging' -m 'LOGLEVEL 6'  


Adjust the LOGLEVEL parameter and record an associated message.
 
 
-c — \ **CREATE**
Format:
One of:

-   **message** <STRING> [-o <TYPE>:<VAL>]
-   **trigger** `<TRIGSPEC> <../19.1triggers.html>`__ -o
   <OBJECTTYPE>:<OBJECTID>
-   **vpc** [-a <ATTR>=<VAL>]...
    
   where <ATTR> is one of **account**, **duration**, **messages**,
   **profile**, **reqresources**, **resources**, **rsvprofile**,
   **starttime**, **user**, or **variables**

Default:
---
Description:
Create a message or trigger and attach it to the specified object, or
create a Virtual Private Cluster (VPC). To create a trigger on a default
object, use the Moab configuration file (moab.cfg) rather than the
**mschedctl** command.
Example:


::

    mschedctl -c message tell the admin to be nice


Create a message on the system table.


::

    mschedctl -c trigger EType=start,AType=exec,Action="/tmp/email $OWNER $TIME" -o rsv:system.1


Create a trigger linked to system.1

+----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| |Note|   | Creating triggers on default objects via ``mschedctl -c trigger`` does not propagate the triggers to individual objects. To propagate triggers to all objects, the triggers must be created within the moab.cfg file; for example: ``NODECFG[DEFAULT] TRIGGER``.   |
+----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+


::

    mschedctl -c vpc -a resources=6,7,8 -a profile=packageA


Create a vpc using TID's 6, 7, and 8 and based on profile packageA

+----------+--------------------------------------------------------------------------------------+
| |Note|   | VPC commands (such as mschedctl -c vpc) are only enabled on hosting center builds.   |
+----------+--------------------------------------------------------------------------------------+

Additional VPC attributes: `-a <#example2>`__
 
 
-d — \ **DESTROY**
Format:
One of:

-  **trigger** <TRIGID>
-  **message:<INDEX>**
-  **vpc:<VPCID>**

Default:
---
Description:
Delete a trigger, message, or VPC.
Example:


::

    mschedctl -d trigger 3


Delete trigger 3.


::

    mschedctl -d message:5


Delete message with index 5.


::

    mschedctl -d vpc:vpc.5


Delete vpc.5.


::

    mschedctl -d vpc:ALL


Delete all VPCs.
 
 
-f — \ **FLUSH**
Format:
{all\|estimates\|fairshare\|usage}
Default:
---
Description:
Flush (clear out) specified statistics
Example:


::

    mschedctl -f usage


Flush usage statistics.
 
 
-k — \ **KILL**
Format:
---
Default:
---
Description:
Stop scheduling and exit the scheduler
Example:


mschedctl -k


Kill the scheduler.
 
 
-l — \ **LIST**
Format:
{**config** \| **gres** \| **message** \| **trans** \| **trigger** \|
**vpc** \| **vpcprofile**} [--flags=verbose] [--xml]
+----------+-----------------------------------------------------------------------------------------------------------------------+
| |Note|   | Using the ``--xml`` argument with the **trans** option returns XML that states if the queuried TID is valid or not.   |
+----------+-----------------------------------------------------------------------------------------------------------------------+

Default:
**config**
Description:
List the generic resources, scheduler configuration, system messages,
triggers, transactions, `virtual private clusters <../20.0vpc.html>`__
or VPC profiles.
Example:


::

    mschedctl -l config


List system parameters.


::

    mschedctl -l gres


List all configured generic resources.


::

    mschedctl -l trans 1


List transaction id 1.


::

    mschedctl -l trigger


List triggers.


::

    mschedctl -l vpc


List VPCs.


::

    mschedctl -l vpc:vpc.1


List VPC vpc.1.
 
 
-L — \ **LOG**
Format:
<INTEGER>
Default:
**7**
Description:
Create a temporary log file with the specified loglevel.
Example:


::

    > mschedctl -L 7  


Create temporary log file with naming convention
'<logfile>.YYYYMMDDHHMMSS'.
 
 
-m — \ **MODIFY**
Format:
One of:

-  **config** [<STRING>]
   [-e]
   [--flags=pers]
   <STRING> is any string which would be acceptable in moab.cfg>

   -  If no string is specified, <STRING> is read from STDIN.
   -  If '-e' is specified, the configuration string will be evaluated
      for correctness but no configuration changes will take place.  Any
      issues with the provided string will be reported to STDERR.
   -  If **--flags=persistent** is specified, changes will be made
      persistent by changing in memory configuration and modifying
      moab.cfg.

-  **trigger**:<TRIGID> <ATTR>=<VAL>
    
   where <ATTR> is one of **action**, **atype**, **etype**,
   **iscomplete**, **oid**, **otype**, **offset**, or **threshold**
-  **vpc**:<VPCID> <ATTR>=<VAL>
    
   where <ATTR> is one of **variables**, or
   <**user**\ \|\ **group**\ \|\ **owner**\ \|\ **qos**\ \|\ **account**>

Default:
---
Description:
Modify a system parameter, trigger, or VPC.
Example:


::

    > mschedctl -m config LOGLEVEL 9


Change the system loglevel to 9.


::

    > mschedctl -m trigger:2 AType=exec,Offset=200,OID=system.1


Change aspects of trigger 2.


::

    > mschedctl -m vpc:packageA.1 variables=blue=dog


Change aspects of vpc packageA.1.


::

    > mschedctl -m vpc:vpc.10 user=craig
              
    vpc USER set to craig


Changes the user of vpc.10 to 'craig'
 
 
-n — \ **NODE TRACE**
Format:
---
Default:
---
Description:
Output a `Resource Trace File <../16.3.2resourcetrace.html>`__ to
STDOUT.
Example:


::

    > mschedctl -n > /tmp/node.trace


 
 
-p — \ **PAUSE**
Format:
---
Default:
---
Description:
Disable scheduling but allow the scheduler to update its cluster and
workload state information.
Example:


::

    > mschedctl -p


 
 
-R — \ **RECYCLE**
Format:
---
Default:
---
Description:
Recycle scheduler immediately (shut it down and restart it using the
original execution environment and command line arguments).
Example:


::

    > mschedctl -R


Recycle scheduler immediately.

+----------+-------------------------------------------------------------+
| |Note|   | To restart Moab with its last known scheduler state, use:   |
|          | ``mschedctl -R savestate``                                  |
+----------+-------------------------------------------------------------+

 
 
-r — \ **RESUME**
Format:
<INTEGER>
Default:
0
Description:
Resume scheduling at the specified time (or immediately if none is
specified).
Example:


::

    > mschedctl -r


Resume scheduling immediately.
 
 
-s — \ **STOP**
Format:
<INTEGER>
Default:
0
Description:
Suspend/stop scheduling at specified iteration (or at the end of the
current iteration if none is specified). If the letter 'I' follows
<ITERATION>, Moab till not process client requests until this iteration
is reached.
Example:


::

    > mschedctl -s 100I


Stop scheduling at iteration 100 and ignore all client requests until
then.
 
 
-S — \ **STEP**
Format:
<INTEGER>
Default:
0
Description:
Step the specified number of iterations (or to the next iteration if
none is specified) and suspend scheduling If the letter 'I' follows
<ITERATION>, Moab will not process client requests until this iteration
is reached.
Example:


::

    > mschedctl -S


Step to the next iteration and stop scheduling.
**Example 1**

Shutting down the Scheduler:


::

    > mschedctl -k

    scheduler will be shutdown immediately



**Example 2**

Creating a `virtual private
cluster <../20.2vpccommands.html#creating>`__:


::

    > mschedctl -c vpc -a resources=14332 -a variables=os=rhel3

    vpc.98


.. rubric:: See Also
   :name: see-also

-  `Moab Client Installation <../2.2installation.html#client>`__ -
   explains how to distribute this command to client nodes

