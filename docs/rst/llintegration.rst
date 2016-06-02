.. rubric:: Maui-Loadleveler Integration Guide
   :name: maui-loadleveler-integration-guide

|
| **Overview:**

| Maui can be used as an external scheduler for Loadleveler. In this
  configuration, Loadleveler manages the job queue and the compute
  resources while maui queries the Loadleveler negotiator via the
  Loadleveler data API to obtain up to date job and node information.
  Using this information, maui directs Loadleveler to manage jobs in
  accordance with specified maui policies, priorities, and reservations.

**Steps:**

Maui drives LL via the Loadleveler scheduling API. To enable this api
and thus the external scheduler, the following steps must be taken:

- set '**SCHEDULER\_API=yes**' in the 'LoadL\_config' file typically
located in the user 'loadl' home directory.

- set the '**NEGOTIATOR\_REMOVE\_COMPLETED**' parameter (also located in
the 'LoadL\_config' file) to a value of at least 5 minutes, ie
'NEGOTIATOR\_REMOVE\_COMPLETED=300'. (This allows maui to obtain job
info from LL required to maintain accurate job statistics)

- recycle negotiator using the command '**llctl recycle**' on the
central manager node.

**Issues:**

The Loadleveler scheduling API is not event driven so Maui has no way of
knowing when a new job is submitted. Under these conditions, it will not
evaluate a newly submitted job until its next scheduling iteration,
typically within 15 to 30 seconds. This lag can be removed by utilizing
Loadleveler's 'SUBMITFILTER'. The Maui command 'schedctl -r 2' can be
added as the last statement in this filter causing Maui to 'wake-up' and
attempt to schedule new jobs immediately. The 'schedctl' command is a
administrative command and so may need an suid wrapper in order to allow
use by non-privileged users. (see `example <schedctlwrapper.html>`__).

NOTE: You can return to Loadleveler default scheduling at any time by
setting 'SCHEDULER\_API=no' in the LoadL\_config file and re-issuing the
'llctl recycle' command.

Maui supports interactive job hostlists but these hostlists must
currently be specified using the network interface Loadleveler utilizes.
For example, an SP node may have two names, node001e and node001sw
representing its ethernet and switch interfaces respectively.
Loadleveler is configured to communicate with the nodes on one of these
interfaces. (This can be determined by issuing 'llstatus' and observing
the name used to specify each node.) Interactive job hostlists must be
specified using the same interface that Loadleveler is configured to
use. Efforts are underway to extend Maui interface tracking to remedy
this.
