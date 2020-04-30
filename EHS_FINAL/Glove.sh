# Project Name:  A.R.I.S. - Augmented Reality Interface System
# University:    University of North Texas
# Developers:    Juan Ruiz, Computer Engineering
# Date Created:  04/07/2020

# Purpose:       The purpose of the Glove.sh is to read the status of the hard reset switch
#                If the switch is toggled to ON it will kill all the programs.
#                Once the switch is toggled back to OFF it will re initiate all the executables

#!/bin/bash

killall -sKILL Chest
killall -sKILL chestCam
killall -sKILL chestButton
killall -sKILL gloveImu

killall -sKILL Glove
killall -sKILL gloveCam
killall -sKILL gloveButton
killall -sKILL gloveFlex
killall -sKILL gloveImu

./gloveButton &

./gloveFlex &

./gloveImu &

./gloveCam &

status=false

while switchStatus=$(gpio read 7)
do

        if [ "$switchStatus" == "1" ] && [ "$status" == "false" ]
        then
                echo "KILLING IT"
                sudo killall -sKILL gloveCam
                sudo killall -sKILL gloveImu
                sudo killall -sKILL gloveFlex

                status=true

        elif [ "$switchStatus" == "0" ] && [ "$status" == "true" ]
        then
                echo "REACTIVATING IT"
                
                ./gloveFlex &

                ./gloveImu &

                ./gloveCam &
                
                status=false

        fi
done 
