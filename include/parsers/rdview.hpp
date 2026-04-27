// -------------------------------------------------------------------------------------------------
// RDView Parser
//      Christopher DeJong / magictrick-dev 
//      April 2026
// -------------------------------------------------------------------------------------------------
// 
// RDView Grammar
// - Reference Notes
//          The following grammar roughly describes how the RDView scripting language works.
//          For the sake of my sanity, I left out some commands that I don't think I will be
//          adding to the engine. Additionally, the grammar tries to closely replicate the original
//          formatting requirements, but there will be some small deviations that would otherwise
//          not be present in the original graphics renderer.
//
//          Additionally, the context free grammar syntax is a bit loose, you should be able to
//          largely follow the intent if you get the basics of CFGs and regular expressions.
//
//          If you see any productions that have a ".[]" with some numerical constant inside,
//          that's how many you should expect of that, likewise if you see ".[n]", it is parameter
//          or property dependent (though it is fixed number). Something like a PolySet, for example,
//          defines vertices and faces, and the number of elements of the vertex is defined by the string.
//          The following values after that is determined by that information.
//
// - Structural 
//          ROOT                    :   DISPLAY FORMAT (SINGLEFRAME_BODY | MULTIFRAME_BODY)
//          SINGLEFRAME_BODY        :   (INCLUDE | PROPERTIES | DEFINITIONS)* WORLD EOF
//          MULTIFRAME_BODY         :   (INCLUDE | PROPERTIES | DEFINITIONS)* (FRAME MULTIFRAMEBODY | EOF)
//          DEFINITIONS             :   OBJECT | OPTION_ARRAY | OPTION_BOOL | OPTION_LIST | 
//                                      OPTION_REAL | OPTION_STRING
// 
//          INCLUDE                 :   string
//          DISPLAY                 :   "Display" string string string
//          FORMAT                  :   "Format" integer integer
//          OBJECT                  :   "ObjectBegin" integer? string WORLD_COMMANDS* "ObjectEnd"
//
//          FRAME                   :   "FrameBegin" integer FRAME_COMMANDS* WORLD "FrameEnd"
//          WORLD                   :   "WorldBegin" WORLD_COMMANDS* "WorldEnd"
//
// - Properties
//          PROPERTIES              :   FRAME_COMMANDS
//          FRAME_COMMANDS          :   BACKGROUND | COLOR | OPACITY | CAMERA | LIGHTING | 
//                                      SURFACE_ATTRIBUTES | MAP_LOAD | ATTRIBUTE_MAPPING | MAP
//          WORLD_COMMANDS          :   OPACITY | COLOR | GEOMETRY | TRANSFORMS | LIGHTING | 
//                                      SURFACE_ATTRIBUTES | ATTRIBUTE_MAPPING
//          CAMERA                  :   CAMERA_AT | CAMERA_EYE | CAMERA_FOV | CAMERA_UP | CLIPPING
//          GEOMETRY                :   POINT | POINT_SET | LINE | LINE_SET | CIRCLE | FILL | CONE |
//                                      CUBE | CURVE | CYLINDER | DISK | HYPERBOLOID | PARABOLOID |
//                                      PATCH | POLY_SET | SPHERE | SQ_SPHERE | SQ_TORUS |
//                                      TORUS | TUBE | OBJECT_INSTANCE
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
//          SQ_SPHERE               :   "SqSphere" numerical integer numerical.[3]
//          SQ_TORUS                :   "SqTorus" numerical.[2] integer numerical.[4]
//          TORUS                   :   "Torus" numerical.[5]
//          TUBE                    :   "Tube" numerical.[7]
//          OBJECT_INSTANCE         :   "ObjectInstance" string numerical.[n]
//          MATRIX                  :   "Matrix" numerical.[16]
//          ROTATE                  :   "Rotate" numerical.[2]
//          SCALE                   :   "Scale" numerical.[3]
//          TRANSLATE               :   "Translate" numerical.[3]
//          XFORMPUSH               :   "XformPush"
//          XFORMPOP                :   "XformPop"
//          AMBIENT_LIGHT           :   "AmbientLight" numerical.[4]
//          FAR_LIGHT               :   "FarLight" numerical.[7]
//          POINT_LIGHT             :   "PointLight" numerical.[9]
//          CONE_LIGHT              :   "ConeLight" numerical.[12]
//          KA                      :   "Ka" real
//          KD                      :   "Kd" real
//          KS                      :   "Ks" real
//          SPECULAR                :   "Specular" real.[4]
//          SURFACE                 :   "Surface" string
//          MAP_LOAD                :   "MapLoad" string string
//          MAP                     :   "Map" string string
//          MAP_SAMPLE              :   "MapSample" string string string
//          MAP_BOUND               :   "MapBound" string real real real
//          MAP_BORDER              :   "MapBorder" string string string
//
// -------------------------------------------------------------------------------------------------
#pragma once
#include <utils/defs.hpp>
#include <filesystem>
#include <string>
#include <iostream>
#include <unordered_map>

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
    RDViewNodeType_SingleframeBody,
    RDViewNodeType_MultiframeBody,
    RDViewNodeType_Definitions,
    RDViewNodeType_Include,
    RDViewNodeType_Display,
    RDViewNodeType_Format,
    RDViewNodeType_Object,
    RDViewNodeType_Frame,
    RDViewNodeType_World,
    RDViewNodeType_Properties,
    RDViewNodeType_FrameCommands,
    RDViewNodeType_WorldCommands,
    RDViewNodeType_Camera,
    RDViewNodeType_Geometry,
    RDViewNodeType_Transforms,
    RDViewNodeType_Lighting,
    RDViewNodeType_SurfaceAttributes,
    RDViewNodeType_AttributeMapping,
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
};

class RDViewNodeVisitor;
class RDViewNodeInterface
{
    public:
        inline          RDViewNodeInterface() { };
        virtual inline ~RDViewNodeInterface() { };

        virtual void    accept(RDViewNodeVisitor *visitor) = 0;

        inline RDViewNodeType get_node_type() const 
        { 
            SIMPLEX_ASSERT(this->node_type != RDViewNodeType_NodeInterface);
            return this->node_type; 
        }

    protected:
        RDViewNodeType node_type = RDViewNodeType_NodeInterface;

};

class RDViewParser
{
    public:
        RDViewParser();
        ~RDViewParser();

    private:
        std::stack<RDViewTokenizer> tokenizer_stack;
        RDViewTokenizer *tokenizer;
};