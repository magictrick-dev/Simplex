# This is a test/example rd file

Display "Cone Test"  "Screen" "rgbsingle"

Background 0.7 0.7 0.7

CameraEye -10 -10 10 
#CameraEye -100 5 0 
CameraAt  5 5 0
CameraUp  0 0 1
CameraFOV 30

WorldBegin

Color 1.0 0.4 0.4

XformPush
Translate 0 0 0
Cone 1.0 1.0 360.0
XformPop


Color 0.4 1.0 0.4

XformPush
Translate 5 0 0
Cone 2.0 1.0 360.0
XformPop

Color 1.0 1.0 1.0
XformPush
Translate 10 0 0
Cone 3.0 1.0 360.0
XformPop

Color 0.4 0.4 1.0
XformPush
Translate 5 10 0
Cone 1.0 2.0 360.0
XformPop

Color 0.0 0.0 0.0
XformPush
Translate 10 10 0
Cone 1.0 3.0 360.0
XformPop



WorldEnd
