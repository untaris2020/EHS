// Project Name:  A.R.I.S. - Augmented Reality Interface System
// University:    University of North Texas
// Developer:     Juan Ruiz, Computer Engineering
// Date Created:  12/24/2019

// Purpose:       The purpose of the cv2.cpp script is to read each of the frame, compress them via JPEG
//                Encode them using base64, and send them to the server.cs via TCP.

//                Run this command to compile - Must have the base64.cpp and base64.h files
//                g++ cv2.cpp base64.cpp -o cv2 `pkg-config --cflags --libs opencv`

#include "base64.h"
#include <iostream>
#include <stdio.h>
#include <time.h>

// OpenCV Libraries
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// Socket Libraries
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

using namespace std;
using namespace cv;

int main()
{

    const char *IP = "129.120.52.216";
    const int PORT = 4060;

    // Sets the frame width and height
    int width = 320;
    int height = 240;

    // Initializing the OpenCV camera
    Mat frame;
    VideoCapture cap;

    // Sets the frame width and height
    cap.open(0);
    cap.set(3, width);
    cap.set(4, height);

    // Frame compression to JPEG and H264
    vector<uchar> buff;
    vector<int> param(2);
    param[0] = IMWRITE_JPEG_QUALITY;
    param[1] = 40;

    // Check if we succeeded
    if (!cap.isOpened())
    {
        cerr << "ERROR! Unable to open camera\n";
        return -1;
    }

    // SOCKET ============================
    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, IP, &serv_addr.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    // ==================================

    // Timers
    time_t start;
    time_t end;

    time(&start);                                         // Gets time from start to determine frame rate

    int count = 0;                                        // Counter that will help determine the frames per second
    int count2 = 1;

    while(1)
    {
        time(&end);                                       // Gets time for each of the frame to determine frame rate

        cap.read(frame);                                  // Wait for a new frame from camera and store it into 'frame'

        imencode(".jpg", frame, buff, param);             // Compresses each frame to JPG

        string temp;
        for(int i = 0; i < buff.size(); i++)
        {
            temp += buff[i];
        }

        string encoded = base64_encode(reinterpret_cast<const unsigned char*>(temp.c_str()), temp.length());      // Encodes the frame into base64

        int size = encoded.length();                      // Determines the length of each base64 frame

        cout << count << " ) " << size << endl;

        send(sock, encoded.c_str(), size, 0);             // Send the data via TCP

        // Determines the frames per second
        if((end - start) > 1)
        {
            cout << count2 << ") Frames per second: " << count / 1 << endl;
            time(&start);
            count = 0;
            count2++;
        }

        count++;
    }

    // The camera will be deinitialized automatically in the VideoCapture destructor
    return 0;

}// END MAIN

