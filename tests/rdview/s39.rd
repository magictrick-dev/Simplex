# This is a test/example rd file
# Polygon clipping test on right boundary
# Correct result looks like half an octagon

Display "Solid Clipped Cube"  "Screen" "rgbsingle"
Format 640 480

CameraEye 0 0 12

WorldBegin

Translate 10 0 0

Scale 5 5 5

Cube

WorldEnd
