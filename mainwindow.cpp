#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <string>
#include <QtCore>
#include <algorithm>    // std::shuffle
#include <array>        // std::array
#include <random>       // std::default_random_engine
#include <chrono>       // std::chrono::system_clock

using namespace std;
using namespace cv;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    initialisation();

    timerControl = new QTimer(this);
    connect(timerControl,SIGNAL(timeout()),this,SLOT(updateGUI()));
    timerControl->start(20);
}

void MainWindow::initialisation(){

    int cells[M][N];
    int tempCells[M*N];

    int counterForFishes = 0;
    int counterForSharks = 0;
    int counterForOcean = 0;

    int thresholdForFishes = (M*N)*0.5;
    int thresholdForSharks = (M*N)*0.25;
    int thresholdForOcean = (M*N)*0.25;



    for(int j=0; j<=M; j++) {
        for(int k=0; k<=N; k++) {

            if(counterForFishes<thresholdForFishes){
                cells[j][k] = 255;
                counterForFishes++;
            }
            if(counterForSharks<thresholdForSharks && counterForFishes==thresholdForFishes){
                cells[j][k] = 100;
                counterForSharks++;
            }
            if(counterForOcean<thresholdForOcean && counterForSharks==thresholdForSharks && counterForFishes==thresholdForFishes){
                cells[j][k] = 0;
                counterForOcean++;
            }
        }
    }

    for(int i=0; i<=M; i++){
        for(int j=0; j<=N; j++) {
            tempCells[i * M + j] = cells[i][j];
        }

    }


    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();

    shuffle (begin(tempCells), end(tempCells), std::default_random_engine(seed));

    for(int i=0; i<M*N; i++){
        cells[i / N][i % N] = tempCells[i];
    }


    matImage = Mat(M,N, CV_8UC3 ,&cells); //CV_16UC1 worked well but didn't show stuff with other intensities, CV_32S works well, will need to try CV_8UC3

    matImageColourConversion();
}

void MainWindow::matImageColourConversion(){
    for (int i=0; i<N; i++){
        for(int j=0; j<M; j++){

           Vec3b pixValue = matImage.at<Vec3b>(Point(i,j));

//            qDebug() << "pixValue[0]" << pixValue[0];
//            qDebug() << "pixValue[1]" << pixValue[1];
//            qDebug() << "pixValue[2]" << pixValue[2];
//            qDebug() << "--------------------------------";


            if(pixValue[0]==255 || pixValue[1]==255 || pixValue[2]==255){
                cv::Vec3b yellow;
                yellow[0] = 0;
                yellow[1] = 255;
                yellow[2] = 255;
                matImage.at<cv::Vec3b>(cv::Point(i,j)) = yellow;
            }

            else if(pixValue[0]==100 || pixValue[1]==100 || pixValue[2]==100){
                cv::Vec3b red;
                red[0] = 0;
                red[1] = 0;
                red[2] = 255;
                matImage.at<cv::Vec3b>(cv::Point(i,j)) = red;
            }

            else if(pixValue[0]==0 || pixValue[1]==0 || pixValue[2]==0){
                cv::Vec3b blue;
                blue[0] = 255;
                blue[1] = 0;
                blue[2] = 0;
                matImage.at<cv::Vec3b>(cv::Point(i,j)) = blue;
            }
        }
    }
}

void MainWindow::updateGUI(){
    //imwrite("test.jpg",matImage);

    namedWindow("test");
    imshow("test",matImage);

    //imshow("test2",imread("test.jpg"));

    QImage image((uchar*)matImage.data, matImage.cols, matImage.rows, matImage.step, QImage::Format_RGB888); //Format_RGB888 or Format_Indexed8
    ui->label->setPixmap(QPixmap::fromImage(image));

    //image.save("X:\\Dropbox\\University Work\\Real Time Programming\\Week 2\\build-GameOfLife-Desktop_Qt_5_5_1_MSVC2012_32bit-Debug\\QImageSAVE.jpg","JPG",-1);
}

MainWindow::~MainWindow()
{
    delete ui;
}
