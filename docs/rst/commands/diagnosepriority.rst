

.. rubric:: diagnose -p
   :name: diagnose--p

**(Maui Priority Diagnostic)**

.. rubric:: Overview:
   :name: overview

The 'diagnose -p' command is used to display 'at a glance' information
about the job priority configuration and its effects on the current idle
jobs. The information presented by this command includes priority
weights, priority components, and the percentage contribution of each
component to the total job priority.

| The command hides information about priority components which have
  been deactivated (ie, by setting the corresponding component priority
  weight to 0). For each displayed priority component, this command
  gives a small amount of context sensitive information. The following
  table documents this information. In all cases, the output is of the
  form <PERCENT>(<CONTEXT INFO) where <PERCENT> is the percentage
  contribution of the associated priority component to the job's total
  priority.

+----------------------+--------------------------------------------+--------------------------------------------------------------------------------+
| Priority Component   | Format                                     | Description                                                                    |
+----------------------+--------------------------------------------+--------------------------------------------------------------------------------+
| Target               | <PERCENT>()                                |                                                                                |
+----------------------+--------------------------------------------+--------------------------------------------------------------------------------+
| QOS                  | <PERCENT>(<QOS>:<QOSPRI>)                  | QOS: QOS associated with job                                                   |
|                      |                                            | QOSPRI: Priority assigned to the QOS                                           |
+----------------------+--------------------------------------------+--------------------------------------------------------------------------------+
| FairShare            | <PERCENT>(<USR>:<GRP>:<ACC>:<QOS>:<CLS>)   | USR: user fs usage - user fs target                                            |
|                      |                                            | GRP: group fs usage - group fs target                                          |
|                      |                                            | ACC: account fs usage - account fs target                                      |
|                      |                                            | QOS: QOS fs usage - QOS fs target                                              |
|                      |                                            | CLS: class fs usage - class fs target                                          |
+----------------------+--------------------------------------------+--------------------------------------------------------------------------------+
| QueueTime            | <PERCENT>(<HOURS>)                         | HOURS: job queue time which is applicable towards priority                     |
+----------------------+--------------------------------------------+--------------------------------------------------------------------------------+
| Expansion Factor     | <PERCENT>(<VALUE>)                         | VALUE: current theoretical minimum XFactor is job were to start immediately    |
+----------------------+--------------------------------------------+--------------------------------------------------------------------------------+
| Resource             | <PERCENT>(<NDE>:<PE>:<PRC>:<MEM>)          | NDE: nodes requested by job                                                    |
|                      |                                            | PE: Processor Equivalents as calculated by all resources requested by job      |
|                      |                                            | PRC: processors requested by job                                               |
|                      |                                            | MEM: real memory requested by job                                              |
+----------------------+--------------------------------------------+--------------------------------------------------------------------------------+
| Bypass               | <PERCENT>(<BPCOUNT>)                       | BPCOUNT: number of time job was bypassed by lower priority jobs via backfill   |
+----------------------+--------------------------------------------+--------------------------------------------------------------------------------+
| User                 |                                            |                                                                                |
+----------------------+--------------------------------------------+--------------------------------------------------------------------------------+
| Group                |                                            |                                                                                |
+----------------------+--------------------------------------------+--------------------------------------------------------------------------------+

Examples:

> diagnose -p

``diagnosing job priority information (partition: ALL)``

| `` Job PRIORITY* QOS(Q:QOSPri) FS(USR:GRP:ACC) QTime(Hours) XFactor( Value) Resource(NDE: PE:PRC:MEM)``
| ``Weights -------- 1000(-:------) 100( 1: 1: 1) 2(-----) 1000(------) 800( 0: 0: 1: 0)``

| ``fr8n01.1300.0 52203 0.00(1:000000) 0.00(000:000:000) 0.00(000.0) 1.92(001.00) 98.08(064:064:064:000)``
| ``fr8n01.1301.0 26603 0.00(1:000000) 0.00(000:000:000) 0.00(000.0) 3.77(001.00) 96.23(032:032:032:000)``
| ``fr1n04.2068.0 13802 0.00(1:000000) 0.00(000:000:000) 0.00(000.0) 7.25(001.00) 92.74(016:016:016:000)``
| ``fr1n04.2067.0 7403 0.00(0:000000) 0.00(000:000:000) 0.01(000.0) 13.54(001.00) 86.45(008:008:008:000)``
| ``fr1n04.2059.0 4328 0.00(5:000000) 0.00(000:000:000) 0.06(000.0) 26.00(001.12) 73.95(004:004:004:000)``
| ``fr1n04.2060.0 4328 0.00(5:000000) 0.00(000:000:000) 0.06(000.0) 26.00(001.12) 73.95(004:004:004:000)``
| ``fr1n04.2061.0 4328 0.00(5:000000) 0.00(000:000:000) 0.06(000.0) 26.00(001.12) 73.95(004:004:004:000)``
| ``fr8n01.1289.0 4328 0.00(5:000000) 0.00(000:000:000) 0.06(000.0) 26.00(001.12) 73.95(004:004:004:000)``
| ``fr8n01.1290.0 4328 0.00(5:000000) 0.00(000:000:000) 0.06(000.0) 26.00(001.12) 73.95(004:004:004:000)``
| ``fr1n04.2062.0 4328 0.00(5:000000) 0.00(000:000:000) 0.06(000.0) 26.00(001.12) 73.95(004:004:004:000)``
| ``fr8n01.1291.0 4328 0.00(5:000000) 0.00(000:000:000) 0.06(000.0) 26.00(001.12) 73.95(004:004:004:000)``
| ``fr8n01.1253.0 4225 0.00(1:000000) 0.00(000:000:000) 0.25(000.1) 24.01(001.01) 75.74(004:004:004:000)``
| ``fr8n01.1256.0 4225 0.00(0:000000) 0.00(000:000:000) 0.25(000.1) 24.01(001.01) 75.74(004:004:004:000)``
| ``fr8n01.1294.0 4208 0.00(0:000000) 0.00(000:000:000) 0.06(000.0) 23.89(001.01) 76.05(004:004:004:000)``
| ``fr8n01.1293.0 1848 0.00(0:000000) 0.00(000:000:000) 0.57(000.1) 56.14(001.04) 43.29(001:001:001:000)``
| ``fr8n01.1260.0 1814 0.00(0:000000) 0.00(000:000:000) 0.58(000.1) 55.32(001.00) 44.10(001:001:001:000)``

``Totals 100.00 0.00 0.00 0.04 11.57 88.39``

| ``* indicates system prio set on job``

Note that the above output is fairly lengthy. You may need to widen your
browser to properly read it. (Likewise, you may need to expand your
terminal to read the actual command output!) As mentioned previously,
the 'diagnose -p' command only displays information for priority
components actually utilized. In the above example, QOS, Fairshare,
QueueTime, ExpansionFactor, and Resource components are all utilized in
determining a job's priority. Other components, such as Service Targets,
and Bypass are not used and thus are not displayed. (See the '`Priority
Overview <../mauiadmin.html#jobprioritization>`__' for more information)
The output consists of a header, a job by job analysis of idle jobs, and
a summary section.

The header provides column labeling and provides configured priority
component and subcomponent weights. In the above example, QOSWEIGHT is
set to 1000 and FSWEIGHT is set to 100. When configuring fairshare, a
site also has the option of weighting the individual components of a
job's overall fairshare, including its user, group, and account
fairshare components. In this output, the user, group, and account
fairshare weights are set to 5, 1, and 1 respectively.

The job by job analysis displays a job's total priority and the
percentage contribution to that priority of each of the priority
components. In this example, job 'fr8n01.1260.0' has a total priority of
1814. Neither QOS nor Fairshare contribute to any of the job's in the
queue because there is not a QOS priority set for QOS 0, 1, or 5. Also,
no fairshare targets are set for any of the users, groups, or accounts
associated with any of the jobs currently idle. Things finally get
interesting when we get to the queue time component column. Job
'fr8n01.1260.0' has only been queued for 0.1 hours, contributing a total
of 0.58% of its total priority (ie, 0.0058 \* 1814 = ~10 priority
points). The Expansion Factor component is much more significant,
contributing 55.32% of this job's total priority or .5532 \* 1814 =
~1004 priority points. (Note the priority weights of 2 for Queuetime and
1000 for XFactor affecting the relative contributions of each of these
components) The final priority component, 'Resource', only has one
active subcomponent, processors (Note that nodes, PEs, and memory are
deactivated by zero priority subcomponent weights). For the job we are
analyzing, the resource component contributes 44.1% of the job's total
priority or .4410 \* 1814 = ~800 priority points. As expects, the
percentages sum to 100 and the corresponding priority points sum to
1814.

At the end of the job by job description, a 'Totals' line is displayed
which documents the average percentage contributions of each priority
component to the current idle jobs. In this example, the queuetime,
xfactor, and resource components contributed an average of 0.04%,
11.57%, and 88.39% to the idle jobs' total priorities.

.. raw:: html

   <div class="navIcons topIcons">

|Home| |Up|


