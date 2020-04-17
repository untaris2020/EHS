// Project Name:  A.R.I.S. - Augmented Reality Interface System
// University:    University of North Texas
// Developers:    Juan Ruiz and Timothy Stern, Computer Engineering
// Date Created:  04/07/2020

// Purpose:       The purpose of the gloveCam.cpp script is to read each of the frame, compress them via JPEG
//                Encode them using base64, and send them to the EHS via TCP.

//                Run this command to compile - Must have the base64.cpp and base64.h files
//                g++ gloveCam.cpp base64.cpp -o gloveCam `pkg-config --cflags --libs opencv`




#include "/home/pi/EHS/globalSettings.h"
#include "base64.h"
#include <iostream>
#include <vector>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

// OpenCV Libraries
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// Socket Libraries
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h> 

int connect_to_server(int32_t * sockfd); 
void streamData(int32_t sockfd); 

using namespace std;
using namespace cv;

#define BUFSIZE 20000
#define ID 5

// Sets the frame width and height
#define width 320
#define height 240

// Initializing the OpenCV camera
Mat frame;
VideoCapture cap;

int main() 
{
    int32_t sockfd;
    char buffer[BUFSIZE];

	
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
		//int n = write(sockfd, buffer, strlen(buffer));
    int n = send(sockfd, buffer, strlen(buffer), 0);

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
    
	// Frame compression to JPEG
	vector<uchar> buff;
	vector<int> param(2);
	param[0] = IMWRITE_JPEG_QUALITY;
	param[1] = 40;

	// Sets the frame width and height
	cap.open(0);
	cap.set(3, width);
	cap.set(4, height);

	uint32_t seqID = 0; 
	uint32_t STREAM = 0;
	char buf[BUFSIZE];
	fd_set rfds; 
    struct timeval tv;
	
	while(1)
	{
	   FD_SET(sockfd,&rfds); 
	   tv.tv_sec = 0; 
	   tv.tv_usec = 16666;
	
	   //printf("Running loop -- select\n");
	    
	   if(select(sockfd+1, &rfds, NULL, NULL, &tv) == -1)
	   {
		  perror("Select Error");
	   }

	   if (FD_ISSET(sockfd, &rfds))
	   {
		   printf("Data Ready...\n");

		   //Data is ready to be read
		   bzero(buf, BUFSIZE);
		   int count = recv(sockfd, buf, sizeof(buf), 0);

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
			   printf("Detected Disconnect\n");
			   return; 
		   }
	   }
	   else if(STREAM)
	   {
      
      //printf("Trying to send data\n");
			//Send data   
			if(seqID < 2147483647)
				seqID++;
			else
				seqID = 0; 
	   
			bzero(buf, BUFSIZE); //Zero out buffer

			//Insert data here
            /*
			data.x = 1;
			data.y = 2;
			data.z = 3;
			data.xGyro = 4;
			data.yGyro = 5;
			data.zGyro = 6;
            */

            cap.read(frame);                                  // Wait for a new frame from camera and store it into 'frame'

            imencode(".jpg", frame, buff, param);             // Compresses each frame to JPG

            string temp;
            for(int i = 0; i < buff.size(); i++)
            {
                temp += buff[i];
            }

            string encoded = base64_encode(reinterpret_cast<const unsigned char*>(temp.c_str()), temp.length());      // Encodes the frame into base64

            //int size = encoded.length();                      // Determines the length of each base64 frame

			/*READ ME: Packet format 
			* When sending data each packet will be truncated with an <EOF> string and started with a <BEG> string
			* Then data is input between using the $ char delimeter
			* When possible include a char delimiter so data can be pulled off in the highest priority 
			* An example is shown below */

			snprintf(buf, sizeof(buf), "<BEG>%d$%s<EOF>", ID, encoded.c_str()); 

			//int n = write(sockfd, buf, strlen(buf));
            int n = send(sockfd, buf, strlen(buf), 0);

			if(n < 0)
			{
				perror("ERROR writing to socket");
			}		
	   }
	   //sleep(1);
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

	printf("Glove Cam Connected...\n");
	return 0;
}