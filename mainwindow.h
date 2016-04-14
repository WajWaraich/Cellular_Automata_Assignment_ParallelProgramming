#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include "opencv2/video/tracking.hpp"
#include <opencv2/core/utility.hpp>
#include "opencv2/videoio.hpp"

#include <omp.h>

#include <iostream>
#include <string>
#include <QtCore>
#include <algorithm>    // std::shuffle
#include <array>        // std::array
#include <random>       // std::default_random_engine
#include <chrono>       // std::chrono::system_clock

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


public slots:
    void updateGUI();

private slots:
    void initialisation();
    void matImageColourConversion();

private:
    QTimer* timerControl;

    static const int M = 300; //300 | 700 | 800 | 1400 | 1800
    static const int N = 302; //400 | 1400 | 850 | 1402 | 1802

    int cells[M][N];

    QTime timer;

    cv::Mat matImage;

    int generationCounter;

    Ui::MainWindow *ui;

};

#endif // MAINWINDOW_H
