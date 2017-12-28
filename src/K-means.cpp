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
#define VALID_NUM 3
#define VALID_PER 0.300

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

struct HC{  
    float height;    
    int num;  
    HC *pnext;
      
};  

float squareDistance(point a,point b){  
    return (a.x-b.x)*(a.x-b.x);  
}  

void Sort(std::vector<point> &centroid, int *cs)
{
    int n=centroid.size();
    for(int i=0;i<n-1;i++)
    {
        for(int j=0;j<n-1-i;j++)
        {
            if(centroid[j].x<centroid[j+1].x)
            {
                swap(centroid[j],centroid[j+1]);
                int temp=cs[j];
                cs[j]=cs[j+1];
                cs[j+1]=temp;
            }
        }
    }
    return;
}

float k_means(std::vector<point> dataset,int k){  
    std::vector<point> centroid;
    std::vector<float> temp_center(k,0.0f);
    int n=1;  
    int len = dataset.size();  
    float delta;  
    srand((int)time(0));  
    int cen = rand();
    //random select centroids  
    while(n<=k){  
        point cp(dataset[(cen+n)%len].x,n);  
        centroid.push_back(cp);  
        n++;  
    }
    //printf("%d\n", cen);
    int *cs = new int[k];  
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
        delta=0;
        for(int i=0;i<k;i++)
        {
            delta+=abs(temp_center[i]-centroid[i].x);
        }
        /*
        if(delta!=delta)
            break;
            */
        //printf("delta:     %f\n", delta);
        if(delta<0.1)
            break;
        for(int i=0,delta=0;i<k;i++)
        {
            temp_center[i]=  centroid[i].x;
        }  
    }

    Sort(centroid,cs);
    for(int i=0;i<k;i++) 
        std::cout<<"x:"<<centroid[i].x<<"\tc:"<<centroid[i].cluster<<"\tcs:"<<cs[i]<<std::endl;
    /*
    centroid[0]=point(166.04,5);
    centroid[1]=point(165.154,4);
    centroid[2]=point(161.478,3);
    centroid[3]=point(157.444,2);
    centroid[4]=point(128.905,1);
    cs[0]=1;
    cs[1]=2;
    cs[2]=9;
    cs[3]=8;
    cs[4]=1;
*/


    HC *head,*p,*q;

    head=(struct HC*)malloc(sizeof(struct HC));
    head->height=centroid[0].x;
    head->num=cs[0];
    p=head;
    for(int i=1;i<centroid.size();i++)
    {
        q=(struct HC*)malloc(sizeof(struct HC));
        q->height=centroid[i].x;
        q->num=cs[i];
        p->pnext=q;
        p=q;
    }
    p->pnext=NULL;

    p=head;
    while(p->pnext!=NULL)
    {
        if(abs(p->height-(p->pnext)->height)<=10)
        {
            p->height=(p->height*p->num+(p->pnext)->height*(p->pnext)->num)/(p->num+(p->pnext)->num);
            p->num+=(p->pnext)->num;
            q=p->pnext;
            p->pnext=(p->pnext)->pnext;
            free(q);
        }
        else
            p=p->pnext;
    }
    
    p=head;
    while(p!=NULL)
    {
        std::cout<<"x:"<<p->height<<"\tcs:"<<p->num<<std::endl;
        p=p->pnext;
    }
    





    
    float max_h=0;
    int max_n=0;
    p=head;
    while(p!=NULL)
    {  
        max_n=max(max_n,p->num);
        p=p->pnext;
    }
    p=head;
    while(p!=NULL)
    {  
        //std::cout<<"x:"<<centroid[i].x<<"\tc:"<<centroid[i].cluster<<"\tcs:"<<cs[i]<<std::endl;
        if(p->num>=VALID_NUM && p->num>=max_n*VALID_PER)
        {
            max_h=p->height;
            break;
        }
        p=p->pnext;

    }
    /*
    std::cout<<"---------------------------"<<std::endl;
    for(int i=0;i<dataset.size();i++)
        std::cout<<"x:"<<dataset[i].x<<"\tc:"<<dataset[i].cluster<<std::endl;
        */
    return max_h;  

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
    return k_means(dataset,5);   
}