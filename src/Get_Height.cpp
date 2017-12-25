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

#include <unistd.h>
#include <sys/prctl.h>
#include <sys/time.h>
using namespace cv;



#define PI 3.1415926
#define H 5
#define PERCENT 0.8
#define SLOPE -2

float Get_Height(std::vector<cv::Point2f> point, float height, float width, float width_left, float h1,std::vector<float> v_angle)
{
	

    
    cv::Point3f v_offset; //坐标变换平移向量集合
    std::string vstr; //雷达序列号
    int no_radar;
    //std::cout<< "high1:"<<height<<"    high2:"<<h1<<"     width:"<<width<<"       wl:"<<width_left<<endl;

    ///////calculate slope
    std::vector<float> results;
    float k; //slope
    //filtering
    
    	std::vector<Point2f> data;
    	float max=width;
    	for(int j=0;j<point.size();j++)
    	{
    		if(point[j].x<max)
    			max=point[j].x;
    	}
    	max=H+width-width_left-max;
    	for(int j=0;j<point.size();j++)
    	{
    		if(H+width-width_left-point[j].x>=max*PERCENT)
    			data.push_back(point[j]);
    	}

    	std::vector<float> slopes;
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
        return H+width-width_left-height1;
/*
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

    


    return 0;*/
}
