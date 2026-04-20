Display "Point Test" "Screen" "rgbsingle"

CameraAt 0 1 0
CameraUp 0 0 1

WorldBegin

Color 1.0 0.0 0.0
Point 0 7 0

# This point should be clipped out of existence
Color 1.0 1.0 0.0
Point 0 -7 0

Color 0.0 1.0 1.0
Point 7 7 0

Color 1.0 0.0 1.0
Point 7 7 6

WorldEnd