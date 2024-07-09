#include <opencv2/core/utility.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

using namespace std;
using namespace cv;

int main(){
	VideoCapture cap("../Dataset/game1_clip1/game1_clip1.mp4");
	Rect roi;
	Mat frame;
	Ptr<Tracker> tracker = TrackerCSRT::create();   // change tracker type here
	cap >> frame;
	roi=selectROI("tracker",frame); // select roi by hand
	if(roi.width==0 || roi.height==0)
		return 0;

	tracker->init(frame,roi);

	while (true){
		cap >> frame;
		if(frame.rows==0 || frame.cols==0)
			break;
		tracker->update(frame,roi);
		rectangle( frame, roi, Scalar( 255, 0, 0 ), 2, 1 );
		imshow("Tracking",frame);
	}
	waitKey();
	return 0;
}
