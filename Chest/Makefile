all: chestCam chestButton

chestCam: chestCam.cpp base64.cpp
	g++ -o chestCam chestCam.cpp base64.cpp `pkg-config --cflags --libs opencv`

chestButton: chestButton.c
	gcc -o chestButton chestButton.c -lwiringPi