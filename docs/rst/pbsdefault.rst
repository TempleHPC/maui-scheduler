.. rubric:: PBS Config - Default Queue Settings
   :name: pbs-config---default-queue-settings

|
| Contributed Suggestions:
| (Brian Haymore brian@chpcs.utah.edu)
| To set a default of one node and 15 minutes of walltime for a
  particular queue, issue the following:

| > qmgr
| Qmgr: set queue <QUEUENAME> resources\_default.nodect = 1
| Qmgr: set queue <QUEUENAME> resources\_default.walltime = 00:15:00
| Qmgr: quit

| To set system wide defaults, set the following:

> qmgr
Qmgr: set server resources\_default.nodect = 1
Qmgr: set server resources\_default.walltime = 00:15:00
Qmgr: quit
