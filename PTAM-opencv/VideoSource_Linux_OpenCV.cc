/*
* Autor : Arnaud GROSJEAN (VIDE SARL)
* This implementation of VideoSource allows to use OpenCV as a source for the video input
* I did so because libCVD failed getting my V4L2 device
*
* INSTALLATION :
* - Copy the VideoSource_Linux_OpenCV.cc file in your PTAM directory
* - In the Makefile:
*	- set the linkflags to
	LINKFLAGS = -L MY_CUSTOM_LINK_PATH -lblas -llapack -lGVars3 -lcvd -lcv -lcxcore -lhighgui
*	- set the videosource to 
	VIDEOSOURCE = VideoSource_Linux_OpenCV.o
* - Compile the project
* - Enjoy !
* 
* Notice this code define two constants for the image width and height (OPENCV_VIDEO_W and OPENCV_VIDEO_H)
*/

#include "VideoSource.h"

#include <cvd/colourspace_convert.h>
#include <cvd/colourspaces.h>
#include <gvars3/instances.h>


using namespace CVD;
using namespace std;
using namespace GVars3;
using namespace cv;



VideoSource::VideoSource(int index_1, int index_2)
{
#ifndef UNITY_PLUGIN
	cout << "  VideoSource_Linux: Opening video source..." << endl;
#ifdef LOCAL_VIDEO
	mptr = new VideoCapture("rec.avi");
	mptr_right = new VideoCapture("rec_right.avi");
		
	VideoCapture* cap = (VideoCapture*)mptr;
	VideoCapture* cap_right = (VideoCapture*)mptr_right;
#else
	if(index_1 <= 0 && index_2 <=0)
	{
		//a defalut way
		mptr = new VideoCapture(0);
		mptr_right = new VideoCapture(1);
	}
	else
	{
		mptr = new VideoCapture(index_1);
		mptr_right = new VideoCapture(index_2);
	}

	VideoCapture* cap = (VideoCapture*)mptr;
	cap->set(CV_CAP_PROP_FOURCC ,CV_FOURCC('M', 'J', 'P', 'G') );
#ifdef CAMERA_FLIP_90
	cap->set(CV_CAP_PROP_FRAME_WIDTH,OPENCV_VIDEO_H);
	cap->set(CV_CAP_PROP_FRAME_HEIGHT,OPENCV_VIDEO_W);
#else
	cap->set(CV_CAP_PROP_FRAME_WIDTH,OPENCV_VIDEO_W);
	cap->set(CV_CAP_PROP_FRAME_HEIGHT,OPENCV_VIDEO_H);
#endif

	VideoCapture* cap_right = (VideoCapture*)mptr_right;
	cap_right->set(CV_CAP_PROP_FOURCC ,CV_FOURCC('M', 'J', 'P', 'G') );
#ifdef CAMERA_FLIP_90
	cap_right->set(CV_CAP_PROP_FRAME_WIDTH,OPENCV_VIDEO_H);
	cap_right->set(CV_CAP_PROP_FRAME_HEIGHT,OPENCV_VIDEO_W);
#else
	cap_right->set(CV_CAP_PROP_FRAME_WIDTH,OPENCV_VIDEO_W);
	cap_right->set(CV_CAP_PROP_FRAME_HEIGHT,OPENCV_VIDEO_H);
#endif
#endif

	if(!cap->isOpened() || !cap_right->isOpened())
	{
		cerr << "Unable to get the camera" << endl;
		cerr << "set up dual cameras" << endl;
		exit(-1);
	}
	cout << "  ... got video source." << endl;

#endif


	//reading calibration parameters
	//read calibration parameters
	
	mx1.create( OPENCV_VIDEO_H, OPENCV_VIDEO_W, CV_16S);
	my1.create( OPENCV_VIDEO_H, OPENCV_VIDEO_W, CV_16S);
	mx2.create( OPENCV_VIDEO_H, OPENCV_VIDEO_W, CV_16S);
	my2.create( OPENCV_VIDEO_H, OPENCV_VIDEO_W, CV_16S);
	Rect validRoi[2];

	FileStorage fs("./save_param/calib_para.yml", CV_STORAGE_READ);
	fs["MX1"] >> mx1;
	fs["MX2"] >> mx2;
	fs["MY1"] >> my1;
	fs["MY2"] >> my2;
	fs.release();


	cv::FileStorage fs_roi("./save_param/validRoi.yml" ,CV_STORAGE_READ);
	if (fs_roi.isOpened())
	{
		fs_roi["validRoi1"] >> validroi1;
		fs_roi["validRoi2"] >> validroi2;
	}
	fs_roi.release();

	image_chop_up = MAX(validroi1.at<int>(0,1),validroi2.at<int>(0,1));
	image_chop_down = MAX(validroi1.at<int>(0,3),validroi2.at<int>(0,3));
	image_chop_left = MAX(validroi1.at<int>(0,0),validroi2.at<int>(0,0));
	image_chop_right = MAX(validroi1.at<int>(0,2),validroi2.at<int>(0,2));

	mirSize = ImageRef(OPENCV_VIDEO_W, OPENCV_VIDEO_H);

};

ImageRef VideoSource::Size()
{ 
  return mirSize;
};

void conversionNB(Mat& frame, Image<byte> &imBW){
	//Mat clone = frame.clone();
	//Mat_<Vec3b>& frame_p = (Mat_<Vec3b>&)clone;
	//for (int i = 0; i < frame.rows; i++){
	//	for (int j = 0; j < frame.cols; j++){	
	//	imBW[i][j] = (frame_p(i,j)[0] + frame_p(i,j)[1] + frame_p(i,j)[2]) / 3;
	//	}
	//}

//#ifdef DEBUG_PRINT
//	double tic=(double)cvGetTickCount();
//#endif

	Mat cvd_image(frame.rows,frame.cols,CV_8UC1,(unsigned char*)imBW.data());
	Mat frame_dst;
	cvtColor(frame, frame_dst, CV_BGR2GRAY);
	//imshow("toCVDImage", frame_dst);
	//waitKey();
	frame_dst.copyTo(cvd_image);

//#ifdef DEBUG_PRINT
//	double toc=(double)cvGetTickCount();
//	double detectionTime = (toc-tic)/((double) cvGetTickFrequency()*1000);
//	cout << "mat to cvd frame in:  " << detectionTime << endl;
//#endif
}

void conversionRGB(Mat& frame, Image<Rgb<byte> > &imRGB){
	//Mat clone = frame.clone();
	//Mat_<Vec3b>& frame_p = (Mat_<Vec3b>&)frame;
	//for (int i = 0; i < frame.rows; i++){
	//	for (int j = 0; j < frame.cols; j++){	
	//	imRGB[i][j].red = frame_p(i,j)[2];
	//	imRGB[i][j].green = frame_p(i,j)[1];
	//	imRGB[i][j].blue = frame_p(i,j)[0];
	//	}
	//}
	
	Mat cvd_image(frame.rows,frame.cols,CV_8UC3,(unsigned char*)imRGB.data());
	Mat frame_dst;
	cvtColor(frame, frame_dst, CV_BGR2RGB);
	frame_dst.copyTo(cvd_image);
}

void VideoSource::GetAndFillFrameBWandRGB(Image<byte> &imBW, Image<Rgb<byte> > &imRGB)
{
	Mat frame,src_flip;
	VideoCapture* cap = (VideoCapture*)mptr;
	*cap >> frame;
	flip(frame.t(), src_flip, 1);
	conversionNB(src_flip, imBW);
	conversionRGB(src_flip, imRGB);
}


 void VideoSource::GetAndFillFrameBWandRGB(CVD::Image<CVD::byte> &imL_BW,CVD::Image<CVD::byte> &imR_BW,CVD::Image<CVD::Rgb<CVD::byte> > &imL_RGB,CVD::Image<CVD::Rgb<CVD::byte> > &imR_RGB)
 {
#ifdef DEBUG_PRINT
	double tic=(double)cvGetTickCount();
#endif

	VideoCapture* cap = (VideoCapture*)mptr;
	VideoCapture* cap_right = (VideoCapture*)mptr_right;

	*cap >> frame;
	*cap_right >> frame_right;

#ifdef DEBUG_PRINT
	double toc=(double)cvGetTickCount();
	double detectionTime = (toc-tic)/((double) cvGetTickFrequency()*1000);
	cout << "get frame in  " << detectionTime << endl;
#endif

#ifdef DEBUG_PRINT
	tic=(double)cvGetTickCount();
#endif

#ifdef SHOW_CVIMAGE
	imshow("frame", frame);
	imshow("frame_right", frame_right);
	waitKey();
#endif

#ifdef CAMERA_FLIP_90
	flip(frame.t(), src_flip, 0);
	flip(frame_right.t(), src_flip_right, 0);

	cv::remap(src_flip, frame_rectify, mx1, my1, CV_INTER_LINEAR);
	cv::remap(src_flip_right, frame_rectify_right, mx2, my2, CV_INTER_LINEAR);
#else
	cv::remap(frame, frame_rectify, mx1, my1, CV_INTER_LINEAR);
	cv::remap(frame_right, frame_rectify_right, mx2, my2, CV_INTER_LINEAR);
#endif

#ifdef SHOW_CVIMAGE
	imshow("frame", frame_rectify);
	imshow("frame_right", frame_rectify_right);
#ifdef FRAME_BY_FRAME
	waitKey();
#else
	waitKey(30);
#endif
#endif

	//frame_rectify = frame_rectify.adjustROI(-image_chop_up, -image_chop_down, -image_chop_left, -image_chop_right).clone();
	//resize(frame_rectify,frame_rectify,frame.size(),INTER_LINEAR );
	//frame_rectify_right = frame_rectify_right.adjustROI(-image_chop_up, -image_chop_down, -image_chop_left, -image_chop_right).clone();
	//resize(frame_rectify_right,frame_rectify_right,frame_right.size(),INTER_LINEAR );



#if TRACK_IMAGE_W != OPENCV_VIDEO_W

//#ifdef DEBUG_PRINT
//	tic=(double)cvGetTickCount();
//#endif
	cv::resize(frame_rectify, frame_rectify_down, cv::Size(TRACK_IMAGE_W, TRACK_IMAGE_H));
	cv::resize(frame_rectify_right, frame_rectify_right_down, cv::Size(TRACK_IMAGE_W, TRACK_IMAGE_H));

//#ifdef DEBUG_PRINT
//	toc=(double)cvGetTickCount();
//	detectionTime = (toc-tic)/((double) cvGetTickFrequency()*1000);
//	cout << " resize frame in:  " << detectionTime << endl;
//#endif

#ifdef FRAME_BY_FRAME
	imshow("frame_down", frame_rectify_down);
	imshow("frame_right_down", frame_rectify_right_down);
	waitKey();
#endif



	conversionNB(frame_rectify_down, imL_BW);
	conversionNB(frame_rectify_right_down, imR_BW);
#else
	conversionNB(frame_rectify, imL_BW);
	conversionNB(frame_rectify_right, imR_BW);
#endif


	conversionRGB(frame_rectify, imL_RGB);
	conversionRGB(frame_rectify_right, imR_RGB);

#ifdef DEBUG_PRINT
	toc=(double)cvGetTickCount();
	detectionTime = (toc-tic)/((double) cvGetTickFrequency()*1000);
	cout << "remap/resize/2cvd  in:  " << detectionTime << endl;
#endif

 }

#ifdef UNITY_PLUGIN
 void VideoSource::GetAndFillFrameBWandRGB(CVD::Image<CVD::byte> &imL_BW,CVD::Image<CVD::byte> &imR_BW,CVD::Image<CVD::Rgb<CVD::byte> > &imL_RGB,CVD::Image<CVD::Rgb<CVD::byte> > &imR_RGB, unsigned char* image_data_RGBA_left, unsigned char* image_data_RGBA_right)
 {
		cv::Mat src(mirSize.x, mirSize.y, CV_8UC4, (unsigned char*)image_data_RGBA_left);
#ifdef SHOW_CVIMAGE
		cv::imshow("src",src);
		cv::waitKey(30);
#endif
		cv::cvtColor(src,src_BGR, CV_RGBA2BGR);

		//CAREFULL!
		//webcam texture is fliped!!! 
		cv::flip(src_BGR.t(),src_flip,0);
#ifdef SHOW_CVIMAGE
		cv::imshow("src_flip",src_flip);
		cv::waitKey();
#endif
		cv::remap( src_flip, frame_rectify, mx1, my1, CV_INTER_LINEAR);
#ifdef SHOW_CVIMAGE
		cv::imshow("frame_rectify",frame_rectify);
		cv::waitKey(30);
#endif

#if TRACK_IMAGE_W != OPENCV_VIDEO_W
		cv::resize(frame_rectify, frame_rectify_down, cv::Size(TRACK_IMAGE_W, TRACK_IMAGE_H));

		conversionNB(frame_rectify_down, imL_BW);
#else
		conversionNB(frame_rectify, imL_BW);
#endif
		conversionRGB(frame_rectify, imL_RGB);
		

		//flip them back and clone back to RGBA texture data
		cv::flip(frame_rectify.t(),frame_rectify,-1);
		cv::cvtColor(frame_rectify,src,CV_BGR2RGBA);
#ifdef SHOW_CVIMAGE
		//cv::imshow("src_back",src);
		//cv::waitKey();
#endif

#ifdef DEBUG_PRINT
	double toc=(double)cvGetTickCount();
	double detectionTime = (toc-tic)/((double) cvGetTickFrequency()*1000);
	cout << "left frame in:  " << detectionTime << endl;
#endif

		//cv::Mat src_right_back(mirSize.x, mirSize.y, CV_8UC4, (unsigned char*)image_data_RGBA_right);
		cv::Mat src_right(mirSize.x, mirSize.y, CV_8UC4, (unsigned char*)image_data_RGBA_right);

#ifdef SHOW_CVIMAGE
		cv::imshow("src_right",src_right);
		cv::waitKey(30);
#endif
		cv::cvtColor(src_right,src_BGR_right, CV_RGBA2BGR);
		cv::flip(src_BGR_right.t(),src_flip_right,0);
		cv::remap( src_flip_right, frame_rectify_right, mx2, my2, CV_INTER_LINEAR);
#ifdef SHOW_CVIMAGE
		cv::imshow("frame_rectify_right",frame_rectify_right);
		cv::waitKey(30);
#endif

#if TRACK_IMAGE_W != OPENCV_VIDEO_W
		resize(frame_rectify_right, frame_rectify_right_down, cv::Size(TRACK_IMAGE_W, TRACK_IMAGE_H));
		conversionNB(frame_rectify_right_down, imR_BW);
#else
		conversionNB(frame_rectify_right, imR_BW);
#endif

		
		conversionRGB(frame_rectify_right, imR_RGB);

		cv::flip(frame_rectify_right.t(),frame_rectify_right,-1 );
		cv::cvtColor(frame_rectify_right,src_right,CV_BGR2RGBA);
		//cv::imshow("src_right",src_right);
		//cv::waitKey();


#ifdef DEBUG_PRINT
	toc=(double)cvGetTickCount();
	detectionTime = (toc-tic)/((double) cvGetTickFrequency()*1000);
	cout << "minus getframe in:  " << detectionTime << endl;
#endif


 }
#endif

 void VideoSource::firstRun()
 {
	VideoCapture* cap = (VideoCapture*)mptr;
	VideoCapture* cap_right = (VideoCapture*)mptr_right;

	*cap >> frame;
	*cap_right >> frame_right;

	
	flip(frame.t(), src_flip, 0);
	flip(frame_right.t(), src_flip_right, 0);

	cv::remap(src_flip, frame_rectify, mx1, my1, CV_INTER_LINEAR);
	cv::remap(src_flip_right, frame_rectify_right, mx2, my2, CV_INTER_LINEAR);
 }

cv::Mat VideoSource::getRectifyLeft()
{
	return frame_rectify;
}

cv::Mat VideoSource::getRectifyRight()
{
	return frame_rectify_right;
}

VideoSource::~VideoSource()
{
#ifndef UNITY_PLUGIN
	VideoCapture* cap = (VideoCapture*)mptr;
	VideoCapture* cap_right = (VideoCapture*)mptr_right;

	cap->release();
	cap_right->release();
#endif
}


cv::Mat VideoSource::get_left_RGBA()
{
	VideoCapture* cap = (VideoCapture*)mptr;
	*cap >> frame;
	Mat frame_dst;
	cvtColor(frame, frame_dst, CV_BGR2RGBA);
//#ifdef SHOW_CVIMAGE
//		cv::imshow("RGBA_left",frame_dst);
//		cv::waitKey(30);
//#endif
	return frame_dst;
}

cv::Mat VideoSource::get_right_RGBA()
{
	VideoCapture* cap_right = (VideoCapture*)mptr_right;
	*cap_right >> frame_right;
	Mat frame_dst;
	cvtColor(frame_right, frame_dst, CV_BGR2RGBA);
//#ifdef SHOW_CVIMAGE
//		cv::imshow("RGBA_right",frame_dst);
//		cv::waitKey(30);
//#endif
	return frame_dst;
}