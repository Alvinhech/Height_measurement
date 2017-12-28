#include "main.h"

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

    for(int i=1;i<=28;i++)
    {

    stringstream ss;
    string str;
    ss<<i;
    ss>>str;
    string out=string("saved_data")+str;
    
    ///////////////////////////////
    //get data
    std::ifstream file_data(out);
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
    if(point.size()<6)
        continue;
    std::vector<float> buffer;
    srand((unsigned)time(NULL));  
    for(int i=0;i<point.size();i++){
        float height_result=0.1*rand() / double(RAND_MAX)+Get_Height(point[i],height, width, width_left,h1,v_angle);
        buffer.push_back(height_result);
        //std::cout << height_result<< std::endl;
    }
    
        float final_result=Cluster(buffer);
        std::cout << "result:\t"<<final_result<< std::endl;
        //sleep(1);
    
    }
    return 0;
}
