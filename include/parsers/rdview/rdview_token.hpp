#pragma once
#include <utils/defs.hpp>
#include <filesystem>
#include <string>
#include <iosfwd>

enum RDViewTokenType
{
    RDViewTokenType_EOF,
    RDViewTokenType_Invalid,
    RDViewTokenType_Integer,
    RDViewTokenType_Real,
    RDViewTokenType_String,
    RDViewTokenType_Boolean,
    RDViewTokenType_Identifier,
    RDViewTokenType_Keyword,
};

enum RDViewKeywordType
{

    // Catchall
    RDViewKeywordType_Invalid,

    // General Commands
    RDViewKeywordType_Display,
    RDViewKeywordType_Format,
    RDViewKeywordType_Include,

    // File Structuring
    RDViewKeywordType_FrameBegin,
    RDViewKeywordType_FrameEnd,
    RDViewKeywordType_WorldBegin,
    RDViewKeywordType_WorldEnd,
    RDViewKeywordType_ObjectBegin,
    RDViewKeywordType_ObjectEnd,
    RDViewKeywordType_ObjectInstance,

    // Drawing Attributes
    RDViewKeywordType_Background,
    RDViewKeywordType_Color,
    RDViewKeywordType_Opacity,

    // Options
    RDViewKeywordType_OptionArray,
    RDViewKeywordType_OptionBool,
    RDViewKeywordType_OptionList,
    RDViewKeywordType_OptionReal,
    RDViewKeywordType_OptionString,

    // Camera
    RDViewKeywordType_CameraAt,
    RDViewKeywordType_CameraEye,
    RDViewKeywordType_CameraFOV,
    RDViewKeywordType_CameraUp,
    RDViewKeywordType_Clipping,

    // Geometry
    RDViewKeywordType_Point,
    RDViewKeywordType_Line,
    RDViewKeywordType_Circle,
    RDViewKeywordType_Fill,
    RDViewKeywordType_Disk,
    RDViewKeywordType_Cone,
    RDViewKeywordType_Cube,
    RDViewKeywordType_Cylinder,
    RDViewKeywordType_Sphere,
    RDViewKeywordType_Torus,
    RDViewKeywordType_Tube,
    RDViewKeywordType_Paraboloid,
    RDViewKeywordType_Hyperboloid,
    RDViewKeywordType_SqSphere,
    RDViewKeywordType_SqTorus,
    RDViewKeywordType_PointSet,
    RDViewKeywordType_LineSet,
    RDViewKeywordType_PolySet,
    RDViewKeywordType_Curve,
    RDViewKeywordType_Patch,
    RDViewKeywordType_Subdivision,

    // Geometric Transformations
    RDViewKeywordType_Translate,
    RDViewKeywordType_Scale,
    RDViewKeywordType_Rotate,
    RDViewKeywordType_Matrix,
    RDViewKeywordType_XformPush,
    RDViewKeywordType_XformPop,

    // Lighting
    RDViewKeywordType_AmbientLight,
    RDViewKeywordType_FarLight,
    RDViewKeywordType_PointLight,
    RDViewKeywordType_ConeLight,

    // Surface Attributes
    RDViewKeywordType_Ka,
    RDViewKeywordType_Kd,
    RDViewKeywordType_Ks,
    RDViewKeywordType_Specular,
    RDViewKeywordType_Surface,

    // Attribute Mapping
    RDViewKeywordType_MapLoad,
    RDViewKeywordType_Map,
    RDViewKeywordType_MapSample,
    RDViewKeywordType_MapBound,
    RDViewKeywordType_MapBorder,

};

struct RDViewToken
{

    inline RDViewToken() { } // NOTE(Chris): std::string_view will complain if not defined...

    RDViewTokenType type;
    size_t line;
    size_t column;
    size_t length;
    size_t offset;

    std::filesystem::path source_file_path;
    std::string_view source_file_contents;

    union
    {
        struct { int64_t value;                 } boolean;
        struct { int64_t value;                 } integer;
        struct { real64_t value;                } real;
        struct { std::string_view value;        } string;
        struct { std::string_view value;        } identifier;
        struct { RDViewKeywordType type;        } keyword;
    };

};

const char *to_string(RDViewTokenType type);
const char *to_string(RDViewKeywordType type);
std::ostream& operator<<(std::ostream &os, const RDViewToken &token);
