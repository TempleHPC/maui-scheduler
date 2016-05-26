
.. rubric:: releaseres
   :name: releaseres

**(Release Reservation)**

+----------+----------------------------------------------------------------------------------+
| |Note|   | This command is deprecated. Use `mrsvctl -r <mrsvctl.html#RELEASE>`__ instead.   |
+----------+----------------------------------------------------------------------------------+

.. rubric:: Synopsis
   :name: synopsis

::

    releaseres [arguments] reservationid [reservationid...]

.. rubric:: Overview
   :name: overview

Release existing reservation.
This command allows Moab Scheduler Administrators to release any user,
group, account, job, or system reservation. Users are allowed to release
reservations on jobs they own. Note that releasing a reservation on an
active job has no effect since the reservation will be automatically
recreated.

.. rubric:: Access
   :name: access

Users can use this command to release any reservation they own.  Level 1
and level 2 Moab administrators may use this command to release any
reservation.
| **Parameters**
| **** 

+------------------+--------------------------------------------------------------+
| RESERVATION ID   |                            Name of reservation to release.   |
+------------------+--------------------------------------------------------------+
|                  |                                                              |
+------------------+--------------------------------------------------------------+

.. rubric:: Example 1
   :name: example-1

   Release two existing reservations.

::

    > releaseres system.1 bob.2

    released User reservation 'system.1'
    released User reservation 'bob.2'

.. |Note| image:: /resources/docs/images/caution.png

