

.. rubric:: diagnose -q
   :name: diagnose--q

**(Maui Queue Diagnostic)**
.. rubric:: Synopsis
   :name: synopsis

diagnose -q
| 

.. rubric:: Overview:
   :name: overview

This command presents information about the queues (classes), and the
jobs in them.
**Example:**

::

    > diagnose -q
    Diagnosing blocked jobs (policylevel SOFT  partition ALL)

    job 67                   has the following hold(s) in place:  Defer
    job 67                   has non-idle expected state (expected state: Deferred)
    job 68                   has the following hold(s) in place:  Defer
    job 68                   has non-idle expected state (expected state: Deferred)
    job 69                   has the following hold(s) in place:  Defer
    job 69                   has non-idle expected state (expected state: Deferred)

