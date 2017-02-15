// Copyright 2008 Isis Innovation Limited
#include "miku.h"
#include "OpenGL.h"
#include <cvd/convolution.h>

#include  <mmsystem.h>

#include    "MMD/PMDModel.h"
#include    "MMD/VMDMotion.h"

using namespace CVD;

cPMDModel    g_clPMDModel;
cVMDMotion    g_clVMDMotion;

float        g_fPrevFrame  =  0.0f;
DWORD        g_dwStartTime  =  0;

float g_left_diff = 0.0f;

MMD_Matrix        g_matPlanarProjection;

Miku::Miku()
{
  //mdEyeRadius = 0.1;
  //mdShadowHalfSize = 2.5 * mdEyeRadius;
  //mbInitialised = false;


  mbInitialised  =  false;

    //SetCurrentDirectory(  "./models"  );
    g_clPMDModel.load("./models/Miku_Hatsune_Ver2.pmd");
	
	//SetCurrentDirectory(  "C:\\models"  );
    //g_clPMDModel.load("C:\\models\\mikuXS.pmd");

    g_clVMDMotion.load("./models/TrueMyHeart.vmd");

    g_clPMDModel.setMotion(  &g_clVMDMotion,  true  );

    g_dwStartTime  =  timeGetTime();
    g_fPrevFrame  =  0.0f;
}

void Miku::DrawStuff(Vector<3> v3CameraPos,int left_right)
{
	  if(!mbInitialised)
        Init();

    mnFrameCounter  ++;

    DWORD  time;
    float  fDiffFrame;
	//for the left draw, left image is used for tracking now, so as for motion timing
	if(left_right == 0)
	{
		time  =  timeGetTime()  -  g_dwStartTime;
		fDiffFrame=  (float)time  /  30.0f  -  g_fPrevFrame;
		g_clPMDModel.updateMotion(fDiffFrame);
		g_clPMDModel.updateSkinning();
		g_fPrevFrame  +=  fDiffFrame;
		g_left_diff = fDiffFrame;
	}
	else if(left_right == 1)
	{
		//fDiffFrame = g_left_diff;
		//g_clPMDModel.updateMotion(fDiffFrame);
		//g_clPMDModel.updateSkinning();
	}

	glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);

    glEnable(  GL_CULL_FACE  );
    glEnable(  GL_ALPHA_TEST  );
    glEnable(  GL_BLEND  );
    glFrontFace(GL_CW);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);

    GLfloat  af[4];  
    af[0]=0.7;  af[1]=0.7;  af[2]=0.7;  af[3]=1.0;
    glLightfv(GL_LIGHT0,  GL_AMBIENT,  af);
    glLightfv(GL_LIGHT0,  GL_DIFFUSE,  af);
	af[0]=1.0;  af[1]=-2.0;  af[2]=1.0;  af[3]=0.0;
    glLightfv(GL_LIGHT0,  GL_POSITION,  af);
    af[0]=1.0;  af[1]=-2.0;  af[2]=1.0;  af[3]=0.0;
    glLightfv(GL_LIGHT0,  GL_POSITION,  af);
    af[0]=1.0;  af[1]=1.0;  af[2]=1.0;  af[3]=1.0;
    glMaterialfv(GL_FRONT_AND_BACK,  GL_SPECULAR,  af);
    glMaterialf(GL_FRONT_AND_BACK,  GL_SHININESS,  50.0);

    glMatrixMode(GL_MODELVIEW);

	//light position in the world coordinate/right-handed
	af[0]=50.0; af[1]=-50.0; af[2]=200.0; af[3]=0.0;
    glLightfv(GL_LIGHT0, GL_POSITION, af);

    // 平面投影行列の作成
    Vector4        vec4Plane = { 0.0f, 1.0f, 0.0f, 0.0f };        
// { a, b, c, d } → ax + by + cz + d = 0 (投影したい平面の方程式)
    Vector3        vec4LightPos = { af[0], af[2], af[1] };    // ライトの位置
    MatrixPlanarProjection( g_matPlanarProjection, &vec4Plane, &vec4LightPos );

    glPushMatrix();
        glScalef(  0.03f,  0.03f,  -0.03f);
        glRotatef(-90.0,  1.0,  0.0,  0.0);
        g_clPMDModel.render();


	  // ステンシルバッファに影の形を描画
        glDisable( GL_CULL_FACE );
        glDisable( GL_TEXTURE_2D );
        glDisable( GL_LIGHTING );

        glEnable( GL_STENCIL_TEST );
        glStencilFunc( GL_ALWAYS, 1, ~0 );
        glStencilOp( GL_REPLACE, GL_KEEP, GL_REPLACE );

        glColorMask( 0, 0, 0, 0 );
        glDepthMask( 0 );

        glMultMatrixf( (const float *)g_matPlanarProjection );
        g_clPMDModel.renderForShadow();        // 影用の描画

        glColorMask( 1, 1, 1, 1 );

        // ステンシルバッファの影の形を塗りつぶす
        float    fWndW = 640.0f,
                fWndH = 480.0f;

        glStencilFunc( GL_EQUAL, 1, ~0);
        glStencilOp( GL_KEEP, GL_KEEP ,GL_KEEP );

        glDisable( GL_DEPTH_TEST );

        glMatrixMode( GL_PROJECTION );
        glPushMatrix();
            glLoadIdentity();
            gluOrtho2D( 0.0f, fWndW, 0.0f, fWndH );

            glMatrixMode( GL_MODELVIEW );
            glPushMatrix();
                glLoadIdentity();

                glColor4f( 0.2f, 0.2f, 0.2f, 0.6f );

                glBegin( GL_TRIANGLE_FAN );
                    glVertex2f( 0.0f, fWndH );
                    glVertex2f( fWndW, fWndH );
                    glVertex2f( fWndW, 0.0f );
                    glVertex2f( 0.0f, 0.0f );
                glEnd();

                glMatrixMode( GL_PROJECTION );
            glPopMatrix();
            glMatrixMode( GL_MODELVIEW );
        glPopMatrix();

        glDepthMask( 1 );
        glEnable( GL_LIGHTING );
        glEnable( GL_DEPTH_TEST );
        glDisable( GL_STENCIL_TEST );


    glPopMatrix();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);  
    glDisable(GL_CULL_FACE);    

};


void Miku::Reset()
{
	mnFrameCounter = 0;
    g_dwStartTime = timeGetTime();
    g_fPrevFrame = 0.0f;
    g_clPMDModel.setMotion( &g_clVMDMotion, true );
};


void Miku::DrawEye()
{
  int nSegments = 45;
  int nSlices = 45;
  
  double dSliceAngle = M_PI / (double)(nSlices);
  double dSegAngle = 2.0 * M_PI / (double)(nSegments);
  
  glColor3f(0.0,0.0,0.0);
  {  // North pole:
    double Z = sin(M_PI/2.0 - dSliceAngle);
    double R = cos(M_PI/2.0 - dSliceAngle);
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0,0,1);
    glVertex3f(0,0,1);
    for(int i=0; i<nSegments;i++)
      {
	glNormal3f(R * sin((double)i * dSegAngle), R * cos((double)i * dSegAngle),  Z);
	glVertex3f(R * sin((double)i * dSegAngle), R * cos((double)i * dSegAngle),  Z);
      }
    glNormal3f(0,R,Z);
    glVertex3f(0,R,Z);
    glEnd();
  }
  
  int nBlueSlice = 3;
  int nWhiteSlice = 6;
  for(int j = 1; j<nSlices;j++)
    {
      if(j == nBlueSlice)
	glColor3f(0,0,1);
      if(j == nWhiteSlice)
	glColor4d(0.92, 0.9, 0.85,1);
      
      glBegin(GL_QUAD_STRIP);
      double zTop = sin(M_PI/2.0 - dSliceAngle * (double)j);
      double zBot = sin(M_PI/2.0 - dSliceAngle * (double)(j+1));
      double rTop = cos(M_PI/2.0 - dSliceAngle * (double)j);
      double rBot = cos(M_PI/2.0 - dSliceAngle * (double)(j+1));
      for(int i=0; i<nSegments;i++)
	{
	  glNormal3f(rTop*sin((double)i*dSegAngle), rTop*cos((double)i*dSegAngle), zTop);
	  glVertex3f(rTop*sin((double)i*dSegAngle), rTop*cos((double)i*dSegAngle), zTop);
	  glNormal3f(rBot*sin((double)i*dSegAngle), rBot*cos((double)i*dSegAngle), zBot);
	  glVertex3f(rBot*sin((double)i*dSegAngle), rBot*cos((double)i*dSegAngle), zBot);
	};
      glNormal3f(0,rTop, zTop);
      glVertex3f(0,rTop, zTop);
      glNormal3f(0,rBot, zBot);
      glVertex3f(0,rBot, zBot);
      glEnd();
    };

  {
    // South pole:
    double Z = sin(M_PI/2.0 - dSliceAngle);
    double R = cos(M_PI/2.0 - dSliceAngle);
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0,0,-1);
    glVertex3f(0,0,-1);
    for(int i=0; i<nSegments;i++)
      {
	glNormal3f(R * sin((double)i * -dSegAngle), R * cos((double)i * -dSegAngle),  -Z);
	glVertex3f(R * sin((double)i * -dSegAngle), R * cos((double)i * -dSegAngle),  -Z);
      }
    glNormal3f(0,R,-Z);
    glVertex3f(0,R,-Z);
    glEnd();
  };
}

void Miku::Init()
{
  //if(mbInitialised) return;
  //mbInitialised = true;
  //// Set up the display list for the eyeball.
  //mnEyeDisplayList = glGenLists(1);
  //glNewList(mnEyeDisplayList,GL_COMPILE);
  //DrawEye();
  //glEndList();
  //MakeShadowTex();


   if(mbInitialised)  return;
    mbInitialised  =  true;

};


void Miku::LookAt(int nEye, Vector<3> v3, double dRotLimit)
{
  Vector<3> v3E = ase3WorldFromEye[nEye].inverse() * v3;
  if(v3E * v3E == 0.0)
    return;
  
  normalize(v3E);
  Matrix<3> m3Rot = Identity;
  m3Rot[2] = v3E;
  m3Rot[0] -= m3Rot[2]*(m3Rot[0]*m3Rot[2]); 
  normalize(m3Rot[0]);
  m3Rot[1] = m3Rot[2] ^ m3Rot[0];
  
  SO3<> so3Rotator = m3Rot;
  Vector<3> v3Log = so3Rotator.ln();
  v3Log[2] = 0.0;
  double dMagn = sqrt(v3Log * v3Log);
  if(dMagn > dRotLimit)
    {
      v3Log = v3Log * ( dRotLimit / dMagn);
    }
  ase3WorldFromEye[nEye].get_rotation() = ase3WorldFromEye[nEye].get_rotation() * SO3<>::exp(-v3Log);
};

void Miku::MakeShadowTex()
{
  const int nTexSize = 256;
  Image<byte> imShadow(ImageRef(nTexSize, nTexSize));
  double dFrac = 1.0 - mdEyeRadius / mdShadowHalfSize;
  double dCenterPos = dFrac * nTexSize / 2 - 0.5;
  ImageRef irCenter; irCenter.x = irCenter.y = (int) dCenterPos;
  int nRadius = int ((nTexSize / 2 - irCenter.x) * 1.05);
  unsigned int nRadiusSquared = nRadius*nRadius;
  ImageRef ir;
  for(ir.y = 0; 2 * ir.y < nTexSize; ir.y++)
    for(ir.x = 0; 2 * ir.x < nTexSize; ir.x++)
      {
	byte val = 0;
	if((ir - irCenter).mag_squared() < nRadiusSquared)
	  val = 255;
	imShadow[ir] = val;
	imShadow[ImageRef(nTexSize - 1 - ir.x, ir.y)] = val;
	imShadow[ImageRef(nTexSize - 1 - ir.x, nTexSize - 1 - ir.y)] = val;
	imShadow[ImageRef(ir.x, nTexSize - 1 - ir.y)] = val;
      }
  
  convolveGaussian(imShadow, 4.0);
  glGenTextures(1, &mnShadowTex);
  glBindTexture(GL_TEXTURE_2D,mnShadowTex);
  glTexImage2D(GL_TEXTURE_2D, 0, 
	       GL_ALPHA, nTexSize, nTexSize, 0, 
	       GL_ALPHA, GL_UNSIGNED_BYTE, imShadow.data()); 
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
};





