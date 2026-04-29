// -------------------------------------------------------------------------------------------------
// RDView Parser
//      Christopher DeJong / magictrick-dev 
//      April 2026
// -------------------------------------------------------------------------------------------------
//
// RDView Grammar
//
// - Structural 
//          ROOT                    :   DISPLAY FORMAT BODY
//          BODY                    :   (INCLUDE | DEFINITIONS)* ((FRAME | WORLD) BODY | EOF)
//          DEFINITIONS             :   OBJECT | OPTION_ARRAY | OPTION_BOOL | OPTION_LIST | 
//                                      OPTION_REAL | OPTION_STRING
// 
//          INCLUDE                 :   "Include" string
//          DISPLAY                 :   "Display" string string string
//          FORMAT                  :   "Format" integer integer
//          OBJECT                  :   "ObjectBegin" integer? string OBJECT_COMMANDS* "ObjectEnd"
//
//          FRAME                   :   "FrameBegin" integer FRAME_COMMANDS* WORLD "FrameEnd"
//          WORLD                   :   "WorldBegin" WORLD_COMMANDS* "WorldEnd"
//
// - Properties
//          FRAME_COMMANDS          :   BACKGROUND | COLOR | OPACITY | CAMERA | LIGHTING | 
//                                      SURFACE_ATTRIBUTES | MAP_LOAD | ATTRIBUTE_MAPPING | MAP
//          WORLD_COMMANDS          :   OPACITY | COLOR | GEOMETRY | TRANSFORMS | LIGHTING |
//                                      SURFACE_ATTRIBUTES | ATTRIBUTE_MAPPING | MAP
//          OBJECT_COMMANDS         :   OPACITY | COLOR | GEOMETRY | TRANSFORMS |
//                                      SURFACE_ATTRIBUTES | ATTRIBUTE_MAPPING | MAP
//          CAMERA                  :   CAMERA_AT | CAMERA_EYE | CAMERA_FOV | CAMERA_UP | CLIPPING
//          GEOMETRY                :   POINT | POINT_SET | LINE | LINE_SET | CIRCLE | FILL | CONE |
//                                      CUBE | CURVE | CYLINDER | DISK | HYPERBOLOID | PARABOLOID |
//                                      PATCH | POLY_SET | SPHERE | SQ_SPHERE | SQ_TORUS |
//                                      TORUS | TUBE | SUBDIVISION | OBJECT_INSTANCE
//          TRANSFORMS              :   MATRIX | ROTATE | SCALE | TRANSLATE | XFORMPUSH | XFORMPOP
//          LIGHTING                :   AMBIENT_LIGHT | FAR_LIGHT | POINT_LIGHT | CONE_LIGHT
//          SURFACE_ATTRIBUTES      :   KA | KD | KS | SPECULAR | SURFACE
//          ATTRIBUTE_MAPPING       :   MAP_SAMPLE | MAP_BOUND | MAP_BORDER
//  
// - Commands
//          OPTION_ARRAY            :   "OptionArray" string integer numerical.[n]
//          OPTION_BOOL             :   "OptionBool" string (integer | "true" | "false" | "on" | "off")
//          OPTION_LIST             :   "OptionList" string integer string.[n]
//          OPTION_REAL             :   "OptionReal" string numerical
//          OPTION_STRING           :   "OptionString" string string
//          BACKGROUND              :   "Background" real real real
//          COLOR                   :   "Color" real real real
//          OPACITY                 :   "Opacity" real
//          CAMERA_AT               :   "CameraAt" numerical.[3]
//          CAMERA_EYE              :   "CameraEye" numerical.[3]
//          CAMERA_FOV              :   "CameraFOV" numerical
//          CAMERA_UP               :   "CameraUp" numerical.[3]
//          CLIPPING                :   "Clipping" numerical.[2]
//          POINT                   :   "Point" numerical.[3]
//          POINT_SET               :   "PointSet" string integer numerical.[n]
//          LINE                    :   "Line" numerical.[6]
//          LINE_SET                :   "LineSet" string integer integer numerical.[n]
//          CIRCLE                  :   "Circle" numerical.[4]
//          FILL                    :   "Fill" numerical.[3]
//          CONE                    :   "Cone" numerical.[3]
//          CUBE                    :   "Cube"
//          CURVE                   :   "Curve" string string integer numerical.[n]
//          CYLINDER                :   "Cylinder" numerical.[4]
//          DISK                    :   "Disk" numerical.[3]
//          HYPERBOLOID             :   "Hyperboloid" numerical.[7]
//          PARABOLOID              :   "Paraboloid" numerical.[4]
//          PATCH                   :   "Patch" string string integer integer numerical.[n]
//          POLY_SET                :   "PolySet" string integer integer numerical.[n] integer.[n]
//          SPHERE                  :   "Sphere" numerical.[4]
//          SQ_SPHERE               :   "SqSphere" numerical integer numerical.[4]
//          SQ_TORUS                :   "SqTorus" numerical.[2] integer numerical.[4]
//          TORUS                   :   "Torus" numerical.[5]
//          TUBE                    :   "Tube" numerical.[7]
//          SUBDIVISION             :   "Subdivision" string string integer integer integer numerical.[n] integer.[n] integer.[n] real.[n]
//          OBJECT_INSTANCE         :   "ObjectInstance" string numerical.[n]
//          MATRIX                  :   "Matrix" numerical.[16]
//          ROTATE                  :   "Rotate" string numerical
//          SCALE                   :   "Scale" numerical.[3]
//          TRANSLATE               :   "Translate" numerical.[3]
//          XFORMPUSH               :   "XformPush"
//          XFORMPOP                :   "XformPop"
//          AMBIENT_LIGHT           :   "AmbientLight" numerical.[4]
//          FAR_LIGHT               :   "FarLight" numerical.[7]
//          POINT_LIGHT             :   "PointLight" numerical.[7]
//          CONE_LIGHT              :   "ConeLight" numerical.[12]
//          KA                      :   "Ka" real
//          KD                      :   "Kd" real
//          KS                      :   "Ks" real
//          SPECULAR                :   "Specular" real.[4]
//          SURFACE                 :   "Surface" string
//          MAP_LOAD                :   "MapLoad" string string
//          MAP                     :   "Map" string string
//          MAP_SAMPLE              :   "MapSample" string string string
//          MAP_BOUND               :   "MapBound" string real real real real
//          MAP_BORDER              :   "MapBorder" string string string
//
// -------------------------------------------------------------------------------------------------
#pragma once
#include <utils/defs.hpp>
#include <filesystem>
#include <string>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <stack>
#include <algorithm>
#include <variant>
#include <exception>

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

inline const char *
to_string(RDViewTokenType type)
{
    switch (type)
    {
        case RDViewTokenType_EOF:           { return "RDViewTokenType_EOF";         } break;
        case RDViewTokenType_Invalid:       { return "RDViewTokenType_Invalid";     } break;
        case RDViewTokenType_Integer:       { return "RDViewTokenType_Integer";     } break;
        case RDViewTokenType_Real:          { return "RDViewTokenType_Real";        } break;
        case RDViewTokenType_String:        { return "RDViewTokenType_String";      } break;
        case RDViewTokenType_Boolean:       { return "RDViewTokenType_Boolean";     } break;
        case RDViewTokenType_Identifier:    { return "RDViewTokenType_Identifier";  } break;
        case RDViewTokenType_Keyword:       { return "RDViewTokenType_Keyword";     } break;
    }

    SIMPLEX_NO_REACH("");
    return "";
}

inline const char *
to_string(RDViewKeywordType type)
{
    switch (type)

    {

        case RDViewKeywordType_Invalid:         { return "Invalid";         } break;
        case RDViewKeywordType_Display:         { return "Display";         } break;
        case RDViewKeywordType_Format:          { return "Format";          } break;
        case RDViewKeywordType_Include:         { return "Include";         } break;
        case RDViewKeywordType_FrameBegin:      { return "FrameBegin";      } break;
        case RDViewKeywordType_FrameEnd:        { return "FrameEnd";        } break;
        case RDViewKeywordType_WorldBegin:      { return "WorldBegin";      } break;
        case RDViewKeywordType_WorldEnd:        { return "WorldEnd";        } break;
        case RDViewKeywordType_ObjectBegin:     { return "ObjectBegin";     } break;
        case RDViewKeywordType_ObjectEnd:       { return "ObjectEnd";       } break;
        case RDViewKeywordType_ObjectInstance:  { return "ObjectInstance";  } break;
        case RDViewKeywordType_Background:      { return "Background";      } break;
        case RDViewKeywordType_Color:           { return "Color";           } break;
        case RDViewKeywordType_Opacity:         { return "Opacity";         } break;
        case RDViewKeywordType_OptionArray:     { return "OptionArray";     } break;
        case RDViewKeywordType_OptionBool:      { return "OptionBool";      } break;
        case RDViewKeywordType_OptionList:      { return "OptionList";      } break;
        case RDViewKeywordType_OptionReal:      { return "OptionReal";      } break;
        case RDViewKeywordType_OptionString:    { return "OptionString";    } break;
        case RDViewKeywordType_CameraAt:        { return "CameraAt";        } break;
        case RDViewKeywordType_CameraEye:       { return "CameraEye";       } break;
        case RDViewKeywordType_CameraFOV:       { return "CameraFOV";       } break;
        case RDViewKeywordType_CameraUp:        { return "CameraUp";        } break;
        case RDViewKeywordType_Clipping:        { return "Clipping";        } break;
        case RDViewKeywordType_Point:           { return "Point";           } break;
        case RDViewKeywordType_Line:            { return "Line";            } break;
        case RDViewKeywordType_Circle:          { return "Circle";          } break;
        case RDViewKeywordType_Fill:            { return "Fill";            } break;
        case RDViewKeywordType_Disk:            { return "Disk";            } break;
        case RDViewKeywordType_Cone:            { return "Cone";            } break;
        case RDViewKeywordType_Cube:            { return "Cube";            } break;
        case RDViewKeywordType_Cylinder:        { return "Cylinder";        } break;
        case RDViewKeywordType_Sphere:          { return "Sphere";          } break;
        case RDViewKeywordType_Torus:           { return "Torus";           } break;
        case RDViewKeywordType_Tube:            { return "Tube";            } break;
        case RDViewKeywordType_Paraboloid:      { return "Paraboloid";      } break;
        case RDViewKeywordType_Hyperboloid:     { return "Hyperboloid";     } break;
        case RDViewKeywordType_SqSphere:        { return "SqSphere";        } break;
        case RDViewKeywordType_SqTorus:         { return "SqTorus";         } break;
        case RDViewKeywordType_PointSet:        { return "PointSet";        } break;
        case RDViewKeywordType_LineSet:         { return "LineSet";         } break;
        case RDViewKeywordType_PolySet:         { return "PolySet";         } break;
        case RDViewKeywordType_Curve:           { return "Curve";           } break;
        case RDViewKeywordType_Patch:           { return "Patch";           } break;
        case RDViewKeywordType_Subdivision:     { return "Subdivision";     } break;
        case RDViewKeywordType_Translate:       { return "Translate";       } break;
        case RDViewKeywordType_Scale:           { return "Scale";           } break;
        case RDViewKeywordType_Rotate:          { return "Rotate";          } break;
        case RDViewKeywordType_Matrix:          { return "Matrix";          } break;
        case RDViewKeywordType_XformPush:       { return "XformPush";       } break;
        case RDViewKeywordType_XformPop:        { return "XformPop";        } break;
        case RDViewKeywordType_AmbientLight:    { return "AmbientLight";    } break;
        case RDViewKeywordType_FarLight:        { return "FarLight";        } break;
        case RDViewKeywordType_PointLight:      { return "PointLight";      } break;
        case RDViewKeywordType_ConeLight:       { return "ConeLight";       } break;
        case RDViewKeywordType_Ka:              { return "Ka";              } break;
        case RDViewKeywordType_Kd:              { return "Kd";              } break;
        case RDViewKeywordType_Ks:              { return "Ks";              } break;
        case RDViewKeywordType_Specular:        { return "Specular";        } break;
        case RDViewKeywordType_Surface:         { return "Surface";         } break;
        case RDViewKeywordType_MapLoad:         { return "MapLoad";         } break;
        case RDViewKeywordType_Map:             { return "Map";             } break;
        case RDViewKeywordType_MapSample:       { return "MapSample";       } break;
        case RDViewKeywordType_MapBound:        { return "MapBound";        } break;
        case RDViewKeywordType_MapBorder:       { return "MapBorder";       } break;

    }

    SIMPLEX_NO_REACH("");
    return "";

}

inline std::ostream& 
operator<<(std::ostream &os, const RDViewToken &token)
{

    std::string path_name = token.source_file_path.string();
    os << path_name << "(" << token.line << "," << token.column << "): ";
    os << to_string(token.type);
    switch (token.type)
    {
        case RDViewTokenType_Integer:       { os << " " << token.integer.value;                      } break;
        case RDViewTokenType_Real:          { os << " " << token.real.value;                         } break;
        case RDViewTokenType_String:        { os << " " << token.string.value;                       } break;
        case RDViewTokenType_Boolean:       { os << " " << (token.boolean.value ? "true" : "false"); } break;
        case RDViewTokenType_Identifier:    { os << " " << token.identifier.value;                   } break;
        case RDViewTokenType_Keyword:       { os << " " << to_string(token.keyword.type);            } break;
        default: { } break;
    }

    return os;

}

class RDViewTokenizer
{
    public:
        RDViewTokenizer(std::string_view source_contents, std::filesystem::path source_path);
        virtual ~RDViewTokenizer();

        RDViewToken get_previous_token() const;
        RDViewToken get_current_token() const;
        RDViewToken get_next_token() const;

        bool shift();
        bool previous_token_is(RDViewTokenType type) const;
        bool current_token_is(RDViewTokenType type) const;
        bool next_token_is(RDViewTokenType type) const;

    private:
        void match(RDViewToken *token);

        void handle_whitespace();
        bool handle_numbers(RDViewToken *token);
        bool handle_strings(RDViewToken *token);
        bool handle_identifiers(RDViewToken *token);

        void format_token_as(RDViewToken *token, RDViewTokenType type);
        RDViewKeywordType classify_identifier(RDViewToken *token);
        void parse_number_value(RDViewToken *token, RDViewTokenType token_type, std::string_view raw);

        void synchronize();
        char peak() const;
        char peak_at(size_t count) const;
        void consume();
        bool is_eof() const;

    private:
        std::filesystem::path path;
        std::string_view source;
        size_t offset;
        size_t step;
        size_t line;
        size_t column;

        RDViewToken tokens[3];
        RDViewToken *previous_token;
        RDViewToken *current_token;
        RDViewToken *next_token;

};

enum RDViewNodeType
{
    RDViewNodeType_NodeInterface,
    RDViewNodeType_Root,
    RDViewNodeType_Body,
    RDViewNodeType_Definitions,
    RDViewNodeType_Include,
    RDViewNodeType_Display,
    RDViewNodeType_Format,
    RDViewNodeType_Object,
    RDViewNodeType_Frame,
    RDViewNodeType_World,
    RDViewNodeType_Camera,
    RDViewNodeType_Geometry,
    RDViewNodeType_Transforms,
    RDViewNodeType_Lighting,
    RDViewNodeType_OptionArray,
    RDViewNodeType_OptionBool,
    RDViewNodeType_OptionList,
    RDViewNodeType_OptionReal,
    RDViewNodeType_OptionString,
    RDViewNodeType_Background,
    RDViewNodeType_Color,
    RDViewNodeType_Opacity,
    RDViewNodeType_CameraAt,
    RDViewNodeType_CameraEye,
    RDViewNodeType_CameraFOV,
    RDViewNodeType_CameraUp,
    RDViewNodeType_Clipping,
    RDViewNodeType_Point,
    RDViewNodeType_PointSet,
    RDViewNodeType_Line,
    RDViewNodeType_LineSet,
    RDViewNodeType_Circle,
    RDViewNodeType_Fill,
    RDViewNodeType_Cone,
    RDViewNodeType_Cube,
    RDViewNodeType_Curve,
    RDViewNodeType_Cylinder,
    RDViewNodeType_Disk,
    RDViewNodeType_Hyperboloid,
    RDViewNodeType_Paraboloid,
    RDViewNodeType_Patch,
    RDViewNodeType_PolySet,
    RDViewNodeType_Sphere,
    RDViewNodeType_SqSphere,
    RDViewNodeType_SqTorus,
    RDViewNodeType_Torus,
    RDViewNodeType_Tube,
    RDViewNodeType_Subdivision,
    RDViewNodeType_ObjectInstance,
    RDViewNodeType_Matrix,
    RDViewNodeType_Rotate,
    RDViewNodeType_Scale,
    RDViewNodeType_Translate,
    RDViewNodeType_XformPush,
    RDViewNodeType_XformPop,
    RDViewNodeType_AmbientLight,
    RDViewNodeType_FarLight,
    RDViewNodeType_PointLight,
    RDViewNodeType_ConeLight,
    RDViewNodeType_Ka,
    RDViewNodeType_Kd,
    RDViewNodeType_Ks,
    RDViewNodeType_Specular,
    RDViewNodeType_Surface,
    RDViewNodeType_MapLoad,
    RDViewNodeType_Map,
    RDViewNodeType_MapSample,
    RDViewNodeType_MapBound,
    RDViewNodeType_MapBorder,
    RDViewNodeType_Primitive,
};

inline const char *
to_string(RDViewNodeType type)
{
    switch (type)
    {
        case RDViewNodeType_NodeInterface:      { return "RDViewNodeType_NodeInterface";     } break;
        case RDViewNodeType_Root:               { return "RDViewNodeType_Root";              } break;
        case RDViewNodeType_Body:               { return "RDViewNodeType_Body";              } break;
        case RDViewNodeType_Definitions:        { return "RDViewNodeType_Definitions";       } break;
        case RDViewNodeType_Include:            { return "RDViewNodeType_Include";           } break;
        case RDViewNodeType_Display:            { return "RDViewNodeType_Display";           } break;
        case RDViewNodeType_Format:             { return "RDViewNodeType_Format";            } break;
        case RDViewNodeType_Object:             { return "RDViewNodeType_Object";            } break;
        case RDViewNodeType_Frame:              { return "RDViewNodeType_Frame";             } break;
        case RDViewNodeType_World:              { return "RDViewNodeType_World";             } break;
        case RDViewNodeType_Camera:             { return "RDViewNodeType_Camera";            } break;
        case RDViewNodeType_Geometry:           { return "RDViewNodeType_Geometry";          } break;
        case RDViewNodeType_Transforms:         { return "RDViewNodeType_Transforms";        } break;
        case RDViewNodeType_Lighting:           { return "RDViewNodeType_Lighting";          } break;
        case RDViewNodeType_OptionArray:        { return "RDViewNodeType_OptionArray";       } break;
        case RDViewNodeType_OptionBool:         { return "RDViewNodeType_OptionBool";        } break;
        case RDViewNodeType_OptionList:         { return "RDViewNodeType_OptionList";        } break;
        case RDViewNodeType_OptionReal:         { return "RDViewNodeType_OptionReal";        } break;
        case RDViewNodeType_OptionString:       { return "RDViewNodeType_OptionString";      } break;
        case RDViewNodeType_Background:         { return "RDViewNodeType_Background";        } break;
        case RDViewNodeType_Color:              { return "RDViewNodeType_Color";             } break;
        case RDViewNodeType_Opacity:            { return "RDViewNodeType_Opacity";           } break;
        case RDViewNodeType_CameraAt:           { return "RDViewNodeType_CameraAt";          } break;
        case RDViewNodeType_CameraEye:          { return "RDViewNodeType_CameraEye";         } break;
        case RDViewNodeType_CameraFOV:          { return "RDViewNodeType_CameraFOV";         } break;
        case RDViewNodeType_CameraUp:           { return "RDViewNodeType_CameraUp";          } break;
        case RDViewNodeType_Clipping:           { return "RDViewNodeType_Clipping";          } break;
        case RDViewNodeType_Point:              { return "RDViewNodeType_Point";             } break;
        case RDViewNodeType_PointSet:           { return "RDViewNodeType_PointSet";          } break;
        case RDViewNodeType_Line:               { return "RDViewNodeType_Line";              } break;
        case RDViewNodeType_LineSet:            { return "RDViewNodeType_LineSet";           } break;
        case RDViewNodeType_Circle:             { return "RDViewNodeType_Circle";            } break;
        case RDViewNodeType_Fill:               { return "RDViewNodeType_Fill";              } break;
        case RDViewNodeType_Cone:               { return "RDViewNodeType_Cone";              } break;
        case RDViewNodeType_Cube:               { return "RDViewNodeType_Cube";              } break;
        case RDViewNodeType_Curve:              { return "RDViewNodeType_Curve";             } break;
        case RDViewNodeType_Cylinder:           { return "RDViewNodeType_Cylinder";          } break;
        case RDViewNodeType_Disk:               { return "RDViewNodeType_Disk";              } break;
        case RDViewNodeType_Hyperboloid:        { return "RDViewNodeType_Hyperboloid";       } break;
        case RDViewNodeType_Paraboloid:         { return "RDViewNodeType_Paraboloid";        } break;
        case RDViewNodeType_Patch:              { return "RDViewNodeType_Patch";             } break;
        case RDViewNodeType_PolySet:            { return "RDViewNodeType_PolySet";           } break;
        case RDViewNodeType_Sphere:             { return "RDViewNodeType_Sphere";            } break;
        case RDViewNodeType_SqSphere:           { return "RDViewNodeType_SqSphere";          } break;
        case RDViewNodeType_SqTorus:            { return "RDViewNodeType_SqTorus";           } break;
        case RDViewNodeType_Torus:              { return "RDViewNodeType_Torus";             } break;
        case RDViewNodeType_Tube:               { return "RDViewNodeType_Tube";              } break;
        case RDViewNodeType_Subdivision:        { return "RDViewNodeType_Subdivision";       } break;
        case RDViewNodeType_ObjectInstance:     { return "RDViewNodeType_ObjectInstance";    } break;
        case RDViewNodeType_Matrix:             { return "RDViewNodeType_Matrix";            } break;
        case RDViewNodeType_Rotate:             { return "RDViewNodeType_Rotate";            } break;
        case RDViewNodeType_Scale:              { return "RDViewNodeType_Scale";             } break;
        case RDViewNodeType_Translate:          { return "RDViewNodeType_Translate";         } break;
        case RDViewNodeType_XformPush:          { return "RDViewNodeType_XformPush";         } break;
        case RDViewNodeType_XformPop:           { return "RDViewNodeType_XformPop";          } break;
        case RDViewNodeType_AmbientLight:       { return "RDViewNodeType_AmbientLight";      } break;
        case RDViewNodeType_FarLight:           { return "RDViewNodeType_FarLight";          } break;
        case RDViewNodeType_PointLight:         { return "RDViewNodeType_PointLight";        } break;
        case RDViewNodeType_ConeLight:          { return "RDViewNodeType_ConeLight";         } break;
        case RDViewNodeType_Ka:                 { return "RDViewNodeType_Ka";                } break;
        case RDViewNodeType_Kd:                 { return "RDViewNodeType_Kd";                } break;
        case RDViewNodeType_Ks:                 { return "RDViewNodeType_Ks";                } break;
        case RDViewNodeType_Specular:           { return "RDViewNodeType_Specular";          } break;
        case RDViewNodeType_Surface:            { return "RDViewNodeType_Surface";           } break;
        case RDViewNodeType_MapLoad:            { return "RDViewNodeType_MapLoad";           } break;
        case RDViewNodeType_Map:                { return "RDViewNodeType_Map";               } break;
        case RDViewNodeType_MapSample:          { return "RDViewNodeType_MapSample";         } break;
        case RDViewNodeType_MapBound:           { return "RDViewNodeType_MapBound";          } break;
        case RDViewNodeType_MapBorder:          { return "RDViewNodeType_MapBorder";         } break;
        case RDViewNodeType_Primitive:          { return "RDViewNodeType_Primitive";         } break;
    }

    SIMPLEX_NO_REACH("");
    return "";
}

enum RDViewPrimitiveType
{
    RDViewPrimitiveType_Integer,
    RDViewPrimitiveType_Real,
    RDViewPrimitiveType_String,
    RDViewPrimitiveType_Boolean,
};

inline const char *
to_string(RDViewPrimitiveType type)
{
    switch (type)
    {
        case RDViewPrimitiveType_Integer:   { return "RDViewPrimitiveType_Integer";  } break;
        case RDViewPrimitiveType_Real:      { return "RDViewPrimitiveType_Real";     } break;
        case RDViewPrimitiveType_String:    { return "RDViewPrimitiveType_String";   } break;
        case RDViewPrimitiveType_Boolean:   { return "RDViewPrimitiveType_Boolean";  } break;
    }

    SIMPLEX_NO_REACH("");
    return "";
}

class RDViewNodeVisitor;
class RDViewNodeInterface
{
    public:
        inline          RDViewNodeInterface() { };
        virtual inline ~RDViewNodeInterface() { };

        virtual void    visit(RDViewNodeVisitor *visitor) = 0;

        inline RDViewNodeType get_node_type() const 
        { 
            SIMPLEX_ASSERT(this->node_type != RDViewNodeType_NodeInterface);
            return this->node_type; 
        }

    protected:
        RDViewNodeType node_type = RDViewNodeType_NodeInterface;

};

struct RDViewNodeRoot;
struct RDViewNodeBody;
struct RDViewNodeDefinitions;
struct RDViewNodeInclude;
struct RDViewNodeDisplay;
struct RDViewNodeFormat;
struct RDViewNodeObject;
struct RDViewNodeFrame;
struct RDViewNodeWorld;
struct RDViewNodeCamera;
struct RDViewNodeGeometry;
struct RDViewNodeTransforms;
struct RDViewNodeLighting;
struct RDViewNodeOptionArray;
struct RDViewNodeOptionBool;
struct RDViewNodeOptionList;
struct RDViewNodeOptionReal;
struct RDViewNodeOptionString;
struct RDViewNodeBackground;
struct RDViewNodeColor;
struct RDViewNodeOpacity;
struct RDViewNodeCameraAt;
struct RDViewNodeCameraEye;
struct RDViewNodeCameraFOV;
struct RDViewNodeCameraUp;
struct RDViewNodeClipping;
struct RDViewNodePoint;
struct RDViewNodePointSet;
struct RDViewNodeLine;
struct RDViewNodeLineSet;
struct RDViewNodeCircle;
struct RDViewNodeFill;
struct RDViewNodeCone;
struct RDViewNodeCube;
struct RDViewNodeCurve;
struct RDViewNodeCylinder;
struct RDViewNodeDisk;
struct RDViewNodeHyperboloid;
struct RDViewNodeParaboloid;
struct RDViewNodePatch;
struct RDViewNodePolySet;
struct RDViewNodeSphere;
struct RDViewNodeSqSphere;
struct RDViewNodeSqTorus;
struct RDViewNodeTorus;
struct RDViewNodeTube;
struct RDViewNodeSubdivision;
struct RDViewNodeObjectInstance;
struct RDViewNodeMatrix;
struct RDViewNodeRotate;
struct RDViewNodeScale;
struct RDViewNodeTranslate;
struct RDViewNodeXformPush;
struct RDViewNodeXformPop;
struct RDViewNodeAmbientLight;
struct RDViewNodeFarLight;
struct RDViewNodePointLight;
struct RDViewNodeConeLight;
struct RDViewNodeKa;
struct RDViewNodeKd;
struct RDViewNodeKs;
struct RDViewNodeSpecular;
struct RDViewNodeSurface;
struct RDViewNodeMapLoad;
struct RDViewNodeMap;
struct RDViewNodeMapSample;
struct RDViewNodeMapBound;
struct RDViewNodeMapBorder;
struct RDViewNodePrimitive;

class RDViewNodeVisitor
{
    public:
        virtual void accept(RDViewNodeInterface *node) { SIMPLEX_NO_REACH("Base node should not be traversed!"); }
        virtual void accept(RDViewNodeRoot *node) { };
        virtual void accept(RDViewNodeBody *node) { };
        virtual void accept(RDViewNodeDefinitions *node) { };
        virtual void accept(RDViewNodeInclude *node) { };
        virtual void accept(RDViewNodeDisplay *node) { };
        virtual void accept(RDViewNodeFormat *node) { };
        virtual void accept(RDViewNodeObject *node) { };
        virtual void accept(RDViewNodeFrame *node) { };
        virtual void accept(RDViewNodeWorld *node) { };
        virtual void accept(RDViewNodeCamera *node) { };
        virtual void accept(RDViewNodeGeometry *node) { };
        virtual void accept(RDViewNodeTransforms *node) { };
        virtual void accept(RDViewNodeLighting *node) { };
        virtual void accept(RDViewNodeOptionArray *node) { };
        virtual void accept(RDViewNodeOptionBool *node) { };
        virtual void accept(RDViewNodeOptionList *node) { };
        virtual void accept(RDViewNodeOptionReal *node) { };
        virtual void accept(RDViewNodeOptionString *node) { };
        virtual void accept(RDViewNodeBackground *node) { };
        virtual void accept(RDViewNodeColor *node) { };
        virtual void accept(RDViewNodeOpacity *node) { };
        virtual void accept(RDViewNodeCameraAt *node) { };
        virtual void accept(RDViewNodeCameraEye *node) { };
        virtual void accept(RDViewNodeCameraFOV *node) { };
        virtual void accept(RDViewNodeCameraUp *node) { };
        virtual void accept(RDViewNodeClipping *node) { };
        virtual void accept(RDViewNodePoint *node) { };
        virtual void accept(RDViewNodePointSet *node) { };
        virtual void accept(RDViewNodeLine *node) { };
        virtual void accept(RDViewNodeLineSet *node) { };
        virtual void accept(RDViewNodeCircle *node) { };
        virtual void accept(RDViewNodeFill *node) { };
        virtual void accept(RDViewNodeCone *node) { };
        virtual void accept(RDViewNodeCube *node) { };
        virtual void accept(RDViewNodeCurve *node) { };
        virtual void accept(RDViewNodeCylinder *node) { };
        virtual void accept(RDViewNodeDisk *node) { };
        virtual void accept(RDViewNodeHyperboloid *node) { };
        virtual void accept(RDViewNodeParaboloid *node) { };
        virtual void accept(RDViewNodePatch *node) { };
        virtual void accept(RDViewNodePolySet *node) { };
        virtual void accept(RDViewNodeSphere *node) { };
        virtual void accept(RDViewNodeSqSphere *node) { };
        virtual void accept(RDViewNodeSqTorus *node) { };
        virtual void accept(RDViewNodeTorus *node) { };
        virtual void accept(RDViewNodeTube *node) { };
        virtual void accept(RDViewNodeSubdivision *node) { };
        virtual void accept(RDViewNodeObjectInstance *node) { };
        virtual void accept(RDViewNodeMatrix *node) { };
        virtual void accept(RDViewNodeRotate *node) { };
        virtual void accept(RDViewNodeScale *node) { };
        virtual void accept(RDViewNodeTranslate *node) { };
        virtual void accept(RDViewNodeXformPush *node) { };
        virtual void accept(RDViewNodeXformPop *node) { };
        virtual void accept(RDViewNodeAmbientLight *node) { };
        virtual void accept(RDViewNodeFarLight *node) { };
        virtual void accept(RDViewNodePointLight *node) { };
        virtual void accept(RDViewNodeConeLight *node) { };
        virtual void accept(RDViewNodeKa *node) { };
        virtual void accept(RDViewNodeKd *node) { };
        virtual void accept(RDViewNodeKs *node) { };
        virtual void accept(RDViewNodeSpecular *node) { };
        virtual void accept(RDViewNodeSurface *node) { };
        virtual void accept(RDViewNodeMapLoad *node) { };
        virtual void accept(RDViewNodeMap *node) { };
        virtual void accept(RDViewNodeMapSample *node) { };
        virtual void accept(RDViewNodeMapBound *node) { };
        virtual void accept(RDViewNodeMapBorder *node) { };
        virtual void accept(RDViewNodePrimitive *node) { };
};

struct RDViewNodeRoot : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeRoot() { this->node_type = RDViewNodeType_Root; }
        inline virtual ~RDViewNodeRoot() { }
        inline void visit(RDViewNodeVisitor *visitor) { visitor->accept(this); }

        RDViewNodeInterface *display = NULL;
        RDViewNodeInterface *format = NULL;
        RDViewNodeInterface *body = NULL;
};

struct RDViewNodeBody : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeBody() { this->node_type = RDViewNodeType_Body; }
        inline virtual ~RDViewNodeBody() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        std::vector<RDViewNodeInterface*> children;
};

struct RDViewNodeDefinitions : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeDefinitions() { this->node_type = RDViewNodeType_Definitions; }
        inline virtual ~RDViewNodeDefinitions() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        std::vector<RDViewNodeInterface*> children;
};

struct RDViewNodeInclude : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeInclude() { this->node_type = RDViewNodeType_Include; }
        inline virtual ~RDViewNodeInclude() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        std::string input_path;
        std::filesystem::path canonical_path;
        RDViewNodeInterface *body;
};

enum RDViewDisplayType
{
    RDViewDisplayType_Screen,   // Screen via GUI.
    RDViewDisplayType_PNM,      // Output directly as PNM.
    RDViewDisplayType_BMP,      // Output directly as BMP.
    RDViewDisplayType_PNG,      // Output directly as PNG.
};

enum RDViewModeType
{
    RDViewModeType_RGB,         // Double buffer, regardless.
    RDViewModeType_RGBSingle,   // Single buffer, immediate draw.
    RDViewModeType_RGBObject,   // Double buffer, render only after object is drawn.
    RDViewModeType_RGBDouble,   // Double buffer, wait until render pass is complete.
};

struct RDViewNodeDisplay : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeDisplay() { this->node_type = RDViewNodeType_Display; }
        inline virtual ~RDViewNodeDisplay() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        std::string display_title;
        RDViewDisplayType display_type;
        RDViewModeType screen_type;
        
};

struct RDViewNodeFormat : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeFormat() { this->node_type = RDViewNodeType_Format; }
        inline virtual ~RDViewNodeFormat() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        int32_t width = 640;
        int32_t height = 480;

};

struct RDViewNodeObject : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeObject() { this->node_type = RDViewNodeType_Object; }
        inline virtual ~RDViewNodeObject() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeFrame : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeFrame() { this->node_type = RDViewNodeType_Frame; }
        inline virtual ~RDViewNodeFrame() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        int32_t frame_number;
        RDViewNodeInterface *world;
        std::vector<RDViewNodeInterface*> children;
};

struct RDViewNodeWorld : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeWorld() { this->node_type = RDViewNodeType_World; }
        inline virtual ~RDViewNodeWorld() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        std::vector<RDViewNodeInterface*> children;
};

struct RDViewNodeCamera : public RDViewNodeInterface
{ 
    public:
        inline RDViewNodeCamera() { this->node_type = RDViewNodeType_Camera; }
        inline virtual ~RDViewNodeCamera() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeGeometry : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeGeometry() { this->node_type = RDViewNodeType_Geometry; }
        inline virtual ~RDViewNodeGeometry() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeTransforms : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeTransforms() { this->node_type = RDViewNodeType_Transforms; }
        inline virtual ~RDViewNodeTransforms() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeLighting : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeLighting() { this->node_type = RDViewNodeType_Lighting; }
        inline virtual ~RDViewNodeLighting() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeOptionArray : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeOptionArray() { this->node_type = RDViewNodeType_OptionArray; }
        inline virtual ~RDViewNodeOptionArray() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        std::string name;
        std::vector<real32_t> values;
};

struct RDViewNodeOptionBool : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeOptionBool() { this->node_type = RDViewNodeType_OptionBool; }
        inline virtual ~RDViewNodeOptionBool() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        std::string name;
        int32_t value;
};

struct RDViewNodeOptionList : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeOptionList() { this->node_type = RDViewNodeType_OptionList; }
        inline virtual ~RDViewNodeOptionList() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        std::string name;
        std::vector<std::string> values;
};

struct RDViewNodeOptionReal : public RDViewNodeInterface 
{ 

    public:
        inline RDViewNodeOptionReal() { this->node_type = RDViewNodeType_OptionReal; }
        inline virtual ~RDViewNodeOptionReal() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        std::string name;
        real32_t value;

};

struct RDViewNodeOptionString : public RDViewNodeInterface 
{ 

    public:
        inline RDViewNodeOptionString() { this->node_type = RDViewNodeType_OptionString; }
        inline virtual ~RDViewNodeOptionString() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        std::string name;
        std::string value;

};

struct RDViewNodeBackground : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeBackground() { this->node_type = RDViewNodeType_Background; }
        inline virtual ~RDViewNodeBackground() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        real32_t red;
        real32_t green;
        real32_t blue;
};

struct RDViewNodeColor : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeColor() { this->node_type = RDViewNodeType_Color; }
        inline virtual ~RDViewNodeColor() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        real32_t red;
        real32_t green;
        real32_t blue;
};

struct RDViewNodeOpacity : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeOpacity() { this->node_type = RDViewNodeType_Opacity; }
        inline virtual ~RDViewNodeOpacity() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        real32_t opacity;
};

struct RDViewNodeCameraAt : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeCameraAt() { this->node_type = RDViewNodeType_CameraAt; }
        inline virtual ~RDViewNodeCameraAt() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        real32_t x;
        real32_t y;
        real32_t z;

};

struct RDViewNodeCameraEye : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeCameraEye() { this->node_type = RDViewNodeType_CameraEye; }
        inline virtual ~RDViewNodeCameraEye() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        real32_t x;
        real32_t y;
        real32_t z;
};

struct RDViewNodeCameraFOV : public RDViewNodeInterface
{
    public:
        inline RDViewNodeCameraFOV() { this->node_type = RDViewNodeType_CameraFOV; }
        inline virtual ~RDViewNodeCameraFOV() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        real32_t FOV;
};

struct RDViewNodeCameraUp : public RDViewNodeInterface
{
    public:
        inline RDViewNodeCameraUp() { this->node_type = RDViewNodeType_CameraUp; }
        inline virtual ~RDViewNodeCameraUp() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        real32_t x;
        real32_t y;
        real32_t z;
};

struct RDViewNodeClipping : public RDViewNodeInterface
{ 
    public:
        inline RDViewNodeClipping() { this->node_type = RDViewNodeType_Clipping; }
        inline virtual ~RDViewNodeClipping() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        real32_t near;
        real32_t far;
};

struct RDViewNodePoint : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodePoint() { this->node_type = RDViewNodeType_Point; }
        inline virtual ~RDViewNodePoint() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        real32_t x;
        real32_t y;
        real32_t z;
};

struct RDViewNodePointSet : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodePointSet() { this->node_type = RDViewNodeType_PointSet; }
        inline virtual ~RDViewNodePointSet() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        std::string format;
        size_t vertices;
        std::vector<real32_t> values;
};

struct RDViewNodeLine : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeLine() { this->node_type = RDViewNodeType_Line; }
        inline virtual ~RDViewNodeLine() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        real32_t x1;
        real32_t y1;
        real32_t z1;

        real32_t x2;
        real32_t y2;
        real32_t z2;
};

struct RDViewNodeLineSet : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeLineSet() { this->node_type = RDViewNodeType_LineSet; }
        inline virtual ~RDViewNodeLineSet() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        std::string format;
        size_t vertices;
        size_t indices;
        std::vector<real32_t> vertex_values;
        std::vector<int32_t> indices;
};

struct RDViewNodeCircle : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeCircle() { this->node_type = RDViewNodeType_Circle; }
        inline virtual ~RDViewNodeCircle() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        real32_t x;
        real32_t y;
        real32_t z;
        real32_t radius;
};

struct RDViewNodeFill : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeFill() { this->node_type = RDViewNodeType_Fill; }
        inline virtual ~RDViewNodeFill() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        real32_t x;
        real32_t y;
        real32_t z;
};

struct RDViewNodeCone : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeCone() { this->node_type = RDViewNodeType_Cone; }
        inline virtual ~RDViewNodeCone() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        real32_t height;
        real32_t radius;
        real32_t theta;
};

struct RDViewNodeCube : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeCube() { this->node_type = RDViewNodeType_Cube; }
        inline virtual ~RDViewNodeCube() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeCurve : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeCurve() { this->node_type = RDViewNodeType_Curve; }
        inline virtual ~RDViewNodeCurve() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeCylinder : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeCylinder() { this->node_type = RDViewNodeType_Cylinder; }
        inline virtual ~RDViewNodeCylinder() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        real32_t radius;
        real32_t z_min;
        real32_t z_max;
        real32_t theta;
};

struct RDViewNodeDisk : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeDisk() { this->node_type = RDViewNodeType_Disk; }
        inline virtual ~RDViewNodeDisk() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        real32_t height;
        real32_t radius;
        real32_t theta;
};

struct RDViewNodeHyperboloid : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeHyperboloid() { this->node_type = RDViewNodeType_Hyperboloid; }
        inline virtual ~RDViewNodeHyperboloid() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        real32_t x1;
        real32_t y1;
        real32_t z1;
        real32_t x2;
        real32_t y2;
        real32_t z2;
        real32_t theta;
};

struct RDViewNodeParaboloid : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeParaboloid() { this->node_type = RDViewNodeType_Paraboloid; }
        inline virtual ~RDViewNodeParaboloid() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        real32_t radius;
        real32_t z_min;
        real32_t z_max;
        real32_t theta;
};

enum RDViewPatchType
{
    RDViewPatchType_Bezier,
};

struct RDViewNodePatch : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodePatch() { this->node_type = RDViewNodeType_Patch; }
        inline virtual ~RDViewNodePatch() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        RDViewPatchType patch_type;
        std::string format;
        std::vector<real32_t> values;
};

struct RDViewNodePolySet : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodePolySet() { this->node_type = RDViewNodeType_PolySet; }
        inline virtual ~RDViewNodePolySet() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        std::string format;
        size_t vertices;
        size_t indices;
        std::vector<real32_t> vertex_values;
        std::vector<int32_t> index_values;
};

struct RDViewNodeSphere : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeSphere() { this->node_type = RDViewNodeType_Sphere; }
        inline virtual ~RDViewNodeSphere() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        real32_t radius;
        real32_t z_min;
        real32_t z_max;
        real32_t theta;
};

struct RDViewNodeSqSphere : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeSqSphere() { this->node_type = RDViewNodeType_SqSphere; }
        inline virtual ~RDViewNodeSqSphere() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        real32_t radius;
        real32_t n;
        real32_t e;
        real32_t z_min;
        real32_t z_max;
        real32_t theta;
};

struct RDViewNodeSqTorus : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeSqTorus() { this->node_type = RDViewNodeType_SqTorus; }
        inline virtual ~RDViewNodeSqTorus() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        real32_t radius_a;
        real32_t radius_b;
        real32_t n;
        real32_t e;
        real32_t phi_min;
        real32_t phi_max;
        real32_t theta_max;
};

struct RDViewNodeTorus : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeTorus() { this->node_type = RDViewNodeType_Torus; }
        inline virtual ~RDViewNodeTorus() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        real32_t radius_a;
        real32_t radius_b;
        real32_t phi_min;
        real32_t phi_max;
        real32_t theta_max;
};

struct RDViewNodeTube : public RDViewNodeInterface
{
    public:
        inline RDViewNodeTube() { this->node_type = RDViewNodeType_Tube; }
        inline virtual ~RDViewNodeTube() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        real32_t x1;
        real32_t y1;
        real32_t z1;
        real32_t x2;
        real32_t y2;
        real32_t z2;
        real32_t radius;
};

struct RDViewNodeSubdivision : public RDViewNodeInterface
{
    public:
        inline RDViewNodeSubdivision() { this->node_type = RDViewNodeType_Subdivision; }
        inline virtual ~RDViewNodeSubdivision() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeObjectInstance : public RDViewNodeInterface
{ 
    public:
        inline RDViewNodeObjectInstance() { this->node_type = RDViewNodeType_ObjectInstance; }
        inline virtual ~RDViewNodeObjectInstance() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        std::string name;
        std::vector<real32_t> parameters;
};

struct RDViewNodeMatrix : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeMatrix() { this->node_type = RDViewNodeType_Matrix; }
        inline virtual ~RDViewNodeMatrix() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        std::vector<real32_t> values;
};

enum RDViewRotationAxis
{
    RDViewRotationAxis_X,
    RDViewRotationAxis_Y,
    RDViewRotationAxis_Z,
};

struct RDViewNodeRotate : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeRotate() { this->node_type = RDViewNodeType_Rotate; }
        inline virtual ~RDViewNodeRotate() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        RDViewRotationAxis axis;
        real32_t angle;
        
};

struct RDViewNodeScale : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeScale() { this->node_type = RDViewNodeType_Scale; }
        inline virtual ~RDViewNodeScale() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        real32_t x;
        real32_t y;
        real32_t z;
};

struct RDViewNodeTranslate : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeTranslate() { this->node_type = RDViewNodeType_Translate; }
        inline virtual ~RDViewNodeTranslate() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        real32_t x;
        real32_t y;
        real32_t z;
};

struct RDViewNodeXformPush : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeXformPush() { this->node_type = RDViewNodeType_XformPush; }
        inline virtual ~RDViewNodeXformPush() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeXformPop : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeXformPop() { this->node_type = RDViewNodeType_XformPop; }
        inline virtual ~RDViewNodeXformPop() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeAmbientLight : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeAmbientLight() { this->node_type = RDViewNodeType_AmbientLight; }
        inline virtual ~RDViewNodeAmbientLight() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        real32_t r;
        real32_t g;
        real32_t b;
        real32_t intensity;
};

struct RDViewNodeFarLight : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeFarLight() { this->node_type = RDViewNodeType_FarLight; }
        inline virtual ~RDViewNodeFarLight() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
        
        real32_t l_x;
        real32_t l_y;
        real32_t l_z;
        real32_t r;
        real32_t g;
        real32_t b;
        real32_t intensity;
};

struct RDViewNodePointLight : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodePointLight() { this->node_type = RDViewNodeType_PointLight; }
        inline virtual ~RDViewNodePointLight() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        real32_t p_x;
        real32_t p_y;
        real32_t p_z;
        real32_t r;
        real32_t g;
        real32_t b;
        real32_t intensity;
};

struct RDViewNodeConeLight : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeConeLight() { this->node_type = RDViewNodeType_ConeLight; }
        inline virtual ~RDViewNodeConeLight() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        real32_t p_x;
        real32_t p_y;
        real32_t p_z;
        real32_t a_x;
        real32_t a_y;
        real32_t a_z;
        real32_t theta_min;
        real32_t theta_max;
        real32_t r;
        real32_t g;
        real32_t b;
        real32_t intensity;
};

struct RDViewNodeKa : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeKa() { this->node_type = RDViewNodeType_Ka; }
        inline virtual ~RDViewNodeKa() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        real32_t value;
};

struct RDViewNodeKd : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeKd() { this->node_type = RDViewNodeType_Kd; }
        inline virtual ~RDViewNodeKd() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        real32_t value;
};

struct RDViewNodeKs : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeKs() { this->node_type = RDViewNodeType_Ks; }
        inline virtual ~RDViewNodeKs() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        real32_t value;
};

struct RDViewNodeSpecular : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeSpecular() { this->node_type = RDViewNodeType_Specular; }
        inline virtual ~RDViewNodeSpecular() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        real32_t r;
        real32_t g;
        real32_t b;
        real32_t n;
};

enum RDViewShaderType
{
    RDViewShaderType_Matte,
    RDViewShaderType_Metal,
    RDViewShaderType_Plastic,
    RDViewShaderType_PaintedPlastic,
};

struct RDViewNodeSurface : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeSurface() { this->node_type = RDViewNodeType_Surface; }
        inline virtual ~RDViewNodeSurface() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        RDViewShaderType shader_type;
};

enum RDViewMapType
{
    RDViewMapType_None,
    RDViewMapType_TextureMap,
    RDViewMapType_BumpMap,
};

struct RDViewNodeMapLoad : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeMapLoad() { this->node_type = RDViewNodeType_MapLoad; }
        inline virtual ~RDViewNodeMapLoad() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        std::string input_path;
        std::filesystem::path canonical_path;
        std::string label;
};

struct RDViewNodeMap : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeMap() { this->node_type = RDViewNodeType_Map; }
        inline virtual ~RDViewNodeMap() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        RDViewMapType map_type;
        std::string label;
        
};

enum RDViewMapLevelType
{
    RDViewMapLevelType_Nearest,
    RDViewMapLevelType_Linear,
};

struct RDViewNodeMapSample : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeMapSample() { this->node_type = RDViewNodeType_MapSample; }
        inline virtual ~RDViewNodeMapSample() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        RDViewMapType map_type;
        RDViewMapLevelType intra_level_type;
        RDViewMapLevelType inter_level_type;
};

struct RDViewNodeMapBound : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeMapBound() { this->node_type = RDViewNodeType_MapBound; }
        inline virtual ~RDViewNodeMapBound() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        RDViewMapType map_type;
        real32_t s_min;
        real32_t t_min;
        real32_t s_max;
        real32_t t_max;
};

enum RDViewBorderType
{
    RDViewMapBorderType_None,
    RDViewMapBorderType_Clamp,
    RDViewMapBorderType_Repeat,
};

struct RDViewNodeMapBorder : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeMapBorder() { this->node_type = RDViewNodeType_MapBorder; }
        inline virtual ~RDViewNodeMapBorder() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        RDViewMapType map_type;
        RDViewBorderType horizontal_border_type;
        RDViewBorderType vertical_border_type;
};

struct RDViewNodePrimitive : public RDViewNodeInterface
{
    public:
        inline RDViewNodePrimitive() { this->node_type = RDViewNodeType_Primitive; }
        inline ~RDViewNodePrimitive() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }

        RDViewPrimitiveType primitive_type;
        std::variant<int64_t, real64_t, bool, std::string> primitive_value;
};

// NOTE(Chris): Base class exception for parsing errors. All deriving exceptions specify how
//              the parser should synchronize after catching an exception.
class RDViewParserError : public std::exception 
{ 
    protected:
        inline void set_message(const std::string &message) { this->message = message; }
        inline void set_message(std::string &&message) { this->message = message; }

        inline virtual const char *what() const noexcept override
        {
            return this->message.c_str();
        }

    private:
        std::string message;
};

// Unexpected Token -   Unexpected tokens are tokens that aren't in the grammar. Symbol characters,
//                      for example the '@' symbol are in this category.
class RDViewParserErrorUT : public RDViewParserError
{
    public:
        inline RDViewParserErrorUT(RDViewToken token)
        {

            std::string location = token.source_file_path.string();
            std::string contents(token.source_file_contents.substr(token.offset, token.length));

            std::stringstream message_stream;
            message_stream  << location << "(" << token.line << ", " << token.column 
                            << "): " << "Unexpected token '" << contents << "' encountered.";
            this->set_message(message_stream.str());

        }

};

// Unexpected Command - Unexpected commands are commands that are defined in areas of the file
//                      where they aren't defined. For example, issuing a "Cube" command outside
//                      a world block or object block is considered an unexpected command.
class RDViewParserErrorUC : public RDViewParserError
{
    public:
        inline RDViewParserErrorUC(RDViewToken token, std::string location)
        {
            std::string location = token.source_file_path.string();
            std::string contents(token.source_file_contents.substr(token.offset, token.length));

            std::stringstream message_stream;
            message_stream  << location << "(" << token.line << ", " << token.column 
                            << "): Unexpected command encountered in " << location << ".";
            this->set_message(message_stream.str());
        }
};

// Invalid Command Format - Invalid command formats arise when a command is defined, but the
//                          input parameters aren't correct. Something like encountering a string
//                          where a real is expected triggers this error.
class RDViewParserErrorICF : public RDViewParserError
{
    public:
        inline RDViewParserErrorICF(RDViewToken token, std::string expected_format)
        {

            std::string location = token.source_file_path.string();
            std::string contents(token.source_file_contents.substr(token.offset, token.length));

            std::stringstream message_stream;
            message_stream  << location << "(" << token.line << ", " << token.column 
                            << "): Incorrect command formatting, " << expected_format << ".";
            this->set_message(message_stream.str());

        }
};

// Invalid Named Reference -    Invalid named references are for when the user attempts to reference
//                              something like an object instance by name that hasn't yet been defined.
class RDViewParserErrorINR : public RDViewParserError
{
    public:
        inline RDViewParserErrorINR(RDViewToken token, std::string name)
        {

            std::string location = token.source_file_path.string();
            std::string contents(token.source_file_contents.substr(token.offset, token.length));

            std::stringstream message_stream;
            message_stream  << location << "(" << token.line << ", " << token.column 
                            << "): Unexpected named reference to: '" << name << "'.";
            this->set_message(message_stream.str());

        }
};

class RDViewParser
{
    public:
        RDViewParser();
        ~RDViewParser();

        inline RDViewNodeInterface* get_root() const { return this->root; }

    private:
        void synchronize_to(RDViewTokenType token_type);
        void synchronize_up_to(RDViewTokenType token_type);

        bool is_previous_token(RDViewTokenType token_type) const;
        bool is_current_token(RDViewTokenType token_type) const;
        bool is_next_token(RDViewTokenType token_type) const;

        RDViewNodeInterface* match_root();
        RDViewNodeInterface* match_body();
        RDViewNodeInterface* match_definitions();

        RDViewNodeInterface* match_include();
        RDViewNodeInterface* match_display();
        RDViewNodeInterface* match_format();
        RDViewNodeInterface* match_object();
        RDViewNodeInterface* match_frame();
        RDViewNodeInterface* match_world();
        RDViewNodeInterface* match_camera();
        RDViewNodeInterface* match_geometry();
        RDViewNodeInterface* match_transforms();
        RDViewNodeInterface* match_lighting();

        RDViewNodeInterface* match_option_array();
        RDViewNodeInterface* match_option_bool();
        RDViewNodeInterface* match_option_list();
        RDViewNodeInterface* match_option_real();
        RDViewNodeInterface* match_option_string();
        RDViewNodeInterface* match_background();
        RDViewNodeInterface* match_color();
        RDViewNodeInterface* match_opacity();
        RDViewNodeInterface* match_camera_at();
        RDViewNodeInterface* match_camera_eye();
        RDViewNodeInterface* match_camera_fov();
        RDViewNodeInterface* match_camera_up();
        RDViewNodeInterface* match_clipping();
        RDViewNodeInterface* match_point();
        RDViewNodeInterface* match_point_set();
        RDViewNodeInterface* match_line();
        RDViewNodeInterface* match_line_set();
        RDViewNodeInterface* match_circle();
        RDViewNodeInterface* match_fill();
        RDViewNodeInterface* match_cone();
        RDViewNodeInterface* match_cube();
        RDViewNodeInterface* match_curve();
        RDViewNodeInterface* match_cylinder();
        RDViewNodeInterface* match_disk();
        RDViewNodeInterface* match_hyperboloid();
        RDViewNodeInterface* match_paraboloid();
        RDViewNodeInterface* match_patch();
        RDViewNodeInterface* match_poly_set();
        RDViewNodeInterface* match_sphere();
        RDViewNodeInterface* match_sq_sphere();
        RDViewNodeInterface* match_sq_torus();
        RDViewNodeInterface* match_torus();
        RDViewNodeInterface* match_tube();
        RDViewNodeInterface* match_subdivision();
        RDViewNodeInterface* match_object_instance();
        RDViewNodeInterface* match_matrix();
        RDViewNodeInterface* match_rotate();
        RDViewNodeInterface* match_scale();
        RDViewNodeInterface* match_translate();
        RDViewNodeInterface* match_xformpush();
        RDViewNodeInterface* match_xformpop();
        RDViewNodeInterface* match_ambient_light();
        RDViewNodeInterface* match_far_light();
        RDViewNodeInterface* match_point_light();
        RDViewNodeInterface* match_cone_light();
        RDViewNodeInterface* match_ka();
        RDViewNodeInterface* match_kd();
        RDViewNodeInterface* match_ks();
        RDViewNodeInterface* match_specular();
        RDViewNodeInterface* match_surface();
        RDViewNodeInterface* match_map_load();
        RDViewNodeInterface* match_map();
        RDViewNodeInterface* match_map_sample();
        RDViewNodeInterface* match_map_bound();
        RDViewNodeInterface* match_map_border();
        RDViewNodeInterface* match_primitive();

    private:
        template <typename T, typename... Args> inline T* create_node(Args ...args)
        {

            static_assert(std::is_base_of<RDViewNodeInterface, T>::value);
            void *memory_buffer = malloc(sizeof(T));
            T *node = new (memory_buffer) T*(args...);
            this->nodes.push_back(node);
            return node;

        }

        inline void destroy_node(RDViewNodeInterface *node)
        {
            node->~RDViewNodeInterface();
            free(node);
        }

    private:
        RDViewNodeInterface* root;
        std::vector<RDViewNodeInterface*> nodes;
        std::stack<RDViewTokenizer> tokenizer_stack;
        RDViewTokenizer *tokenizer;

};