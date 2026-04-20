Display "Cylinder Test" "Screen" "rgbdouble"

Background 0.5 0.5 0.5

CameraEye -20 -10 10
CameraAt 1 -3 3
CameraUp 0 0 1
CameraFOV 30.0

FrameBegin 1
WorldBegin

Translate 0 0 0

XformPush # Row of cylinders

Color 0.0 0.0 0.0

Cylinder 1.0 0.0 1.0 360.0

Translate 3.0 0.0 0.0

XformPush
Translate 0 0 1
Scale 2.0 2.0 2.0
Cylinder 1.0 0.0 1.0 360.0
XformPop

Translate 3.0 0.0 0.0

XformPush
Translate 0 0 3
Scale 3.0 3.0 3.0
Cylinder 1.0 0.0 1.0 360.0
XformPop

Translate 0.0 -5.0 0.0

XformPush
Scale 1.0 1.0 3.0
Cylinder 1.0 0.0 1.0 360.0
XformPop

Translate -6.0 0.0 0.0

XformPush
Scale 1.0 1.0 2.0
Cylinder 1.0 0.0 1.0 360.0
XformPop

XformPop

WorldEnd
FrameEnd
