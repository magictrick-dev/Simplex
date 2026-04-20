# A test of the zbuffer

Display "Z buffer test" "Screen" "rgbsingle"

Format 500 500

CameraFOV 40.0

CameraEye 0 0 -20
CameraAt 0 0 0 
CameraUp 0 1 0 

Clipping 1 100

ObjectBegin "Blade"
XformPush
Translate 0 3 0
Rotate "Y" 60
Scale 0.02  1  5

Cube
XformPop
ObjectEnd # Blade

WorldBegin

Color 1.0 0.0 0.0
ObjectInstance "Blade"

Rotate "Z" 30
Color 0.0 0.5 1.0
ObjectInstance "Blade"

Rotate "Z" 30
Color 1.0 1.0 0.0
ObjectInstance "Blade"

Rotate "Z" 30
Color 0.5 0.0 1.0
ObjectInstance "Blade"

Rotate "Z" 30
Color 0.0 1.0 0.0
ObjectInstance "Blade"

Rotate "Z" 30
Color 1.0 0.0 0.5
ObjectInstance "Blade"

Rotate "Z" 30
Color 0.0 1.0 1.0
ObjectInstance "Blade"

Rotate "Z" 30
Color 1.0 0.5 0.0
ObjectInstance "Blade"

Rotate "Z" 30
Color 0.0 0.0 1.0
ObjectInstance "Blade"

Rotate "Z" 30
Color 0.5 1.0 0.0
ObjectInstance "Blade"

Rotate "Z" 30
Color 1.0 0.0 1.0
ObjectInstance "Blade"

Rotate "Z" 30
Color 0.0 1.0 0.5
ObjectInstance "Blade"

WorldEnd
