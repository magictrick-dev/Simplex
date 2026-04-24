#pragma once

enum RDTokenType
{
    RDTokenType_Integer,
    RDTOkenType_Real,
    RDTokenType_String,
    RDTokenType_Boolean,
    RDTokenType_Identifier,
};

enum RDIdentifierType
{
    // General Commands
    RDIdentifierType_Display,
    RDIdentifierType_Format,
    RDIdentifierType_Include,

    // File Structuring
    RDIdentifierType_FrameBegin,
    RDIdentifierType_FrameEnd,
    RDIdentifierType_WorldBegin,
    RDIdentifierType_WorldEnd,
    RDIdentifierType_ObjectBegin,
    RDIdentifierType_ObjectEnd,
    RDIdentifierType_ObjectInstance,

    // Drawing Attributes
    RDIdentifierType_Background,
    RDIdentifierType_Color,
    RDIdentifierType_Opacity,

    // Options
    RDIdentifierType_OptionArray,
    RDIdentifierType_OptionBool,
    RDIdentifierType_OptionList,
    RDIdentifierType_OptionReal,
    RDIdentifierType_OptionString,

    // Camera
    RDIdentifierType_CameraAt,
    RDIdentifierType_CameraEye,
    RDIdentifierType_CameraFOV,
    RDIdentifierType_CameraUp,
    RDIdentifierType_Clipping,

    // Geometry
    RDIdentifierType_Point,
    RDIdentifierType_Line,
    RDIdentifierType_Circle,
    RDIdentifierType_Fill,
    RDIdentifierType_Disk,
    RDIdentifierType_Cone,
    RDIdentifierType_Cube,
    RDIdentifierType_Cylinder,
    RDIdentifierType_Sphere,
    RDIdentifierType_Torus,
    RDIdentifierType_Tube,
    RDIdentifierType_Paraboloid,
    RDIdentifierType_Hyperboloid,
    RDIdentifierType_SqSphere,
    RDIdentifierType_SqTorus,
    RDIdentifierType_PointSet,
    RDIdentifierType_LineSet,
    RDIdentifierType_PolySet,
    RDIdentifierType_Curve,
    RDIdentifierType_Patch,
    RDIdentifierType_Subdivision,

    // Geometric Transformations
    RDIdentifierType_Translate,
    RDIdentifierType_Scale,
    RDIdentifierType_Rotate,
    RDIdentifierType_Matrix,
    RDIdentifierType_XformPush,
    RDIdentifierType_XformPop,

    // Lighting
    RDIdentifierType_AmbientLight,
    RDIdentifierType_FarLight,
    RDIdentifierType_PointLight,
    RDIdentifierType_ConeLight,

    // Surface Attributes
    RDIdentifierType_Ka,
    RDIdentifierType_Kd,
    RDIdentifierType_Ks,
    RDIdentifierType_Specular,
    RDIdentifierType_Surface,

    // Attribute Mapping
    RDIdentifierType_MapLoad,
    RDIdentifierType_Map,
    RDIdentifierType_MapSample,
    RDIdentifierType_MapBound,
    RDIdentifierType_MapBorder,
};