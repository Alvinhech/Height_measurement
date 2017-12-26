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
#define PERCENT 0.70
#define SLOPE -2
int find(std::vector<float> slopes)
{
    int front=0,rear=slopes.size()-1,flag=0;
    int i=0;

    while(i<slopes.size()-1)
    {    
        for(int j=i;j<=slopes.size()-2;j++,i++)
        {
            if(slopes[j]<=SLOPE&&slopes[j+1]<=SLOPE&&flag==0)
            {
                front=j;
                flag=1;
            }
            if(slopes[j]>SLOPE&&flag==1)
            {
                rear=j;
                flag=0;
                break;
            }


        }
        
    }
    return front;

}
float print_Height(std::vector<cv::Point2f> data, float height, float width, float width_left, float h1)
{
        std::vector<float> slopes;
        float k; //slope
        //calculate slope
        for(int j=0;j<data.size()-1;j++)
        {
            k=(data[j+1].y-data[j].y)/(data[j+1].x-data[j].x);
            //std::cout<<k<<"\t";
            slopes.push_back(k);
        }
        //find center slope(represents shoulder)
        
        float height1,height2;
        
        height1=data[find(slopes)].x;
        //height2=data[rear].x;
        return H+width-width_left-height1;
}

void save_slope(std::vector<float> slopes)
{
        std::ofstream file_output("save_slope.txt",ios_base::app);
        if(!file_output.is_open())
        {
             return -1;
        }
        file_output<<"---------------------------"<<std::endl;
        file_output<<slopes.size()<<std::endl;
        for(int i=0; i<slopes.size(); i++)
        {
             
            file_output<<slopes[i]<<std::endl;
        }
        file_output.close();
}

float paint_Height(std::vector<cv::Point2f> data, float height, float width, float width_left, float h1)
{
       
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

        

        std::vector<float> slopes;
        float k; //slope 
        //calculate slope
        for(int j=0;j<data.size()-1;j++)
        {
            k=(data[j+1].y-data[j].y)/(data[j+1].x-data[j].x);
            //std::cout<<k<<"\t";
            slopes.push_back(k);
        }
        //find center slope(represents shoulder)
        //save_slope(slopes);
        float height1,height2;
        
        height1=data[find(slopes)].x;

        
        //height2=data[rear].x;
        line(picture,Point(height1+width_left,h1+10),Point(height1+width_left,height+10),Scalar(0,0,0));
        //line(picture,Point(height2+width_left,h1+10),Point(height2+width_left,height+10),Scalar(0,0,0));


        

        ///std::cout << "height(cm):"<<H+width-width_left-height1<<" 1-2(cm):" <<(height1-height2)<< std::endl;
        //std::cout<<"----------------------------------------"<<std::endl;
        imshow("picture",picture);
        waitKey(0);

    


    return H+width-width_left-height1;
}

float Get_Height(std::vector<cv::Point2f> point, float height, float width, float width_left, float h1,std::vector<float> v_angle)
{
	

    
    cv::Point3f v_offset; //坐标变换平移向量集合
    std::string vstr; //雷达序列号
    int no_radar;
    //std::cout<< "high1:"<<height<<"    high2:"<<h1<<"     width:"<<width<<"       wl:"<<width_left<<endl;


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
        return print_Height(data,height,width,width_left,h1);
        //return paint_Height(data,height,width,width_left,h1);
    	
}
