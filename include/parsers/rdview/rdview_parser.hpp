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
//          BODY                    :   (INCLUDE | DEFINITIONS)* (FRAME BODY | EOF)
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
//          SUBDIVISION             :   "Subdivision" string string integer integer integer 
//                                      numerical.[n] integer.[n] integer.[n] real.[n]
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
#include <unordered_set>
#include <vector>
#include <stack>
#include <algorithm>
#include <variant>
#include <exception>
#include <parsers/rdview/rdview_token.hpp>
#include <parsers/rdview/rdview_lexer.hpp>
#include <parsers/rdview/rdview_node_types.hpp>

// NOTE(Chris): Base class exception for parsing errors. All deriving exceptions specify how
//              the parser should synchronize after catching an exception.
class RDViewParserError : public std::exception 
{ 
    public:
        inline virtual const char *what() const noexcept override
        {
            return this->message.c_str();
        }

    protected:
        inline void set_message(const std::string &message) { this->message = message; }
        inline void set_message(std::string &&message) { this->message = message; }

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
        inline RDViewParserErrorUC(RDViewToken token, std::string place)
        {
            std::string location = token.source_file_path.string();
            std::string contents(token.source_file_contents.substr(token.offset, token.length));

            std::stringstream message_stream;
            message_stream  << location << "(" << token.line << ", " << token.column 
                            << "): Unexpected command encountered in " << place << ".";
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

// Include Not Found -  Pretty self explanatory.
class RDViewParserErrorINF : public RDViewParserError
{
    public:
        inline RDViewParserErrorINF(RDViewToken token, std::string path)
        {

            std::string location = token.source_file_path.string();
            std::string contents(token.source_file_contents.substr(token.offset, token.length));

            std::stringstream message_stream;
            message_stream  << location << "(" << token.line << ", " << token.column 
                            << "): include path '" << path << "' was not found.";
            this->set_message(message_stream.str());

        }
};

// No Implement     - Pretty self explanatory
class RDViewParserErrorNI : public RDViewParserError
{
    public:
        inline RDViewParserErrorNI(RDViewToken token)
        {

            std::string location = token.source_file_path.string();
            std::string contents(token.source_file_contents.substr(token.offset, token.length));

            std::stringstream message_stream;
            message_stream  << location << "(" << token.line << ", " << token.column 
                            << "): No implementation for this.";
            this->set_message(message_stream.str());

        }
};

class RDViewParser
{
    public:
        RDViewParser(std::string_view source_file, std::filesystem::path source_path, std::ostream& os = std::cout);
        ~RDViewParser();

        // NOTE(Chris): We will need to handle the Rule-Of-5 behavior at some point.
        //              Right now, we can just assume things are properly handled.
        RDViewParser(RDViewParser &&other) = delete;
        inline RDViewParser& operator=(const RDViewParser &other) = delete;

        bool match_everything();
        inline bool is_valid() const { return (this->root != NULL && this->error_count == 0); }
        inline RDViewNodeInterface* get_root() const { return this->root; }

        std::ostream &output_stream;

    private:
        void synchronize_to(RDViewTokenType token_type);

        bool is_previous_token(RDViewTokenType token_type) const;
        bool is_current_token(RDViewTokenType token_type) const;
        bool is_next_token(RDViewTokenType token_type) const;

        bool expect_keyword(RDViewKeywordType keyword_type, std::string error);
        bool expect_type(RDViewTokenType token_type);
        void consume();

        RDViewToken fetch_type_and_consume(RDViewTokenType token_type);
        real32_t fetch_numerical_and_consume();

        template <typename T, typename... Args> 
        void throw_error(Args... args)
        {

            static_assert(std::is_base_of<RDViewParserError, T>::value);

            this->error_count++;
            throw T(args...);

        }

        template <typename T, typename... Args> 
        void throw_error_and_recover(Args... args)
        {

            static_assert(std::is_base_of<RDViewParserError, T>::value);
            this->error_count++;

            // NOTE(Chris): Do we actually need to try/catch this?
            try 
            { 
                throw T(args...); 
            } 
            catch (RDViewParserError &e) 
            { 
                output_stream << e.what() << std::endl;
            }

        }

        RDViewNodeInterface* match_root();
        RDViewNodeInterface* match_body();

        RDViewNodeInterface* match_include();
        RDViewNodeInterface* match_display();
        RDViewNodeInterface* match_format();
        RDViewNodeInterface* match_object();
        RDViewNodeInterface* match_frame();
        RDViewNodeInterface* match_world();

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

    private:
        template <typename T, typename... Args> inline T* create_node(Args ...args)
        {

            static_assert(std::is_base_of<RDViewNodeInterface, T>::value);
            void *memory_buffer = malloc(sizeof(T));
            T *node = new (memory_buffer) T(args...);
            this->nodes.push_back(node);
            return node;

        }

        inline void destroy_node(RDViewNodeInterface *node)
        {
            node->~RDViewNodeInterface();
            free(node);
        }

    private:
        size_t warning_count = 0;
        size_t error_count = 0;
        RDViewNodeInterface* root;
        std::vector<RDViewNodeInterface*> nodes;
        std::stack<RDViewTokenizer> tokenizer_stack;
        RDViewTokenizer *tokenizer;

};
