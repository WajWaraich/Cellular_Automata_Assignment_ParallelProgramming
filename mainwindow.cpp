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

    generationCounter = 0;

    timerControl = new QTimer(this);
    connect(timerControl,SIGNAL(timeout()),this,SLOT(updateGUI()));
    timerControl->start(20);
}

void MainWindow::initialisation(){

    //cells[M][N]; //might need to add "int" before deceleration
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

    matImageColourConversion();
}

void MainWindow::matImageColourConversion(){

    matImage = Mat(M,N, CV_8UC3 ,&cells); //CV_16UC1 worked well but didn't show stuff with other intensities, CV_32S works well, will need to try CV_8UC3

    for (int i=0; i<M; i++){
        for(int j=0; j<N; j++){

            //black = shark
            //green = fish
            //white = ocean

            if(cells[i][j]<0){
                cv::Vec3b yellow;
                yellow[0] = 0;
                yellow[1] = 0;
                yellow[2] = 0;
                matImage.at<cv::Vec3b>(cv::Point(j,i)) = yellow;
            }

            else if(cells[i][j]>0){
                cv::Vec3b red;
                red[0] = 0;
                red[1] = 255;
                red[2] = 0;
                matImage.at<cv::Vec3b>(cv::Point(j,i)) = red;
            }

            else if(cells[i][j]==0){
                cv::Vec3b blue;
                blue[0] = 255;
                blue[1] = 255;
                blue[2] = 255;
                matImage.at<cv::Vec3b>(cv::Point(j,i)) = blue;
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

//--------------MADE THE PROGRAM VERY SLOW!------------------------------------
//            std::random_device rd; // obtain a random number from hardware
//            std::mt19937 eng(rd()); // seed the generator
//            std::uniform_int_distribution<> distr(1, 32); // define the range

//            for(int n=1; n<33; ++n)
//                std::cout << distr(eng) << ' '; // generate numbers

            int content = cells[i][j];

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

                else if(previousGenerationCells[i][j]==11){
                    cells[i][j] = 0;
                }

                else if(abs(previousGenerationCells[i][j])==21){
                    cells[i][j] = 0;
                }

                //For random killing of the shark
                /*else if(previousGenerationCells[i][j]<0 && distr(eng)==1){
                    cells[i][j] = 0;
                }*/

                else if(previousGenerationCells[i][j]<0 && sharkCounter>=6 && fishCounter==0){
                    cells[i][j] = 0;
                }

                else if(previousGenerationCells[i][j]>0 && sharkCounter>=5 || fishCounter==8){
                    cells[i][j] = 0;
                }

                else if(previousGenerationCells[i][j]>0){
                    cells[i][j] = content+1;
                }
                else if(previousGenerationCells[i][j]<0){
                    cells[i][j] = content-1;
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

    generationCounter++;

    this->ui->lcdNumber->display(generationCounter);
}

MainWindow::~MainWindow()
{
    delete ui;
}
