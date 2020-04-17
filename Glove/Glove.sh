# Project Name:  A.R.I.S. - Augmented Reality Interface System
# University:    University of North Texas
# Developers:    Juan Ruiz, Computer Engineering
# Date Created:  04/07/2020

# Purpose:       The purpose of the Glove.sh is to read the status of the hard reset switch
#                If the switch is toggled to ON it will kill all the programs.
#                Once the switch is toggled back to OFF it will re initiate all the executables

#!/bin/bash

/home/pi/EHS/Glove/gloveCam &
sleep 26
/home/pi/EHS/Glove/gloveButton &
/home/pi/EHS/Glove/gloveFlex &
/home/pi/EHS/ImuC/imu &

status=false

while switchStatus=$(gpio read 7)
do

        if [ "$switchStatus" == "1" ] && [ "$status" == "false" ]
        then
                echo "KILLING IT"
                killall -sKILL gloveCam
                killall -sKILL gloveFlex
                killall -sKILL imu

                status=true

        elif [ "$switchStatus" == "0" ] && [ "$status" == "true" ]
        then
                echo "REACTIVATING IT"
                /home/pi/EHS/Glove/gloveCam &
                sleep 26
                /home/pi/EHS/Glove/gloveFlex &
                /home/pi/EHS/ImuC/imu &

                status=false

        fi
done &