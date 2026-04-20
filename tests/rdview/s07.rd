# This is a test/example rd file

Display "Oh No!"  "Screen" "rgbdouble"
Format 500 500 

# Testing filling and double buffering

Background 0.3 0.3 0.7

# The next three lines will be ignored by your renderer
CameraEye 250 250 -250
CameraAt  250 250 0
CameraUp  0.0 -1.0 0.0

FrameBegin 1

WorldBegin

#Face

Color 1.0 1.0 1.0

Circle  250 250 0 200

Fill 250 250 0


# Left eye

Color 0.0 0.0 1.0

Circle 170 190 0 30

Fill 170 190 0

# Right eye

Circle 330 190 0 30

Fill 330 190 0

# Mouth

Color 1.0 0.0 0.0

Circle 250 320 0 60

Circle 250 320 0 40

Fill 250 261 0

WorldEnd
FrameEnd
