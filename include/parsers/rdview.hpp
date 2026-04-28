// -------------------------------------------------------------------------------------------------
// RDView Parser
//      Christopher DeJong / magictrick-dev 
//      April 2026
// -------------------------------------------------------------------------------------------------
//
// RDView Lexer
// - Implementation Notes
//          The lexer is a tokenizer design that uses a DFA to classify tokens. The implementation
//          expects that the tokenizer's source file to remain in memory because it uses a string_view
//          to the source. Tokens, by extension, also require this property and for a token to be valid
//          for use, the source text must be in memory.
//
//          This effectively means you MUST NOT cache tokens. In short, the parser will immediately
//          discord tokens as it consumes them. The parser itself does not rely on the text files
//          being in memory after it successfully generates the AST.
//
//          TODO(Chris):    I intend to decouple tokens from the source and store their respective
//                          values internal using std::variant, meaning tokens CAN be cached for use
//                          on the GUI-side. The plan is to eventually use the tokenizer for syntax
//                          highlighting later down the line (since its faster than using the AST).
//
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
//          MULTIFRAME_BODY         :   (INCLUDE | PROPERTIES | DEFINITIONS)* (FRAME MULTIFRAME_BODY | EOF)
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
//          PROPERTIES              :   FRAME_COMMANDS
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
//
// RDViewParser
// - Implementation Notes
//          The RDViewParser uses a standard recursive descent parser. Since the RDView language
//          specification allows for file imports (basically just a copy-and-paste) but doesn't
//          verify circular dependencies, I decided that I will do basic circular dependency checks
//          since it isn't a hard process to do.
//
//          RDView files either contain a single world block or multiple frame + world blocks in series.
//          Certain commands are only valid outside the frame block, outside the world block, or only
//          in the world block. The root source file *must* define the display and format attributes
//          in order *first* before any other commands. Object definitions must be tracked and unique,
//          so much like a language that uses a symbol-table to track variable names and scope, we
//          need to ensure that objects are also properly defined before they are used.
//
//              - Root-only Commands
//              - Frame-only Commands
//              - World-only Commands
//              - World and Frame Commands
//              - Root, World, and Frame Commands
//
//          Regarding includes, we make some basic assumptions that they are root-level only includes;
//          meaning it's just a convienent way to separate more complex frame-sequences across multiple
//          files. The include process simply lexically analyzes it, and treats it as if you're just
//          adding more nodes in that spot.
//
//          Error handling in this parser is designed to be expressive enough that it will help a
//          developer spot and find errors. In this rare case, exception handling is clean enough
//          for this problem. If any error is present, the AST's final generation is considered
//          invalid and traversal is considered undefined.
//
//          The visitor interface allows a developer to construct a custom tree-walk interpreter
//          if required. Interfacing with an engine simply requires crafting a tree-walk visitor
//          that invokes the engine interfaces that correspond to the commands in RDView. As you
//          might imagine, certain commands might not exist in the engine interface, so a new visitor
//          must be made for each engine to connects to. In cases where these don't exist, these just
//          map to no-ops. A print visitor is defined for debugging and testing. A pretty-printer
//          also exists to regurgitate a similar source file for comparison; useful to ensure that
//          the grammar interpretation reflects developer intent.
//
// -------------------------------------------------------------------------------------------------
#pragma once
#include <utils/defs.hpp>
#include <filesystem>
#include <string>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <stack>
#include <algorithm>

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
    RDViewNodeType_ObjectCommands,
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

enum RDViewPrimitiveType
{
    RDViewPrimitiveType_Integer,
    RDViewPrimitiveType_Real,
    RDViewPrimitiveType_String,
    RDViewPrimitiveType_Boolean,
};

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
struct RDViewNodeSingleframeBody;
struct RDViewNodeMultiframeBody;
struct RDViewNodeDefinitions;
struct RDViewNodeInclude;
struct RDViewNodeDisplay;
struct RDViewNodeFormat;
struct RDViewNodeObject;
struct RDViewNodeFrame;
struct RDViewNodeWorld;
struct RDViewNodeProperties;
struct RDViewNodeFrameCommands;
struct RDViewNodeWorldCommands;
struct RDViewNodeObjectCommands;
struct RDViewNodeCamera;
struct RDViewNodeGeometry;
struct RDViewNodeTransforms;
struct RDViewNodeLighting;
struct RDViewNodeSurfaceAttributes;
struct RDViewNodeAttributeMapping;
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
        virtual void accept(RDViewNodeSingleframeBody *node) { };
        virtual void accept(RDViewNodeMultiframeBody *node) { };
        virtual void accept(RDViewNodeDefinitions *node) { };
        virtual void accept(RDViewNodeInclude *node) { };
        virtual void accept(RDViewNodeDisplay *node) { };
        virtual void accept(RDViewNodeFormat *node) { };
        virtual void accept(RDViewNodeObject *node) { };
        virtual void accept(RDViewNodeFrame *node) { };
        virtual void accept(RDViewNodeWorld *node) { };
        virtual void accept(RDViewNodeProperties *node) { };
        virtual void accept(RDViewNodeFrameCommands *node) { };
        virtual void accept(RDViewNodeWorldCommands *node) { };
        virtual void accept(RDViewNodeObjectCommands *node) { };
        virtual void accept(RDViewNodeCamera *node) { };
        virtual void accept(RDViewNodeGeometry *node) { };
        virtual void accept(RDViewNodeTransforms *node) { };
        virtual void accept(RDViewNodeLighting *node) { };
        virtual void accept(RDViewNodeSurfaceAttributes *node) { };
        virtual void accept(RDViewNodeAttributeMapping *node) { };
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
};

struct RDViewNodeSingleframeBody : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeSingleframeBody() { this->node_type = RDViewNodeType_SingleframeBody; }
        inline virtual ~RDViewNodeSingleframeBody() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeMultiframeBody : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeMultiframeBody() { this->node_type = RDViewNodeType_MultiframeBody; }
        inline virtual ~RDViewNodeMultiframeBody() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeDefinitions : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeDefinitions() { this->node_type = RDViewNodeType_Definitions; }
        inline virtual ~RDViewNodeDefinitions() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeInclude : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeInclude() { this->node_type = RDViewNodeType_Include; }
        inline virtual ~RDViewNodeInclude() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeDisplay : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeDisplay() { this->node_type = RDViewNodeType_Display; }
        inline virtual ~RDViewNodeDisplay() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeFormat : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeFormat() { this->node_type = RDViewNodeType_Format; }
        inline virtual ~RDViewNodeFormat() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
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
};

struct RDViewNodeWorld : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeWorld() { this->node_type = RDViewNodeType_World; }
        inline virtual ~RDViewNodeWorld() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeProperties : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeProperties() { this->node_type = RDViewNodeType_Properties; }
        inline virtual ~RDViewNodeProperties() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeFrameCommands : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeFrameCommands() { this->node_type = RDViewNodeType_FrameCommands; }
        inline virtual ~RDViewNodeFrameCommands() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeWorldCommands : public RDViewNodeInterface
{
    public:
        inline RDViewNodeWorldCommands() { this->node_type = RDViewNodeType_WorldCommands; }
        inline virtual ~RDViewNodeWorldCommands() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeObjectCommands : public RDViewNodeInterface
{
    public:
        inline RDViewNodeObjectCommands() { this->node_type = RDViewNodeType_ObjectCommands; }
        inline virtual ~RDViewNodeObjectCommands() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
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

struct RDViewNodeSurfaceAttributes : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeSurfaceAttributes() { this->node_type = RDViewNodeType_SurfaceAttributes; }
        inline virtual ~RDViewNodeSurfaceAttributes() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeAttributeMapping : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeAttributeMapping() { this->node_type = RDViewNodeType_AttributeMapping; }
        inline virtual ~RDViewNodeAttributeMapping() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeOptionArray : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeOptionArray() { this->node_type = RDViewNodeType_OptionArray; }
        inline virtual ~RDViewNodeOptionArray() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeOptionBool : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeOptionBool() { this->node_type = RDViewNodeType_OptionBool; }
        inline virtual ~RDViewNodeOptionBool() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeOptionList : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeOptionList() { this->node_type = RDViewNodeType_OptionList; }
        inline virtual ~RDViewNodeOptionList() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeOptionReal : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeOptionReal() { this->node_type = RDViewNodeType_OptionReal; }
        inline virtual ~RDViewNodeOptionReal() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeOptionString : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeOptionString() { this->node_type = RDViewNodeType_OptionString; }
        inline virtual ~RDViewNodeOptionString() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeBackground : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeBackground() { this->node_type = RDViewNodeType_Background; }
        inline virtual ~RDViewNodeBackground() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeColor : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeColor() { this->node_type = RDViewNodeType_Color; }
        inline virtual ~RDViewNodeColor() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeOpacity : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeOpacity() { this->node_type = RDViewNodeType_Opacity; }
        inline virtual ~RDViewNodeOpacity() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeCameraAt : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeCameraAt() { this->node_type = RDViewNodeType_CameraAt; }
        inline virtual ~RDViewNodeCameraAt() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeCameraEye : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeCameraEye() { this->node_type = RDViewNodeType_CameraEye; }
        inline virtual ~RDViewNodeCameraEye() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeCameraFOV : public RDViewNodeInterface
{
    public:
        inline RDViewNodeCameraFOV() { this->node_type = RDViewNodeType_CameraFOV; }
        inline virtual ~RDViewNodeCameraFOV() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeCameraUp : public RDViewNodeInterface
{
    public:
        inline RDViewNodeCameraUp() { this->node_type = RDViewNodeType_CameraUp; }
        inline virtual ~RDViewNodeCameraUp() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeClipping : public RDViewNodeInterface
{ 
    public:
        inline RDViewNodeClipping() { this->node_type = RDViewNodeType_Clipping; }
        inline virtual ~RDViewNodeClipping() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodePoint : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodePoint() { this->node_type = RDViewNodeType_Point; }
        inline virtual ~RDViewNodePoint() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodePointSet : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodePointSet() { this->node_type = RDViewNodeType_PointSet; }
        inline virtual ~RDViewNodePointSet() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeLine : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeLine() { this->node_type = RDViewNodeType_Line; }
        inline virtual ~RDViewNodeLine() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeLineSet : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeLineSet() { this->node_type = RDViewNodeType_LineSet; }
        inline virtual ~RDViewNodeLineSet() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeCircle : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeCircle() { this->node_type = RDViewNodeType_Circle; }
        inline virtual ~RDViewNodeCircle() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeFill : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeFill() { this->node_type = RDViewNodeType_Fill; }
        inline virtual ~RDViewNodeFill() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeCone : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeCone() { this->node_type = RDViewNodeType_Cone; }
        inline virtual ~RDViewNodeCone() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
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
};

struct RDViewNodeDisk : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeDisk() { this->node_type = RDViewNodeType_Disk; }
        inline virtual ~RDViewNodeDisk() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeHyperboloid : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeHyperboloid() { this->node_type = RDViewNodeType_Hyperboloid; }
        inline virtual ~RDViewNodeHyperboloid() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeParaboloid : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeParaboloid() { this->node_type = RDViewNodeType_Paraboloid; }
        inline virtual ~RDViewNodeParaboloid() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodePatch : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodePatch() { this->node_type = RDViewNodeType_Patch; }
        inline virtual ~RDViewNodePatch() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodePolySet : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodePolySet() { this->node_type = RDViewNodeType_PolySet; }
        inline virtual ~RDViewNodePolySet() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeSphere : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeSphere() { this->node_type = RDViewNodeType_Sphere; }
        inline virtual ~RDViewNodeSphere() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeSqSphere : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeSqSphere() { this->node_type = RDViewNodeType_SqSphere; }
        inline virtual ~RDViewNodeSqSphere() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeSqTorus : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeSqTorus() { this->node_type = RDViewNodeType_SqTorus; }
        inline virtual ~RDViewNodeSqTorus() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeTorus : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeTorus() { this->node_type = RDViewNodeType_Torus; }
        inline virtual ~RDViewNodeTorus() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeTube : public RDViewNodeInterface
{
    public:
        inline RDViewNodeTube() { this->node_type = RDViewNodeType_Tube; }
        inline virtual ~RDViewNodeTube() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
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
};

struct RDViewNodeMatrix : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeMatrix() { this->node_type = RDViewNodeType_Matrix; }
        inline virtual ~RDViewNodeMatrix() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeRotate : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeRotate() { this->node_type = RDViewNodeType_Rotate; }
        inline virtual ~RDViewNodeRotate() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeScale : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeScale() { this->node_type = RDViewNodeType_Scale; }
        inline virtual ~RDViewNodeScale() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeTranslate : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeTranslate() { this->node_type = RDViewNodeType_Translate; }
        inline virtual ~RDViewNodeTranslate() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
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
};

struct RDViewNodeFarLight : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeFarLight() { this->node_type = RDViewNodeType_FarLight; }
        inline virtual ~RDViewNodeFarLight() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodePointLight : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodePointLight() { this->node_type = RDViewNodeType_PointLight; }
        inline virtual ~RDViewNodePointLight() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeConeLight : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeConeLight() { this->node_type = RDViewNodeType_ConeLight; }
        inline virtual ~RDViewNodeConeLight() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeKa : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeKa() { this->node_type = RDViewNodeType_Ka; }
        inline virtual ~RDViewNodeKa() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeKd : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeKd() { this->node_type = RDViewNodeType_Kd; }
        inline virtual ~RDViewNodeKd() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeKs : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeKs() { this->node_type = RDViewNodeType_Ks; }
        inline virtual ~RDViewNodeKs() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeSpecular : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeSpecular() { this->node_type = RDViewNodeType_Specular; }
        inline virtual ~RDViewNodeSpecular() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeSurface : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeSurface() { this->node_type = RDViewNodeType_Surface; }
        inline virtual ~RDViewNodeSurface() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeMapLoad : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeMapLoad() { this->node_type = RDViewNodeType_MapLoad; }
        inline virtual ~RDViewNodeMapLoad() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeMap : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeMap() { this->node_type = RDViewNodeType_Map; }
        inline virtual ~RDViewNodeMap() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeMapSample : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeMapSample() { this->node_type = RDViewNodeType_MapSample; }
        inline virtual ~RDViewNodeMapSample() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeMapBound : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeMapBound() { this->node_type = RDViewNodeType_MapBound; }
        inline virtual ~RDViewNodeMapBound() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodeMapBorder : public RDViewNodeInterface 
{ 
    public:
        inline RDViewNodeMapBorder() { this->node_type = RDViewNodeType_MapBorder; }
        inline virtual ~RDViewNodeMapBorder() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

struct RDViewNodePrimitive : public RDViewNodeInterface
{
    public:
        inline RDViewNodePrimitive() { this->node_type = RDViewNodeType_Primitive; }
        inline ~RDViewNodePrimitive() { }
        inline virtual void visit(RDViewNodeVisitor *visitor) override { visitor->accept(this); }
};

class RDViewParser
{
    public:
        RDViewParser();
        ~RDViewParser();

    private:
        
        // Core structural nodes.
        RDViewNodeInterface* match_root();
        RDViewNodeInterface* match_singleframe_body();
        RDViewNodeInterface* match_multiframe_body();
        RDViewNodeInterface* match_definitions();

        // Semi-structural nodes.
        RDViewNodeInterface* match_include();
        RDViewNodeInterface* match_display();
        RDViewNodeInterface* match_format();
        RDViewNodeInterface* match_object();
        RDViewNodeInterface* match_frame();
        RDViewNodeInterface* match_world();
        RDViewNodeInterface* match_properties();
        RDViewNodeInterface* match_frame_commands();
        RDViewNodeInterface* match_world_commands();
        RDViewNodeInterface* match_object_commands();
        RDViewNodeInterface* match_camera();
        RDViewNodeInterface* match_geometry();
        RDViewNodeInterface* match_transforms();
        RDViewNodeInterface* match_lighting();
        RDViewNodeInterface* match_surface_attributes();
        RDViewNodeInterface* match_attribute_mapping();

        // Commands
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
        std::vector<RDViewNodeInterface*> nodes;
        std::stack<RDViewTokenizer> tokenizer_stack;
        RDViewTokenizer *tokenizer;

};