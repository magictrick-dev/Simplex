# This is a test/example rd file

Display "Point Test"  "Screen" "rgbsingle"

# Testing color


# The next three lines will be ignored by your renderer
CameraEye 320 240 -240
CameraAt  320 240 0
CameraUp  0.0 -1.0 0.0


FrameBegin 1

Background 0.3 0.3 0.3

WorldBegin

Color 0.9 0.5 0.2

Point 320 240 0

WorldEnd
FrameEnd
