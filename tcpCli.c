/* 
 *  * tcpclient.c - A simple TCP client
 *   * usage: tcpclient <host> <port>
 *    */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <time.h>

#define BUFSIZE 1024

/* 
 *  * error - wrapper for perror
 *   */
void error(char *msg) 
{
	    perror(msg);
	        exit(0);
}

int main() 
{
    int32_t sockfd, n;
    uint32_t portno = 4050;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char * hostname = "192.168.1.123";
    char buf[BUFSIZE];

    int STREAM = 0; 

    fd_set rfds, wfds; 
    struct timeval tv;
    int retval; 

    uint32_t seqID = 0; 

    struct GloveIMUData
    {
	  int32_t x;
	  int32_t y;
	  int32_t z; 
	  int32_t xGyro; 
	  int32_t yGyro; 
	  int32_t zGyro;   
    } data; 

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
    {
	    error("ERROR opening socket");
    }				    

    printf("Socket created...Getting host by name\n");
    
    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);

    if (server == NULL) 
    {
       fprintf(stderr,"ERROR, no such host as %s\n", hostname);
       exit(0);
    }

        printf("Got host by name attempting to write addr\n");

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);
    printf("Connecting\n");


    /* connect: create a connection with the server */
    if (connect(sockfd, &serveraddr, sizeof(serveraddr)) < 0) 
    {
       error("ERROR connecting");
    }
    printf("Connected\n");

      /*Introduce yourself*/ 
      snprintf(buf, sizeof(buf), "<BEG>%d$%d$%d$%d$%d$%d$%d$%d<EOF>", 1, -1, -1, -1, -1, -1, -1, -1); 

      n = write(sockfd, buf, strlen(buf));

      if(n < 0)
      {
         error("ERROR writing to socket");
      }		


    int i = 0; 
									    
    while(1)
    {
       //First we check if socket has data or can be sent data 
       FD_ZERO(&rfds);
       FD_SET(sockfd, &rfds);	
       tv.tv_sec = 0; 
       tv.tv_usec = 5;

       if(select(sockfd+1, &rfds, NULL, NULL, &tv) == -1)
       {
          error("Select Error");
       }

       if (FD_ISSET(sockfd, &rfds))
       {
	   printf("Received Msg\n");
           //Data is ready to be read
	   char buffer[256]; 
	   bzero(buffer, 256);
	   int count = recv(sockfd, buffer, sizeof(buffer), 0);
	
	   if(!strncmp(buffer, "START", 5))
	   {
		printf("Stream started\n");   
	   	STREAM = 1; 
	   }
	   else if(!strncmp(buffer, "STOP", 5))
	   {
	        STREAM = 0;
	   }

       }
       else if(STREAM)
       {
          	//Send data   
      
       		if(seqID < 4294967295)
       		{
          		seqID++;
       		}
       		else
       		{
          		seqID = 0; 
       		}
       
       		bzero(buf, BUFSIZE); //Zero out buffer

       		//Insert data here
       		data.x = 1;
       		data.y = 2;
       		data.z = 3;
       		data.xGyro = 4;
       		data.yGyro = 5;
       		data.zGyro = 6;


       		/*READ ME: Packet format 
		* When sending data each packet will be truncated with an <EOF> string and started with a <BEG> string
		* Then data is input between using the $ char delimeter
		* When possible include a char delimiter so data can be pulled off in the highest priority 
		* An example is shown below */

       		snprintf(buf, sizeof(buf), "<BEG>%d$%d$%d$%d$%d$%d$%d<EOF>", seqID, data.x, data.y,data.z, data.xGyro, data.yGyro, data.zGyro); 


       		n = write(sockfd, buf, strlen(buf));

       		if(n < 0)
       		{
          		error("ERROR writing to socket");
       		}		
       		sleep(.1);  
       }
    }
    return 0;
}
