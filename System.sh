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

#      ./System.sh Chest - Will give the shell script Chest.sh rights to run on the system
#                          It will also run the make file and create the executables for the Chest
#                          (This command is useful if you made an IP and portn no change and you want to run the make file from the current directory.)

#      ./System.sh Glove - Will give the shell script Glove.sh rights to run on the system
#                          It will also run the make file and create the executables for the Glove
#                          (This command is useful if you made an IP and portn no change and you want to run the make file from the current directory.)

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
       killall -sKILL Chest
       killall -sKILL chestCam
       killall -sKILL chestButton

       killall -sKILL Glove
       killall -sKILL gloveCam
       killall -sKILL gloveButton
       killall -sKILL gloveFlex
       killall -sKILL imu
       killall -sKILL gpio
       killall -sKILL bash
        echo "MANUALLY KILLED"
        
elif [ "$1" == "init" ]
then

        chmod +x /home/pi/EHS/Chest/Chest.sh
        chmod +x /home/pi/EHS/Glove/Glove.sh
        
        cd /home/pi/EHS/Chest && make
        cd /home/pi/EHS/Chest/ImuC && make
        
        cd /home/pi/EHS/Glove && make
        cd /home/pi/EHS/Glove/ImuC && make
        
        echo ""
        echo "--------------------------------------------"
        echo "Shell Scripts Active and Executables created"
        echo "               READY TO RUN                 "
        echo "--------------------------------------------"
        echo ""
        
elif [ "$1" == "Chest" ]
then

        chmod +x /home/pi/EHS/Chest/Chest.sh 
        cd /home/pi/EHS/Chest && make
        cd /home/pi/EHS/Chest/ImuC && make
        
        echo ""
        echo "------------------------------------------------------"
        echo "Shell Scripts Active and Executables created for CHEST"
        echo "                     READY TO RUN                     "
        echo "------------------------------------------------------"
        echo ""
        
elif [ "$1" == "Glove" ]
then

        chmod +x /home/pi/EHS/Glove/Glove.sh
        cd /home/pi/EHS/Glove && make
        cd /home/pi/EHS/Glove/ImuC && make
        
        echo ""
        echo "------------------------------------------------------"
        echo "Shell Scripts Active and Executables created for GLOVE"
        echo "                     READY TO RUN                     "
        echo "------------------------------------------------------"
        echo ""

fi
