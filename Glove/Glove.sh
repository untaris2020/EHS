# Project Name:  A.R.I.S. - Augmented Reality Interface System
# University:    University of North Texas
# Developers:    Juan Ruiz, Computer Engineering
# Date Created:  04/07/2020

# Purpose:       The purpose of the Glove.sh is to read the status of the hard reset switch
#                If the switch is toggled to ON it will kill all the programs.
#                Once the switch is toggled back to OFF it will re initiate all the executables

#!/bin/bash

/home/pi/Glove/gloveCam &
sleep 26
/home/pi/Glove/gloveButton &
/home/pi/Glove/gloveFlex &
/home/pi/IMUC/getbno055 &

status=false

while switchStatus=$(gpio read 7)
do

        if [ "$switchStatus" == "1" ] && [ "$status" == "false" ]
        then
                echo "KILLING IT"
                killall -sKILL gloveCam
                killall -sKILL gloveButton
                killall -sKILL gloveFlex
                killall -sKILL getbno055

                status=true

        elif [ "$switchStatus" == "0" ] && [ "$status" == "true" ]
        then
                echo "REACTIVATING IT"
                /home/pi/Glove/gloveCam &
                sleep 26
                /home/pi/Glove/gloveButton &
                /home/pi/Glove/gloveFlex &
                /home/pi/IMUC/getbno055 &

                status=false

        fi
done &