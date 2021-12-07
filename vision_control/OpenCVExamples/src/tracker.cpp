#if 0
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/core/ocl.hpp>

using namespace cv;
using namespace std;

// Convert to string
#define SSTR( x ) static_cast< std::ostringstream & >( \
( std::ostringstream() << std::dec << x ) ).str()

int main(int argc, char **argv)
{
    double dWidth,NewdWidth;
    double dHeight,NewdHeight;
    bool ok;

    // List of tracker types in OpenCV 3.2
    // NOTE : GOTURN implementation is buggy and does not work.
    string trackerTypes[6] = {"BOOSTING", "MIL", "KCF", "TLD","MEDIANFLOW", "GOTURN"};
    // vector <string> trackerTypes(types, std::end(types));

    // Create a tracker
    string trackerType = trackerTypes[2];

    // Read video
    VideoCapture video;

    Mat frame;

    Ptr<Tracker> tracker;

    if (trackerType == "BOOSTING")
        tracker = TrackerBoosting::create();
    if (trackerType == "MIL")
        tracker = TrackerMIL::create();
    if (trackerType == "KCF")
        tracker = TrackerKCF::create();
    if (trackerType == "TLD")
        tracker = TrackerTLD::create();
    if (trackerType == "MEDIANFLOW")
        tracker = TrackerMedianFlow::create();
    if (trackerType == "GOTURN")
        tracker = TrackerGOTURN::create();

    //open device
    video.open(-1);

    // Exit if video is not opened
    if(!video.isOpened())
    {
        cout << "Could open video device" << endl;
        return 1;

    }
    //configure capture to reduce CPU usage required to track video
    dWidth = video.get(CV_CAP_PROP_FRAME_WIDTH);
    dHeight = video.get(CV_CAP_PROP_FRAME_HEIGHT);
    video.set(CV_CAP_PROP_FRAME_WIDTH,dWidth/2);
    video.set(CV_CAP_PROP_FRAME_HEIGHT,dHeight/2);

    //check new configuration
    NewdWidth = video.get(CV_CAP_PROP_FRAME_WIDTH);
    NewdHeight = video.get(CV_CAP_PROP_FRAME_HEIGHT);
    if((NewdWidth !=dWidth/2) && (NewdHeight !=dHeight/2))
    {
      cout << "Could reduce capture size on the video device" << endl;
      return 1;
    }
    printf("dWidth:%f \n",NewdWidth);
    printf("dHeight:%f \n",NewdHeight);

    // Read first frame
    ok = video.read(frame);

    // Define initial boundibg box
    Rect2d bbox(287, 23, 86, 320);

    // Uncomment the line below to select a different bounding box
    bbox = selectROI(frame, false);

    // Display bounding box.
    rectangle(frame, bbox, Scalar( 255, 0, 0 ), 2, 1 );
    imshow("Tracking", frame);

    tracker->init(frame, bbox);

    while(1)
    {
        // Start timer
        double timer = (double)getTickCount();

        if(video.read(frame))
        {
            // Update the tracking result
            ok = tracker->update(frame, bbox);

            // Calculate Frames per second (FPS)
            float fps = getTickFrequency() / ((double)getTickCount() - timer);

            if (ok)
            {
                // Tracking success : Draw the tracked object
                rectangle(frame, bbox, Scalar( 255, 0, 0 ), 2, 1 );
            }
            else
            {
                // Tracking failure detected.
                putText(frame, "Tracking failure detected", Point(10,80), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0,0,255),2);
            }

            // Display tracker type on frame
            putText(frame, trackerType + " Tracker", Point(10,20), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50,170,50),2);

            // Display FPS on frame
            putText(frame, "FPS : " + SSTR(int(fps)), Point(10,50), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50,170,50), 2);

            // Display frame.
            imshow("Tracking", frame);

            // Exit if ESC pressed.
            int k = waitKey(1);
            if(k == 27)
            {
                break;
            }
        }
    }
}
#endif
