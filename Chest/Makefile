
all: chestCam.cpp base64.cpp chestButton.c
	g++ chestCam.cpp base64.cpp -o chestCam `pkg-config --cflags --libs opencv`
	gcc chestButton.c -o chestButton -lwiringPi

clean:
	$(RM) chestCam chestButton

