#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <netdb.h>
#include <sys/socket.h>
#include "getbno055.h"

#define PORT 4060
#define SA struct sockaddr

/* ------------------------------------------------------------ *
 * Global variables and defaults                                *
 * ------------------------------------------------------------ */
int verbose = 0;
int outflag = 0;
int argflag = 0; // 1 dump, 2 reset, 3 load calib, 4 write calib
char opr_mode[9] = {0};
char pwr_mode[8] = {0};
char datatype[256];
char senaddr[256] = "0x28";
char htmfile[256];
char calfile[256];

typedef enum Type
{
    SYSTEM = 0,
    EHS_IMU = 1,
    GLOVE_IMU = 2,
    TOGGLE_SCREEN = 3,
    HEAD_CAM = 4,
    GLOVE_CAM = 5,
    FORCE_SENSOR = 6
}Type;

struct Header
{
    Type type; //Type of data packet
    uint size; //size of data packet in bits
    uint time; //Unix epoch time
    uint seqID; //Sequence ID of the packet
};


int main(int argc, char *argv[]) {

   int res = -1;       // res = function retcode: 0=OK, -1 = Error
   int sequenceID = 1;
   /* ---------------------------------------------------------- *
    * Process the cmdline parameters                             *
    * ---------------------------------------------------------- */
   //parseargs(argc, argv);

   /* ----------------------------------------------------------- *
    * get current time (now), write program start if verbose      *
    * ----------------------------------------------------------- */
   time_t tsnow = time(NULL);
   if(verbose == 1) printf("Debug: ts=[%lld] date=%s", (long long) tsnow, ctime(&tsnow));

   /* ----------------------------------------------------------- *
    * "-a" open the I2C bus and connect to the sensor i2c address *
    * ----------------------------------------------------------- */
   get_i2cbus(senaddr);


   /* ----------------------------------------------------------- *
    *  "-t acc " reads accelerometer data from the sensor.        *
    * ----------------------------------------------------------- */
   //if(strcmp(datatype, "acc") == 0) {
      struct bnoacc bnodA;
      res = get_acc(&bnodA);
      if(res != 0) {
         printf("Error: Cannot read accelerometer data.\n");
         exit(-1);
      }

      /* ----------------------------------------------------------- *
       * print the formatted output string to stdout (Example below) *
       * ACC -45.00 264.00 939.00 (ACC X Y Z)                        *
       * ----------------------------------------------------------- */
      printf("ACC %3.2f %3.2f %3.2f\n", bnodA.adata_x, bnodA.adata_y, bnodA.adata_z);


   //} /* End reading Accelerometer */


   /* ----------------------------------------------------------- *
    *  "-t qua" reads the Quaternation data from the sensor.      *
    * This requires the sensor to be in fusion mode (mode > 7).   *
    * ----------------------------------------------------------- */
   //if(strcmp(datatype, "qua") == 0) {

      int mode = get_mode();
      if(mode < 8) {
         printf("Error getting Quaternation, sensor mode %d is not a fusion mode.\n", mode);
         exit(-1);
      }

      struct bnoqua bnodQ;
      res = get_qua(&bnodQ);
      if(res != 0) {
         printf("Error: Cannot read Quaternation data.\n");
         exit(-1);
      }

      /* ----------------------------------------------------------- *
       * print the formatted output string to stdout (Example below) *
       * QUA 0.83 0.13 -0.05 -0.54 (QUA W X Y Z)                     *
       * ----------------------------------------------------------- */
      printf("QUA %3.2f %3.2f %3.2f %3.2f\n", bnodQ.quater_w, bnodQ.quater_x, bnodQ.quater_y, bnodQ.quater_z);


  // } /* End reading Quaternation data */

  // SOCKET ============================
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;

    // socket create and varification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("129.120.52.216");
    servaddr.sin_port = htons(PORT);

    // connect the client socket to server socket
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("connected to the server..\n");
    for(int i=0; i < 10000000; i++){
        struct bnoqua bnodQ;
        res = get_qua(&bnodQ);
        printf("QUA %3.2f %3.2f %3.2f %3.2f\n", bnodQ.quater_w, bnodQ.quater_x, bnodQ.quater_y, bnodQ.quater_z);


        struct bnoacc bnodA;
        res = get_acc(&bnodA);
        printf("ACC %3.2f %3.2f %3.2f\n", bnodA.adata_x, bnodA.adata_y, bnodA.adata_z);

        sleep(1);

    }



    // close the socket
    close(sockfd);
    // ==================================


   exit(0);
}
