# This is a test/example rd file

Display "Pop Sphere Test"  "Screen" "rgbsingle"
Format 640 480

#This file provides a test of XformPush, XformPop, and Sphere

FrameBegin 1

CameraEye   -5.0 -6.0 7.0
CameraAt    1.0 1.0 1.0
CameraUp   0 0 1

CameraFOV  30.0

WorldBegin

XformPush

Scale 1.2 1.2 1.2

Translate 1.0 1.0 1.0

Color 1.0 0.5 0.7

Cube

Scale 0.8 0.8 0.8
Color 0.5 1.0 0.7

Cube

XformPop

XformPush

Color 1.0 1.0 0.7

Translate 1.0 1.0 1.0

Sphere 1.0 -1.0 1.0 360.0

XformPop

WorldEnd
FrameEnd






