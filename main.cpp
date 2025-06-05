#include "mainwindow.h"
#include<iostream>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();



//    for(int i=0;i<10000000;i++)
//    {
//        std::cout<<i<<std::endl;
//    }
    return a.exec();
}
