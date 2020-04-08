# Project Name:  A.R.I.S. - Augmented Reality Interface System
# University:    University of North Texas
# Developers:    Juan Ruiz, Computer Engineering
# Date Created:  04/07/2020

# Purpose:       The purpose of the Button.sh shell script is to kill all processes for both the glove and chest.
#                The script also simulates the on and off switch (in case the button is not connected to the Pi)

# To run type the following
#       ./Button.sh

#       ./Button.sh on - will simulate the toggle of the switch ON means it has encountered an issue and you want
#                        kill all programs and reset them

#       ./Button.sh off - will simulte the toggle of the switch OFF means it will run all the executables

#       ./Button.sh stop - Will kill all the processes for both the chest and glove scripts

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
fi
