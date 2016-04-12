#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <string>
#include <QtCore>
#include <algorithm>    // std::shuffle
#include <array>        // std::array
#include <random>       // std::default_random_engine
#include <chrono>       // std::chrono::system_clock
#include <iterator>

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
    timerControl->start(2000);
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
                cells[j][k] = 1;
                counterForFishes++;
            }
            if(counterForSharks<thresholdForSharks && counterForFishes==thresholdForFishes){
                cells[j][k] = -1;
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

    int previousGenerationCells[M][N];
    //copy(begin(cells), end(cells), begin(previousGenerationCells));
    //qDebug()<< sizeof(cells);

    memcpy(*previousGenerationCells,*cells,sizeof(cells));

    for(int i=0; i<M; i++){
        for(int j=0; j<N; j++){
            if(i!=0 || i!=M-1 || j!=0 || j!=N-1){
                int fishCounter = 0;
                int sharkCounter = 0;

                int fishBreedingAge = 0;
                int sharkBreedingAge = 0;

                if(previousGenerationCells[i][j]==0){
                    for(int k=0; k<8; k++){

                        int i_tempValueUsedForSubing;
                        int j_tempValueUsedForSubing;

                        switch(k){

                        case 0:
                            i_tempValueUsedForSubing = -1;
                            j_tempValueUsedForSubing = -1;
                            break;
                        case 1:
                            i_tempValueUsedForSubing = -1;
                            j_tempValueUsedForSubing = 0;
                            break;
                        case 2:
                            i_tempValueUsedForSubing = -1;
                            j_tempValueUsedForSubing = 1;
                            break;
                        case 3:
                            i_tempValueUsedForSubing = 0;
                            j_tempValueUsedForSubing = -1;
                            break;
                        case 4:
                            i_tempValueUsedForSubing = 0;
                            j_tempValueUsedForSubing = 1;
                            break;
                        case 5:
                            i_tempValueUsedForSubing = 1;
                            j_tempValueUsedForSubing = -1;
                            break;
                        case 6:
                            i_tempValueUsedForSubing = 1;
                            j_tempValueUsedForSubing = 0;
                            break;
                        case 7:
                            i_tempValueUsedForSubing = 1;
                            j_tempValueUsedForSubing = 1;
                            break;
                        }

                        if(previousGenerationCells[i+i_tempValueUsedForSubing][j+j_tempValueUsedForSubing]>0){
                            fishCounter++;
                            if(previousGenerationCells[i-1][j-1]>=3){
                                fishBreedingAge++;
                            }
                        }

                        if(previousGenerationCells[i+i_tempValueUsedForSubing][j+j_tempValueUsedForSubing]<0){
                            sharkCounter++;
                            if(previousGenerationCells[i-1][j-1]<=4){
                                sharkBreedingAge++;
                            }
                        }
                    }

                    if(fishCounter>=4 && fishBreedingAge>=3 && sharkCounter<4){
                        cells[i][j] = 1;
                    }
                    if(sharkCounter>=4 && sharkBreedingAge>=3 && fishCounter<4){
                        cells[i][j] = -1;
                    }
                }
                else if(previousGenerationCells[i][j]>0){
                    cells[i][j] = (cells[i][j])+1;
                }
                else if(previousGenerationCells[i][j]<0){
                    cells[i][j] = (cells[i][j])-1;
                }
            }
        }
    }

    matImageColourConversion();

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
