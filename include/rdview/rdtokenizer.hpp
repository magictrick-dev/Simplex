#pragma once
#include <utils/defs.hpp>
#include <filesystem>
#include <string>

enum RDViewTokenType
{
    RDViewTokenType_EOF,
    RDViewTokenType_Invalid,
    RDViewTokenType_Integer,
    RDViewTokenType_Real,
    RDViewTokenType_String,
    RDViewTokenType_Boolean,
    RDViewTokenType_Identifier,
};

enum RDViewIdentifierType
{

    // Catchall
    RDViewIdentifierType_Invalid,

    // General Commands
    RDViewIdentifierType_Display,
    RDViewIdentifierType_Format,
    RDViewIdentifierType_Include,

    // File Structuring
    RDViewIdentifierType_FrameBegin,
    RDViewIdentifierType_FrameEnd,
    RDViewIdentifierType_WorldBegin,
    RDViewIdentifierType_WorldEnd,
    RDViewIdentifierType_ObjectBegin,
    RDViewIdentifierType_ObjectEnd,
    RDViewIdentifierType_ObjectInstance,

    // Drawing Attributes
    RDViewIdentifierType_Background,
    RDViewIdentifierType_Color,
    RDViewIdentifierType_Opacity,

    // Options
    RDViewIdentifierType_OptionArray,
    RDViewIdentifierType_OptionBool,
    RDViewIdentifierType_OptionList,
    RDViewIdentifierType_OptionReal,
    RDViewIdentifierType_OptionString,

    // Camera
    RDViewIdentifierType_CameraAt,
    RDViewIdentifierType_CameraEye,
    RDViewIdentifierType_CameraFOV,
    RDViewIdentifierType_CameraUp,
    RDViewIdentifierType_Clipping,

    // Geometry
    RDViewIdentifierType_Point,
    RDViewIdentifierType_Line,
    RDViewIdentifierType_Circle,
    RDViewIdentifierType_Fill,
    RDViewIdentifierType_Disk,
    RDViewIdentifierType_Cone,
    RDViewIdentifierType_Cube,
    RDViewIdentifierType_Cylinder,
    RDViewIdentifierType_Sphere,
    RDViewIdentifierType_Torus,
    RDViewIdentifierType_Tube,
    RDViewIdentifierType_Paraboloid,
    RDViewIdentifierType_Hyperboloid,
    RDViewIdentifierType_SqSphere,
    RDViewIdentifierType_SqTorus,
    RDViewIdentifierType_PointSet,
    RDViewIdentifierType_LineSet,
    RDViewIdentifierType_PolySet,
    RDViewIdentifierType_Curve,
    RDViewIdentifierType_Patch,
    RDViewIdentifierType_Subdivision,

    // Geometric Transformations
    RDViewIdentifierType_Translate,
    RDViewIdentifierType_Scale,
    RDViewIdentifierType_Rotate,
    RDViewIdentifierType_Matrix,
    RDViewIdentifierType_XformPush,
    RDViewIdentifierType_XformPop,

    // Lighting
    RDViewIdentifierType_AmbientLight,
    RDViewIdentifierType_FarLight,
    RDViewIdentifierType_PointLight,
    RDViewIdentifierType_ConeLight,

    // Surface Attributes
    RDViewIdentifierType_Ka,
    RDViewIdentifierType_Kd,
    RDViewIdentifierType_Ks,
    RDViewIdentifierType_Specular,
    RDViewIdentifierType_Surface,

    // Attribute Mapping
    RDViewIdentifierType_MapLoad,
    RDViewIdentifierType_Map,
    RDViewIdentifierType_MapSample,
    RDViewIdentifierType_MapBound,
    RDViewIdentifierType_MapBorder,

};

struct RDViewToken
{

    inline RDViewToken() { } // NOTE(Chris): std::string_view will complain if not defined...

    RDViewTokenType type;
    size_t line;
    size_t column;
    size_t length;

    union
    {
        struct { int64_t value;                } boolean;
        struct { int64_t value;                } integer;
        struct { real64_t value;               } real;
        struct { std::string_view value;       } string;
        struct { std::string_view value;       } identifier;
        struct { RDViewIdentifierType type;    } keyword;
    };

};

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
        RDViewIdentifierType classify_identifier(RDViewToken *token);

        void synchronize();
        char peak() const;
        char peak_at(size_t count) const;
        void consume();
        bool is_eof() const;
        void format_token_as(RDViewToken *token, RDViewTokenType type);

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