**Throttling Policies (Maui 2.3.0 - 3.0.6)**
Maui's early style throttling policies are controlled via a large number
of independent parameters. Throttling policies control and constrain
*instantaneous* resource usage. They would, for example, allow a site to
limit a user to running only 6 jobs at any given time or prevent a group
from utilizing more than 40 total processors at any given time. They
DONOT control historical usage. This is handled using Maui's
`Fairshare <6.3fairshare.html>`__ facility. Also, unlike Maui 3.0.7 and
higher, early style throttling did not allow credential specific limits.

| Subject to the above constraints, Maui's early throttling policy
  facility is still a very useful tool in establishing *fair*
| resource usage and may be used in conjunction with Fairshare,
  `QOS <7.3qos.html>`__, and `Priority <5.1jobprioritization.html>`__
  features to establish significant control over cluster behavior. The
  table below lists the parameters associated with Maui's early style
  throttling.

| **NOTE**: In all cases, three parameters are grouped together. The
  first, '**MAX\*POLICY**' must be set to **ON** in order for the policy
  to be enforced. The second, '**MAX\*COUNT**', constrains the '*hard
  limit*' which the scheduler must never violate under any conditions.
  The third, '**SMAX\*COUNT**', is called a '*soft limit*' and if
  specified, will set a lower, more constraining limit which the
  scheduler should never violate unless no other jobs are available.

+--------------------------+--------------------------+--------------------------+
| **Parameter**            | **Details**              | **Example**              |
+--------------------------+--------------------------+--------------------------+
| **MAXJOBPERUSERPOLICY**  | limits the maximum total | ``MAXJOBPERUSERPOLICY ON |
| **MAXJOBPERUSERCOUNT**   | number of jobs any given | ``                       |
| **SMAXJOBPERUSERCOUNT**  | user may have active     | ``MAXJOBPERUSERCOUNT  4` |
|                          | (running) simultaneously | `                        |
|                          |                          | (allow each user to run  |
|                          |                          | up to 4 jobs             |
|                          |                          | simultaneously)          |
+--------------------------+--------------------------+--------------------------+
| **MAXPROCPERUSERPOLICY** | limits the maximum total | ``MAXPROCPERUSERPOLICY O |
| **MAXPROCPERUSERCOUNT**  | number of processors any | N``                      |
| **SMAXPROCPERUSERCOUNT** | given user may have      | ``MAXPROCPERUSERCOUNT  3 |
|                          | active (allocated to     | 2``                      |
|                          | running jobs)            | (allow each user to      |
|                          | simultaneously           | utilize up to 32         |
|                          |                          | processors               |
|                          |                          | simultaneously)          |
+--------------------------+--------------------------+--------------------------+
| **MAXNODEPERUSERPOLICY** | limits the maximum total | ``MAXNODEPERUSERPOLICY O |
| **MAXNODEPERUSERCOUNT**  | number of nodes any      | N``                      |
| **SMAXNODEPERUSERCOUNT** | given user may have      | ``MAXNODEPERUSERCOUNT  1 |
|                          | active (allocated to     | 6``                      |
|                          | running jobs)            | ``SMAXNODEPERUSERCOUNT 8 |
|                          | simultaneously           | ``                       |
|                          |                          | (allow each user to      |
|                          |                          | utilize up to 8 nodes    |
|                          |                          | simultaneously by        |
|                          |                          | default. If no other     |
|                          |                          | jobs can run and idle    |
|                          |                          | nodes are available,     |
|                          |                          | allow each user to       |
|                          |                          | utilize up to 16 nodes   |
|                          |                          | simultaneously.)         |
+--------------------------+--------------------------+--------------------------+
| **MAXPROCSECONDPERUSERPO | limits the maximum total | ``MAXPROCSECONDPERUSERPO |
| LICY**                   | number of                | LICY ON``                |
| **MAXPROCSECONDPERUSERCO | processor-seconds any    | ``MAXPROCSECONDPERUSERCO |
| UNT**                    | given user may have      | UNT  20000``             |
| **SMAXPROCSECONDPERUSERC | active (allocated to     | (allow each user to      |
| OUNT**                   | running jobs)            | utilize up to 20000      |
|                          | simultaneously. NOTE:    | processor-seconds        |
|                          | processor-seconds        | simultaneously)          |
|                          | associated with any      |                          |
|                          | given job is calculated  |                          |
|                          | as PROCESSORS \*         |                          |
|                          | REMAININGWALLTIME        |                          |
+--------------------------+--------------------------+--------------------------+
| **MAXJOBQUEUEDPERUSERPOL | limits the maximum total | ``MAXJOBQUEUEDPERUSERPOL |
| ICY**                    | number of idle jobs      | ICY ON``                 |
| **MAXJOBQUEUEDPERUSERCOU | associated with each     | ``MAXJOBQUEUEDPERUSERCOU |
| NT**                     | user which Maui will     | NT  3``                  |
| **SMAXJOBQUEUEDPERUSERCO | consider eligible for    | (Maui will only consider |
| UNT**                    | scheduling               | 3 idle jobs per user in  |
|                          |                          | each scheduling          |
|                          |                          | iteration)               |
+--------------------------+--------------------------+--------------------------+
| **MAXPEPERUSERPOLICY**   | limits the maximum total | ``MAXPEPERUSERPOLICY ON` |
| **MAXPEPERUSERCOUNT**    | number of                | `                        |
| **SMAXPEPERUSERCOUNT**   | `processor-equivalents < | ``MAXPEPERUSERCOUNT  48` |
|                          | 3.2environment.html#PEov | `                        |
|                          | erview>`__               | (allow each user have up |
|                          | any given user may have  | to 48 PE's allocated to  |
|                          | active (running)         | active jobs              |
|                          | simultaneously           | simultaneously)          |
+--------------------------+--------------------------+--------------------------+
| **MAXJOBPERGROUPPOLICY** | limits the maximum total | ``MAXJOBPERGROUPPOLICY O |
| **MAXJOBPERGROUPCOUNT**  | number of jobs any given | N``                      |
| **SMAXJOBPERGROUPCOUNT** | group may have active    | ``MAXJOBPERGROUPCOUNT  4 |
|                          | (running) simultaneously | ``                       |
|                          |                          | (allow each group to run |
|                          |                          | up to 4 jobs             |
|                          |                          | simultaneously)          |
+--------------------------+--------------------------+--------------------------+
| **MAXPROCPERGROUPPOLICY* | limits the maximum total | ``MAXPROCPERGROUPPOLICY  |
| *                        | number of processors any | ON``                     |
| **MAXPROCPERGROUPCOUNT** | given group may have     | ``MAXPROCPERGROUPCOUNT   |
| **SMAXPROCPERGROUPCOUNT* | active (allocated to     | 32``                     |
| *                        | running jobs)            | (allow each group to     |
|                          | simultaneously           | utilize up to 32         |
|                          |                          | processors               |
|                          |                          | simultaneously)          |
+--------------------------+--------------------------+--------------------------+
| **MAXNODEPERGROUPPOLICY* | limits the maximum total | ``MAXNODEPERGROUPPOLICY  |
| *                        | number of nodes any      | ON``                     |
| **MAXNODEPERGROUPCOUNT** | given group may have     | ``MAXNODEPERGROUPCOUNT   |
| **SMAXNODEPERGROUPCOUNT* | active (allocated to     | 16``                     |
| *                        | running jobs)            | ``SMAXNODEPERGROUPCOUNT  |
|                          | simultaneously           | 8``                      |
|                          |                          | (allow each group to     |
|                          |                          | utilize up to 8 nodes    |
|                          |                          | simultaneously by        |
|                          |                          | default. If no other     |
|                          |                          | jobs can run and idle    |
|                          |                          | nodes are available,     |
|                          |                          | allow each group to      |
|                          |                          | utilize up to 16 nodes   |
|                          |                          | simultaneously.)         |
+--------------------------+--------------------------+--------------------------+
| **MAXPROCSECONDPERGROUPP | limits the maximum total | ``MAXPROCSECONDPERGROUPP |
| OLICY**                  | number of                | OLICY ON``               |
| **MAXPROCSECONDPERGROUPC | processor-seconds any    | ``MAXPROCSECONDPERGROUPC |
| OUNT**                   | given group may have     | OUNT  20000``            |
| **SMAXPROCSECONDPERGROUP | active (allocated to     | (allow each group to     |
| COUNT**                  | running jobs)            | utilize up to 20000      |
|                          | simultaneously. NOTE:    | processor-seconds        |
|                          | processor-seconds        | simultaneously)          |
|                          | associated with any      |                          |
|                          | given job is calculated  |                          |
|                          | as PROCESSORS \*         |                          |
|                          | REMAININGWALLTIME        |                          |
+--------------------------+--------------------------+--------------------------+
| **MAXJOBQUEUEDPERGROUPPO | limits the maximum total | ``MAXJOBQUEUEDPERGROUPPO |
| LICY**                   | number of idle jobs      | LICY ON``                |
| **MAXJOBQUEUEDPERGROUPCO | associated with each     | ``MAXJOBQUEUEDPERGROUPCO |
| UNT**                    | group which Maui will    | UNT  3``                 |
| **SMAXJOBQUEUEDPERGROUPC | consider eligible for    | (Maui will only consider |
| OUNT**                   | scheduling               | 3 idle jobs per group in |
|                          |                          | each scheduling          |
|                          |                          | iteration)               |
+--------------------------+--------------------------+--------------------------+
| **MAXPEPERGROUPPOLICY**  | limits the maximum total | ``MAXPEPERGROUPPOLICY ON |
| **MAXPEPERGROUPCOUNT**   | number of                | ``                       |
| **SMAXPEPERGROUPCOUNT**  | `processor-equivalents < | ``MAXPEPERGROUPCOUNT  48 |
|                          | 3.2environment.html#PEov | ``                       |
|                          | erview>`__               | (allow each group have   |
|                          | any given group may have | up to 48 PE's allocated  |
|                          | active (running)         | to active jobs           |
|                          | simultaneously           | simultaneously)          |
+--------------------------+--------------------------+--------------------------+
| **MAXJOBPERACCOUNTPOLICY | limits the maximum total | ``MAXJOBPERACCOUNTPOLICY |
| **                       | number of jobs any given |  ON``                    |
| **MAXJOBPERACCOUNTCOUNT* | account may have active  | ``MAXJOBPERACCOUNTCOUNT  |
| *                        | (running) simultaneously |  4``                     |
| **SMAXJOBPERACCOUNTCOUNT |                          | (allow each account to   |
| **                       |                          | run up to 4 jobs         |
|                          |                          | simultaneously)          |
+--------------------------+--------------------------+--------------------------+
| **MAXPROCPERACCOUNTPOLIC | limits the maximum total | ``MAXPROCPERACCOUNTPOLIC |
| Y**                      | number of processors any | Y ON``                   |
| **MAXPROCPERACCOUNTCOUNT | given account may have   | ``MAXPROCPERACCOUNTCOUNT |
| **                       | active (allocated to     |   32``                   |
| **SMAXPROCPERACCOUNTCOUN | running jobs)            | (allow each account to   |
| T**                      | simultaneously           | utilize up to 32         |
|                          |                          | processors               |
|                          |                          | simultaneously)          |
+--------------------------+--------------------------+--------------------------+
| **MAXNODEPERACCOUNTPOLIC | limits the maximum total | ``MAXNODEPERACCOUNTPOLIC |
| Y**                      | number of nodes any      | Y ON``                   |
| **MAXNODEPERACCOUNTCOUNT | given account may have   | ``MAXNODEPERACCOUNTCOUNT |
| **                       | active (allocated to     |   16``                   |
| **SMAXNODEPERACCOUNTCOUN | running jobs)            | ``SMAXNODEPERACCOUNTCOUN |
| T**                      | simultaneously           | T 8``                    |
|                          |                          | (allow each account to   |
|                          |                          | utilize up to 8 nodes    |
|                          |                          | simultaneously by        |
|                          |                          | default. If no other     |
|                          |                          | jobs can run and idle    |
|                          |                          | nodes are available,     |
|                          |                          | allow each account to    |
|                          |                          | utilize up to 16 nodes   |
|                          |                          | simultaneously.)         |
+--------------------------+--------------------------+--------------------------+
| **MAXPROCSECONDPERACCOUN | limits the maximum total | ``MAXPROCSECONDPERACCOUN |
| TPOLICY**                | number of                | TPOLICY ON``             |
| **MAXPROCSECONDPERACCOUN | processor-seconds any    | ``MAXPROCSECONDPERACCOUN |
| TCOUNT**                 | given account may have   | TCOUNT  20000``          |
| **SMAXPROCSECONDPERACCOU | active (allocated to     | (allow each account to   |
| NTCOUNT**                | running jobs)            | utilize up to 20000      |
|                          | simultaneously. NOTE:    | processor-seconds        |
|                          | processor-seconds        | simultaneously)          |
|                          | associated with any      |                          |
|                          | given job is calculated  |                          |
|                          | as PROCESSORS \*         |                          |
|                          | REMAININGWALLTIME        |                          |
+--------------------------+--------------------------+--------------------------+
| **MAXJOBQUEUEDPERACCOUNT | limits the maximum total | ``MAXJOBQUEUEDPERACCOUNT |
| POLICY**                 | number of idle jobs      | POLICY ON``              |
| **MAXJOBQUEUEDPERACCOUNT | associated with each     | ``MAXJOBQUEUEDPERACCOUNT |
| COUNT**                  | account which Maui will  | COUNT  3``               |
| **SMAXJOBQUEUEDPERACCOUN | consider eligible for    | (Maui will only consider |
| TCOUNT**                 | scheduling               | 3 idle jobs per account  |
|                          |                          | in each scheduling       |
|                          |                          | iteration)               |
+--------------------------+--------------------------+--------------------------+
| **MAXPEPERACCOUNTPOLICY* | limits the maximum total | ``MAXPEPERACCOUNTPOLICY  |
| *                        | number of                | ON``                     |
| **MAXPEPERACCOUNTCOUNT** | `processor-equivalents < | ``MAXPEPERACCOUNTCOUNT   |
| **SMAXPEPERACCOUNTCOUNT* | 3.2environment.html#PEov | 48``                     |
| *                        | erview>`__               | (allow each account have |
|                          | any given account may    | up to 48 PE's allocated  |
|                          | have active (running)    | to active jobs           |
|                          | simultaneously           | simultaneously)          |
+--------------------------+--------------------------+--------------------------+
