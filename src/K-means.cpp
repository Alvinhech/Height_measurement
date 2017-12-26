/* 
    K-means Algorithm 
*/  


#include <fstream>  
#include <vector>  
#include <ctime>  
#include <cstdlib>  
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <sstream>
#include <iostream>
#include <string>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <unistd.h>
#include <sys/prctl.h>
#include <sys/time.h>

using namespace cv; 
/* run this program using the console pauser or add your own getch, system("pause") or input loop */  
typedef struct Point{  
    float x;    
    int cluster;  
    Point (){}  
    Point (float a,int c){  
        x = a;   
        cluster = c;  
    }  
}point;  

float squareDistance(point a,point b){  
    return (a.x-b.x)*(a.x-b.x);  
}  

float k_means(std::vector<point> dataset,int k){  
    std::vector<point> centroid;
    float temp_center[2]={0.0,0.0};  
    int n=1;  
    int len = dataset.size();  
    float delta;  
    srand((int)time(0));  
    //random select centroids  
    while(n<=k){  
        int cen = rand()%len;  
        point cp(dataset[cen].x,n);  
        centroid.push_back(cp);  
        n++;  
    }
  
    while(1){  
  
        //update cluster for all the points  
        for(int i=0;i<len;i++)
        {  
            n=1;  
            float shortest = INT_MAX;  
            int cur = dataset[i].cluster;  
            while(n<=k)
            {  
                float temp=squareDistance(dataset[i],centroid[n-1]);              
                if(temp<shortest)
                {  
                    shortest = temp;  
                    cur = n;  
                }  
                n++;  
            }  
            dataset[i].cluster = cur;  
        }  
        //update cluster centroids  
        int *cs = new int[k];  
        for(int i=0;i<k;i++) cs[i] = 0;  
        for(int i=0;i<k;i++){  
            centroid[i] = point(0,i+1);  
        }  
        for(int i=0;i<len;i++){  
            centroid[dataset[i].cluster-1].x += dataset[i].x;   
            cs[dataset[i].cluster-1]++;  
        }  
        for(int i=0;i<k;i++){  
            centroid[i].x /= cs[i];   
        }
        //std::cout<<temp_center[0]<<"\t"<<temp_center[1]<<"\t"<<centroid[0].x<<"\t"<<centroid[1].x<<std::endl;
        delta=abs(max(temp_center[0],temp_center[1])-max(centroid[0].x,centroid[1].x));
        //printf("delta:\t%d\n", delta);
        if(delta<1)
            break;
        temp_center[0]=  centroid[0].x;
        temp_center[1]=  centroid[1].x;
        /*
        cout<<"time:"<<time<<endl;  
        for(int i=0;i<k;i++){  
            cout<<"x:"<<centroid[i].x<<"\tc:"<<centroid[i].cluster<<endl;  
        } 
        */    
        //sleep(1000);
  
    }  
    
    return max(centroid[0].x,centroid[1].x);  

//  cout<<endl;  
//  for(int i=0;i<centroid.size();i++){  
//      cout<<"x:"<<centroid[i].x<<"\tc:"<<centroid[i].cluster<<endl;  
//  }  
}  
float Cluster(std::vector<float> buffer) 
{  
    //std::cout<<"666"<<std::endl;
    std::vector<point> dataset;
    for(int i=0;i<buffer.size();i++)
        dataset.push_back(point(buffer[i],0));
    return k_means(dataset,2);   
}