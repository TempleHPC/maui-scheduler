#!/bin/sh
#
# maui	This script will start and stop the MAUI Scheduler
#
# chkconfig: 345 85 85
# description: maui
#
ulimit -n 32768
# Source the library functions
. /etc/rc.d/init.d/functions

MAUI_PREFIX=/opt/maui

# let see how we were called
case "$1" in
	start) 
		echo -n "Starting MAUI Scheduler: "
		daemon $MAUI_PREFIX/sbin/maui
		echo
		;;
	stop)
		echo -n "Shutting down MAUI Scheduler: "
		killproc maui
		echo
		;;
	status)
		status maui
		;;
	restart)
		$0 stop
		$0 start
		;;
	*)
		echo "Usage: maui {start|stop|restart|status}"
		exit 1
esac
