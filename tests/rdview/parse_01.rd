# This is a file to validate parsing rules.
# We should note that the Format command is required going forward.
Display "Parse 01" "Screen" "rgbdouble"
Format 1280 960

# Required for 3D rendering, ignored for basic 2D rendering
# during the first rendering assignment.
CameraEye   320     240    -240
CameraAt    320     240     0
CameraUp    0.0    -1.0     0.0
CameraFOV   60

# Whitespace is ignored, but we can make use of tabs to make things look cleaner.
# Additionally, FrameBegin becomes required syntax, 
FrameBegin 0
    WorldBegin
        Point 320 240 0
    WorldEnd
FrameEnd

