ALLBIN=chestImu gloveImu chestButton chestCam gloveFlex gloveCam gloveButton cal

chest: TARGET=chest
glove: TARGET=glove

chest: CHEST
glove: GLOVE

clean:
	rm -f *.o ${ALLBIN}

GLOVE: gloveCam gloveButton gloveFlex gloveImu cal

CHEST: chestCam chestButton chestImu cal

gloveCam: gloveCam.cpp base64.cpp
	g++ -o gloveCam gloveCam.cpp base64.cpp `pkg-config --cflags --libs opencv`

gloveButton: gloveButton.c
	gcc -o gloveButton gloveButton.c -lwiringPi

gloveFlex: gloveFlex.c
	gcc -o gloveFlex gloveFlex.c -lwiringPi

gloveImu: i2c_bno055.c gloveImu.c
	gcc -o gloveImu i2c_bno055.c gloveImu.c

chestCam: chestCam.cpp base64.cpp
	g++ -o chestCam chestCam.cpp base64.cpp `pkg-config --cflags --libs opencv`

chestButton: chestButton.c
	gcc -o chestButton chestButton.c -lwiringPi

chestImu: i2c_bno055.c chestImu.c
	gcc -o chestImu i2c_bno055.c chestImu.c

cal: calibrate.c i2c_bno055.c
	gcc -o cal calibrate.c i2c_bno055.c 


