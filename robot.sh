#!/bin/sh

### BEGIN INIT INFO
# Provides:          robot
# Required-Start:    $local_fs $syslog $remote_fs dbus $bluetooth 
# Required-Stop:     $local_fs $syslog $remote_fs
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: start ROBOT on the hikey board
# Description:       start all process required for the robot:
#                    - keyboard (control the robot)
#                    - gamepad (control the robot)
#                    - motion_control
#                    - vision
#                    - sound cmd
### END INIT INFO

# Author: T.GAUTIER
# /etc/init.d/robot.sh: start and stop the robot
#

PATH_ROBOT_EXEC=/media/linaro/DATA/Hikey/Hikey

#wait system mount /media/linaro/DATA partitionls !!! 
#required to modify Required-Start or cp exe in bin directory during the generation
sleep 10

test -x /media/linaro/DATA/Hikey/Hikey/gamepad/Debug/gamepad || exit 0


case "$1" in
  start)
        /media/linaro/DATA/Hikey/Hikey/gamepad/Debug/gamepad >/tmp/gamepad.txt
	;;
  stop)
	;;

  reload|force-reload)
	;;

  restart)
	;;

  try-restart)
	;;

  status)
	;;

  *)
	log_action_msg "Usage: /etc/init.d/ssh {start|stop|reload|force-reload|restart|try-restart|status}" || true
	exit 1
esac

exit 0