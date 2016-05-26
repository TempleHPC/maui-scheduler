Testing
Testing is highly advised anytime you are updating your version of the
scheduler, trying a new configuration, or adding new resources. Maui
gives you several options for safely testing out your new environment.
If you are running Maui for the first time,

It is difficult to advise on all of the possible ways of testing.
However, below are a few general tips.

- Multiple instantiations of Maui can run simultaneously, even on the
same host. This can be very helpful when testing new versions of code.
The current 'production' version of Maui can continue to run the actual
job scheduling, while you are simultaneously evaluating the new code.

| To do so, you must simply avoid conflicts. This includes user
  interface port, logfiles, checkpoint files, and stats files. Many
  sites handle this issue by creating a number of directories parallel
  to the main 'maui' directory. Of particular use is a 'test' directory
  and a 'simulation' directory.
| - when initially installing maui, create a mauitest and mauisim
  directory parallel to

MAUIHOMEDIR

SERVERMODE

NORMAL

SIMULATION

TEST
