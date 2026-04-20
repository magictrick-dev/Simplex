# This is a test/example rd file

Display "Camera Test"  "Screen" "rgbsingle"
Format 640 480

FrameBegin 1

# Provides a simple test of world to camera coordinates
# Should produce an identical transformation as 
# s12.rd

CameraEye  0 0 3
CameraAt   0 0 -1
CameraUp   0 1 0 

WorldBegin

Color 1.0 0.5 0.7

# Line is x1 y1 z1  x2 y2 z2

Line -1.0 -1.0 1.0   1.0 1.0 1.0

WorldEnd
FrameEnd






