
all: gloveCam.cpp base64.cpp gloveButton.c gloveFlex.c
	g++ gloveCam.cpp base64.cpp -o gloveCam `pkg-config --cflags --libs opencv`
	gcc gloveButton.c -o gloveButton -lwiringPi
	gcc gloveFlex.c -o gloveFlex -lwiringPi

clean:
	$(RM) gloveCam gloveButton gloveFlex

