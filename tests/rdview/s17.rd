# This is a test/example rd file

Display "Camera Test 2"  "Screen" "rgbsingle"
Format 640 480

FrameBegin 1

# This provides a test of arbitrary camera viewpoint and change of field
# of view

CameraEye   -5.0 -6.0 7.0
CameraAt    1.0 1.0 1.0
CameraUp   0 0 1

CameraFOV  30.0

WorldBegin

Scale 1.2 1.2 1.2

Translate 1.0 1.0 1.0

Color 1.0 0.5 0.7

Cube

Scale 0.8 0.8 0.8
Color 0.5 1.0 0.7

Cube
WorldEnd
FrameEnd






