**NOTE:**
For all Maui 3.0 versions, throttling policies must be specified using
the old style convention requiring two or more parameters. The first
parameter, **\*POLICY**, indicates whether the policy is enabled. The
**\*COUNT** parameter specifies the policy hard limit. The **\*SCOUNT**
parameter specifies the optional policy soft limit. For example, In Maui
3.2, you might limit the number of jobs per user using the statement
'``USERCFG[DEFAULT] MAXJOB=4,6``'. In Maui 3.0, you would do the same
thing by specifying the following:

| ------
| ``MAXJOBPERUSERPOLICY ON``
| ``MAXJOBPERUSERCOUNT 4``
| ``SMAXJOBPERUSERCOUNT 6``
| **------**

**** The following translation must be used to specify policy **X** for
credential **Y**

| Maui 3.2
| ----
| ``USERCFG<Y> <X>=<SOFTLIMIT>[,<HARDLIMIT>]``
| ----

| Maui 3.2
| ----
| ``<X>PER<Y>POLICY ON``
| ``<X>PER<Y>COUNT <HARDLIMIT>``
| ``S<X>PER<Y>COUNT <SOFTLIMIT>``
| ----

If you have any questions, please send a note to us at
`help <mailto:brian@chpc.utah.edu>`__
