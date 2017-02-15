// -*- c++ -*-
// Copyright 2008 Isis Innovation Limited
//
// System.h
//
// Defines the System class
//
// This stores the main functional classes of the system, like the
// mapmaker, map, tracker etc, and spawns the working threads.
//
#ifndef __SYSTEM_H
#define __SYSTEM_H
#include "VideoSource.h"
#include "GLWindow2.h"
#include "PTAM_AR_dll.h"
#include <cvd/image.h>
#include <cvd/rgb.h>
#include <cvd/byte.h>

class ATANCamera;
class Map;
class MapMaker;
class Tracker;
class ARDriver;
class MapViewer;
//class OgreAR;
//class CameraParameters;

class System
{
public:
  System(int index_1, int index_2);
  void Run();
  
private:
  //VideoSource mVideoSource;
  GLWindow2 mGLWindow;
  //CVD::Image<CVD::Rgb<CVD::byte> > mimFrameRGB;
  //CVD::Image<CVD::byte> mimFrameBW;

  //CVD::Image<CVD::byte> mimFrameBW_right;
  //CVD::Image<CVD::Rgb<CVD::byte>> mimFrameRGB_right;
  
  //Map *mpMap; 
  //MapMaker *mpMapMaker; 
  //Tracker *mpTracker; 
  //ATANCamera *mpCamera;
  ARDriver *mpARDriver;
  MapViewer *mpMapViewer;

  PTAM_AR mPTAM_AR;

  //for OgreAR rendering
  //OgreAR *mOgreAR;
  //CameraParameters *CameraParams, *CameraParamsUnd;

  //for ar drawing, indicating whether ar drawing is on.
  //ar drawing requires left and right draw. and needed to be resize for exentand mon
  bool mDraw_ar_start;
  
  bool mbDone;

  static void GUICommandCallBack(void* ptr, std::string sCommand, std::string sParams);
};



#endif
