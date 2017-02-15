// -*- c++ -*-
// Copyright 2008 Isis Innovation Limited
//
// miku.h
// Declares the miku class
// miku is a trivial AR app which draws some 3D graphics
// Draws a bunch of 3d eyeballs remniscient of the 
// AVL logo
//
#ifndef __MIKU_H
#define __MIKU_H
#include <TooN/TooN.h>
using namespace TooN;
#include "OpenGL.h"

class Miku
{
 public:
  Miku();
  //for ar stereo left and right drawing. MMD miku motion needs to be synced for left and right draw.
  //@param int left_right 0 for left, 1 for right
  void DrawStuff(Vector<3> v3CameraPos, int left_right);
  void Reset();
  void Init();

  
 protected:
  bool mbInitialised;
  void DrawEye();
  void LookAt(int nEye, Vector<3> v3, double dRotLimit);
  void MakeShadowTex();
 
  GLuint mnEyeDisplayList;
  GLuint mnShadowTex;
  double mdEyeRadius;
  double mdShadowHalfSize;
  SE3<> ase3WorldFromEye[4];
  int mnFrameCounter;

};


#endif
