# This is a test/example rd file

Display "Cylinder Test"  "Screen" "rgbsingle"

Background 0.7 0.7 0.7

CameraEye -10 -10 10 
#CameraEye -100 5 0 
CameraAt  0 0 2
CameraUp  0 0 1
CameraFOV 30

WorldBegin

Color 1.0 0.8 0.8

Cylinder 5 0 1 360

Cylinder 4 1 2 360

Cylinder 3 2 3 360

Cylinder 2 3 4 360

Cylinder 1 4 5 360

WorldEnd
