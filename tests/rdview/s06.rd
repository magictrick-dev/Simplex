# This is a test/example rd file

Display "Fill Test"  "Screen" "rgbsingle"
Format 400 400 

# Testing filling

# The next three lines will be ignored by your renderer
CameraEye 200 200 -200
CameraAt  200 200 0
CameraUp  0.0 -1.0 0.0

FrameBegin 1

WorldBegin

# Draw some squares

Line  50 350 0  350 350 0

Line 350 350 0  350  50 0

Line 350  50 0   50  50 0

Line  50  50 0   50 350 0


Line 150  50 0  150 150 0

Line 150 150 0  250 150 0

Line 250 150 0  250  50 0


Line 150 350 0  150 251 0

Line 151 250 0  249 250 0

Line 250 251 0  250 350 0


Color 1.0  0.5 0.5

Fill 200 300 0

Color 0.5 0.5 1.0

Fill 200 100 0

Color 0.5 1.0 0.5

Fill 200 200 0

WorldEnd
FrameEnd
