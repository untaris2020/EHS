C=gcc
CFLAGS= -O3 -Wall -g
LIBS=
AR=ar

ALLBIN=imu

all: ${ALLBIN}

clean:
	rm -f *.o ${ALLBIN}

imu: i2c_bno055.o imu.o
	$(CC) i2c_bno055.o imu.o -o imu ${LIBS}

cal: calbibrate.o i2c_bno055.o 
	$(cc) i2c_bno055.o calbibrate.o -o cal