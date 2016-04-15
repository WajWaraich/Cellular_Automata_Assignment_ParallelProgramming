#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <string>
#include <QtCore>
#include <algorithm>
#include <array>
#include <random>
#include <chrono>
#include <iterator>

using namespace std;
using namespace cv;

int argcMainWindow;
char argvMainWindow[];


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    initialisation(); //Initialisation function is called which
                      //initialisis the grid with random states

    generationCounter = 0; //Counter used to calculate the generations

    //Timer is used to execute the updateGUI() function in an inifinite loop
    timerControl = new QTimer(this);
    connect(timerControl,SIGNAL(timeout()),this,SLOT(updateGUI()));
    timerControl->start(0);
}

void MainWindow::initialisation(){

    timer.start();

    this->ui->radioButton->setChecked(false);

    /*=============================================================================
     | Code used for testing MPI, got it to print out the number of prcessors and
     | the name of the machine that is hosting it, additionally managed to get
     | the rank working.
     *===========================================================================*/

    //    MPI_Init(NULL, NULL);
    //    MPI_Comm_size(MPI_COMM_WORLD,&nprocs);
    //    MPI_Comm_rank(MPI_COMM_WORLD,&myid);

    //    qDebug() << "nprocs" << nprocs;
    //    qDebug() << "myid" << myid;
    //    char processor_name[MPI_MAX_PROCESSOR_NAME];

    //    MPI_Comm_size(MPI_COMM_WORLD,&numberOfProcessors);
    //    qDebug() << "Number of processor = " << numberOfProcessors;

    //    MPI_Get_processor_name(processor_name, &namelen);
    //    qDebug()<<"Process Name:" << processor_name;


    int tempCells[M*N];

    int counterForFishes = 0;
    int counterForSharks = 0;
    int counterForOcean = 0;

    int thresholdForFishes = (M*N)*0.50;
    int thresholdForSharks = (M*N)*0.25;
    int thresholdForOcean = (M*N)*0.25;

    /*=============================================================================
     | Initialisation of the grid with all the fishes, sharks and the ocean
     | reandomised - the randomisation comes from the shuffle function using
     | a seed and a temp array is used to convert between a 1D array and a 2D array
     *===========================================================================*/

    for(int j=0; j<=M; j++) {
        for(int k=0; k<=N; k++) {

            if(counterForFishes<thresholdForFishes){
                cells[j][k] = 1;
                counterForFishes++;
            }
            else if(counterForSharks<thresholdForSharks && counterForFishes==thresholdForFishes){
                cells[j][k] = -1;
                counterForSharks++;
            }
            else if(counterForOcean<thresholdForOcean && counterForSharks==thresholdForSharks && counterForFishes==thresholdForFishes){
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

    if(this->ui->radioButton->isChecked()){
        matImageColourConversion();
    }

    matImageColourConversion();
    imwrite("initalGrid.jpg",matImage);
}

/*=============================================================================
 | Function responsible for converting the cells array into a mat image
 | with the correct colours, this code can be executed in parallel as it is
 | purely used for displaying purposes
 *===========================================================================*/
void MainWindow::matImageColourConversion(){

    matImage = Mat(M,N, CV_8UC3);

    for (int i=0; i<N; i++){
        for(int j=0; j<M; j++){

            //black = shark
            //green = fish
            //white = ocean

            if(cells[i][j]<0){
                Vec3b black;
                black[0] = 0;
                black[1] = 0;
                black[2] = 0;
                matImage.at<Vec3b>(Point(i,j)) = black;
            }

            else if(cells[i][j]>0){
                cv::Vec3b green;
                green[0] = 0;
                green[1] = 255;
                green[2] = 0;
                matImage.at<Vec3b>(Point(i,j)) = green;
            }

            else if(cells[i][j]==0){
                cv::Vec3b white;
                white[0] = 255;
                white[1] = 255;
                white[2] = 255;
                matImage.at<Vec3b>(Point(i,j)) = white;
            }
        }
    }
}

/*=============================================================================
 | updateGUI() function responsible for computing and displaying each generation
 | of cells, updateGUI() is inside a infinite for loop hence it will be executed
 | for every generation
 *===========================================================================*/
void MainWindow::updateGUI(){

    int previousGenerationCells[M][N];

    memcpy(*previousGenerationCells,*cells,sizeof(cells)); //copying the cells array over to a temporary array
                                                           //i.e. it is a snapshot of the cells array
                                                           //that is used for decision making

#pragma omp parallel num_threads(8) //OpenMP code associated with setting the amount of threads
    {
#pragma omp for //OpenMP code referring to the "for" directive
        for(int i=0; i<M; i++){
            for(int j=0; j<N; j++){

                int randomNumber = rand() % 32 + 1; //Random int variable used for killing a shark randomly

                int fishCounter = 0;
                int sharkCounter = 0;

                int fishBreedingAge = 0;
                int sharkBreedingAge = 0;

                /*=============================================================================
                 | Computing neighbouring cells of any boarder cells, i.e. ghost cells are used
                 | to compute the state of the boarder cell, the main point of this is to
                 | seperate the boarder cells from all the other cells
                 *===========================================================================*/

                if(i==0 || i==M-1 || j==0 || j==N-1){
                    int forSubbingPurposes1;
                    int forSubbingPurposes2;



                    if(i==0 || i==M-1){

                        int i_tempValueUsedForSubing;
                        int j_tempValueUsedForSubing;

                        if(i==0){
                            forSubbingPurposes1 = M-1;
                            forSubbingPurposes2 = 1;
                        }
                        else if(i==(M-1)){
                            forSubbingPurposes1 = -(M-1);
                            forSubbingPurposes2 = -1;
                        }

                        for(int k=0; k<8; k++){

                            //Simple switch case used to loop through all the 8 neighbours
                            switch(k){

                            case 0:
                                i_tempValueUsedForSubing = forSubbingPurposes1;
                                j_tempValueUsedForSubing = -1;
                                break;
                            case 1:
                                i_tempValueUsedForSubing = forSubbingPurposes1;
                                j_tempValueUsedForSubing = 0;
                                break;
                            case 2:
                                i_tempValueUsedForSubing = forSubbingPurposes1;
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
                                i_tempValueUsedForSubing = forSubbingPurposes2;
                                j_tempValueUsedForSubing = -1;
                                break;
                            case 6:
                                i_tempValueUsedForSubing = forSubbingPurposes2;
                                j_tempValueUsedForSubing = 0;
                                break;
                            case 7:
                                i_tempValueUsedForSubing = forSubbingPurposes2;
                                j_tempValueUsedForSubing = 1;
                                break;
                            }

                            if(previousGenerationCells[i+i_tempValueUsedForSubing][j+j_tempValueUsedForSubing]>0){
                                fishCounter++;
                                if(previousGenerationCells[i+i_tempValueUsedForSubing][j+j_tempValueUsedForSubing]>=3){
                                    fishBreedingAge++;
                                }
                            }

                            if(previousGenerationCells[i+i_tempValueUsedForSubing][j+j_tempValueUsedForSubing]<0){
                                sharkCounter++;
                                if(previousGenerationCells[i+i_tempValueUsedForSubing][j+j_tempValueUsedForSubing]<=4){
                                    sharkBreedingAge++;
                                }
                            }
                        }
                    }

                    else{

                        int i_tempValueUsedForSubing;
                        int j_tempValueUsedForSubing;

                        if(j==0){
                            forSubbingPurposes1 = N-1;
                            forSubbingPurposes2 = 1;
                        }
                        else if(j==(N-1)){
                            forSubbingPurposes1 = -(N-1);
                            forSubbingPurposes2 = -1;
                        }

                        for(int k=0; k<8; k++){

                            //Simple switch case used to loop through all the 8 neighbours
                            switch(k){

                            case 0:
                                i_tempValueUsedForSubing = -1;
                                j_tempValueUsedForSubing = forSubbingPurposes1;
                                break;
                            case 1:
                                i_tempValueUsedForSubing = 0;
                                j_tempValueUsedForSubing = forSubbingPurposes1;
                                break;
                            case 2:
                                i_tempValueUsedForSubing = 1;
                                j_tempValueUsedForSubing = forSubbingPurposes1;
                                break;
                            case 3:
                                i_tempValueUsedForSubing = -1;
                                j_tempValueUsedForSubing = 0;
                                break;
                            case 4:
                                i_tempValueUsedForSubing = 1;
                                j_tempValueUsedForSubing = 0;
                                break;
                            case 5:
                                i_tempValueUsedForSubing = -1;
                                j_tempValueUsedForSubing = forSubbingPurposes2;
                                break;
                            case 6:
                                i_tempValueUsedForSubing = 0;
                                j_tempValueUsedForSubing = forSubbingPurposes2;
                                break;
                            case 7:
                                i_tempValueUsedForSubing = 1;
                                j_tempValueUsedForSubing = forSubbingPurposes2;
                                break;
                            }
                            if(previousGenerationCells[i+i_tempValueUsedForSubing][j+j_tempValueUsedForSubing]>0){
                                fishCounter++;
                                if(previousGenerationCells[i+i_tempValueUsedForSubing][j+j_tempValueUsedForSubing]>=3){
                                    fishBreedingAge++;
                                }
                            }

                            if(previousGenerationCells[i+i_tempValueUsedForSubing][j+j_tempValueUsedForSubing]<0){
                                sharkCounter++;
                                if(previousGenerationCells[i+i_tempValueUsedForSubing][j+j_tempValueUsedForSubing]<=4){
                                    sharkBreedingAge++;
                                }
                            }
                        }

                    }

                }
                /*=============================================================================
                 | Computing all the neighbouring cells of any non-boarder cells, calculating
                 | the amount of fishes and sharks that around that particular (i,j) cell and
                 | appending the counters accordingly which will be used later for decision-making
                 *===========================================================================*/
                else{
                    for(int k=0; k<8; k++){

                        int i_tempValueUsedForSubing;
                        int j_tempValueUsedForSubing;

                        //Simple switch case used to loop through all the 8 neighbours
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
                            if(previousGenerationCells[i+i_tempValueUsedForSubing][j+j_tempValueUsedForSubing]>=3){
                                fishBreedingAge++;
                            }
                        }

                        if(previousGenerationCells[i+i_tempValueUsedForSubing][j+j_tempValueUsedForSubing]<0){
                            sharkCounter++;
                            if(previousGenerationCells[i+i_tempValueUsedForSubing][j+j_tempValueUsedForSubing]<=4){
                                sharkBreedingAge++;
                            }
                        }
                    }
                }

                /*=============================================================================
                 | All the if statements referring to the set of rules, using the temporary
                 | cells array to base the decisions on and then modifying the actual cells array
                 | with the state that cell will be in the next generation
                 *===========================================================================*/
                if(previousGenerationCells[i][j]==0){

                    if(fishCounter>=4 && fishBreedingAge>=3 && sharkCounter<4){
                        cells[i][j] = 1;
                    }
                    if(sharkCounter>=4 && sharkBreedingAge>=3 && fishCounter<4){
                        cells[i][j] = -1;
                    }
                }

                else if(previousGenerationCells[i][j]>0 && previousGenerationCells[i][j]==11){
                    cells[i][j] = 0;
                }

                else if(previousGenerationCells[i][j]<0 && abs(previousGenerationCells[i][j])==21){
                    cells[i][j] = 0;
                }

                else if(previousGenerationCells[i][j]<0 && sharkCounter>=6 && fishCounter==0){
                    cells[i][j] = 0;
                }

                else if(previousGenerationCells[i][j]>0 && sharkCounter>=5){
                    cells[i][j] = 0;
                }

                else if(previousGenerationCells[i][j]>0 && fishCounter==8){
                    cells[i][j] = 0;
                }

                else if(previousGenerationCells[i][j]<0 && randomNumber==5){
                    cells[i][j] = 0;
                }

                else if(previousGenerationCells[i][j]>0){
                    cells[i][j] = (previousGenerationCells[i][j])+1;
                }

                else if(previousGenerationCells[i][j]<0){
                    cells[i][j] = (previousGenerationCells[i][j])-1;
                }
            }
        }
    }

    if(this->ui->radioButton->isChecked()){
        matImageColourConversion(); //Calling the matImageColourConversion() function to convert the cells array into a graphical form
    }

    /*=============================================================================
     | Displaying the results using QImage and applying the Pixmap to the label
     *===========================================================================*/

    if(this->ui->radioButton->isChecked()){
        QImage image((uchar*)matImage.data, matImage.cols, matImage.rows, matImage.step, QImage::Format_RGB888);
        //ui->label->setPixmap(QPixmap::fromImage(image));
        ui->label->setPixmap(QPixmap::fromImage(image.scaled(850,800,Qt::KeepAspectRatio,Qt::FastTransformation)));

    }

    /*=============================================================================
     | For bench marking purposes and to finalise the MPI call
     *===========================================================================*/
    if(generationCounter==1000){
        int timeElapsedInMiliseconds = timer.elapsed();
        this->ui->lcdNumber_2->display(timeElapsedInMiliseconds);
        qDebug() << "timeElapsedInMiliseconds:" << timeElapsedInMiliseconds;
        MPI_Finalize();
    }

    this->ui->lcdNumber->display(generationCounter); //Used for displaying the counter in real-time

    /*=============================================================================
     | Just used to store the image at different intervals for the report
     *===========================================================================*/

    if(generationCounter==100){
        matImageColourConversion();
        imwrite("100Grid.jpg",matImage);
    }

    if(generationCounter==500){
        matImageColourConversion();
        imwrite("500Grid.jpg",matImage);
    }

    if(generationCounter==1000){
        matImageColourConversion();
        imwrite("1000Grid.jpg",matImage);
    }

    if(generationCounter==2000){
        matImageColourConversion();
        imwrite("2000Grid.jpg",matImage);
    }

    if(generationCounter==4000){
        matImageColourConversion();
        imwrite("4000Grid.jpg",matImage);
    }

    if(generationCounter==8000){
        matImageColourConversion();
        imwrite("8000Grid.jpg",matImage);
    }

    if(generationCounter==10000){
        matImageColourConversion();
        imwrite("10000Grid.jpg",matImage);
    }

    generationCounter++;


}

MainWindow::~MainWindow()
{
    delete ui;
}
