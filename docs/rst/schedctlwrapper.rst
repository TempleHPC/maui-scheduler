.. rubric:: Event Driven LL Interface
   :name: event-driven-ll-interface

**Creating an event driven interface for Loadleveler requires two
steps.**
| **1) Create and enable the Loadleveler submit filter**
| **2) Create a suid wrapper to allow global use of the 'schedctl -r'
  command**
| ****

**Step 1) Create a submit filter.**

**The LL submit filter functions by taking a users job command file as
STDIN and sending the command file LL should process to STDOUT. If the
submit filter is successful, it should exit with a status of 0. A simple
sample script is provided below:**

| **-----------**
| **``#!/bin/perl -w``**
| **````**
| **``while (<STDIN>)``**
| **`` {``**
| **`` $line = $_;``**
| **`` print STDOUT $line;``**
| **`` }``**
| **````**
| **``# Call wakeup script to maui scheduler``\ ````**

| **``system("/usr/local/sbin/wakeup > /dev/null");``**
| **````**
| **``exit(0);``**
| **------------**

**1) create file 'wakeup.c'**

| **To enable use of this script you must set SUBMITFILTER parameter in
  the LoadL\_config file.**
| ****

**Step 2) Create wrapper**

**The maui command 'schedctl' is not normally available to general end
users. The wrapper allows users to issue a command to request that
scheduling resume to process a newly submitted job.**

| wakeup.c
| -----
| #include <sys/types.h>
| #include <pwd.h>

| #define MAUIADMIN "loadl"
| #define SCHEDCTLCMD /usr/local/bin/schedctl

void main()

| {
| struct passwd \*buf;

| if ((buf = getpwnam(MAUIADMIN)) == NULL)
| exit(1);

setuid(buf->pw\_uid);

system("SCHEDCTLCMD -r 1");

| exit(0);
| }
| -----

**2) edit 'MAUIADMIN' and 'SCHEDCTLCMD' #defines in 'wakeup.c' as
needed**

**3) compile code**

**4) make 'wakeup' owned by MAUIADMIN**

**> chown loadl wakeup**

**5) make 'wakeup' setuid for owner**

**> chmod 4711 wakeup**

**6) verify non-admin users can successfully run 'wakeup'**

****
**Now submit a job. If all works correctly, Maui should detect and, if
resources are available, start the job within one second.**
