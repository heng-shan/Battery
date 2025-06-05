#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QThread>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>
#include<opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>


#include<chrono>
#include <QApplication>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <pthread.h>
#include<QCoreApplication>
#include<QProcess>
#include<QTextStream>
#include<time.h>
#include <string.h>
#include <charconv>
#include <algorithm>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>
#include<opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <cmath>
#include <time.h>
#include <chrono>
#include<string.h>

#include "MvCameraControl.h"

class MyThread : public QThread
{
    Q_OBJECT
public:
    MyThread();
    ~MyThread();

protected:
    void run();

signals:
    // 自定义信号，传递数据
    void curNumber(QString path);
    void sent_a(QString path,int a);
    void sent_img(QString path);
    void sent_mat(cv::Mat src);
};



#endif // MYTHREAD_H
