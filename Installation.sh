#!/bin/bash

# Updates the Pi
sudo apt-get update

# Installs Python3
sudo apt-get install python3

# Installs OpenCv
sudo apt-get install python3-opencv

# Installs OpenCv C++ Dependencies
sudo apt-get install libopencv-dev

# Installs Pip3
sudo apt install python3-pip

# Installs the IMU Library
sudo pip3 install adafruit-circuitpython-bno055

# Installs the wiringpi Library
sudo apt-get install wiringpi