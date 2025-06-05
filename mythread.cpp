#include "mythread.h"
#include <QDebug>
#include "mainwindow.h"

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
MyThread::MyThread()
{
}

MyThread::~MyThread()
{
}





//相机标定
void calibParaL (cv::Mat &cameraMatrix, cv::Mat &distCoeffs)
{
    double F = 25.9383/1000.0;
    double sx = 8.30608/1000000;
    double sy = 8.3/1000000;
    double k1h = 348.969;      //k1k2k3p1p2
    double k2h = -2.39843e+07;
    double k3h = 9.45931e+11;
    double p1h = 0.0812523;
    double p2h = 0.0385081;  //p2
    double cx=602.815;
    double cy=528.685;

    cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
    cameraMatrix.at<double>(0, 0) = F / sx;
    cameraMatrix.at<double>(0, 1) = 0.0;
    cameraMatrix.at<double>(0, 2) = cx;
    cameraMatrix.at<double>(1, 1) = F  / sy;
    cameraMatrix.at<double>(1, 2) = cy;

    distCoeffs = cv::Mat::zeros(5, 1, CV_64F);
    distCoeffs.at<double>(0, 0) = -k1h*F*F ;
    distCoeffs.at<double>(1, 0) = k2h*F*F*F*F;
    distCoeffs.at<double>(2, 0) = p2h*F;
    distCoeffs.at<double>(3, 0) = p1h*F;
    distCoeffs.at<double>(4, 0) = k3h*F*F*F*F*F*F;
}





//第一次粗定位电池大概所在区域
void coarse_position(cv::Mat &src,cv::Mat &dst,int top,int bottom,int left,int right)
{

    dst=src.clone();
    dst.rows= abs(bottom-top);
    dst.cols=abs(left-right);
    int count=0;
    //std::cout<<"gg:  "<<src.channels();
    for(int i=bottom;i<top;i++)
    {

        uchar *ptr=src.ptr<uchar>(i);
//        ptr[left]=255;
//        ptr[right]=255;
        uchar *ptr1=dst.ptr<uchar>(count);
        for(int j=left;j<right;j++)
        {
            ptr1[j-left]=ptr[j];
            //ptr[j]=255;
        }
        count++;
    }
 //   cv::imwrite("/home/zw/gg.bmp",dst);

}





void find_circul_center(cv::Mat &src,int &h_center,int &w_center,int &r,int &s)
{
    int h_c[src.rows],w_c[src.cols];
    memset(h_c,0,sizeof(h_c));
    memset(w_c,0,sizeof(w_c));
    int left,right,top,bottom;
    for(int i=0;i<src.rows;i++)
    {
        uchar *ptr=src.ptr<uchar>(i);
        left=0,right=0;
        for(int j=0;j<src.cols;j++)
        {
            if(ptr[j]==255)
            {
                left=j;
                break;
            }
        }
        for(int j=src.cols-1;j>0;j--)
        {
            if(ptr[j]==255)
            {
                right=j;
                break;
            }
        }

        if(left&&right)
        {
            w_c[int((right+left)/2)]++;
        }
    }

    for(int j=0;j<src.cols;j++)
    {
        top=0,bottom=0;
        for(int i=0;i<src.rows;i++)
        {
            uchar *ptr=src.ptr<uchar>(i);
            if(ptr[j]==255)
            {
                top=i;
                break;
            }
        }

        for(int i=src.rows-1;i>0;i--)
        {
            uchar *ptr=src.ptr<uchar>(i);
            if(ptr[j]==255)
            {
                bottom=i;
                break;
            }
        }

        if(top&&bottom)
        {
            h_c[int((top+bottom)/2)]++;
        }
    }

    h_center=std::max_element(h_c,h_c+src.rows)-h_c;
    w_center=std::max_element(w_c,w_c+src.cols)-w_c;



    int rr[300]={0};

    //下面是确定半径
    for(int i=0;i<src.rows;i++)
    {
        uchar *ptr=src.ptr<uchar>(i);
        left=0,right=0;
        for(int j=0;j<src.cols;j++)
        {
            if(ptr[j]==255)
            {
                left=j;
                break;
            }
        }
        for(int j=src.cols-1;j>0;j--)
        {
            if(ptr[j]==255)
            {
                right=j;
                break;
            }
        }

        if(left&&right)
        {
            int dis_y=(i-h_center)*(i-h_center);
            int temp_r= sqrt(dis_y+(left-w_center)*(left-w_center));
            int temp_r2= sqrt(dis_y+(right-w_center)*(right-w_center));
            rr[temp_r]++;
            rr[temp_r2]++;
        }
    }


    r=std::max_element(rr,rr+300)-rr;
    //计算半径方差
    //long long int s=0;
    s=0;
    for(int i=10;i<300;i++)
    {
        if(rr[i])
        {
            s+=rr[i]*(i-r)*(i-r);
        }
    }
   // std::cout<<"方差为：   "<<s<<"\n";

}




//裁剪不同尺寸的圆
void get_area(cv::Mat &src,cv::Mat &dst,int center_x,int center_y,int begin,int end)
{

    cv::Mat mask=cv::Mat::zeros(cv::Size(src.cols,src.rows),CV_8UC1);
    cv::circle(mask,cv::Point(center_x,center_y),end,cv::Scalar(255),-1);
    cv::circle(mask,cv::Point(center_x,center_y),begin,cv::Scalar(0),-1);
    src.copyTo(dst,mask);
}



//定位锁孔位置
void alignment(cv::Mat &src,int center_x,int center_y,int r_begin,int r_end,int &theta1,int &theta2,int &theta3,int &theta4)
{
    int min_theta=0,min_theta2=0;
    double PI=3.1415926/180.0;
    int s[181]={0};
    int s2[361]={0};
    for(double theta=0;theta<=180;theta+=0.4)
    {
        for(int p=r_begin;p<=r_end;p++)
        {
            int x=cos(PI*theta)*p;
            int y=sin(PI*theta)*p;
            s[int (theta)]+=src.ptr<uchar>(int(center_y-y))[int(center_x-x)];
        }
    }

    for(double theta=180;theta<=360;theta+=0.4)
    {
        for(int p=r_begin;p<=r_end;p++)
        {
            int x=cos(PI*theta)*p;
            int y=sin(PI*theta)*p;

            s2[int (theta)]+=src.ptr<uchar>(int(center_y-y))[int(center_x-x)];
        }
    }



//    int minn=9999999;
//    for(int i=0;i<=180;i++)
//    {
//        for(int j=i+160;j<=i+200;j++)
//        {
//            if((i+160<180)||(i+200>360))
//            {
//                continue;
//            }
//            if(s[i]+s2[j]<minn)
//            {
//                min_theta=i;
//                min_theta2=j;
//                minn=s[i]+s2[j];
//            }
//        }
//    }




     min_theta=std::min_element(s,s+180)-s;
     min_theta2=std::min_element(s2+180,s2+360)-s2;

     int bias=15;

     if(min_theta2-min_theta>170&&(min_theta2-min_theta)<190)
     {
         theta1=min_theta-bias;
         theta2=min_theta+bias;
         theta3=min_theta2-bias;
         theta4=min_theta2+bias;
     }
     else
     {
         theta1=0;
         theta2=0;
         theta3=0;
         theta4=0;
     }

//     theta1=min_theta-bias;
//     theta2=min_theta+bias;
//     theta3=min_theta2-bias;
//     theta4=min_theta2+bias;

}




void find_circul_center1(cv::Mat &src,int &h_center,int &w_center,int &r,int &s,double theta1,double theta2,double theta3,double theta4)
{

    int h_c[src.rows],w_c[src.cols];
    //std::cout<<theta1<<" "<<theta2<<" "<<theta3<<" "<<theta4<<'\n';
    double PI=3.14159265358979323846;

    double t1=theta1*PI/180.0;
    double t2=theta2*PI/180.0;
    double t3=theta3*PI/180.0;
    double t4=theta4*PI/180.0;

    memset(h_c,0,sizeof(h_c));
    memset(w_c,0,sizeof(w_c));

    int rr[300]={0};



    for(double theta=0;theta<360;theta+=0.5)
    {
        if((theta>theta1&&theta<theta2)||(theta>theta3&&theta<theta4))
        {
//            for(double pp=r;pp>0;pp-=0.5)
//            {
//                int xx=w_center-cos(theta*PI/180.0)*pp;
//                int yy=h_center-sin(theta*PI/180.0)*pp;
//                src.ptr<uchar>(yy)[xx]=255;
//            }
            continue;
        }
        for(double pp=r;pp>0;pp=pp-1)
        {
            int xx=w_center-cos(theta*PI/180.0)*pp;
            int yy=h_center-sin(theta*PI/180.0)*pp;

            if(src.ptr<uchar>(yy)[xx])
            {
                rr[int(pp)]++;
                break;
            }
        }
    }

    r=std::max_element(rr,rr+300)-rr;
    //计算半径方差
    //long long int s=0;
    s=0;
    for(int i=10;i<300;i++)
    {
        if(rr[i])
        {
            s+=rr[i]*(i-r)*(i-r);
        }
    }

}








//划痕检测定位

int get_area(char* path)
{
    cv::Mat src,dst,dst2,dst3,dst4,dst5;
    std::vector<std::vector<cv::Point>> contours;     // 每个轮廓由一系列点组成
    std::vector<cv::Vec4i> hierarchy;

        src = cv::imread(path, 0);
        int row[1280]={0},col[1280]={0};
        cv::threshold(src,dst,0,255,CV_THRESH_OTSU);

        cv::Mat kernel1 = getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3), cv::Point(-1, -1));
        morphologyEx(dst, dst2, cv::MORPH_OPEN, kernel1);
        //cv::imwrite(rp,dst2);
        dst3=src;
        src=dst2;
        //粗定位中心点
        int h_c[src.rows],w_c[src.cols];
        memset(h_c,0,sizeof(h_c));
        memset(w_c,0,sizeof(w_c));
        int left,right,top,bottom;
        for(int i=0;i<src.rows;i++)
        {
            uchar *ptr=src.ptr<uchar>(i);
            left=0,right=0;
            for(int j=0;j<src.cols;j++)
            {
                if(ptr[j]==255)
                {
                    left=j;
                    break;
                }
            }
            for(int j=src.cols-1;j>0;j--)
            {
                if(ptr[j]==255)
                {
                    right=j;
                    break;
                }
            }

            if(left&&right)
            {
                w_c[int((right+left)/2)]++;
            }
        }

        for(int j=0;j<src.cols;j++)
        {
            top=0,bottom=0;
            for(int i=0;i<src.rows;i++)
            {
                uchar *ptr=src.ptr<uchar>(i);
                if(ptr[j]==255)
                {
                    top=i;
                    break;
                }
            }

            for(int i=src.rows-1;i>0;i--)
            {
                uchar *ptr=src.ptr<uchar>(i);
                if(ptr[j]==255)
                {
                    bottom=i;
                    break;
                }
            }

            if(top&&bottom)
            {
                h_c[int((top+bottom)/2)]++;
            }
        }

        int h_center=std::max_element(h_c,h_c+src.rows)-h_c;
        int w_center=std::max_element(w_c,w_c+src.cols)-w_c;

        //cv::circle(dst3,cv::Point(w_center,h_center),100,cv::Scalar(255),1);
        dst4=0;
        int r=80;
        get_area(dst3,dst4, w_center,h_center,0,r);
        //GaussianBlur(dst4, dst5, cv::Size(7, 7), 0, 0);
        //cv::bilateralFilter(dst4,dst5,5,5*2,5/2);
        cv::medianBlur(dst4,dst5,3);



        cv::threshold(dst5,dst4 ,0,255,CV_THRESH_OTSU);



        int ll,rr;
        for(int i=h_center-r;i<h_center+r;i++)
        {
            uchar *ptr=dst4.ptr<uchar>(i);

            ll=0,rr=0;
            for(int j=w_center-r;j<w_center+r;j++)
            {
                if(ptr[j])
                {
                    ll=j;
                    break;
                }
            }
            for(int j=w_center+r;j>w_center-r;j--)
            {
                if(ptr[j])
                {
                    rr=j;
                    break;
                }
            }
            if(ll==0||rr==0)
            {
                continue;
            }

            for(int j=ll;j<=rr;j++)
            {
                ptr[j]=255-ptr[j];
            }
        }



        cv::Mat kernel = getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5), cv::Point(-1, -1));
        morphologyEx(dst4, dst4, cv::MORPH_CLOSE, kernel);


        findContours(dst4, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));


        int largest_area = 0;
        int largest_contour_index = 0;

        for (int i = 0; i < contours.size(); i++) {
            int a = contours[i].size();
            if (a > largest_area) {
                largest_area = a;
                largest_contour_index = i;
            }
        }

        if(contours.size()==0)
        {
            std::cout<<0<<"\n";
        }
        else
        {
            std::cout<<contours[largest_contour_index].size()<<"\n";
        }
    return contours[largest_contour_index].size();

}







void MyThread::run()
{
    qDebug() << "当前线程对象的地址：" << QThread::currentThread();

//    cv::Mat src=cv::imread("/home/zw/nn.bmp",0);
//    emit sent_mat(src);

    int nRet = MV_OK;

     void* handle = NULL;
     unsigned char * pData = NULL;

     unsigned char *pDataForSaveImage = NULL;
     do
     {
         MV_CC_DEVICE_INFO_LIST stDeviceList;
         memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));

         // 枚举设备
         // enum device
         nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &stDeviceList);
         if (MV_OK != nRet)
         {
             printf("MV_CC_EnumDevices fail! nRet [%x]\n", nRet);
             break;
         }

         printf("camera index:");

         unsigned int nIndex = 0;


         // 选择设备并创建句柄
         // select device and create handle
         nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]);
         if (MV_OK != nRet)
         {
             printf("MV_CC_CreateHandle fail! nRet [%x]\n", nRet);
             break;
         }

         // 打开设备
         // open device
         nRet = MV_CC_OpenDevice(handle);
         if (MV_OK != nRet)
         {
             printf("MV_CC_OpenDevice fail! nRet [%x]\n", nRet);
             break;
         }

         // ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera)
         if (stDeviceList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE)
         {
             printf("this is Gige carame\n");
             int nPacketSize = MV_CC_GetOptimalPacketSize(handle);
             if (nPacketSize > 0)
             {
                 nRet = MV_CC_SetIntValue(handle,"GevSCPSPacketSize",nPacketSize);
                 if(nRet != MV_OK)
                 {
                     printf("Warning: Set Packet Size fail nRet [0x%x]!\n", nRet);
                 }
             }
             else
             {
                 printf("Warning: Get Packet Size fail nRet [0x%x]!\n", nPacketSize);
             }
         }


         // ch:获取数据包大小 | en:Get payload size
         MVCC_INTVALUE stParam;
         memset(&stParam, 0, sizeof(MVCC_INTVALUE));
         nRet = MV_CC_GetIntValue(handle, "PayloadSize", &stParam);
         if (MV_OK != nRet)
         {
             printf("Get PayloadSize fail! nRet [0x%x]\n", nRet);
             break;
         }

         // 开始取流
         // start grab image
         nRet = MV_CC_StartGrabbing(handle);
         if (MV_OK != nRet)
         {
             printf("MV_CC_StartGrabbing fail! nRet [%x]\n", nRet);
             break;
         }

         MV_FRAME_OUT pFrame={0};
         memset(&pFrame,0,sizeof(MV_FRAME_OUT));

         memset(&pFrame.stFrameInfo, 0, sizeof(MV_FRAME_OUT_INFO_EX));
         pFrame.pBufAddr = (unsigned char *)malloc(sizeof(unsigned char) * stParam.nCurValue);

         if (NULL == pFrame.pBufAddr)
         {
             break;
         }

         int img_count=751;



         for(int mmm=1;mmm<31;mmm++)
         {


             nRet=MV_CC_GetImageBuffer(handle,&pFrame,1000);
             pDataForSaveImage = (unsigned char*)malloc(pFrame.stFrameInfo.nWidth * pFrame.stFrameInfo.nHeight+2048);
             if (NULL == pDataForSaveImage)
             {
                 break;
             }
             MV_SAVE_IMAGE_PARAM_EX stSaveParam;
             memset(&stSaveParam, 0, sizeof(MV_SAVE_IMAGE_PARAM_EX));
             stSaveParam.enImageType = MV_Image_Bmp;
             stSaveParam.enPixelType = pFrame.stFrameInfo.enPixelType;
             stSaveParam.nBufferSize = pFrame.stFrameInfo.nHeight*pFrame.stFrameInfo.nWidth+1078;
             stSaveParam.nWidth      = pFrame.stFrameInfo.nWidth;
             stSaveParam.nHeight     = pFrame.stFrameInfo.nHeight;
             stSaveParam.pData       = pFrame.pBufAddr;
             stSaveParam.nDataLen    = pFrame.stFrameInfo.nFrameLen;
             stSaveParam.pImageBuffer = pDataForSaveImage;


             nRet = MV_CC_SaveImageEx2(handle, &stSaveParam);

              cv::Mat src=cv::Mat(stSaveParam.nHeight,stSaveParam.nWidth,CV_8UC1,stSaveParam.pImageBuffer+1078,0);

             //std::cout<<"get a frame!"<<std::endl;
             std::string save_path="/home/zw/image/7_12/"+std::to_string(img_count)+".bmp";

             char path[100];
             strcpy(path,save_path.c_str());


             cv::imwrite(path,src);
img_count++;





             cv::Mat cameraMatrix;
             cv::Mat distCoeffs;
             cv::Mat dst,dst2,dst3,dst4,dst5;
             calibParaL(cameraMatrix,distCoeffs);

             int x0,y0,r;

             coarse_position(src,dst,900,100,200,1000);

             cv::undistort(dst,dst2,cameraMatrix,distCoeffs);

             int s=0,s1=0,s2=0,s3=0,s4=0,s5=0,s6=0;
             cv::threshold(dst2,dst3,0,255,CV_THRESH_OTSU);

             cv::Mat kernel1 = getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3), cv::Point(-1, -1));
             morphologyEx(dst3, dst5, cv::MORPH_CLOSE, kernel1);

             //cv::imwrite(bw,dst5);
             find_circul_center(dst3,y0,x0,r,s);
             cv::Mat show_img;
             get_area(dst2,show_img,x0,y0,0,r);
             cv::Rect rect(x0-r,y0-r,r*2,r*2);
             show_img=show_img(rect);

             cv::Mat temp_img=show_img;
             std::string save_path_rect="/home/zw/image/7_13/rect/"+std::to_string(1)+".bmp";
             //cv::resize(show_img,temp_img,cv::Size(400,400),0,0,cv::INTER_LINEAR);
             cv::imwrite(save_path_rect,temp_img);
             QString send_path=QString::fromStdString(save_path_rect);
             emit sent_img(send_path);

             //std::cout<<"参数为：   "<<r<<"\n";
             double c1=0.71*r;    //此处值为120.7，本意为121
             double c2=0.76*r;     //此处为129.2  ，意为130
             double c3=0.77*r;     //为130.9 ，  意为131
             double c4=0.96*r;     //149.6
             double c5=0.30*r;      // 51
             double c6=0.80*r;      //136  ,即为之前的135
             double c7=0.75*r;

             double c8=0.785*r;

             //此处为定位锁孔位置，得到两个锁孔的四个夹角theta1，theta2，theta3，theta4
             int begin=c1,end=c3;
             cv::Mat hh;
             get_area(dst2,hh,x0,y0,begin,end);
             cv::threshold(hh,dst3,0,255,CV_THRESH_OTSU);
             //cv::imwrite(sp,dst3);
             int theta1,theta2,theta3,theta4;
             alignment(dst3,x0,y0,begin,end,theta1,theta2,theta3,theta4);
             cv::Mat kernel = getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3), cv::Point(-1, -1));


             //此处过锁孔,判断内环，c2为130，内环的中间位置
             begin=0,end=c2+3;
             hh=0;
             get_area(dst2,hh,x0,y0,begin,end);
             cv::threshold(hh,dst3,0,255,CV_THRESH_OTSU);
             //cv::imwrite(bw,dst3);
             morphologyEx(dst3, dst5, cv::MORPH_CLOSE, kernel);
             find_circul_center1(dst5,y0,x0,end,s1,theta1,theta2,theta3,theta4);
             //cv::imwrite(sp,dst5);



             //这里判断外环，c4为150，外环中间部分
             begin=0,end=c4;
             hh=0;
             get_area(dst2,hh,x0,y0,begin,end);
             cv::threshold(hh,dst3,0,255,CV_THRESH_OTSU);
             //cv::imwrite(rp,dst3);
             morphologyEx(dst3, dst5, cv::MORPH_CLOSE, kernel);
             find_circul_center1(dst5,y0,x0,end,s2,theta1,theta2,theta3,theta4);
             //cv::imwrite(sp,dst5);



             //这里判断内圆，c5为50，检测内圆
             begin=0,end=c5;
             hh=0;
             get_area(dst2,hh,x0,y0,begin,end);
             //cv::imwrite(bw,hh);
             cv::threshold(hh,dst3,0,255,CV_THRESH_OTSU);
             //cv::imwrite(bw,dst3);
             morphologyEx(dst3, dst5, cv::MORPH_CLOSE, kernel);
             find_circul_center1(dst5,y0,x0,end,s3,theta1,theta2,theta3,theta4);





             //类似于之前135那个地方,也属于内环
             begin=0,end=c6-1;
             //end=c6+10;
             //end=c6+10;
             hh=0;
             get_area(dst2,hh,x0,y0,begin,end);
             cv::threshold(hh,dst3,0,255,CV_THRESH_OTSU);
             morphologyEx(dst3, dst5, cv::MORPH_CLOSE, kernel);
             //cv::imwrite(rp,dst3);
             find_circul_center1(dst5,y0,x0,end,s4,theta1,theta2,theta3,theta4);




//             begin=0,end=c7-1;
//             //cv::circle(dst2,cv::Point(x0,y0),127,cv::Scalar(0,255,0),-1);

//             //end=c6+10;
//             //end=c6+10;
//             hh=0;
//             get_area(dst2,hh,x0,y0,begin,end);
//             cv::threshold(hh,dst3,0,255,CV_THRESH_OTSU);
//             morphologyEx(dst3, dst5, cv::MORPH_CLOSE, kernel);
//             //cv::imwrite(rp,dst2);
//             find_circul_center1(dst5,y0,x0,end,s5,theta1,theta2,theta3,theta4);

     //        begin=c7,end=r;
     //
     //        hh=0;
     //        cv::threshold(dst2,dst3,0,255,CV_THRESH_OTSU);
     //        get_area(dst3,hh,x0,y0,begin,end);
     //        //cv::threshold(hh,dst3,0,255,CV_THRESH_OTSU);
     //
     //        cv::Mat kernel1 = getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3), cv::Point(-1, -1));
     //        morphologyEx(hh, dst5, cv::MORPH_CLOSE, kernel1);
     //        cv::imwrite(rp,dst5);
     //        s5=get_out_circul(dst5,y0,x0,begin,end);
             //find_circul_center1(dst5,y0,x0,end,s4,theta1,theta2,theta3,theta4);



             begin=0,end=c8;
             //cv::circle(dst2,cv::Point(x0,y0),127,cv::Scalar(0,255,0),-1);

             //end=c6+10;
             //end=c6+10;
             hh=0;
             get_area(dst2,hh,x0,y0,begin,end);
             cv::threshold(hh,dst3,0,255,CV_THRESH_OTSU);
             morphologyEx(dst3, dst5, cv::MORPH_CLOSE, kernel);
             //cv::imwrite(rp,dst2);
             find_circul_center1(dst5,y0,x0,end,s6,theta1,theta2,theta3,theta4);


             int flag=0;

             if(s>2000||s1>4500||s2>2000||s3>2000||s4>2000||s5>15000||s6>3500)
             {
                 flag=1;
             }


             std::cout<<img_count<<".";
             //std::cout<<"方差为：   "<<s<<" "<<s1<<" "<<s2<<" "<<s3<<" "<<s4<<" "<<s5<<"\n";
             if(flag)
             {



                 emit sent_a(send_path,1);
                 std::cout<<"   bad"<<"\n";
             }
             else
             {
                 emit sent_a(send_path,0);
                 std::cout<<"   good"<<"\n";
             }

















//             FILE* fp = fopen(path, "wb");
//             if (NULL == fp)
//             {
//                 printf("fopen failed\n");
//                 break;
//             }

//             fwrite(stSaveParam.pImageBuffer,1,stSaveParam.nHeight*stSaveParam.nWidth+1078,fp);
//             fclose(fp);



             nRet=MV_CC_FreeImageBuffer(handle,&pFrame);
             if (pDataForSaveImage)
             {
                 free(pDataForSaveImage);
                 pDataForSaveImage = NULL;
             }


         }

         // 停止取流
         // end grab image
         nRet = MV_CC_StopGrabbing(handle);
         if (MV_OK != nRet)
         {
             printf("MV_CC_StopGrabbing fail! nRet [%x]\n", nRet);
             break;
         }

         // 销毁句柄
         // destroy handle
         nRet = MV_CC_DestroyHandle(handle);
         if (MV_OK != nRet)
         {
             printf("MV_CC_DestroyHandle fail! nRet [%x]\n", nRet);
             break;
         }
     } while (0);

     if (nRet != MV_OK)
     {
         if (handle != NULL)
         {
             MV_CC_DestroyHandle(handle);
             handle = NULL;
         }
     }

     if (pData)
     {
         free(pData);
         pData = NULL;
     }


     printf("exit.\n");
    qDebug() << "run() 执行完毕，子程序退出...";
}
