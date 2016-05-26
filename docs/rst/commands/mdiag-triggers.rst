
.. rubric:: mdiag -T
   :name: mdiag--t

**(Moab Trigger Diagnostics)**
**Synopsis**

::

    mdiag -T [triggerid]

**Overview**

The 'mdiag -t' command is used to present information about each
Trigger. The information presented includes Name, State, Action, Event
Time.
**Example 1**


::

    > mdiag -T
    [test@node01 moab]$ mdiag -T
    TrigID    Event  MFire   Offset Thresh  AType           ActionDate       State  Launchtime
    ------ -------- ------ -------- ------ ------ -------------------- ----------- -----------
    9         start  FALSE        -      -   exec            -00:00:02      Active   -00:00:01
      Object:           rsv:test.1
      PID:              23786
      Timeout:          00:01:00
      Action Data:      $HOME/bin/1.sh
      Sets on Success:  Initialized
      Output Buffer:    /opt/moab/spool/1.sh.oWo5APo
      Error Buffer:     /opt/moab/spool/1.sh.epbq65C

    10        start  FALSE        -      -   exec            -00:00:02    Inactive           -
      Object:           rsv:test.1
      Action Data:      $HOME/bin/2.sh
      Sets on Success:  COLOR
      Requires:         Initialized

    11       cancel  FALSE        -      -   exec                    -    Inactive           -
      Object:           rsv:test.1
      Action Data:      $HOME/bin/cancel.email $USERNAME
      Requires:         USERNAME


