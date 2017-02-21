
## PTAM stereo

this is the PTAM stereo visual studio project for VisionerTech VMG-PROV. It is used to sense the environment and localize the headset.


## Requirement:

1.  Recommended specs: Intel Core i5-4460/8G RAM/GTX 660/at least two USB3.0/
2.  Windows x64 version.(tested on win7/win10)

## Installation

1.  Download and install the  Visual Studio 2012, the download address is here: https://www.microsoft.com/zh-cn/download/details.aspx?id=30682

2.  Download and install OpenCV(version 2.4.X) as:
http://docs.opencv.org/2.4.11/doc/tutorials/introduction/windows_install/windows_install.html

3.  Open "/PTAM-opencv/Build/Win32/PTAM.sln", then change settings for visual studio project linked with OpenCV:
http://docs.opencv.org/2.4.11/doc/tutorials/introduction/windows_visual_studio_Opencv/windows_visual_studio_Opencv.html

## How to Run

1. Put camera parameters got from stereo_calib or stereo_calib_executable to "PTAM_stereo-master/PTAM-opencv/Build/Win32/exe"
2. Change "PTAM_stereo-master/PTAM-opencv/Build/Win32/exe/Camera.cfg" based on left eye camera parameters, for example, the data in "save_param/intrinsics.yml" is like below:
        image_width: 1080
        image_height: 1080
        M1: !!opencv-matrix
         rows: 3
         cols: 3
         dt: d
         data: [ 1.1101959818598448e+003, 0., 6.6146645449411790e+002, 0.,
             1.1007281480206175e+003, 6.0699701717373546e+002, 0., 0., 1. ]

      M1 is the intrinsic parameter of left camera(https://en.wikipedia.org/wiki/Camera_resectioning),so the parameters in Camera.cfg should be like:

      Camera.Parameters=[ 111.019/1080 110.072/1080 661.466/1080 606.997/1080 0 ]
3. Build "/PTAM-opencv/Build/Win32/PTAM.sln" release version
4. Run PTAM.exe in "PTAM_stereo-master\PTAM-opencv\Build\Win32\exe" with command window like:

    PTAM.exe 0 1

    Put your hands in front of the left camera, check whether the video has consistent changes. If not, the camera index is swop because of different system. So camera index order should be changed as:

    PTAM.exe 1 0
5. Put your camera towards a plane scene, and press "space" button
![alt text](https://github.com/VisionerTech/PTAM_stereo/blob/master/readme_images/space.png "space")
6. There will be gridding in the window, and feature points are also shown. If these don't appear, press "Reset" button and change the camera towards a more planar scene. Then press "Spacebar" button.
![alt text](https://github.com/VisionerTech/PTAM_stereo/blob/master/readme_images/space2.png "space2")
7. Press "View Map Off", then the point cloud and pose track are available to see.
![alt text](https://github.com/VisionerTech/PTAM_stereo/blob/master/readme_images/map.png "map")
8. Press "Draw AR Off", then a miku will be shown the screen. Maxmize this window, and press "Shift+Ctrl+Right", it will be shown on the headset screen.
![alt text](https://github.com/VisionerTech/PTAM_stereo/blob/master/readme_images/miku.jpg "miku")
