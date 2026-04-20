# This is a test/example rd file

Display "Rotation Test"  "Screen" "rgbsingle"
Format 640 480

#This file provides a test of anisotropic scaling and rotation

FrameBegin 1

CameraEye   -5.0 -6.0 7.0
CameraAt    1.0 1.0 -1.0
CameraUp   0 0 1

CameraFOV  30.0

WorldBegin

XformPush
Color 1.0 1.0 1.0

Rotate "Z" -30.0

Scale 0.05 1.0 1.0

Cube

XformPush
Color 0.5 1.0 0.5
Translate 30 0 0 
Rotate "X" 50
Cube

XformPop
XformPop

XformPush
Color 0.5 0.5 1.0
Translate -2 0 0 
Rotate "Y" -30
Scale 1.0 1.0 0.05

Cube 
XformPop

WorldEnd
FrameEnd






