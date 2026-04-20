# This is a test/example rd file

Display "Circle Test"  "Screen" "rgbsingle"
Format 500 300 

# Testing circles

# The next three lines will be ignored by your renderer
CameraEye 250 150 -150
CameraAt  250 150 0
CameraUp  0.0 -1.0 0.0

FrameBegin 1

Background 0.3 0.3 0.3

WorldBegin

# x y z of center, then radius

Circle 250 150 0 20

Circle 250 150 0 40

Circle 250 150 0 60

Circle 250 150 0 80

Circle 250 150 0 100

WorldEnd
FrameEnd
