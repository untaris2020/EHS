// Project Name:  A.R.I.S. - Augmented Reality Interface System
// University:    University of North Texas
// Developers:    Juan Ruiz and Timothy Stern, Computer Engineering
// Date Created:  04/07/2020

// Purpose:       The purpose of the gloveFlex.c script is to read each of the fingers inputs and send them to the
//				  EHS via TCP.

//                Run this command to compile
//                gcc gloveFlex.c -o gloveFlex -lwiringPi


#include <wiringPi.h>
#include <mcp3004.h>

#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

// Socket Libraries
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

#define BUFSIZE 1024
#define ID 0
#define portno 6002
#define hostname "127.0.0.1"

//BASE is a new pin base for the chip of the analog pins
#define BASE 200

// Defining each of the ADC channels for the fingers
#define CHAN0 0
#define CHAN1 1
#define CHAN2 2
#define CHAN3 3
#define CHAN4 4

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
	struct GloveIMUData
    {
      int32_t thumbFinger;
      int32_t indexFinger;
      int32_t middleFinger;
      int32_t ringFinger;
      int32_t littleFinger;
    } data;

	// Checks if it is able to detect any sensors
    if(wiringPiSetup() == -1)
    {
        exit(1);
    }

	// Declaration of boolean variables for debouncing
    mcp3004Setup(BASE, CHAN0); // 3004 and 3008 are the same 4/8 channels
 	  bool thumbStatus = false;
    bool indexStatus = false;
    bool middleStatus = false;
    bool ringStatus = false;
    bool littleStatus = false;

    bool debouncer = false;

    // Select variable declaration
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

	   printf("Running loop -- select\n");

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
      printf("Trying to send data\n");
			//Send data
      if(seqID < 2147483647)
				seqID++;
			else
				seqID = 0;

			bzero(buf, BUFSIZE); //Zero out buffer

			//Insert data here
            int thumbFinger = analogRead(BASE + CHAN1);
            int indexFinger = analogRead(BASE + CHAN0);
            int middleFinger = analogRead(BASE + CHAN4);
            int ringFinger = analogRead(BASE + CHAN3);
            int littleFinger = analogRead(BASE + CHAN2);

            if(thumbFinger > 50)
            {
              data.thumbFinger = 1;
              thumbStatus = true;
            }
            else
            {
              data.thumbFinger = 0;
				      thumbStatus = false;
            }

            if(indexFinger > 50)
            {
              data.indexFinger = 1;
				      indexStatus = true;
            }
            else
            {
              data.indexFinger = 0;
				      indexStatus = false;
            }

            if(middleFinger > 50)
            {
              data.middleFinger = 1;
				      middleStatus = true;
            }
            else
            {
              data.middleFinger = 0;
				      middleStatus = false;
            }

            if(ringFinger > 50)
            {
              data.ringFinger = 1;
              ringStatus = true;
            }
            else
            {
              data.ringFinger = 0;
				      ringStatus = false;
            }

            if(littleFinger > 50)
            {
              data.littleFinger = 1;
				      littleStatus = true;
            }
            else
            {
              data.littleFinger = 0;
				      littleStatus = false;
            }

			/*READ ME: Packet format
			* When sending data each packet will be truncated with an <EOF> string and started with a <BEG> string
			* Then data is input between using the $ char delimeter
			* When possible include a char delimiter so data can be pulled off in the highest priority
			* An example is shown below */

			snprintf(buf, sizeof(buf), "<BEG>%d$%d$%d$%d$%d$%d<EOF>", ID,
                                                                data.thumbFinger,
                                                                data.indexFinger,
                                                                data.middleFinger,
                                                                data.ringFinger,
                                                                data.littleFinger);

			int n;
			if((thumbStatus == true && (indexStatus == true || middleStatus == true || ringStatus == true || littleStatus == true)) && debouncer == false)
			{
				//cout << "<BEG>" + finger0 + finger1 + finger2 + finger3 + finger4 + "<EOF>" << endl;
				n = write(sockfd, buf, strlen(buf));
				debouncer = true;

			}
			else if((thumbStatus == false && indexStatus == false && middleStatus == false && ringStatus == false && littleStatus == false) && debouncer == true)
			{
				n = write(sockfd, buf, strlen(buf));
				//cout << "<BEG>" + finger0 + finger1 + finger2 + finger3 + finger4 + "<EOF>" << endl;
				debouncer = false;

			}

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

	printf("Connected...\n");
	return 0;
}
