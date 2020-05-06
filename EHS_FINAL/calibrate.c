//Calibrate the imu into the state it needs to be in 
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <time.h>
#include "getbno055.h"

char senaddr[256] = "0x28";

void print_calstat();

int main()
{
    int FullyCal = 0;

   
    get_i2cbus(senaddr);

    printf("Calibrating IMU\n");

    struct bnocal cal;

    opmode_t mode = ndof; 

   //Check Mode
   int notMode = 1; 
   int res =0;
   int des_mode = ndof;
   while(notMode)
   {
      int mode = get_mode();
      if(mode != 11) {
         printf("Error sensor mode %d is not a valid mode.Attempting to set correct mode\n", mode);
         res = set_mode(des_mode);
         if(res != 0) {
            printf("Error: could not set sensor mode [0x%02X].\n", des_mode);
            exit(-1);
         }
         sleep(3);
      }
      else
      {
         notMode = 0; 
      }
   }

	//Attempt to load the config file 
	
	res = load_cal("bno055.cal");
		if(res != 0) {
		printf("Error: Cannot load cal file\n");
		exit(-1);
	}


    while(!FullyCal)
    {
        print_calstat();

        sleep(5);
    }
}



void print_calstat() {
   struct bnocal bnoc;
   /* -------------------------------------------------------- *
    *  Check the sensors calibration state                     *
    * -------------------------------------------------------- */
   int res = get_calstatus(&bnoc);
   if(res != 0) {
      printf("Error: Cannot read calibration state.\n");
      exit(-1);
   }

   /* -------------------------------------------------------- *
    *  Convert the status code into a status message           *
    * -------------------------------------------------------- */
    printf("Sensor System Calibration = ");
    switch(bnoc.scal_st) {
      case 0:
         printf("Uncalibrated\n");
         break;
      case 1:
         printf("Minimal Calibrated\n");
         break;
      case 2:
         printf("Mostly Calibrated\n");
         break;
      case 3:
         printf("Fully calibrated\n");
         break;
   }

   printf("    Gyroscope Calibration = ");
   switch(bnoc.gcal_st) {
      case 0:
         printf("Uncalibrated\n");
         break;
      case 1:
         printf("Minimal Calibrated\n");
         break;
      case 2:
         printf("Mostly Calibrated\n");
         break;
      case 3:
         printf("Fully calibrated\n");
         break;
   }

   printf("Accelerometer Calibration = ");
   switch(bnoc.acal_st) {
      case 0:
         printf("Uncalibrated\n");
         break;
      case 1:
         printf("Minimal Calibrated\n");
         break;
      case 2:
         printf("Mostly Calibrated\n");
         break;
      case 3:
         printf("Fully calibrated\n");
         break;
   }

   printf(" Magnetometer Calibration = ");
   switch(bnoc.mcal_st) {
      case 0:
         printf("Uncalibrated\n");
         break;
      case 1:
         printf("Minimal Calibrated\n");
         break;
      case 2:
         printf("Mostly Calibrated\n");
         break;
      case 3:
         printf("Fully calibrated\n");
         break;
   }
}



