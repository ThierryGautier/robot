#if 0
/**

 * @file video_webcam.cpp
 * @brief Background subtraction tutorial sample code
 * @author T.GAUTIER
 */

//opencv
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
//C
#include <stdio.h>
//C++
#include <iostream>
#include <sstream>

using namespace cv;
using namespace std;

// Global variables
Mat frame; //current frame
Mat fgMaskMOG2; //fg mask fg mask generated by MOG2 method
Ptr<BackgroundSubtractor> pMOG2; //MOG2 Background subtractor
int keyboard; //input from keyboard

/** Function Headers */
void help();
void processVideo(char* videoFilename);

void help()
{
    cout
    << "--------------------------------------------------------------------------" << endl
    << "This program shows how to use background subtraction methods provided by "  << endl
    << " OpenCV. You can process both videos (-vid)."                               << endl
                                                                                    << endl
    << "Usage:"                                                                     << endl
    << "./bs {-vid <video filename>}"                                               << endl
    << "for example: ./video_webcam -vid /dev/video0"                               << endl
    << "--------------------------------------------------------------------------" << endl
    << endl;
}

/**
 * @function main
 */
int main(int argc, char* argv[])
{
    //print help information
    help();

    //check for the input parameter correctness
    if(argc != 3) {
        cerr <<"Incorret input list" << endl;
        cerr <<"exiting..." << endl;
        return EXIT_FAILURE;
    }

    //create GUI windows
    namedWindow("Frame");
    namedWindow("FG Mask MOG 2");

    //create Background Subtractor objects
    pMOG2 = createBackgroundSubtractorMOG2(); //MOG2 approach

    if(strcmp(argv[1], "-vid") == 0) {
        //input data coming from a video
        processVideo(argv[2]);
    }
    else {
        //error in reading input parameters
        cerr <<"Please, check the input parameters." << endl;
        cerr <<"Exiting..." << endl;
        return EXIT_FAILURE;
    }
    //destroy GUI windows
    destroyAllWindows();
    return EXIT_SUCCESS;
}

/**
 * @function processVideo
 */
void processVideo(char* videoFilename) {
    //create the capture object
    //VideoCapture capture(videoFilename);
    VideoCapture capture(0);
    if(!capture.isOpened()){
        //error in opening the video input
        cerr << "Unable to open video file: " << videoFilename << endl;
        exit(EXIT_FAILURE);
    }
#if 1
    {
      stringstream dFrameWidth;
      stringstream dFrameHeight;
      stringstream dFrameRate;
        dFrameWidth << capture.get(CAP_PROP_FRAME_WIDTH);
      dFrameHeight << capture.get(CAP_PROP_FRAME_HEIGHT);
      dFrameRate << capture.get(CAP_PROP_FPS);

      string stFrameWidth = dFrameWidth.str();
      string stFrameHeight = dFrameHeight.str();
      string stFrameRate = dFrameRate.str();
    }
#endif


    //read input data. ESC or 'q' for quitting
    while( (char)keyboard != 'q' && (char)keyboard != 27 ){
        //read the current frame
        if(!capture.read(frame)) {
            cerr << "Unable to read next frame." << endl;
            cerr << "Exiting..." << endl;
            exit(EXIT_FAILURE);
        }
        //update the background model
        pMOG2->apply(frame, fgMaskMOG2);

        //get the frame number and write it on the current frame
        stringstream ss;
        rectangle(frame, cv::Point(10, 2), cv::Point(100,20),
                  cv::Scalar(255,255,255), -1);
        ss << capture.get(CAP_PROP_POS_FRAMES);
        string frameNumberString = ss.str();
        putText(frame, frameNumberString.c_str(), cv::Point(15, 15),
                FONT_HERSHEY_SIMPLEX, 0.5 , cv::Scalar(0,0,0));

        //show the current frame
        imshow("Frame", frame);

        //show the fg masks
        imshow("FG Mask MOG 2", fgMaskMOG2);

        //get the input from the keyboard
        keyboard = waitKey( 20 );
    }
    //delete capture object
    capture.release();
}
#endif