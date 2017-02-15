// Copyright 2008 Isis Innovation Limited
#include "System.h"
#include "OpenGL.h"
#include <gvars3/instances.h>
#include <stdlib.h>
#include "ATANCamera.h"
#include "MapMaker.h"
#include "Tracker.h"
#include "ARDriver.h"
#include "MapViewer.h"
#include "cameraparameters.h"
#include "definition.h"
#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>

using namespace CVD;
using namespace std;
using namespace GVars3;


System::System(int index_1, int index_2)
  : mGLWindow(ImageRef(TRACK_IMAGE_W, TRACK_IMAGE_H), "PTAM")
{
  GUI.RegisterCommand("exit", GUICommandCallBack, this);
  GUI.RegisterCommand("quit", GUICommandCallBack, this);
  
	//mimFrameBW.resize(mVideoSource.Size());
	//mimFrameRGB.resize(mVideoSource.Size());
	//mimFrameBW_right.resize(mVideoSource.Size());
	//mimFrameRGB_right.resize(mVideoSource.Size());
 // // First, check if the camera is calibrated.
 // // If not, we need to run the calibration widget.
 // Vector<NUMTRACKERCAMPARAMETERS> vTest;
 // 
 // vTest = GV3::get<Vector<NUMTRACKERCAMPARAMETERS> >("Camera.Parameters", ATANCamera::mvDefaultParams, HIDDEN);
 // mpCamera = new ATANCamera("Camera");
 // Vector<2> v2;
 // if(v2==v2) ;
 // if(vTest == ATANCamera::mvDefaultParams)
 //   {
 //     cout << endl;
 //     cout << "! Camera.Parameters is not set, need to run the CameraCalibrator tool" << endl;
 //     cout << "  and/or put the Camera.Parameters= line into the appropriate .cfg file." << endl;
 //     exit(1);
 //   }
  

  mPTAM_AR.init(index_1, index_2);


  //mpMap = new Map;
  //mpMapMaker = new MapMaker(*mpMap, *mpCamera);
  //mpTracker = new Tracker(mVideoSource.Size(), *mpCamera, *mpMap, *mpMapMaker);
  mpARDriver = new ARDriver(*mPTAM_AR.mpCamera, mPTAM_AR.mVideoSize, mGLWindow);
  mpMapViewer = new MapViewer(*mPTAM_AR.mpMap, mGLWindow);
  
  GUI.ParseLine("GLWindow.AddMenu Menu Menu");
  GUI.ParseLine("Menu.ShowMenu Root");
  GUI.ParseLine("Menu.AddMenuButton Root Reset Reset Root");
  GUI.ParseLine("Menu.AddMenuButton Root Spacebar PokeTracker Root");
  GUI.ParseLine("DrawAR=0");
  GUI.ParseLine("DrawMap=0");
  GUI.ParseLine("Menu.AddMenuToggle Root \"View Map\" DrawMap Root");
  GUI.ParseLine("Menu.AddMenuToggle Root \"Draw AR\" DrawAR Root");

  
  mbDone = false;

  mDraw_ar_start =false;


};

//int g_count = 0;

void System::Run()
{
  while(!mbDone)
    {
   //   g_count++;
	  //if(g_count == 15)
	  //{
		 // mPTAM_AR.mpTracker->mbUserPressedSpacebar = true;
	  //}
      // We use two versions of each video frame:
      // One black and white (for processing by the tracker etc)
      // and one RGB, for drawing.

      // Grab new video frame...
      //mVideoSource.GetAndFillFrameBWandRGB(mimFrameBW, mimFrameRGB); 
	  //mVideoSource.GetAndFillFrameBWandRGB(mimFrameBW, mimFrameBW_right, mimFrameRGB, mimFrameRGB_right);

#ifdef DEBUG_PRINT
	double tic=(double)cvGetTickCount();
#endif

      static bool bFirstFrame = true;
      if(bFirstFrame)
	{
	  mpARDriver->Init();
	  bFirstFrame = false;
	}
      
      mGLWindow.SetupViewport();
      mGLWindow.SetupVideoOrtho();
      mGLWindow.SetupVideoRasterPosAndZoom();
      
      if(!mPTAM_AR.mpMap->IsGood())
		mpARDriver->Reset();
      
      static gvar3<int> gvnDrawMap("DrawMap", 0, HIDDEN|SILENT);
      static gvar3<int> gvnDrawAR("DrawAR", 0, HIDDEN|SILENT);
      
      bool bDrawMap = mPTAM_AR.mpMap->IsGood() && *gvnDrawMap;
      bool bDrawAR = mPTAM_AR.mpMap->IsGood() && *gvnDrawAR;
      
      //mpTracker->TrackFrame(mimFrameBW, !bDrawAR && !bDrawMap);

	  //mpTracker->TrackFrame(mimFrameBW, mimFrameBW_right, !bDrawAR && !bDrawMap);


	  mPTAM_AR.processs_frame(!bDrawAR && !bDrawMap);
      
	  //cv::Mat left_rgba = mPTAM_AR.mVideoSource_ptr->get_left_RGBA();
	  //cv::Mat right_rgba = mPTAM_AR.mVideoSource_ptr->get_right_RGBA();
	  //mPTAM_AR.update_frame(left_rgba.data, right_rgba.data, 1024, 1280, true);

	  
#ifdef DEBUG_PRINT
	double toc=(double)cvGetTickCount();
	double detectionTime = (toc-tic)/((double) cvGetTickFrequency()*1000);
	cout << " one frame in:  " << detectionTime << endl;
#endif
      if(bDrawMap)
	mpMapViewer->DrawMap(mPTAM_AR.mpTracker->GetCurrentPose());
      else if(bDrawAR)
	//mpARDriver->Render(mimFrameRGB, mpTracker->GetCurrentPose());
	  {
		  if(!mDraw_ar_start)
		  //AR left and right drawing, window size width*2
		  {
			  mGLWindow.set_size(ImageRef(OPENCV_VIDEO_W*2, OPENCV_VIDEO_H));
			  mDraw_ar_start=true;
		  }
		bool drawAR = mPTAM_AR.mpTracker->mTrackingQuality != Tracker::BAD;
		mpARDriver->Render(mPTAM_AR.mimFrameRGB, mPTAM_AR.mimFrameRGB_right, mPTAM_AR.mpTracker->GetCurrentPose(),drawAR);
	  }
	  else
	  {
		  if(mDraw_ar_start)
		  {
			mDraw_ar_start = false;
			mGLWindow.set_size(ImageRef(TRACK_IMAGE_W, TRACK_IMAGE_H));
		  }
	  }


#ifdef DEBUG_PRINT
#ifdef UNITY_PLUGIN
	double *translation = get_translation(&mPTAM_AR);
	cout<<"translate x "<<translation[0]<<endl;
	cout<<"translate y "<<translation[1]<<endl;
	cout<<"translate z "<<translation[2]<<endl;
	double *rotation = get_rotation(&mPTAM_AR);
	//cout<<"rotation 0 "<<rotation[0]<<endl;
	//cout<<"rotation 1 "<<rotation[1]<<endl;
	//cout<<"rotation 2 "<<rotation[2]<<endl;
	//cout<<"rotation 3 "<<rotation[3]<<endl;
	//cout<<"rotation 4 "<<rotation[4]<<endl;
	//cout<<"rotation 5 "<<rotation[5]<<endl;
	//cout<<"rotation 6 "<<rotation[6]<<endl;
	//cout<<"rotation 7 "<<rotation[7]<<endl;
	//cout<<"rotation 8 "<<rotation[8]<<endl;
#endif
#endif


      //      mGLWindow.GetMousePoseUpdate();
      string sCaption;
      if(bDrawMap)
	sCaption = mpMapViewer->GetMessageForUser();
      else
	sCaption = mPTAM_AR.mpTracker->GetMessageForUser();
      mGLWindow.DrawCaption(sCaption);
      mGLWindow.DrawMenus();
      mGLWindow.swap_buffers();
      mGLWindow.HandlePendingEvents();

	  //OgreAR
	 // mOgreAR->mTexture->getBuffer()->blitFromMemory(mOgreAR->mPixelBox);
	 // mOgreAR->mTexture_right->getBuffer()->blitFromMemory(mOgreAR->mPixelBox_right);

	 // //			mOgreAR->ogreNode->setVisible(true);
		//	//mOgreAR->ogreNode_right->setVisible(true);
		//	//mOgreAR->draw_object(mpTracker->GetCurrentPose(), mpTracker->mGridRef, mpTracker->mGridVertices, *CameraParamsUnd);
		//	////mOgreAR->ogreNode->setPosition(0,0,1);

	 // if(mpTracker->mTrackingQuality != Tracker::BAD)
		//{

		//	mOgreAR->ogreNode->setVisible(true);
		//	mOgreAR->ogreNode_right->setVisible(true);
		//	mOgreAR->draw_object(mpTracker->GetCurrentPose(), mpTracker->mGridRef, mpTracker->mGridVertices, *CameraParamsUnd);
		//}

		//


	 // if(mOgreAR->root->renderOneFrame() == false) break;
	 //   Ogre::WindowEventUtilities::messagePump();

		/// KEYBOARD INPUT
/*	    mOgreAR->keyboard->capture();
	    if (mOgreAR->keyboard->isKeyDown(OIS::KC_ESCAPE)) break;*/	

    }
}

void System::GUICommandCallBack(void *ptr, string sCommand, string sParams)
{
  if(sCommand=="quit" || sCommand == "exit")
    static_cast<System*>(ptr)->mbDone = true;
}








