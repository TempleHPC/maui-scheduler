

.. rubric:: **schedctl**
   :name: schedctl

| 
| Overview:

| The 'schedctl' command controls various aspects of scheduling
  behavior. It is used to manage scheduling activity, kill the
  scheduler, and create resource trace files.

Format:

|   schedctl { -k \| -n \| -r [ <RESUMETIME> ] \| { -s \| -S } [
  <ITERATION> ] }

Flags:

-k

shutdown the scheduler at the completion of the current scheduling
iteration

-n

dump a node table trace to <STDOUT> (for use in simulations)

-r [ <RESUMETIME> ]

resume scheduling in <RESUMETIME> seconds or immediately if not
specified

-s [ <ITERATION> ]

suspend scheduling at iteration <ITERATION> or at the completion of the
current scheduling iteration if not specified. If <ITERATION> is
followed by the letter 'I', maui will not process client requests until
this iteration is reached.

-S [ <ITERATION> ]

suspend scheduling in <ITERATION> more iterations or in one more
iteration if not specified. If <ITERATION> is followed by the letter
'I', maui will not process client requests until <ITERATION> more
scheduling iterations have been completed.

Example:

Shut maui down

**> schedctl -k**

| maui shutdown

Example:

Stop maui scheduling

**> schedctl -s**

| maui will stop scheduling immediately

Example:

Resume maui scheduling

**schedctl -r**

| maui will resume scheduling immediately

Example:

Stop maui scheduling in 100 more iterations. Specify that maui should
not respond to client requests until that point is reached.

**> schedctl -S 100I**

maui will stop scheduling in 100 iterations

.. raw:: html

   <div class="navIcons topIcons">

|Home| |Up|


