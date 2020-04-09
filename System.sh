# Project Name:  A.R.I.S. - Augmented Reality Interface System
# University:    University of North Texas
# Developers:    Juan Ruiz, Computer Engineering
# Date Created:  04/09/2020

# Purpose:       The purpose of the System.sh shell script is to kill all processes for both the glove and chest.
#                The script also simulates the on and off switch (in case the button is not connected to the Pi)

# To run type the following
#       ./System.sh

#       ./System.sh on - Will simulate the toggle of the switch ON means it has encountered an issue and you want
#                        kill all programs and reset them

#       ./System.sh off - Will simulte the toggle of the switch OFF means it will run all the executables

#       ./System.sh stop - Will kill all the processes for both the chest and glove scripts

#       ./System.sh init - Will give the shell scripts Chest.sh and Glove.sh rights to run on the system
#                          It will also run the make files and create the executables

#!/bin/bash

if [ "$1" == "on" ]
then
        gpio mode 7 up
        echo "BUTTON IS ON"

elif [ "$1" == "off" ]
then
        gpio mode 7 down
        echo "BUTTON IS OFF"

elif [ "$1" == "stop" ]
then

        killall -sKILL chestCam
        killall -sKILL chestButton

        killall -sKILL gloveCam
        killall -sKILL gloveButton
        killall -sKILL gloveFlex
        killall -sKILL imu
        echo "MANUALLY KILLED"
        
elif [ "$1" == "init" ]
then

        chmod +x /home/pi/EHS/Chest/Chest.sh
        chmod +x /home/pi/EHS/Glove/Glove.sh
        
        cd /home/pi/EHS/Chest && make
        cd /home/pi/EHS/Glove && make
        
        echo ""
        echo "--------------------------------------------"
        echo "Shell Scripts Active and Executables created"
        echo "READY TO RUN"




fi
