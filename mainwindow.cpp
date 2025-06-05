#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mythread.h"
#include <QDebug>
#include<cstring>
#include <qpainter.h>
#include<iostream>






static QImage cvMat_to_QImage(const cv::Mat &mat ) {
  switch ( mat.type() )
  {
     // 8-bit, 4 channel
     case CV_8UC4:
     {
        QImage image( mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB32 );
        return image;
     }

     // 8-bit, 3 channel
     case CV_8UC3:
     {
        QImage image( mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888 );
        return image.rgbSwapped();
     }

     // 8-bit, 1 channel
     case CV_8UC1:
     {
        static QVector<QRgb>  sColorTable;
        // only create our color table once
        if ( sColorTable.isEmpty() )
        {
           for ( int i = 0; i < 256; ++i )
              sColorTable.push_back( qRgb( i, i, i ) );
        }
        QImage image( mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Indexed8 );
        image.setColorTable( sColorTable );
        return image;
     }

     default:
        qDebug("Image format is not supported: depth=%d and %d channels\n", mat.depth(), mat.channels());
        break;
  }
  return QImage();
}






cv::Mat QImage_to_cvMat( const QImage &image, bool inCloneImageData = true ) {
  switch ( image.format() )
  {
     // 8-bit, 4 channel
     case QImage::Format_RGB32:
     {
        cv::Mat mat( image.height(), image.width(), CV_8UC4, const_cast<uchar*>(image.bits()), image.bytesPerLine() );
        return (inCloneImageData ? mat.clone() : mat);
     }

     // 8-bit, 3 channel
     case QImage::Format_RGB888:
     {
        if ( !inCloneImageData ) {
           qWarning() << "ASM::QImageToCvMat() - Conversion requires cloning since we use a temporary QImage";
        }
        QImage swapped = image.rgbSwapped();
        return cv::Mat( swapped.height(), swapped.width(), CV_8UC3, const_cast<uchar*>(swapped.bits()), swapped.bytesPerLine() ).clone();
     }

     // 8-bit, 1 channel
     case QImage::Format_Indexed8:
     {
        cv::Mat  mat( image.height(), image.width(), CV_8UC1, const_cast<uchar*>(image.bits()), image.bytesPerLine() );

        return (inCloneImageData ? mat.clone() : mat);
     }

     default:
        qDebug("Image format is not supported: depth=%d and %d format\n", image.depth(), image.format());
        break;
  }

  return cv::Mat();
}




static QPixmap cvMatToQPixmap(const cv::Mat &inMat)
{
    return QPixmap::fromImage(cvMat_to_QImage(inMat));
}



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QPalette pa;
    pa.setColor(QPalette::WindowText,Qt::red);
    QFont ft;
    ft.setPointSize(20);
   // ui->pbn->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb(255, 0, 0);");

//    ui->pbn->setFont(ft);
//    ui->pbn->setPalette(pa);
//    ui->pbn_2->setFont(ft);
//    ui->pbn_2->setPalette(pa);


    QString global_path;
    this->setWindowFlags(Qt::WindowSystemMenuHint);
    this->setWindowTitle(QString("Battery"));
    ui->pbn->setStyleSheet("border:2px groove gray;border-radius:10px;padding:2px 4px;color: rgb(255, 255, 255); background-color: rgb(0, 0, 0);");
    ui->pbn_2->setStyleSheet("border:2px groove gray;border-radius:10px;padding:2px 4px;color: rgb(255, 255, 255); background-color: rgb(0, 0, 0);");
     QPalette palette(this->palette());

     palette.setColor(QPalette::Background, Qt::black);

    this->setPalette(palette);
//    QString ini_good_path="/home/alv/images/basic/16.bmp";
//    global_path=ini_good_path;
//    QPixmap ini_good_pix(global_path);
//    ui->label->setScaledContents(true);
//    ui->label->setPixmap(ini_good_pix);


   // this->setWindowFlags(QT::FramelessWindowHint);
    qDebug() << "主线程对象地址：" << QThread::currentThread();
    MyThread *subThread = new MyThread;
   int static global_img=0;
    connect(subThread, &MyThread::curNumber, this,
             [=](QString path) {
       // global_path=path;

//        const QString label_style =
//            "min-width:300px;min-height:300px;max-width:300px;max-height:300px;border-radius:150px;border:1px solid black;background:red";
//        ui->label->setStyleSheet(label_style);
        global_img++;
        std::cout<<"gloobal_img"<<global_img<<std::endl;
        QPixmap pix(path);
        ui->label->setScaledContents(true);
        ui->label->setPixmap(pix);
       // ui->label_3->setText(path);
      });






    connect(subThread, &MyThread::sent_img, this,
             [=](QString path) {

        QPixmap pix(path);


        ui->label->setScaledContents(true);
        ui->label->setPixmap(pix);

      });


//qRegisterMetaType("Mat");
//    connect(subThread, &MyThread::sent_mat, this,
//             [=](cv::Mat dev) {

//        QPixmap pix=cvMatToQPixmap(dev);


//        ui->label->setScaledContents(true);
//        ui->label->setPixmap(pix);

//      });



    connect(subThread,&MyThread::sent_a,this,[=](QString path,int a){
        if(a!=0)
        {
            QPalette pa;
            pa.setColor(QPalette::WindowText,Qt::red);
            QFont ft;
            ft.setPointSize(20);
            ui->label_2->setFont(ft);
            ui->label_2->setPalette(pa);
            ui->label_2->setText("BAD");
            QPixmap pix(path);
            ui->label_4->setScaledContents(true);
            ui->label_4->setPixmap(pix);






        }
        else{
            QPalette pa;
            pa.setColor(QPalette::WindowText,Qt::green);
            QFont ft;
            ft.setPointSize(20);
            ui->label_2->setFont(ft);
            ui->label_2->setPalette(pa);
            ui->label_2->setText("GOOD");




//            std::string locate_path="/home/alv/images/locate/"+std::to_string(a)+".bmp";
//            //QString oring_path=""

//            //QString qpath11=QString::fromStdString(org_path);
//            char loca_path1[100];

//            strcpy(loca_path1,locate_path.c_str());
//            FILE* fp2 = fopen(loca_path1, "wb");
//            if (NULL == fp2)
//            {
//                printf("fopen failed\n");
//            }
//             fwrite(stSaveParam.pImageBuffer,1,stSaveParam.nWidth*stSaveParam.nHeight,fp2);
//             fclose(fp2);


        }
        //ui->label_2->setNum(a);
    });

//    connect(subThread, &MyThread::curNumber, this,
//             [=](int num) { ui->label->setNum(num); });


     connect(ui->pbn, &QPushButton::clicked, this, [=]() {
         // 启动子线程
         subThread->start();
     });

     connect(ui->pbn_2,&QPushButton::clicked,this,[=](){
         subThread->terminate();
         this->close();
        //return a.exec();
     });

     // 线程资源释放
     connect(this, &MainWindow::destroyed, this, [=]() {
         subThread->quit();
         subThread->wait();
         subThread->deleteLater();
     });

}

MainWindow::~MainWindow()
{
    delete ui;
}

