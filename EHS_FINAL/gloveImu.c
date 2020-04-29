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
#include <math.h>
#include <stdarg.h>
#include <assert.h>
#include "getbno055.h"



#define BUFSIZE 1024
#define ID 2
float xPos, yPos, zPos, linx, liny, linz, newxpos, newypos, newzpos;
float aAvx = 0;
float aAvy = 0;
float aAvz = 0;
float lAv = 0;
float lAvx = 0;
float lAvy = 0;
float lAvz = 0;
char senaddr[256] = "0x28";
int first = 1;

int connect_to_server(int32_t * sockfd);

void streamData(int32_t sockfd);
void kalmanFilter();

typedef struct {
  /* Dimensions */
  int rows;
  int cols;

  /* Contents of the matrix */
  float** data;
} Matrix;

Matrix alloc_matrix(int rows, int cols);
void set_matrix(Matrix m, ...);
void multiply_matrix(Matrix a, Matrix b, Matrix c);
void add_matrix(Matrix a, Matrix b, Matrix c);
void print_matrix(Matrix m);
void subtract_matrix(Matrix a, Matrix b, Matrix c);

int main()
{
    int32_t sockfd;
    int res = 0;
    char buffer[BUFSIZE];

    get_i2cbus(senaddr);

	//Check Mode
	int notMode = 1;
	int des_mode = ndof;
 /*
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
   */

	//Attempt to load the config file

	res = load_cal("bno055.cal");
		if(res != 0) {
		printf("Error: Cannot load cal file\n");
		exit(-1);
	}






  //Comment out line when doing testing with system
  //printData();
    kalmanFilter();



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
			/*int mode = get_mode();
              if(mode < 8) {
                 //printf("Error getting Quaternation, sensor mode %d is not a fusion mode.\n", mode);
                 opmode_t newmode = compass; //compass = 0x09
                 res = set_mode(newmode);
                      if(res != 0) {
                         printf("Error: could not set sensor mode\n");
                         exit(-1);
                      }

              }
            */

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
        //Uncomment this line when ready to start reading
            kalmanFilter();
			/*READ ME: Packet format
			* When sending data each packet will be truncated with an <EOF> string and started with a <BEG> string
			* Then data is input between using the $ char delimeter
			* When possible include a char delimiter so data can be pulled off in the highest priority
			* An example is shown below */
      //printf("quarter w: %d\n", bnodQ.quater_w);
			if(!ERR_FLAG)
			{
				snprintf(buf, sizeof(buf), "<BEG>%d$%d$%f$%f$%f$%f$%f$%f$%f<EOF>", ID,
																			seqID,
																			xPos*100,
																			yPos*100,
																			zPos*100,
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

	printf("Glove IMU Connected...\n");
	return 0;
}
void printData(){

    struct bnoacc bnodA;
    struct bnoqua bnodQ;
    struct bnolin bnodL;
    float normal;
    while(1){


      for(int i=0; i < 50; i++){

      }
      for(int i=0; i < 50; i++){

      }
      for(int i=0; i < 50; i++){

      }
      get_acc(&bnodA);
        printf("Accelerometer Data (x,y,x): %f %f %f\n", bnodA.adata_x, bnodA.adata_y, bnodA.adata_z);
        //sleep(0.1);
      get_lin(&bnodL);
        printf("Linear Accelerometer Data (x,y,x): %f %f %f\n", bnodL.linacc_x, bnodL.linacc_y, bnodL.linacc_z);
        //sleep(0.1);
      get_qua(&bnodQ);
        printf("Quaternion Data (x,y,z,w): %f %f %f %f\n", bnodQ.quater_x, bnodQ.quater_y, bnodQ.quater_z, bnodQ.quater_w);
        //sleep(0.1);



      //normal = sqrt(

      sleep(0.5);
    }

}
void kalmanFilter(){
    struct bnoacc bnodA;
    struct bnolin bnodL;

    get_acc(&bnodA);
    get_lin(&bnodL);

    if(first){

        for(int i=0; i < 1000; i++){
        get_acc(&bnodA);
        aAvx += bnodA.adata_x;
        aAvy += bnodA.adata_y;
        aAvz += bnodA.adata_z;

        }

          aAvx/=1000;
          aAvy/=1000;
          aAvz/=1000;

        for(int i=0; i < 1000; i++){
            get_lin(&bnodL);
            lAvx += bnodL.linacc_x;
            lAvy += bnodL.linacc_y;
            lAvz += bnodL.linacc_z;

          }
          lAvx/=1000;
          lAvy/=1000;
          lAvz/=1000;

          first = 0;

    }
    float BNO055_SAMPLERATE_DELAY_MS = 10;
    float ACCEL_VEL_TRANSITION  = BNO055_SAMPLERATE_DELAY_MS/1000; //converting to microseconds
    float ACCEL_POS_TRANSITION = 0.5 * ACCEL_VEL_TRANSITION * ACCEL_VEL_TRANSITION;
    xPos = xPos + ACCEL_POS_TRANSITION * bnodL.linacc_x;
    yPos = yPos + ACCEL_POS_TRANSITION * bnodL.linacc_y;
    zPos = zPos + ACCEL_POS_TRANSITION * bnodL.linacc_z;

    Matrix matrix;
    matrix = alloc_matrix(6,1);

    Matrix state;
    state = alloc_matrix(6,6);
    Matrix controlV;
    controlV = alloc_matrix(3, 1);
    Matrix controlM;
    controlM = alloc_matrix(6, 3);
    Matrix tempState;
    tempState = alloc_matrix(6, 1);
    Matrix tempDev;
    tempDev = alloc_matrix(6,1);

    /*
                        bnodA.adata_x,
                        bnodA.adata_y,
                        bnodA.adata_z,
                        bnodL.linacc_x,
                        bnodL.linacc_y,
                        bnodL.linacc_z);
    */

    set_matrix(matrix, xPos,
                        yPos,
                        zPos,
                        ACCEL_POS_TRANSITION,
                        ACCEL_POS_TRANSITION,
                        ACCEL_POS_TRANSITION);

    set_matrix(state, 1.0, 0.0, 0.0, 0.01, 0.0, 0.0,
                      0.0, 1.0, 0.0, 0.0, 0.01, 0.0,
                      0.0, 0.0, 1.0, 0.0, 0.0, 0.01,
                      0.0, 0.0, 0.0, 1.0, 0.0, 0.0,
                      0.0, 0.0, 0.0, 0.0, 1.0, 0.0,
                      0.0, 0.0, 0.0, 0.0, 0.0, 1.0);




    set_matrix(controlV, lAvx, lAvy, lAvz);

    set_matrix(controlM, ACCEL_POS_TRANSITION, 0.0, 0.0,
                         0.0, ACCEL_POS_TRANSITION, 0.0,
                         0.0, 0.0, ACCEL_POS_TRANSITION,
                         ACCEL_VEL_TRANSITION, 0.0, 0.0,
                         0.0, ACCEL_VEL_TRANSITION, 0.0,
                         0.0, 0.0, ACCEL_VEL_TRANSITION);

    /*
    matrix: 6 rows 1 column
    State:6 rows 6 columns
    controlV 3 rows 1 column
    controlM 6 rows 3 columns
    tempState 6 rows 1 column
    tempDev 6 rows 1 column


    */
    //assert(a.cols == b.rows);
    //assert(a.rows == c.rows);
    //assert(b.cols == c.cols);

    xPos = xPos + ACCEL_POS_TRANSITION * bnodL.linacc_x;
    yPos = yPos + ACCEL_POS_TRANSITION * bnodL.linacc_y;
    zPos = zPos + ACCEL_POS_TRANSITION * bnodL.linacc_z;

    set_matrix(matrix, xPos,
                    yPos,
                    zPos,
                    ACCEL_POS_TRANSITION,
                    ACCEL_POS_TRANSITION,
                    ACCEL_POS_TRANSITION);


    //printf("\nValues before execution\n");
    //print_matrix(matrix);
    multiply_matrix(state, matrix, tempState);
    multiply_matrix(controlM, controlV, tempDev);
    //assert(a.rows == b.rows);
    //assert(a.rows == c.rows);
    //assert(a.cols == b.cols);
    //assert(a.cols == c.cols);
    //add_matrix(tempState, tempDev, matrix);
    subtract_matrix(tempState, tempDev, matrix);
    //printf("\nValues After Execution\n");
    //print_matrix(matrix);


    xPos = matrix.data[0][0];
    yPos = matrix.data[0][1];
    zPos = matrix.data[0][2];
    printf("X data: %f \n", matrix.data[0][0]);
    printf("Y data: %f \n", matrix.data[0][1]);
    printf("Z data: %f \n", matrix.data[0][2]);


    /*
    double v2p = 0.001;
    set_identity_matrix(f.state_transition);
    set_seconds_per_timestep(f, time_diff(before , after); );

    set_matrix(f.observation_model,
	     1.0, 0.0, 0.0, 0.0, 0.0, 0.0,
	     0.0, 1.0, 0.0, 0.0, 0.0, 0.0,
	     0.0, 0.0, 1.0, 0.0, 0.0, 0.0);


    double pos = 0.000001;
    set_matrix(f.process_noise_covariance,
	     xpos, 0.0, 0.0, 0.0, 0.0, 0.0,
	     0.0, ypos, 0.0, 0.0, 0.0, 0.0,
	     0.0, 0.0, zpos, 0.0, 0.0, 0.0,
	     0.0, 0.0, 0.0, 1.0, 0.0, 0.0,
	     0.0, 0.0, 0.0, 0.0, 1.0, 0.0,
	     0.0, 0.0, 0.0, 0.0, 0.0, 1.0);

    set_matrix(f.observation_noise_covariance,
	     pos * noise, 0.0,
	     0.0, pos * noise);

	set_matrix(f.state_estimate, 0.0, 0.0, 0.0);
    set_identity_matrix(f.estimate_covariance);
    double trillion = 1000.0 * 1000.0 * 1000.0 * 1000.0;
    scale_matrix(f.estimate_covariance, trillion);
    */
}
double time_diff(struct timeval x , struct timeval y)
{
	double x_ms , y_ms , diff;

	x_ms = (double)x.tv_sec*1000000 + (double)x.tv_usec;
	y_ms = (double)y.tv_sec*1000000 + (double)y.tv_usec;

	diff = (double)y_ms - (double)x_ms;

	return diff;
}
Matrix alloc_matrix(int rows, int cols) {
  Matrix m;
  m.rows = rows;
  m.cols = cols;
  m.data = (float**) malloc(sizeof(float*) * m.rows);
  for (int i = 0; i < m.rows; ++i) {
    m.data[i] = (float*) malloc(sizeof(float) * m.cols);
    assert(m.data[i]);
    for (int j = 0; j < m.cols; ++j) {
      m.data[i][j] = 0.0;
    }
  }
  return m;
}
void set_matrix(Matrix m, ...) {
  va_list ap;
  va_start(ap, m);

  for (int i = 0; i < m.rows; ++i) {
    for (int j = 0; j < m.cols; ++j) {
      m.data[i][j] = va_arg(ap, double);
    }
  }

  va_end(ap);
}
void multiply_matrix(Matrix a, Matrix b, Matrix c) {

  assert(a.cols == b.rows);
  assert(a.rows == c.rows);
  assert(b.cols == c.cols);
  for (int i = 0; i < c.rows; ++i) {
    for (int j = 0; j < c.cols; ++j) {
      /* Calculate element c.data[i][j] via a dot product of one row of a
	 with one column of b */
      c.data[i][j] = 0.0;
      for (int k = 0; k < a.cols; ++k) {
      	c.data[i][j] += a.data[i][k] * b.data[k][j];
      }
    }
  }
}
void add_matrix(Matrix a, Matrix b, Matrix c) {
  assert(a.rows == b.rows);
  assert(a.rows == c.rows);
  assert(a.cols == b.cols);
  assert(a.cols == c.cols);
  for (int i = 0; i < a.rows; ++i) {
    for (int j = 0; j < a.cols; ++j) {
      c.data[i][j] = a.data[i][j] + b.data[i][j];
    }
  }
}
void print_matrix(Matrix m) {
  for (int i = 0; i < m.rows; ++i) {
    for (int j = 0; j < m.cols; ++j) {
      if (j > 0) {
	      printf(" ");
      }
      printf("%f", m.data[i][j]);
    }
    printf("\n");
  }
}
void subtract_matrix(Matrix a, Matrix b, Matrix c) {
  assert(a.rows == b.rows);
  assert(a.rows == c.rows);
  assert(a.cols == b.cols);
  assert(a.cols == c.cols);
  for (int i = 0; i < a.rows; ++i) {
    for (int j = 0; j < a.cols; ++j) {
      c.data[i][j] = a.data[i][j] - b.data[i][j];
    }
  }
}
