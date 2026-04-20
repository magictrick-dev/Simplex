# This is a test/example rd file
# Polygon clipping test on left, top, right, and bottom boundaries

Display "Solid Clipped Cube"  "Screen" "rgbsingle"
Format 640 480

CameraEye 0 0 10

WorldBegin

Rotate "Z" 45
Scale 5 5 5

Cube

WorldEnd
