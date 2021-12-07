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
#                    - voice_control
#                    - sound_control
#                    - vision_control
### END INIT INFO

# Author: T.GAUTIER
# /etc/init.d/robot.sh: start and stop the robot process
# cmd:sudo update-rc.d robot.sh defaults

#CPU used for COM RT
CPU_COM_RT_MASK=0x00000001 # is processor 0 see man of taskset

#list of kernel threads in charge to manage bluetooth
KTHREAD_IRQ_RFCOMM_NAME=irq/8-uart-pl01

KWORKER_U17_0_HCI0_NAME=kworker/u17:0-hci0
KWORKER_U17_1_HCI0_NAME=kworker/u17:1-hci0
KWORKER_U17_2_HCI0_NAME=kworker/u17:2-hci0

#list of devices used for the project
GAMEPAD_DEVICE=/dev/input/js0
#FRDM_KV31_DEVICE=/dev/ttyS1 # ubuntu 20.04.3 LTS 
FRDM_KV31_DEVICE=/dev/ttyAML1 # debian 11 

#path of all exec used for the project
EXEC_PATH=/usr/local/bin/

LOGFILE=/tmp/robot.txt

echo script robot.sh start > $LOGFILE

echo step 1 >> $LOGFILE

#get pid of kernel thread in charge to manage bluetooth
#KTHREAD_IRQ_RFCOMM_PID=$(pgrep $KTHREAD_IRQ_RFCOMM_NAME)
#KTHREAD_RFCOMM_PID=$(pgrep $KTHREAD_RFCOMM_NAME)

#check all binaries required to manage the robot
test -x $EXEC_PATH/supervisor || exit -1
test -x $EXEC_PATH/gamepad || exit -2
test -x $EXEC_PATH/motion_control || exit -3
#test -x $EXEC_PATH/voice_control || exit -4
#test -x $EXEC_PATH/sound_control || exit -5
test -x $EXEC_PATH/vision_control || exit -6

echo step 2 >> $LOGFILE

case "$1" in
  start)
        #set affinity of kernel thread CPU7
        #taskset -p $CPU_COM_RT_MASK $KTHREAD_IRQ_RFCOMM_PID
        #taskset -p $CPU_COM_RT_MASK $KTHREAD_RFCOMM_PID
       
        #set priority =-60 of kernel thread in charge to manage bluetooth
        #chrt -r -p 60 $KTHREAD_IRQ_RFCOMM_PID
        #chrt -r -p 60 $KTHREAD_RFCOMM_PID

        echo step 4 >> $LOGFILE
        # start supevisor process
#        taskset -a $CPU_COM_RT_MASK $EXEC_PATH/supervisor >/tmp/supervisor.log &
        taskset -a $CPU_COM_RT_MASK $EXEC_PATH/supervisor &
     
        echo step 5 >> $LOGFILE
        #wait gamepad connection device $GAMEPAD_DEVICE
        while [ ! -c $GAMEPAD_DEVICE ] ; do sleep 1; echo step 3 Waiting gamepad >> $LOGFILE ; done

        # start robot application, launch gamepad process
        $EXEC_PATH/gamepad $GAMEPAD_DEVICE >/tmp/gamepad.log &

        echo step 6 >> $LOGFILE
        
        taskset -a $CPU_COM_RT_MASK $EXEC_PATH/motion_control $FRDM_KV31_DEVICE >/tmp/motion_control.log &
#        taskset -a $CPU_COM_RT_MASK $EXEC_PATH/motion_control $FRDM_KV31_DEVICE &

        echo step 7 >> $LOGFILE

        #start voice control
        #$EXEC_PATH/voice_control > /tmp/voice_control.log &

        #start sound control
        #$EXEC_PATH/sound_control > /tmp/sound_control.log &

        #start vision control
#        $EXEC_PATH/vision_control --camera 1 >/tmp/vision_control.log &
#       $EXEC_PATH/vision_control --camera 1 &
#        $EXEC_PATH/vision_control >/tmp/vision_control.log &

	;;
  stop)
        #Termination request (SIGTERM) all process
        p=$(pidof supervisor)
        echo "Termination request $p..."
        kill -TERM $p

        p=$(pidof gamepad)
        echo "Termination request $p..."
        kill -TERM $p

        p=$(pidof motion_control)
        echo "Termination request $p..."
        kill -TERM $p

        p=$(pidof voice_control)
        echo "Termination request $p..."
        kill -TERM $p

        p=$(pidof sound_control)
        echo "Termination request $p..."
        kill -TERM $p

        p=$(pidof vision_control)
        echo "Termination request $p..."
        kill -TERM $p

        rm $LOGFILE
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
	echo "Usage: /etc/init.d/robot.sh {start|stop}" || true
	exit 1
esac

exit 0
