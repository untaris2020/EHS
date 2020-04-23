/*
 *  * tcpclient.c - A simple TCP client
 *   * usage: tcpclient <host> <port>
 *    */
#include "/home/pi/EHS/globalSettings.h"
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

#define BUFSIZE 1024
#define ID 2

char senaddr[256] = "0x28";


int connect_to_server(int32_t * sockfd);

void streamData(int32_t sockfd);

int main()
{
    int32_t sockfd;
    int res = 0;
    char buffer[BUFSIZE];

    get_i2cbus(senaddr);

	//Check Mode
	int notMode = 1;
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

	//Cal should be loaded. Verify calibration


	// int notCal = 1;
	// while(notCal)
	// {
	// 	struct bnocal bnoc;
	// 	res = get_calstatus(&bnoc);
	// 		if(res != 0) {
	// 		printf("Error: Cannot read calibration state.\n");
	// 		exit(-1);
	// 	}

	// 	printf("\nSensor System Calibration = ");

	// 	printf("Gyroscope Calibration = ");
	// 	switch(bnoc.gcal_st) {
	// 		case 0:
	// 			printf("Gyroscope Uncalibrated\n");
	// 			break;
	// 		case 1:
	// 			printf("Gyroscope Minimal Calibrated\n");
	// 			break;
	// 		case 2:
	// 			printf("Gyroscope Mostly Calibrated\n");
	// 			break;
	// 		case 3:
	// 			printf("Gyroscope Fully calibrated\n");
	// 			printf("Accelerometer Calibration = ");
	// 			switch(bnoc.acal_st) {
	// 				case 0:
	// 					printf("Accelerometer Uncalibrated\n");
	// 					break;
	// 				case 1:
	// 					printf("Accelerometer Minimal Calibrated\n");
	// 					break;
	// 				case 2:
	// 					printf("Accelerometer Mostly Calibrated\n");
	// 					break;
	// 				case 3:
	// 					printf("Accelerometer Fully calibrated\n");
	// 					printf("Magnetometer Calibration = ");
	// 					switch(bnoc.mcal_st) {
	// 						case 0:
	// 							printf("Magnetometer Uncalibrated\n");
	// 							break;
	// 						case 1:
	// 							printf("Magnetometer Minimal Calibrated\n");
	// 							break;
	// 						case 2:
	// 							printf("Magnetometer Mostly Calibrated\n");
	// 							break;
	// 						case 3:
	// 							printf("Magnetometer Fully calibrated\n");
	// 							switch(bnoc.scal_st) {
	// 							case 0:
	// 								printf("System Uncalibrated\n");
	// 								break;
	// 							case 1:
	// 								printf("System Minimal Calibrated\n");
	// 								break;
	// 							case 2:
	// 								printf("System Mostly Calibrated\n");
	// 								break;
	// 							case 3:
	// 								printf("System Fully calibrated\n");
	// 								notCal = 0;
	// 								break;
	// 					}
	// 					break;
	// 			}
	// 			break;
	// 	}
	// 	break;
	// 	}
	// 	sleep(3);
	// }

	while(1)
	{
		//run the connection until one is established
		while(connect_to_server(&sockfd))
		{
			//Give the server a little time
			sleep(1);
		}

		/*Sending first time registration message*/
		snprintf(buffer, sizeof(buffer), "<BEG>%d$%s<EOF>", ID, "REG");
		int n = write(sockfd, buffer, strlen(buffer));

		if(n < 0)
		{
			perror("ERROR writing to socket");
			return -1;
		}

		streamData(sockfd);
	}

    return 0;
}

void streamData(int32_t sockfd)
{
	int res = -1;
	int ERR_FLAG = 0;

	uint32_t seqID = 0;
	uint32_t STREAM = 0;
	char buf[BUFSIZE];
	fd_set rfds;
    struct timeval tv;

	while(1)
	{
       struct bnoacc bnodA;
	   struct bnoqua bnodQ;
	   FD_SET(sockfd,&rfds);
	   tv.tv_sec = 0;
	   tv.tv_usec = 1;

	   if(select(sockfd+1, &rfds, NULL, NULL, &tv) == -1)
	   {
		  perror("Select Error");
	   }

	   if (FD_ISSET(sockfd, &rfds))
	   {
		   //Data is ready to be read

		   bzero(buf, BUFSIZE);
		   recv(sockfd, buf, sizeof(buf), 0);

		   if(!strncmp(buf, "START", 5))
		   {
				printf("Stream started\n");
				STREAM = 1;
		   }
		   else if(!strncmp(buf, "STOP", 5))
		   {
				printf("Stream stopped\n");
				STREAM = 0;
		   }
		   else if(!strcmp(buf, ""))
		   {
			   return;
		   }
	   }
	   else if(STREAM)
	   {
			//Send data
			if(seqID < 2147483647)
				seqID++;
			else
				seqID = 0;

			bzero(buf, BUFSIZE); //Zero out buffer

			// int mode = get_mode();
			// if(mode < 8) {
			// 	printf("Error getting Quaternation, sensor mode %d is not a fusion mode.\n", mode);
			// 	ERR_FLAG = 1;
			// }
			// else
			// {


			//checking if mode is set correctly
			int mode = get_mode();
              if(mode < 8) {
                 //printf("Error getting Quaternation, sensor mode %d is not a fusion mode.\n", mode);
                 opmode_t newmode = compass; //compass = 0x09
                 res = set_mode(newmode);
                      if(res != 0) {
                         printf("Error: could not set sensor mode\n");
                         exit(-1);
                      }

              }
				res = get_acc(&bnodA);
				if(res != 0) {
					printf("Error: Cannot read accelerometer data.\n");
					ERR_FLAG = 1;
				}

				// res = get_eul(&bnodE);
				// if(res != 0) {
				// 	printf("Error: Cannot read Euler orientation data.\n");
				// 	exit(-1);
				// }

				res = get_qua(&bnodQ);
				if(res != 0) {
					printf("Error: Cannot read Quaternation data.\n");
					ERR_FLAG = 1;
				}
			//}

			/*READ ME: Packet format
			* When sending data each packet will be truncated with an <EOF> string and started with a <BEG> string
			* Then data is input between using the $ char delimeter
			* When possible include a char delimiter so data can be pulled off in the highest priority
			* An example is shown below */

			if(!ERR_FLAG)
			{
				snprintf(buf, sizeof(buf), "<BEG>%d$%d$%f$%f$%f$%f$%f$%f$%f<EOF>", ID,
																			seqID,
																			bnodA.adata_x,
																			bnodA.adata_y,
																			bnodA.adata_z,
																			bnodQ.quater_w,
																			bnodQ.quater_x,
																			bnodQ.quater_y,
																			bnodQ.quater_z);

				int n = write(sockfd, buf, strlen(buf));

				if(n < 0)
				{
					perror("ERROR writing to socket");
				}
			}
			sleep(.02);
	   }
	}
}

int connect_to_server(int32_t * sockfd)
{
	struct sockaddr_in serveraddr;
    struct hostent *server;

	printf("Connecting to IP: %s on PORT: %d\n", hostname, portno);

    /* socket: create the socket */
    *sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (*sockfd < 0)
    {
	    perror("ERROR opening socket");
		return 1;
    }

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);

    if (server == NULL)
    {
       fprintf(stderr,"ERROR, no such host as %s\n", hostname);
       return 1;
    }

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);

    /* connect: create a connection with the server */
    if (connect(*sockfd, (const struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
    {
       perror("ERROR connecting");
	   return 1;
    }

	printf("Chest IMU Connected...\n");
	return 0;
}


