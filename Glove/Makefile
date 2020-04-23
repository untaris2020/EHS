all: gloveCam gloveButton gloveFlex

gloveCam: gloveCam.cpp base64.cpp
	g++ -o gloveCam gloveCam.cpp base64.cpp `pkg-config --cflags --libs opencv`

gloveButton: gloveButton.c
	gcc -o gloveButton gloveButton.c -lwiringPi

gloveFlex: gloveFlex.c
	gcc -o gloveFlex gloveFlex.c -lwiringPi