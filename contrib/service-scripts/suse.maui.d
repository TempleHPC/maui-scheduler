#!/bin/sh
#
# Maui Scheduler	This script will start and stop the Maui Scheduler (SUSE)
#
# chkconfig:	345 85 85
# description:	maui
#
# Source the library functions
. /etc/rc.status

# ENVIRONMENT
MAUI=/opt/maui/sbin/maui
#
# Check for parameters in /etc/sysconfig,
#	 if so load them into the environment
[ -f /etc/sysconfig/maui ] && . /etc/sysconfig/maui
#
# make sure the binary exists
#	exit if not
[ -x $MAUI ] || exit

#
# What command are we to do?
#
case "$1" in

	start)
		echo -n "Starting the Maui scheduler: "
		startproc $MAUI
		rc_status -v
		;;

	stop)
    echo -n "Shutting down Maui Scheduler: "
    killproc `basename $MAUI`
    rc_status -v
    ;;

	status)
		checkproc `basename $MAUI`
		rc_status -v
		;;

	restart)
		$0 stop
		$0 start
		;;
	
  *)
    echo "Usage: maui {start|stop|restart|status}"
    exit 1

esac
