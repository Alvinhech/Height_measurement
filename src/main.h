
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>

#include <zmq.hpp>
#include "serialization.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "rplidar.h" //RPLIDAR standard sdk, all-in-one header

#ifndef _countof
#define _countof(_Array) (int)(sizeof(_Array) / sizeof(_Array[0]))
#endif

#ifdef _WIN32
#include <Windows.h>
#define delay(x)   ::Sleep(x)
#else
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/time.h>



static inline void delay(_word_size_t ms)
{
    while (ms>=1000)
    {
        usleep(1000*1000);
        ms-=1000;
    };
    if (ms!=0)
        usleep(ms*1000);
}
#endif

using namespace rp::standalone::rplidar;
using namespace cv;


#define CLUSTER_DISTANCE 21
#define PI 3.1415926
#define N_SIZE 360
#define VIBRATION_DISTANCE 10
#define BUFFER_SIZE 3

extern float Get_Height(std::vector<cv::Point2f>, float, float, float, float,std::vector<float>);
extern int find(std::vector<float> slopes);


extern float Cluster(std::vector<float> buffer);
