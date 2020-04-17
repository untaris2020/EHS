// Project Name:  A.R.I.S. - Augmented Reality Interface System
// University:    University of North Texas
// Developers:    Juan Ruiz and Timothy Stern, Computer Engineering
// Date Created:  04/07/2020

// Purpose:       The purpose of the gloveButton.c script is to read the status of the reset button and send the status to the
//				  EHS via TCP.

//                Run this command to compile
//                gcc chestButton.c -o chestButton -lwiringPi


#include "/home/pi/EHS/globalSettings.h"
#include <wiringPi.h>
#include <stdbool.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <time.h>

#define gpioPORT 7
#define BUFSIZE 1024
#define ID 0 
 
int connect_to_server(int32_t * sockfd);
void streamData(int32_t sockfd); 

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
	//Declaration of variables for Button
    wiringPiSetup();
    pinMode(gpioPORT, INPUT);
	bool status = false;

	struct GloveIMUData
    {
	  char * status; 
    } data;

	uint32_t seqID = 0; 
	uint32_t STREAM = 0;
	char buf[BUFSIZE];
	fd_set rfds; 
    struct timeval tv;
	
	while(1)
	{
	   FD_SET(sockfd,&rfds); 
	   tv.tv_sec = 0; 
	   tv.tv_usec = 1;
	
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


			// If the gpioPORT is 1 (if the button is toggled) then it will send an INACTIVE message to the server letting
			// it know that it has some issues and that is resetting the pinMode
			// if the gpioPORT is 0 it means that everything is running smoothly
            if(digitalRead(gpioPORT) == 1 && status == false)
            {
                data.status = "INACTIVE";
				status = true;
            }
            else if(digitalRead(gpioPORT) == 0 && status == true)
            {
                data.status = "ACTIVE";
				status = false;
            }

			/*READ ME: Packet format 
			* When sending data each packet will be truncated with an <EOF> string and started with a <BEG> string
			* Then data is input between using the $ char delimeter
			* When possible include a char delimiter so data can be pulled off in the highest priority 
			* An example is shown below */

			snprintf(buf, sizeof(buf), "<BEG>%d$%s<EOF>", ID, data.status); 

			int n = write(sockfd, buf, strlen(buf));

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

	printf("Chest Button Connected...\n");
	return 0;
}