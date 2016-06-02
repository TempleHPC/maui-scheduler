**Job Attributes/Flags Overview**
****
|
| ****

| **Job Attributes**
| ****

+----------------+----------------+----------------+----------------+----------------+
| **Attribute**  | **Format**     | **Default**    | **Description* | **Example**    |
|                |                |                | *              |                |
+----------------+----------------+----------------+----------------+----------------+
| **FLAGS**      | <FLAG>[;<FLAG> | [NONE]         | specifies job  | ``FLAGS=ADVRES |
|                | ]...           |                | specific flags | ;DEDICATED``   |
|                |                |                |                | (The job       |
|                |                |                |                | should only    |
|                |                |                |                | utilize        |
|                |                |                |                | reserved       |
|                |                |                |                | resources and  |
|                |                |                |                | should only    |
|                |                |                |                | use resources  |
|                |                |                |                | on hosts which |
|                |                |                |                | can be         |
|                |                |                |                | exclusively    |
|                |                |                |                | dedicated)     |
+----------------+----------------+----------------+----------------+----------------+
| **PDEF**       | <PARTITION\_NA | **[DEFAULT]**  | specifies the  | ``PDEF=P1``    |
|                | ME>            |                | default        | (The object is |
|                |                |                | partition      | assigned the   |
|                |                |                | associated     | default        |
|                |                |                | with the       | partition      |
|                |                |                | object.        | ``P1``)        |
+----------------+----------------+----------------+----------------+----------------+
| **PLIST\***    | <PARTITION\_NA | **[ALL]**      | specifies the  | ``PLIST=OldSP: |
|                | ME>[^\|&]      |                | list of        | Cluster1:O3K`` |
|                | [:<PARTITION\_ |                | partitions the | (The object    |
|                | NAME>[^\|&]].. |                | object can     | can access     |
|                | .              |                | access. If no  | resources      |
|                |                |                | partition list | located in the |
|                |                |                | is specified,  | ``OldSP``,     |
|                |                |                | the object is  | ``Cluster1``,  |
|                |                |                | granted        | and/or ``O3K`` |
|                |                |                | default access | partitions)    |
|                |                |                | to all         |                |
|                |                |                | partitions.    |                |
+----------------+----------------+----------------+----------------+----------------+
| **QDEF**       | <QOS\_NAME>    | [DEFAULT]      | specifies the  | ``QDEF=premium |
|                |                |                | default QOS    | ``             |
|                |                |                | associated     | (The object is |
|                |                |                | with the       | assigned the   |
|                |                |                | object.        | default QOS    |
|                |                |                |                | ``premium``)   |
+----------------+----------------+----------------+----------------+----------------+
| **QLIST\***    | <QOS\_NAME>[^\ | <QDEF>         | specifies the  | ``QLIST=premiu |
|                | |&]            |                | list of QoS's  | m:express:bott |
|                | [:<QOS\_NAME>[ |                | the object can | omfeeder``     |
|                | ^\|&]]...      |                | access. If no  | (The object    |
|                |                |                | QOS list is    | can access any |
|                |                |                | specified, the | of the 3 QOS's |
|                |                |                | object is      | listed)        |
|                |                |                | granted access |                |
|                |                |                | only to its    |                |
|                |                |                | default        |                |
|                |                |                | partition/     |                |
+----------------+----------------+----------------+----------------+----------------+

****
**\*NOTE**: By default, jobs may access QOS's based on the 'logical or'
of the access lists associated with all job credentials. For example, a
job associated with user *John*, group *staff*, and class *batch* may
utilize QOS's accessible by any of the individual credentials. Thus the
job's QOS access list, or QLIST, equals the 'or' of the user, group, and
class QLIST's. (i.e., JOBQLIST = USERQLIST \| GROUPQLIST \| CLASSQLIST).
If the ampersand symbol, '&', is associated with any list, this list is
logically and'd with the other lists. If the carat symbol, '^', is
associated with any object QLIST, this list is exclusively set,
regardless of other object access lists using the following order of
precedence user, group, account, QOS, and class. These special symbols
affect the behavior of both QOS and partition access lists.

--------------

**Job Flags**
+----------------+----------------+----------------+----------------+----------------+
| **Flag**       | **Format**     | **Default**    | **Description* | **Example**    |
|                |                |                | *              |                |
+----------------+----------------+----------------+----------------+----------------+
| **ADVRES**     | ADVRES[:<RESID | Use available  | specifies the  | ``FLAGS=ADVRES |
|                | >]             | resources      | job may only   | :META.1``      |
|                |                | where ever     | utilize        | (The job may   |
|                |                | found, whether | accessible,    | only utilize   |
|                |                | inside a       | reserved       | resources      |
|                |                | reservation or | resources. If  | located in the |
|                |                | not.           | <RESID> is     | ``META.1``     |
|                |                |                | specified,     | reservation)   |
|                |                |                | only resources |                |
|                |                |                | in the         |                |
|                |                |                | specified      |                |
|                |                |                | reservation    |                |
|                |                |                | may be         |                |
|                |                |                | utilized.      |                |
+----------------+----------------+----------------+----------------+----------------+
| **BENCHMARK**  | BENCHMARK      | N/A            | N/A            | ``FLAGS=BENCHM |
|                |                |                |                | ARK``          |
+----------------+----------------+----------------+----------------+----------------+
| **BESTEFFORT** | BESTEFFORT     | N/A            | N/A            | ``FLAGS=BESTEF |
|                |                |                |                | FORT``         |
+----------------+----------------+----------------+----------------+----------------+
| **BYNAME**     | BYNAME         | N/A            | N/A            | ``FLAGS=BYNAME |
|                |                |                |                | ``             |
+----------------+----------------+----------------+----------------+----------------+
| **DEDICATED**  | DEDICATED      | Use resources  | specifies that | FLAGS=DEDICATE |
|                |                | according to   | the job should | D              |
|                |                | the global     | not share node | (The job will  |
|                |                | `NODEACCESSPOL | resources with | only allocate  |
|                |                | ICY <a.fparame | tasks from any | resources from |
|                |                | ters.html#node | other job      | nodes which    |
|                |                | accesspolicy>` |                | can be         |
|                |                | __             |                | exclusively    |
|                |                |                |                | dedicated to   |
|                |                |                |                | this job)      |
+----------------+----------------+----------------+----------------+----------------+
| **HOSTLIST**   | HOSTLIST=<HOST | The job may    | specifies the  | ``HOSTLIST=nod |
|                | NAME>          | utilize any    | list of hosts  | e003:node006:n |
|                | [:<HOSTNAME>]. | available      | which should   | ode009``       |
|                | ..             | resource       | be used by the | (Maui will     |
|                |                | regardless of  | job. If more   | allocate       |
|                |                | hostname       | hosts are      | resources      |
|                |                |                | specified than | using the      |
|                |                |                | are needed to  | specified      |
|                |                |                | meet the jobs  | hosts)         |
|                |                |                | total task     |                |
|                |                |                | requirements,  |                |
|                |                |                | Maui will      |                |
|                |                |                | select needed  |                |
|                |                |                | hosts from the |                |
|                |                |                | list. If fewer |                |
|                |                |                | hosts are      |                |
|                |                |                | specified than |                |
|                |                |                | are needed to  |                |
|                |                |                | meet the job's |                |
|                |                |                | total task     |                |
|                |                |                | requirements,  |                |
|                |                |                | Maui will      |                |
|                |                |                | select all     |                |
|                |                |                | listed hosts   |                |
|                |                |                | and attempt to |                |
|                |                |                | locate         |                |
|                |                |                | additional     |                |
|                |                |                | resources      |                |
|                |                |                | elsewhere.     |                |
+----------------+----------------+----------------+----------------+----------------+
| **NOQUEUE**    | NOQUEUE        | Jobs remain    | specifies that | ``FLAGS=NOQUEU |
|                |                | queued until   | the job should | E``            |
|                |                | the are able   | be removed it  | (The job       |
|                |                | to run         | is is unable   | should be      |
|                |                |                | to allocate    | removed unless |
|                |                |                | resources and  | it can start   |
|                |                |                | start          | running at     |
|                |                |                | execution      | submit time.)  |
|                |                |                | immediately.   |                |
+----------------+----------------+----------------+----------------+----------------+
| **PREEMPTEE**  | PREEMPTEE      | Jobs may not   | Specifies that | ``FLAGS=PREEMP |
|                |                | be preempted   | the job may be | TEE``          |
|                |                | by other jobs  | preempted by   | (The job may   |
|                |                |                | other jobs     | be preempted   |
|                |                |                | which have the | by other jobs  |
|                |                |                | **PREEMPTOR**  | which have the |
|                |                |                | flag set.      | '**PREEMPTOR** |
|                |                |                |                | '              |
|                |                |                |                | flag set)      |
+----------------+----------------+----------------+----------------+----------------+
| **PREEMPTOR**  | PREEMPTOR      | Jobs may not   | Specifies that | ``FLAGS=PREEMP |
|                |                | preempt other  | the job may    | TOR``          |
|                |                | jobs           | preempt other  | (The job may   |
|                |                |                | jobs which     | preempt other  |
|                |                |                | have the       | jobs which     |
|                |                |                | **PREEMPTEE**  | have the       |
|                |                |                | flag set       | '**PREEMPTEE** |
|                |                |                |                | '              |
|                |                |                |                | flag set)      |
+----------------+----------------+----------------+----------------+----------------+
| **PRESTART**   | PRESTART       | Jobs are       | **NOTE**: used | ``FLAGS=PRESTA |
|                |                | started only   | only in        | RT``           |
|                |                | after the      | simulation     |                |
|                |                | first          | mode to        |                |
|                |                | scheduling     | pre-populate a |                |
|                |                | iteration      | system.        |                |
+----------------+----------------+----------------+----------------+----------------+
| **RESTARTABLE* | RESTARTABLE    | Jobs may not   | Specifies jobs | FLAGS=RESTARTA |
| *              |                | be restarted   | can be         | BLE            |
|                |                | if preempted.  | 'requeued' and | (The           |
|                |                |                | later          | associated job |
|                |                |                | restarted if   | can be         |
|                |                |                | preempted      | preempted and  |
|                |                |                |                | restarted at a |
|                |                |                |                | later date)    |
+----------------+----------------+----------------+----------------+----------------+
| **SHAREDNODE** | SHAREDNODE     | N/A            | N/A            | N/A            |
+----------------+----------------+----------------+----------------+----------------+
| **SPAN**       | SPAN           | Jobs may only  | Allows jobs to | FLAGS=SPAN     |
|                |                | access         | utilize        | (The job can   |
|                |                | resources      | resources from | be allocated   |
|                |                | within a       | multiple       | and utilize    |
|                |                | single         | partitions     | resources from |
|                |                | partition      | simultaneously | more than one  |
|                |                |                |                | accessible     |
|                |                |                |                | partition      |
|                |                |                |                | simultaneously |
|                |                |                |                | .)             |
+----------------+----------------+----------------+----------------+----------------+
