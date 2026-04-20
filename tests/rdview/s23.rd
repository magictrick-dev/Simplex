Display "Sphere Test" "Screen" "rgbdouble"

Background 0.5 0.5 0.5

CameraEye -10 -20 10
CameraAt 3 0 0
CameraUp 0 0 1
CameraFOV 30.0

FrameBegin 1
WorldBegin

Translate 0 0 0

XformPush # Row of spheres


Sphere 1.0 -1.0 1.0 360.0

Translate 4.0 0.0 0.0

XformPush
Scale 2.0 2.0 2.0
Sphere 1.0 -1.0 1.0 360.0
XformPop

Translate 8.0 0.0 0.0

XformPush
Scale 3.0 3.0 3.0
Sphere 1.0 -1.0 1.0 360.0
XformPop

XformPop

WorldEnd
FrameEnd
