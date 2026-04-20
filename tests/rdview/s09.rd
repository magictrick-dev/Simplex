# This is a test/example rd file

Display "Device Coord Test"  "Screen" "rgbsingle"
Format 600 400

WorldBegin


Color 1.0 0.5 0.7

# In a fully functioning transformation pipeline, the next two lines
# must be uncommented for the line to be seen.
#Scale 0.005 -0.005 1
#Translate -300 -200 -1


# This is an intermediate debugging version for device coordinates

# Line is x1 y1 z1  x2 y2 z2

Line 100 100 0 500 300 0

WorldEnd
