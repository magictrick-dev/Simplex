# This is a test/example rd file

Display "Line Test"  "Screen" "rgbsingle"
Format 640 480

# Testing line drawing

# The next three lines will be ignored by your renderer
CameraEye 320 240 -240
CameraAt  320 240 0
CameraUp  0.0 -1.0 0.0

FrameBegin 1
WorldBegin

# This line will also be ignored by your renderer
#Translate 0.5 0.5 0.0

Line 324 240 0  440 240 0
Line 324 242 0  440 300 0
Line 324 244 0  440 360 0
Line 322 244 0  380 360 0
Line 320 244 0  320 360 0
Line 318 244 0  260 360 0
Line 316 244 0  200 360 0
Line 316 242 0  200 300 0
Line 316 240 0  200 240 0
Line 316 238 0  200 180 0
Line 316 236 0  200 120 0
Line 318 236 0  260 120 0
Line 320 236 0  320 120 0
Line 322 236 0  380 120 0
Line 324 236 0  440 120 0
Line 324 238 0  440 180 0

WorldEnd
FrameEnd
