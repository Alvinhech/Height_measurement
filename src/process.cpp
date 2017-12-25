/*************************************************
Copyright:nljz
Author: Alvin He
Date:2017.12.22
Description: when radar is in the right or left of the gate, process and calculate height of people's shoulder
**************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <sstream>
#include <iostream>
#include <string>

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
#define H 5
#define PERCENT 0.8
#define SLOPE -2

int main()
{
	std::ifstream file_calib("new_angle.txt");
    if(!file_calib.is_open())
    {
        return -1;
    }

    float height; // h:雷达距离检测区域下沿高度
    float width; // w:检测区域宽度
    float width_left; // w_l:雷达距离检测区域左侧的横向宽度
    std::vector<float> v_angle; //标定角度集合
    cv::Point3f v_offset; //坐标变换平移向量集合
    std::string vstr; //雷达序列号
    std::vector<cv::Mat> vMat; //坐标变换旋转矩阵集合
    int no_radar;

    while(!file_calib.eof())
    {
        std::string str;
        std::getline(file_calib, str);
        if(str.empty())
        	break;
        std::stringstream sttr(str);
        std::string seril_no;
        sttr >> no_radar >> seril_no;
        vstr=seril_no;
        sttr >> height >> width >> width_left;
        for(int i=0;i<5;i++)
        {
            float angle;
            sttr >> angle;
            v_angle.push_back(angle);
        }

        float z_angle;
        cv::Point3f tmp;
        sttr >> tmp.x >> tmp.y >> tmp.z >> z_angle;
        //std::cout<< tmp.x << " " << tmp.y << " " << tmp.z << " " << z_angle << std::endl;
        v_offset=tmp;


    }

    file_calib.close();
    float h1=width_left/tan((float)(PI*(v_angle[1])/180.0));
    //std::cout<< "high1:"<<height<<"    high2:"<<h1<<"     width:"<<width<<"       wl:"<<width_left<<endl;


    ///////////////////////////////
    //get data
    std::ifstream file_data("new_data");
    if(!file_data.is_open())
    {
        return -1;
    }
    std::vector<int> point_num;
    std::vector<std::vector<Point2f> > point;
    while(!file_data.eof())
    {
    	std::vector<Point2f> temp;
    	std::string str;
        std::getline(file_data, str);
        if(str.empty())
        	break;
        std::getline(file_data, str);
        std::stringstream sttr(str);
        int n;
        sttr >> n;
		point_num.push_back(n);
	    //std::cout<<n<<std::endl;
	    float x,y;
	    for(int i=0;i<n;i++)
	    {
	      	std::getline(file_data, str);
	        std::stringstream sttr1(str);
	        sttr1 >> x >> y;
	        temp.push_back(Point2f(x,y));
	    }
	    point.push_back(temp);
    }
    //std::cout<<point.size()<<std::endl;  //153

    ///////calculate slope
    std::vector<float> results;
    float k; //slope
    //filtering
    for(int i=0;i<point.size();i++)
    {
    	std::vector<Point2f> data;
    	float max=width;
    	for(int j=0;j<point[i].size();j++)
    	{
    		if(point[i][j].x<max)
    			max=point[i][j].x;
    	}
    	max=H+width-width_left-max;
    	for(int j=0;j<point[i].size();j++)
    	{
    		if(H+width-width_left-point[i][j].x>=max*PERCENT)
    			data.push_back(point[i][j]);
    	}

    	std::vector<float> slopes;

		//paint
		Mat picture(300,300,CV_8UC3,Scalar(255,255,255));
        circle(picture,Point(width_left,10),10,Scalar(0,0,0));

        Point P0=Point(0,h1+10);
        Point P2=Point(width,height+10);
        rectangle(picture,P0,P2,Scalar(0,0,0));

        for(int i=0; i<data.size(); i++)
        {
            float x = data[i].x;
            float z = data[i].y;
            //std::cout<<"("<<x<<","<<z<<")"<<std::endl;
            circle(picture,Point(x+width_left,z+10),1,Scalar(0,0,0));
        }




        //calculate slope
        for(int j=0;j<data.size()-1;j++)
    	{
    		k=(data[j+1].y-data[j].y)/(data[j+1].x-data[j].x);
    		//std::cout<<k<<"\t";
    		slopes.push_back(k);
    	}
    	//find center slope(represents shoulder)
    	int front=0,rear=slopes.size()-1,flag=0;
    	float height1,height2;
    	for(int j=0;j<=slopes.size()-1;j++)
    	{
    		if(slopes[j]<=SLOPE&&flag==0)
    		{
    			front=j;
    			flag=1;
    		}
    		if(slopes[j]>SLOPE&&flag==1)
    		{
    			rear=j;
    			break;
    		}


    	}
    	height1=data[front].x;
    	height2=data[rear].x;
    	line(picture,Point(height1+width_left,h1+10),Point(height1+width_left,height+10),Scalar(0,0,0));
    	line(picture,Point(height2+width_left,h1+10),Point(height2+width_left,height+10),Scalar(0,0,0));


		

      	std::cout << "height(cm):"<<H+width-width_left-height1<<" 1-2(cm):" <<(height1-height2)<< std::endl;
      	//std::cout<<"----------------------------------------"<<std::endl;
      	imshow("picture",picture);
      	waitKey(0);

    }


    return 0;
}
