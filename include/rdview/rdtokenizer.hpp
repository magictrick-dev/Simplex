#pragma once
#include <utils/defs.hpp>
#include <filesystem>
#include <string>

enum RDViewTokenType
{
    RDTokenType_Invalid,
    RDTokenType_Integer,
    RDTOkenType_Real,
    RDTokenType_String,
    RDTokenType_Boolean,
    RDTokenType_Identifier,
};

enum RDViewIdentifierType
{
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

    RDViewTokenType type;
    size_t line;
    size_t column;
    size_t length;

    union
    {
        struct boolean      { int64_t value; };
        struct integer      { int64_t value; };
        struct real         { real64_t value; };
        struct string       { std::string_view value; };
        struct identifier   { std::string_view value; };
        struct keyword      { RDViewIdentifierType type; };
    };

};

struct RDViewSourceFile
{
    std::filesystem::path path;
    std::filesystem::path parent_path;
    std::string_view source;
    size_t offset;
    size_t step;
    size_t line;
    size_t column;
};

enum RDViewTokenizerResult
{
    RDViewTokenizerResult_OK,
    RDViewTokenizerResult_NoSourcesIncludes,
    RDViewTokenizerResult_FileNotFound,
    RDViewTokenizerResult_FileNotAccessible,
    RDViewTokenizerResult_CircularSource,
};

class RDViewTokenizer
{
    public:
        RDViewTokenizer();
        virtual ~RDViewTokenizer();

        RDViewTokenizerResult tokenize();
        RDViewTokenizerResult include_source(std::filesystem::path path_to, 
                                             std::filesystem::path included_from = "");
        
        inline size_t count() const { return this->tokens.size(); }
        inline RDViewToken& get_token_at(size_t index) { return this->tokens[index]; }
        inline RDViewToken& operator[](size_t index) { return this->tokens[index]; };
        inline const RDViewToken& get_token_at(size_t index) const { return this->tokens[index]; }
        inline const RDViewToken& operator[](size_t index) const { return this->tokens[index]; }

    private:
        void consume_whitespace();
        bool handle_numbers();
        bool handle_strings();
        bool handle_identifiers();

        void synchronize();

    private:
        std::vector<RDViewSourceFile*> source_files;
        std::vector<std::filesystem::path, RDViewSourceFile*> source_map;
        std::vector<RDViewToken> tokens;

};