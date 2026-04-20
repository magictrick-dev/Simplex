# This is a test/example rd file

Display "Line Test"  "Screen" "rgbsingle"

# Testing the display size (Format command) and color line drawing

Format 80 60

# The next three lines will be ignored by your renderer
CameraEye 40 30 -30
CameraAt  40 30 0
CameraUp  0.0 -1.0 0.0

FrameBegin 1

Background 0.3 0.3 0.3

WorldBegin

#This line will be ignored by your renderer
Translate 0.5 0.5 0.0

Color 1.0 0.0 0.0
Line 44 30 0 54 30 0

Color 1.0 0.2308 0
Line 44 32 0 54 37 0

Color 1.0 0.6 0
Line 44 34 0 54 44 0

Color 0.7778 1.0 0
Line 42 34 0 47 44 0

Color 0.3333 1.0 0
Line 40 34 0 40 44 0

Color 0.0667 1.0 0
Line 38 34 0 33 44 0

Color 0 1.0 0.1429
Line 36 34 0 26 44 0

Color 0 1.0 0.4545
Line 36 32 0 26 37 0

Color 0 1.0 1.0
Line 36 30 0 26 30 0

Color 0 0.4545 1.0
Line 36 28 0 26 23 0

Color 0 0.1429 1.0
Line 36 26 0 26 16 0

Color 0.0667 0 1.0
Line 38 26 0 33 16 0

Color 0.3333 0 1.0
Line 40 26 0 40 16 0

Color 0.7778 0 1.0
Line 42 26 0 47 16 0

Color 1.0 0 0.6
Line 44 26 0 54 16 0

Color 1.0 0 0.2308
Line 44 28 0 54 23 0

WorldEnd
FrameEnd

