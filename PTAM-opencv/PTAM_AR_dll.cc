#include "PTAM_AR_dll.h"
#include "ATANCamera.h"
#include "MapMaker.h"
#include "Tracker.h"
#include <fstream>
#include <string>


using namespace CVD;
using namespace std;
using namespace GVars3;

PTAM_AR::PTAM_AR()
{


}

bool PTAM_AR::init(int index_1, int index_2)
{
	msg = -1;

	mVideoSource_ptr = new VideoSource(index_1,index_2);
	mVideoSize = mVideoSource_ptr->Size();
#if TRACK_IMAGE_W != OPENCV_VIDEO_W
	mimFrameBW.resize(ImageRef(TRACK_IMAGE_W,TRACK_IMAGE_H));
	mimFrameBW_right.resize(ImageRef(TRACK_IMAGE_W, TRACK_IMAGE_H));
#else
	mimFrameBW.resize(mVideoSize);
	mimFrameBW_right.resize(mVideoSize);
#endif
	
	mimFrameRGB.resize(mVideoSize);
	mimFrameRGB_right.resize(mVideoSize);


	//for dll use only, dll part takes no "settings.cfg" should read camera.cfg alone;
	std::ifstream File_in("./camera.cfg");
	if(!File_in.is_open())
	{
		cout<<"can't read camera.cfg"<<endl;
		return false;
	}
	std::string key,value;
	std::string line;
	while(std::getline(File_in,line))
	{
		std::stringstream line_to_read(line);
		std::getline(line_to_read, key, '[');
		std::getline(line_to_read, value, ']');
	}

	if(key != "Camera.Parameters=")
	{
		cout<<"Camera.parameters not match"<<endl;
		return false;
	}

	std::stringstream value_to_read(value);
	float p1,p2,p3,p4,p5;

	value_to_read>>p1>>p2>>p3>>p4>>p5;

	File_in.close();


	Vector<NUMTRACKERCAMPARAMETERS> vTest;

	vTest = makeVector(p1,p2,p3,p4,p5);

	//vTest = GV3::get<Vector<NUMTRACKERCAMPARAMETERS> >("Camera.Parameters", ATANCamera::mvDefaultParams, HIDDEN);
	//mpCamera = new ATANCamera("Camera");
	mpCamera = new ATANCamera("Camera",p1,p2,p3,p4,p5);
	//Vector<2> v2;
	//if(v2==v2) ;
	if(vTest == ATANCamera::mvDefaultParams)
    {
		cout << endl;
		cout << "! Camera.Parameters is not set, need to run the CameraCalibrator tool" << endl;
		cout << "  and/or put the Camera.Parameters= line into the appropriate .cfg file." << endl;
		return false;
    }
 //// 
	mpMap = new Map;
	mpMapMaker = new MapMaker(*mpMap, *mpCamera);
	mpTracker = new Tracker(ImageRef(TRACK_IMAGE_W, TRACK_IMAGE_H), *mpCamera, *mpMap, *mpMapMaker);
	return true;
}

void PTAM_AR::processs_frame(bool bDraw)
{
	mVideoSource_ptr->GetAndFillFrameBWandRGB(mimFrameBW, mimFrameBW_right, mimFrameRGB, mimFrameRGB_right);

#ifdef DEBUG_PRINT
	double tic=(double)cvGetTickCount();
#endif

	mpTracker->TrackFrame(mimFrameBW, mimFrameBW_right, bDraw);

#ifdef DEBUG_PRINT
	double toc=(double)cvGetTickCount();
	double detectionTime = (toc-tic)/((double) cvGetTickFrequency()*1000);
	cout << "track frame in  " << detectionTime << endl;
#endif

	mTranslation[0] = mpTracker->GetCurrentPose().get_translation()[0];
	mTranslation[1] = mpTracker->GetCurrentPose().get_translation()[1];
	mTranslation[2] = mpTracker->GetCurrentPose().get_translation()[2];

	mRotation[0] = mpTracker->GetCurrentPose().get_rotation().get_matrix().my_data[0];
	mRotation[1] = mpTracker->GetCurrentPose().get_rotation().get_matrix().my_data[1];
	mRotation[2] = mpTracker->GetCurrentPose().get_rotation().get_matrix().my_data[2];
	mRotation[3] = mpTracker->GetCurrentPose().get_rotation().get_matrix().my_data[3];
	mRotation[4] = mpTracker->GetCurrentPose().get_rotation().get_matrix().my_data[4];
	mRotation[5] = mpTracker->GetCurrentPose().get_rotation().get_matrix().my_data[5];
	mRotation[6] = mpTracker->GetCurrentPose().get_rotation().get_matrix().my_data[6];
	mRotation[7] = mpTracker->GetCurrentPose().get_rotation().get_matrix().my_data[7];
	mRotation[8] = mpTracker->GetCurrentPose().get_rotation().get_matrix().my_data[8];

}

#ifdef UNITY_PLUGIN
void PTAM_AR::update_frame(unsigned char* image_data_RGBA_left, unsigned char* image_data_RGBA_right, int frame_width, int frame_height, bool bDraw)
{
	double tic=(double)cvGetTickCount();


	mVideoSource_ptr->GetAndFillFrameBWandRGB(mimFrameBW, mimFrameBW_right, mimFrameRGB, mimFrameRGB_right, image_data_RGBA_left, image_data_RGBA_right);


	double toc=(double)cvGetTickCount();
	double detectionTime = (toc-tic)/((double) cvGetTickFrequency()*1000);

	cout << "dll function call time pass :  " << detectionTime << endl;

	msg = mpTracker->m_msg;
	//msg = detectionTime;

	mpTracker->TrackFrame(mimFrameBW, mimFrameBW_right, bDraw);
	
	msg = mpTracker->m_msg;

	mTranslation[0] = mpTracker->GetCurrentPose().get_translation()[0];
	mTranslation[1] = mpTracker->GetCurrentPose().get_translation()[1];
	mTranslation[2] = mpTracker->GetCurrentPose().get_translation()[2];

	mRotation[0] = mpTracker->GetCurrentPose().get_rotation().get_matrix().my_data[0];
	mRotation[1] = mpTracker->GetCurrentPose().get_rotation().get_matrix().my_data[1];
	mRotation[2] = mpTracker->GetCurrentPose().get_rotation().get_matrix().my_data[2];
	mRotation[3] = mpTracker->GetCurrentPose().get_rotation().get_matrix().my_data[3];
	mRotation[4] = mpTracker->GetCurrentPose().get_rotation().get_matrix().my_data[4];
	mRotation[5] = mpTracker->GetCurrentPose().get_rotation().get_matrix().my_data[5];
	mRotation[6] = mpTracker->GetCurrentPose().get_rotation().get_matrix().my_data[6];
	mRotation[7] = mpTracker->GetCurrentPose().get_rotation().get_matrix().my_data[7];
	mRotation[8] = mpTracker->GetCurrentPose().get_rotation().get_matrix().my_data[8];

}
#endif

PTAM_AR::~PTAM_AR()
{
	delete mVideoSource_ptr;
}


#ifdef UNITY_PLUGIN
PTAM_AR* create_PTAM(int index_0, int index_1)
{
	PTAM_AR* PTAM_ptr = new PTAM_AR();
	if(PTAM_ptr->init(index_0, index_1))
		return PTAM_ptr;
	return NULL;
}

void process_frame(PTAM_AR* PTAM_ptr)
{
	PTAM_ptr->processs_frame(false);
}

void update_frame(PTAM_AR* PTAM_ptr, unsigned char* img_rgba_left, unsigned char* img_rgba_right, int frame_width, int frame_height)
{
	PTAM_ptr->update_frame(img_rgba_left, img_rgba_right, frame_width, frame_height, false);
}

double* get_translation(PTAM_AR* PTAM_ptr)
{

	return PTAM_ptr->mTranslation;
}

double get_message(PTAM_AR* PTAM_ptr)
{
	return PTAM_ptr->msg;
}

double* get_rotation(PTAM_AR* PTAM_ptr)
{

	return PTAM_ptr->mRotation;
}

void init_stereo(PTAM_AR* PTAM_ptr)
{
	
	PTAM_ptr->mpTracker->mbUserPressedSpacebar = true;

}

void delte_PTAM(PTAM_AR* PTAM_ptr)
{
	delete PTAM_ptr;
}
#endif