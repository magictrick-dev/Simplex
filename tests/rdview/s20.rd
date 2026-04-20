# This is a test/example rd file

Display "PolySet"  "Screen" "rgbsingle"

Format 640 480
CameraFOV  45.0  # Degrees

ObjectBegin "Icosahedron"
# Icosahedron
PolySet "P"
12 # Vertices
20 # Faces

# Vertex points
 1.618034  1.0  0.0
-1.618034  1.0  0.0
 1.618034 -1.0  0.0
-1.618034 -1.0  0.0

 1.0  0.0  1.618034
 1.0  0.0 -1.618034
-1.0  0.0  1.618034
-1.0  0.0 -1.618034

 0.0  1.618034  1.0
 0.0 -1.618034  1.0
 0.0  1.618034 -1.0
 0.0 -1.618034 -1.0

# Face indices
 0  8  4 -1
 0  5 10 -1
 2  4  9 -1
 2 11  5 -1
 1  6  8 -1
 1 10  7 -1
 3  9  6 -1
 3  7 11 -1

 0 10  8 -1
 1  8 10 -1
 2  9 11 -1
 3 11  9 -1

 4  2  0 -1
 5  0  2 -1
 6  1  3 -1
 7  3  1 -1

 8  6  4 -1
 9  4  6 -1
10  5  7 -1
11  7  5 -1

ObjectEnd # Icosahedron

FrameBegin 1

CameraEye  -5.0  -6.0  7.0
CameraAt    1.0   1.0  1.0
CameraUp    0.0   0.0  1.0

Clipping    1.0   1.0e+09


WorldBegin


XformPush

Translate 1.0 1.0 1.0

Scale 2.0 2.0 2.0

Color 1.0 0.5 0.7

ObjectInstance "Icosahedron"
XformPop

XformPush

Color 1.0 1.0 1.0

Translate 1.0 1.0 1.0

Sphere 1.0 -1.0 1.0 360.0

XformPop

WorldEnd
FrameEnd
