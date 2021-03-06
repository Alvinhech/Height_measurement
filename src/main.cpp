/************************************************* 
Copyright:nljz 
Author: Alvin He
Date:2017.12.22
Description: when radar is in the right or left of the gate, run radar and get 2d coordinates
**************************************************/  

#include "main.h"

/*
 * 过滤掉检测区域外目标数据，只保留进入检测区域内的数据
*/
template<typename T>
void ThresholdData(std::vector<std::pair<T,T> > &data, std::vector<T>& angles, T& height, T& width, T& width_left, T& h1)
{
    
    T width_right = width - width_left;
    T delt;
    if((T)PI < angles[4])
    {
        delt = -((T)N_SIZE - angles[4]);
    }
    else
    {
        delt = angles[4];
    }
    
    for(int pos=0;pos<data.size();++pos)
    {
        T i= data[pos].first - delt; 
        if(i<=angles[0])
        {
            data[pos].second =  data[pos].second > height/cos((T)(PI*i/180.0)) ? 0.0:data[pos].second;
        }
        else if(i>=angles[3])
        {
            data[pos].second = data[pos].second > height/cos((T)( (PI*((T)N_SIZE - i)/180.0) )) ? 0.0:data[pos].second;   
        }
        else if(i>angles[0] && i<angles[1])
        {
           data[pos].second = data[pos].second > width_left/sin((T) (PI*i/180.0)) ? 0.0:data[pos].second;   
        }
        else if(i>=angles[1] && i<=angles[2])
        {
            data[pos].second = (T)0.0 ;
        }
        else if(i>angles[2] && i<angles[3])
        {
            data[pos].second = data[pos].second > width_right/sin((T)(PI*((T)N_SIZE - i)/180.0)) ? 0.0:data[pos].second;   
        }
    }
}
/*
 * 角度取整
*/
template<typename T>
std::vector<T> process(std::vector<std::pair<T,T> > &data)
{
    int count = data.size();
   
    std::vector<T> vecT(N_SIZE);
    for(int i=0; i<N_SIZE; ++i)
    {
       vecT[i] = 0.0;
    }

    for(int i=0; i<data.size(); ++i)
    {
       vecT[(int)(data[i].first + (T)0.5)] = data[i].second;
    } 

    return vecT;
}

/*
 * 集中候选点
*/
template<typename T>
void detectHands(std::vector<T>& input_data, std::vector<int>& results)
{
    if(input_data.size() != N_SIZE)
    {
        return false;
    }

    results.clear();

    for(int i=0; i<input_data.size(); ++i)
    {
        if(abs(input_data[i] - (T)0.0) > 1e-3 )
        {
            results.push_back(i);
        }
    }
}


bool checkRPLIDARHealth(RPlidarDriver * drv)
{
    u_result     op_result;
    rplidar_response_device_health_t healthinfo;

    op_result = drv->getHealth(healthinfo);
    if (IS_OK(op_result))
    { 
        // the macro IS_OK is the preperred way to judge whether the operation is succeed.
        printf("RPLidar health status : %d\n", healthinfo.status);
        //printf("1\n");
        if (healthinfo.status == RPLIDAR_STATUS_ERROR)
        {
            //printf("2\n");
            fprintf(stderr, "Error, rplidar internal error detected. Please reboot the device to retry.\n");
            // enable the following code if you want rplidar to be reboot by software
            // drv->reset();
            return false;
        }
        else
        {
            //printf("3\n");
            return true;   
        }
    } 
    else
    {
        fprintf(stderr, "Error, cannot retrieve the lidar health code: %x\n", op_result);
        return false;
    }
}


#include <signal.h>
bool ctrl_c_pressed = false;
void ctrlc(int)
{
    ctrl_c_pressed = true;
}



//save data for later usage
void Save_data(string file_name,std::vector<cv::Point2f> vPoints)
{
    stringstream ss;
    string str;
    ss<<save_filename;
    ss>>str;
    std::ofstream file_output(file_name+str,ios_base::app);
    //std::ofstream file_output(file_name);
    if(!file_output.is_open())
    {
         return -1;
    }
    file_output<<"---------------------------"<<std::endl;
    file_output<<vPoints.size()<<std::endl;
    for(int i=0; i<vPoints.size(); i++)
    {
        float x = vPoints[i].x;
        float z = vPoints[i].y; 
        file_output<<x<<" "<<z<<std::endl;
    }
    file_output.close();
}

int main(int argc, const char * argv[]) 
{
  
    //std::ifstream file_calib("CalbrationFile_Radar.txt");
    std::ifstream file_calib("new_angle.txt");
    if(!file_calib.is_open())
    {
        return -1;
    }
    
    std::vector<float> height; // h:雷达距离检测区域下沿高度
    std::vector<float> width; // w:检测区域宽度
    std::vector<float> width_left; // w_l:雷达距离检测区域左侧的横向宽度
    std::vector<std::vector<float> > vv_angles; //标定角度集合
    std::vector<cv::Point3f> v_offset; //坐标变换平移向量集合
    std::vector<std::string> vstr; //雷达序列号
    std::vector<cv::Mat> vMat; //坐标变换旋转矩阵集合
    
    // rotate: 绕x轴旋转矩阵
    cv::Mat rotate = cv::Mat::eye(3,3,CV_32FC1);
    rotate.at<float>(1,1) = -1.0f;
    rotate.at<float>(2,2) = -1.0f;

    int no_radar = -1;
    int line_no = 0;
    // h:雷达距离检测区域下沿高度
    // w:检测区域宽度
    // w_l:雷达距离检测区域左侧的横向宽度
    float h,w,w_l;
    while(!file_calib.eof())
    {
        std::string str;
        std::getline(file_calib, str);
        if(str.empty()) break;

        std::stringstream sttr(str);

        std::string seril_no;
        sttr >> no_radar >> seril_no;
        vstr.push_back(seril_no);

        sttr >> h >> w >> w_l;
        height.push_back(h);
        width.push_back(w);
        width_left.push_back(w_l);

        std::vector<float> v_angle;
        for(int i=0;i<5;i++)
        {
            float angle;
            sttr >> angle;
            v_angle.push_back(angle);

            std::cout<< angle << " ";
        }
        vv_angles.push_back(v_angle);

        float z_angle;
        cv::Point3f tmp;
        sttr >> tmp.x >> tmp.y >> tmp.z >> z_angle;
        std::cout<< tmp.x << " " << tmp.y << " " << tmp.z << " " << z_angle << std::endl;

        cv::Mat z_rotate = cv::Mat::eye(3,3,CV_32FC1);
        float a = cos(PI*z_angle/180.0f);
        float b = sin(PI*z_angle/180.0f);
        z_rotate.at<float>(0,0) = a;
        z_rotate.at<float>(0,1) = b;
        z_rotate.at<float>(1,0) = -b; 
        z_rotate.at<float>(1,1) = a;
        /*
        std::cout<<z_rotate<<std::endl;
        std::cout<<rotate<<std::endl; 
        std::cout<<z_rotate*rotate<<std::endl;    
        */ 
        vMat.push_back(rotate*z_rotate);

        v_offset.push_back(tmp);
        
        line_no++;
    }

    file_calib.close();


    const char * opt_com_path = NULL;
    _u32         opt_com_baudrate = 115200;
    u_result     op_result;

    printf("Ultra simple LIDAR data grabber for RPLIDAR.\n"
           "Version: "RPLIDAR_SDK_VERSION"\n");

    // read serial port from the command line...
    if (argc>1) opt_com_path = argv[1]; // or set to a fixed value: e.g. "com3" 

    // read baud rate from the command line if specified...
    if (argc>2) opt_com_baudrate = strtoul(argv[2], NULL, 10);


    if (!opt_com_path) 
    {
#ifdef _WIN32
        // use default com port
        opt_com_path = "\\\\.\\com3";
#else
        opt_com_path = "/dev/ttyUSB0";
#endif
    }

  
  /**********************找寻雷达序列号对应的端口号****************************/
    int com_no = -1;
    int valid_count = 0;
    std::map<int,std::string> map_com;
    while(com_no < 10)
    {
        com_no++;

#ifdef _WIN32
        std::string opt_com_path = std::string("\\\\.\\com") + to_string(com_no);
#else
        std::string opt_com_path = std::string("/dev/ttyUSB") + std::to_string(com_no);
#endif

        // create the driver instance
        RPlidarDriver * drv = RPlidarDriver::CreateDriver(RPlidarDriver::DRIVER_TYPE_SERIALPORT);
        rplidar_response_device_info_t devinfo;

        if (!drv) 
        {
            fprintf(stderr, "insufficent memory, exit\n");
            com_no--;
            RPlidarDriver::DisposeDriver(drv);
            continue;
            //exit(-2);
        }

        // make connection...
        if (IS_FAIL(drv->connect(opt_com_path.c_str(), opt_com_baudrate)))
        {
            RPlidarDriver::DisposeDriver(drv);
            continue;
        }
    	
        // retrieving the device info
        op_result = drv->getDeviceInfo(devinfo);

        if (IS_FAIL(op_result)) 
        {
            fprintf(stderr, "Error, cannot get device info.\n");
            
            RPlidarDriver::DisposeDriver(drv);
            continue;
        }

        // print out the device serial number, firmware and hardware version number..
        printf("RPLIDAR S/N: ");
        char buf[32];
        int offset = 0;
        for (int pos = 0; pos < 16 ;++pos) 
        {
            int j = sprintf(buf+offset,"%02X",devinfo.serialnum[pos]);
            offset += j;
            printf("%02X", devinfo.serialnum[pos]);
        }

        std::string str_serial_no(buf);

        bool b_valid = false;
        for(int i=0;i<vstr.size();i++)
        {
            if(str_serial_no == vstr[i])
            {
                map_com[i] = opt_com_path;
                b_valid = true;
            }            
        }
      
        printf("\n"
                "Firmware Ver: %d.%02d\n"
                "Hardware Rev: %d\n"
                , devinfo.firmware_version>>8
                , devinfo.firmware_version & 0xFF
                , (int)devinfo.hardware_version);
   
        if(b_valid)
        {
            valid_count++;
        }

        RPlidarDriver::DisposeDriver(drv);
   }

    std::cout<<"Total num of valid radar is "<<valid_count<<std::endl;
    
    if(valid_count != line_no )
    {
        //std::cout<<"need to connect three radar..."<<std::endl;
        return -1;
    }  



/**********************开启多进程****************************/
    
    pid_t fpid; //fpid表示fork函数返回的值  
    int i=-1;
    for(i=0; i<valid_count; i++)
    {
        fpid=fork(); 
        if(fpid == 0 | fpid == -1)
        {
            break;
        }
    }

    signal(SIGINT, ctrlc);

    if (fpid < 0)   
        printf("error in fork!");   
    else if (fpid == 0) {
        
        int id_radar = i;
        int id_p = getpid();
        printf("I am the child process, my process id is %d\n",id_p);   

        zmq::context_t context (1);
        zmq::socket_t publisher (context, ZMQ_PUB);
        publisher.connect("ipc:///tmp/radar_results");
    
        std::stringstream sttr_win;
        sttr_win<<id_radar;
        std::string str_win = sttr_win.str();
       
        
        // create the driver instance

        RPlidarDriver * drv = RPlidarDriver::CreateDriver(RPlidarDriver::DRIVER_TYPE_SERIALPORT);
        rplidar_response_device_info_t devinfo;

        
        if (!drv) 
        {
            fprintf(stderr, "insufficent memory, exit\n");
            exit(-2);
        }
        
        // make connection...
        if (IS_FAIL(drv->connect(map_com[id_radar].c_str(), opt_com_baudrate)))
        {
            fprintf(stderr, "Error, cannot bind to the specified serial port %s.\n", opt_com_path);
            goto on_finished;
        }

        
        // retrieving the device info
        op_result = drv->getDeviceInfo(devinfo);

        if (IS_FAIL(op_result)) 
        {
            fprintf(stderr, "Error, cannot get device info.\n");
            goto on_finished;
        }

        // check health...
        if (!checkRPLIDARHealth(drv)) 
        {
            //printf("1\n");
            goto on_finished;
        }

        std::vector<float> vp_after(N_SIZE);
        std::vector<float> vp_last_after(N_SIZE);
      
        struct timeval tv;
        struct timezone tz;
        gettimeofday(&tv,&tz);
        drv->startMotor();
        // start scan...
        drv->startScan();
        // fetech result and print it out...
        std::vector<float> buffer;
        srand((unsigned)time(NULL));
        while (1) 
        {
            float final_result;  
            rplidar_response_measurement_node_t nodes[N_SIZE*2];
            size_t  count = _countof(nodes);
             
            op_result = drv->grabScanData(nodes, count);

            gettimeofday(&tv,&tz);
            //std::cout << IS_OK(op_result);
            if (IS_OK(op_result)) 
            {
                std::vector<std::pair<float,float> > vp;
                
                drv->ascendScanData(nodes, count);
                //printf("\t%d\n", count);
                for (int pos = 0; pos < (int)count ; ++pos) 
                { 
                    vp.push_back(std::pair<float,float>((nodes[pos].angle_q6_checkbit >> RPLIDAR_RESP_MEASUREMENT_ANGLE_SHIFT)/64.0f, 
                                                        nodes[pos].distance_q2/40.0f)); 
                }
                //filter out things not in the area(necessary)
                float h1=width_left[id_radar]/tan((float)(PI*(180.0f-vv_angles[id_radar][1])/180.0));
                ThresholdData<float>(vp, vv_angles[id_radar], height[id_radar], width[id_radar], width_left[id_radar],h1);
                
                //aline, get the length of each angle 
                vp_after = process<float>(vp);

                std::vector<int> results;
                //detect obeject, not necessarily hand
                detectHands(vp_after,results);
                //2d coordinate
                std::vector<cv::Point2f> vPoints;
                for(int i=0; i<results.size(); ++i)
                {
                    float len = vp_after[results[i]];
                    float x,z;
                    if(results[i]<=vv_angles[id_radar][1])
                    {
                        x = -( len*sin(((float)results[i])*PI/180.0) );
                        z = len*cos( ((float)results[i])*PI/180.0 );
                    }
                    else
                    {
                        x = len*sin( ((float)N_SIZE - (float)results[i])*PI/180.0 );
                        z = len*cos( ((float)N_SIZE - (float)results[i])*PI/180.0 );
                    }

                    vPoints.push_back(cv::Point2f(x,z));
                }
                //not enough points
                if(vPoints.size()<20)
                {
                    //good sample
                    if(buffer.size()>=BUFFER_SIZE)
                    {
                        final_result=Cluster(buffer);
                        std::cout << "result:\t"<<final_result<< std::endl;
                        std::vector<float> ().swap(buffer);
                        save_filename++;
                    }
                    //bad sample
                    else if(buffer.size()>0&&buffer.size()<BUFFER_SIZE)
                    {
                        std::vector<float> ().swap(buffer);
                        save_filename++;
                    }
                    //std::cout << "size:\t"<<buffer.size()<<std::endl;
                    continue;

                }
                Save_data("saved_data",vPoints);
                //std::cout<<vPoints.size()<<std::endl;
                float height_result=0.1*rand() / double(RAND_MAX)+Get_Height(vPoints,height[id_radar], width[id_radar], width_left[id_radar],h1,vv_angles[id_radar]);
                buffer.push_back(height_result);
                std::cout << height_result<<"\t"<<vPoints.size()<< std::endl;
                /*
                //radar number, timeval, timezone
                Radar_Results radar_results;
                radar_results.id_radar = id_radar;
                radar_results.time_stamp_sec = tv.tv_sec;
                radar_results.time_stamp_usec = tv.tv_usec;
                
                int valid_count = 0;
                for(int i=0; i<vPoints_cluster.size(); ++i)
                {
                    
                    float x = vPoints_cluster[i].x;
                    float z = vPoints_cluster[i].y;
                    std::cout<<"("<<x<<","<<z<<")"<<std::endl;
                    //coordinate info of object's center
                    std::pair<float,cv::Point3f> p_info;
                    //coordinate transformation
                    cv::Mat pt_l(3,1,CV_32FC1);
                    pt_l.at<float>(0,0) = x; 
                    pt_l.at<float>(1,0) = 0.0f;
                    pt_l.at<float>(2,0) = z; 
                    cv::Mat m_wrd = vMat[id_radar]*pt_l;
                    cv::Point3f pt_tmp(m_wrd.at<float>(0,0), m_wrd.at<float>(1,0), m_wrd.at<float>(2,0));
                    cv::Point3f pt_wrd = pt_tmp + v_offset[id_radar];
                    //std::cout<<"("<<pt_wrd.x<<","<<pt_wrd.y<<","<<pt_wrd.z<<")"<<std::endl;





                    p_info.first = 1.0f;
                    p_info.second = pt_wrd;
                    radar_results.results.push_back(p_info);

                    cv::RotatedRect rbox = minAreaRect(cv::Mat(vpp[i]));
                    //
                    cv::Rect brect = rbox.boundingRect();
                    
                    //std::cout<<brect.x<<","<<brect.y<<","<<brect.width<<","<<brect.height<<std::endl;
                    
                    radar_results.numPts_per_class.push_back(vpp[i].size());
                    radar_results.vrect.push_back(brect);
                   
                    valid_count++;
                }
                
                

                radar_results.nums_results = valid_count;            

                std::ostringstream os;  
		        boost::archive::text_oarchive oa(os);  
		        oa << radar_results;						
		        std::string content = "LADAR$" + os.str();//content保存了序列化后的数据。  

                zmq::message_t message(content.size());
		        memcpy(message.data(), content.c_str() , content.size());
                publisher.send(message);
            
                */
               
            }  
        }
        drv->stop();
        drv->stopMotor();
        // done!
on_finished:
        RPlidarDriver::DisposeDriver(drv);

        std::cout<<"child "<<id_radar<<" kill..."<<std::endl;
    } 
    else {

        printf("I am the parent process, my process id is %d\n",getpid());   
        while(!ctrl_c_pressed)
        {
            sleep(1);
        }

        std::cout<<"parent exit..."<<std::endl;
    }   
  
    return 0;
}




