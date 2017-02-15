// -*- c++ *--
// Copyright 2008 Isis Innovation Limited
//
// VideoSource.h
// Declares the VideoSource class
// 
// This is a very simple class to provide video input; this can be
// replaced with whatever form of video input that is needed.  It
// should open the video input on construction, and provide two
// function calls after construction: Size() must return the video
// format as an ImageRef, and GetAndFillFrameBWandRGB should wait for
// a new frame and then overwrite the passed-as-reference images with
// GreyScale and Colour versions of the new frame.
//
#ifndef __VIDEOSOURCE_H
#define __VIDEOSOURCE_H
#include <cvd/image.h>
#include <cvd/byte.h>
#include <cvd/rgb.h>
#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include "definition.h"

struct VideoSourceData;

class VideoSource
{
public:
	VideoSource(int index_1, int index_2);
	~VideoSource();
	void GetAndFillFrameBWandRGB(CVD::Image<CVD::byte> &imBW, CVD::Image<CVD::Rgb<CVD::byte> > &imRGB);
	void GetAndFillFrameBWandRGB(CVD::Image<CVD::byte> &imL_BW,CVD::Image<CVD::byte> &imR_BW,CVD::Image<CVD::Rgb<CVD::byte> > &imL_RGB,CVD::Image<CVD::Rgb<CVD::byte> > &imR_RGB);
#ifdef UNITY_PLUGIN
	//for unity plugin, unity open webcam and pass in RGBA data
	void GetAndFillFrameBWandRGB(CVD::Image<CVD::byte> &imL_BW,CVD::Image<CVD::byte> &imR_BW,CVD::Image<CVD::Rgb<CVD::byte> > &imL_RGB,CVD::Image<CVD::Rgb<CVD::byte> > &imR_RGB, unsigned char* image_data_RGBA_left, unsigned char* image_data_RGBA_right);
#endif

	CVD::ImageRef Size();

	void firstRun();
	cv::Mat getRectifyLeft();
	cv::Mat getRectifyRight();
	cv::Mat get_left_RGBA();
	cv::Mat get_right_RGBA();
  
private:
	void *mptr;
	void *mptr_right;
	CVD::ImageRef mirSize;
	 //stereo calibration remap matrix.
	cv::Mat mx1,my1,mx2,my2;
	//stereo calibration remap roi
  	cv::Mat validroi1,validroi2;
	int image_chop_left,image_chop_right,image_chop_up,image_chop_down; 
	cv::Mat frame,src_flip;
	cv::Mat frame_right, src_flip_right;
	cv::Mat frame_rectify;
	cv::Mat frame_rectify_right;
#if TRACK_IMAGE_W != OPENCV_VIDEO_W
	cv::Mat frame_rectify_down;
	cv::Mat frame_rectify_right_down;
#endif
#ifdef UNITY_PLUGIN
	cv::Mat src_BGR;
	cv::Mat src_BGR_right;
#endif

	
};
#endif