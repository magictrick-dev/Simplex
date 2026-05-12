#include <parsers/rdview/rdview_parser.hpp>
#include <utils/test_registry.hpp>

RDViewParser::
RDViewParser(std::string_view source_file, std::filesystem::path source_path, std::ostream &os)
    : output_stream(os)
{
    this->tokenizer_stack.emplace(source_file, source_path);
    this->tokenizer = &this->tokenizer_stack.top();
    this->root = NULL;
    this->error_count = 0;
}

RDViewParser::
~RDViewParser()
{

    for (auto &node : this->nodes)
    {
        destroy_node(node);
        node = nullptr;
    }

    this->nodes.clear();

}

bool RDViewParser::
match_everything()
{
    
    try
    {
        RDViewNodeRoot *node = this->match_root();
        this->root = node;
        return (this->error_count == 0);
    }
    catch (RDViewParserError &e)
    {
        output_stream << e.what() << std::endl;
        this->root = NULL;
    }

    return (this->root != NULL && this->error_count != 0);
    
}

void RDViewParser::
synchronize_to(RDViewTokenType token_type)
{

    while (true)
    {

        auto current_token = this->tokenizer->get_current_token();
        if (current_token.type == RDViewTokenType_EOF || 
            current_token.type == token_type) break;
        this->consume();

    }

}

bool RDViewParser:: 
is_previous_token(RDViewTokenType token_type) const
{
    const bool result = (this->tokenizer->previous_token_is(token_type));
    return result;
}

bool RDViewParser:: 
is_current_token(RDViewTokenType token_type) const
{
    const bool result = (this->tokenizer->current_token_is(token_type));
    return result;
}

bool RDViewParser:: 
is_next_token(RDViewTokenType token_type) const
{
    const bool result = (this->tokenizer->next_token_is(token_type));
    return result;
}

bool RDViewParser::
expect_keyword(RDViewKeywordType keyword_type, std::string error)
{
    const bool result = (this->tokenizer->is_current_keyword(keyword_type));
    if (result == false) this->throw_error<RDViewParserErrorUC>(this->tokenizer->get_current_token(), error);
    return result;
}

bool RDViewParser::
expect_type(RDViewTokenType token_type)
{
    const bool result = (this->tokenizer->current_token_is(token_type));
    if (result == false) this->throw_error<RDViewParserErrorUT>(this->tokenizer->get_current_token());
    return result;
}

RDViewToken RDViewParser::
fetch_type_and_consume(RDViewTokenType token_type)
{
    auto result = this->tokenizer->get_current_token();
    if (!this->tokenizer->current_token_is(token_type)) this->throw_error<RDViewParserErrorUT>(result);
    this->consume();
    return result;
}

void RDViewParser::
consume()
{
    this->tokenizer->shift();
}

real32_t RDViewParser::
fetch_numerical_and_consume()
{
    auto token = this->tokenizer->get_current_token();
    if (this->is_current_token(RDViewTokenType_Real))
    {
        this->consume();
        return (real32_t)token.real.value;
    }
    else if (this->is_current_token(RDViewTokenType_Integer))
    {
        this->consume();
        return (real32_t)token.integer.value;
    }
    this->throw_error<RDViewParserErrorUT>(token);
    return 0.0f;
}

RDViewNodeRoot* RDViewParser::
match_root()
{
    
    try
    {

        // NOTE(Chris): We are enforcing that display and format commands exist in the script here.
        //              This is a deviation from the original specification.
        auto display_node = this->match_display();
        auto format_node = this->match_format();
        auto body_node = this->match_body();

        RDViewNodeRoot *root_node = this->create_node<RDViewNodeRoot>();
        root_node->display = display_node;
        root_node->format = format_node;
        root_node->body = body_node;
        return root_node;

    }
    catch (RDViewParserError &e)
    {
        output_stream << e.what() << std::endl;
        return NULL;
    }

}

RDViewNodeInterface* RDViewParser::
match_body()
{

    RDViewNodeBody *body = this->create_node<RDViewNodeBody>();

    while (!this->is_current_token(RDViewTokenType_EOF))
    {

        auto current_token = this->tokenizer->get_current_token();

        try
        {

            this->expect_type(RDViewTokenType_Keyword);
            RDViewKeywordType current_keyword = current_token.keyword.type;

            // NOTE(Chris): If we encounter a world begin, we will short circuit matching entirely and break
            //              from the loop once matching is complete. This closely aligns with the original
            //              behavior. We expect an EOF afterwards.
            RDViewNodeInterface *result = NULL;
            if (current_keyword == RDViewKeywordType_WorldBegin)
            {
                
                result = this->match_world();
                SIMPLEX_CHECK_PTR(result);
                body->children.push_back(result);
                break;
                
            }

            switch (current_keyword)
            {

                case RDViewKeywordType_Include:         { result = this->match_include();       } break;
                case RDViewKeywordType_ObjectBegin:     { result = this->match_object();        } break;
                case RDViewKeywordType_OptionArray:     { result = this->match_option_array();  } break;
                case RDViewKeywordType_OptionBool:      { result = this->match_option_bool();   } break;
                case RDViewKeywordType_OptionList:      { result = this->match_option_list();   } break;
                case RDViewKeywordType_OptionReal:      { result = this->match_option_real();   } break;
                case RDViewKeywordType_OptionString:    { result = this->match_option_string(); } break;

                // NOTE(Chris): We might want to remove this and enforce it per-frame?
                case RDViewKeywordType_CameraAt:        { result = this->match_camera_at();     } break;
                case RDViewKeywordType_CameraEye:       { result = this->match_camera_eye();    } break;
                case RDViewKeywordType_CameraUp:        { result = this->match_camera_up();     } break;
                case RDViewKeywordType_CameraFOV:       { result = this->match_camera_fov();    } break;
                case RDViewKeywordType_Clipping:        { result = this->match_clipping();      } break;

                case RDViewKeywordType_FrameBegin:      { result = this->match_frame();         } break;

                default:
                {
                    // NOTE(Chris): Any other commands are invalid in this context.
                    this->consume();
                    this->throw_error<RDViewParserErrorUC>(current_token, "script body");
                }
            }

            // NOTE(Chris): No matter what, we should get a valid token back, the try/catch
            //              is responsible for resynchronizing correctly.
            SIMPLEX_CHECK_PTR(result);
            body->children.push_back(result);

        }
        catch (RDViewParserError &e)
        {
            output_stream << e.what() << std::endl;
            this->synchronize_to(RDViewTokenType_Keyword);
        }

    }

    return body;

}

RDViewNodeInterface* RDViewParser::
match_include()
{
    
    auto command = this->tokenizer->get_current_token();
    this->expect_keyword(RDViewKeywordType_Include, "script root (expected 'Include')");
    this->consume();

    auto path = this->fetch_type_and_consume(RDViewTokenType_String);

    std::string user_path(path.string.value);
    std::filesystem::path canonical_path = std::filesystem::weakly_canonical(user_path);

    if (!std::filesystem::exists(canonical_path))
    {
        this->throw_error_and_recover<RDViewParserErrorINF>(command, user_path);
    }

    // TODO(Chris): Push a new parser, match on it, then set.

    RDViewNodeInclude *include = this->create_node<RDViewNodeInclude>();
    include->input_path = user_path;
    include->canonical_path = canonical_path;

    return include;
}

RDViewNodeInterface* RDViewParser::
match_display()
{

    auto command = this->tokenizer->get_current_token();
    this->expect_keyword(RDViewKeywordType_Display, "script header (expected 'Display')");
    this->consume();

    auto name   = this->fetch_type_and_consume(RDViewTokenType_String);
    auto format = this->fetch_type_and_consume(RDViewTokenType_String);
    auto mode   = this->fetch_type_and_consume(RDViewTokenType_String);

    RDViewDisplayType format_type = RDViewNodeDisplay::map_display_type(format.string.value);
    RDViewModeType mode_type = RDViewNodeDisplay::map_mode_type(mode.string.value);
    
    if (format_type == RDViewDisplayType_Invalid)
        this->throw_error<RDViewParserErrorICF>(format, "invalid display format type.");

    if (mode_type == RDViewModeType_Invalid)
        this->throw_error<RDViewParserErrorICF>(mode, "invalid display mode type.");

    RDViewNodeDisplay *display = this->create_node<RDViewNodeDisplay>();
    display->name = name.string.value;
    display->format = format_type;
    display->mode = mode_type;

    return display;

}

RDViewNodeInterface* RDViewParser::
match_format()
{

    this->expect_keyword(RDViewKeywordType_Format, "script header (expected 'Format')");
    this->consume();

    auto width = this->fetch_type_and_consume(RDViewTokenType_Integer);
    auto height = this->fetch_type_and_consume(RDViewTokenType_Integer);

    RDViewNodeFormat *format = this->create_node<RDViewNodeFormat>();
    format->width = width.integer.value;
    format->height = height.integer.value;

    return format;

}

RDViewNodeInterface* RDViewParser::
match_object()
{
    auto current_token = this->tokenizer->get_current_token();
    this->consume();
    throw RDViewParserErrorNI(current_token);
    SIMPLEX_NO_IMPLEMENTATION("");
    return nullptr;
}

RDViewNodeInterface* RDViewParser::
match_frame()
{

    this->expect_keyword(RDViewKeywordType_FrameBegin, "script body (expected 'FrameBegin')");
    this->consume();

    auto frame_number_token = this->fetch_type_and_consume(RDViewTokenType_Integer);
    int32_t frame_number = frame_number_token.integer.value;

    std::vector<RDViewNodeInterface*> nodes;
    RDViewNodeInterface *world_node = NULL;

    while (!this->is_current_token(RDViewTokenType_EOF))
    {

        auto current_token = this->tokenizer->get_current_token();

        try
        {

            this->expect_type(RDViewTokenType_Keyword);
            RDViewKeywordType current_keyword = current_token.keyword.type;
            if (current_keyword == RDViewKeywordType_FrameEnd) break;

            // NOTE(Chris): Encountering "WorldBegin" will immediately cause us to
            //              to break out of the parsing loop so that any additional
            //              commands are considered improperly placed.
            if (current_keyword == RDViewKeywordType_WorldBegin)
            {
                world_node = this->match_world();           
                break;
            }

            RDViewNodeInterface *result = NULL;
            switch (current_keyword)
            {

                case RDViewKeywordType_Background:      { result = this->match_background();    } break;
                case RDViewKeywordType_Color:           { result = this->match_color();         } break;
                case RDViewKeywordType_Opacity:         { result = this->match_opacity();       } break;

                case RDViewKeywordType_CameraAt:        { result = this->match_camera_at();     } break;
                case RDViewKeywordType_CameraEye:       { result = this->match_camera_eye();    } break;
                case RDViewKeywordType_CameraUp:        { result = this->match_camera_up();     } break;
                case RDViewKeywordType_CameraFOV:       { result = this->match_camera_fov();    } break;
                case RDViewKeywordType_Clipping:        { result = this->match_clipping();      } break;

                case RDViewKeywordType_AmbientLight:    { result = this->match_ambient_light(); } break;
                case RDViewKeywordType_FarLight:        { result = this->match_far_light();     } break;
                case RDViewKeywordType_PointLight:      { result = this->match_point_light();   } break;
                case RDViewKeywordType_ConeLight:       { result = this->match_cone_light();    } break;

                case RDViewKeywordType_Ka:              { result = this->match_ka();            } break;
                case RDViewKeywordType_Kd:              { result = this->match_kd();            } break;
                case RDViewKeywordType_Ks:              { result = this->match_ks();            } break;
                case RDViewKeywordType_Specular:        { result = this->match_specular();      } break;
                case RDViewKeywordType_Surface:         { result = this->match_surface();       } break;

                case RDViewKeywordType_MapLoad:         { result = this->match_map_load();      } break;
                case RDViewKeywordType_MapSample:       { result = this->match_map_sample();    } break;
                case RDViewKeywordType_MapBound:        { result = this->match_map_bound();     } break;
                case RDViewKeywordType_MapBorder:       { result = this->match_map_border();    } break;
                case RDViewKeywordType_Map:             { result = this->match_map();           } break;

                default:
                {
                    // NOTE(Chris): Any other commands are invalid in this context.
                    this->consume();
                    this->throw_error<RDViewParserErrorUC>(current_token, "frame body");
                }
            }

            // NOTE(Chris): No matter what, we should get a valid token back, the try/catch
            //              is responsible for resynchronizing correctly.
            SIMPLEX_CHECK_PTR(result);
            nodes.push_back(result);

        }
        catch (RDViewParserError &e)
        {
            output_stream << e.what() << std::endl;
            this->synchronize_to(RDViewTokenType_Keyword);
        }

    }

    this->expect_keyword(RDViewKeywordType_FrameEnd, "script body (expected 'FrameEnd')");
    this->consume();

    // NOTE(Chris): We generate afterwards since we need to ensure there is a ending token,
    //              otherwise we need to handle memory cleanup on exception (gross).
    RDViewNodeFrame *frame = this->create_node<RDViewNodeFrame>();
    frame->children = nodes;
    frame->world = world_node;
    frame->frame_number = frame_number;

    return frame;

}

RDViewNodeInterface* RDViewParser::
match_world()
{

    this->expect_keyword(RDViewKeywordType_WorldBegin, "script body (expected 'WorldBegin')");
    this->consume();

    std::vector<RDViewNodeInterface*> nodes;

    while (!this->is_current_token(RDViewTokenType_EOF))
    {

        auto current_token = this->tokenizer->get_current_token();

        try
        {

            this->expect_type(RDViewTokenType_Keyword);
            RDViewKeywordType current_keyword = current_token.keyword.type;
            if (current_keyword == RDViewKeywordType_WorldEnd) break;

            RDViewNodeInterface *result = NULL;
            switch (current_keyword)
            {

                case RDViewKeywordType_Color:           { result = this->match_color();             } break;
                case RDViewKeywordType_Opacity:         { result = this->match_opacity();           } break;

                case RDViewKeywordType_AmbientLight:    { result = this->match_ambient_light();     } break;
                case RDViewKeywordType_FarLight:        { result = this->match_far_light();         } break;
                case RDViewKeywordType_PointLight:      { result = this->match_point_light();       } break;
                case RDViewKeywordType_ConeLight:       { result = this->match_cone_light();        } break;

                case RDViewKeywordType_Point:           { result = this->match_point();             } break;
                case RDViewKeywordType_PointSet:        { result = this->match_point_set();         } break;
                case RDViewKeywordType_Line:            { result = this->match_line();              } break;
                case RDViewKeywordType_LineSet:         { result = this->match_line_set();          } break;
                case RDViewKeywordType_Circle:          { result = this->match_circle();            } break;
                case RDViewKeywordType_Fill:            { result = this->match_fill();              } break;
                case RDViewKeywordType_Cone:            { result = this->match_cone();              } break;
                case RDViewKeywordType_Cube:            { result = this->match_cube();              } break;
                case RDViewKeywordType_Curve:           { result = this->match_curve();             } break;
                case RDViewKeywordType_Cylinder:        { result = this->match_cylinder();          } break;
                case RDViewKeywordType_Disk:            { result = this->match_disk();              } break;
                case RDViewKeywordType_Hyperboloid:     { result = this->match_hyperboloid();       } break;
                case RDViewKeywordType_Paraboloid:      { result = this->match_paraboloid();        } break;
                case RDViewKeywordType_Patch:           { result = this->match_patch();             } break;
                case RDViewKeywordType_PolySet:         { result = this->match_poly_set();          } break;
                case RDViewKeywordType_Sphere:          { result = this->match_sphere();            } break;
                case RDViewKeywordType_SqSphere:        { result = this->match_sq_sphere();         } break;
                case RDViewKeywordType_SqTorus:         { result = this->match_sq_torus();          } break;
                case RDViewKeywordType_Torus:           { result = this->match_torus();             } break;
                case RDViewKeywordType_Tube:            { result = this->match_tube();              } break;
                case RDViewKeywordType_Subdivision:     { result = this->match_subdivision();       } break;
                case RDViewKeywordType_ObjectInstance:  { result = this->match_object_instance();   } break;

                case RDViewKeywordType_Matrix:          { result = this->match_matrix();            } break;
                case RDViewKeywordType_Rotate:          { result = this->match_rotate();            } break;
                case RDViewKeywordType_Scale:           { result = this->match_scale();             } break;
                case RDViewKeywordType_Translate:       { result = this->match_translate();         } break;
                case RDViewKeywordType_XformPush:       { result = this->match_xformpush();         } break;
                case RDViewKeywordType_XformPop:        { result = this->match_xformpop();          } break;

                case RDViewKeywordType_Ka:              { result = this->match_ka();                } break;
                case RDViewKeywordType_Kd:              { result = this->match_kd();                } break;
                case RDViewKeywordType_Ks:              { result = this->match_ks();                } break;
                case RDViewKeywordType_Specular:        { result = this->match_specular();          } break;
                case RDViewKeywordType_Surface:         { result = this->match_surface();           } break;

                case RDViewKeywordType_MapSample:       { result = this->match_map_sample();        } break;
                case RDViewKeywordType_MapBound:        { result = this->match_map_bound();         } break;
                case RDViewKeywordType_MapBorder:       { result = this->match_map_border();        } break;
                case RDViewKeywordType_Map:             { result = this->match_map();               } break;

                default:
                {
                    // NOTE(Chris): Any other commands are invalid in this context.
                    this->consume();
                    this->throw_error<RDViewParserErrorUC>(current_token, "world body");
                }
            }

            // NOTE(Chris): No matter what, we should get a valid token back, the try/catch
            //              is responsible for resynchronizing correctly.
            SIMPLEX_CHECK_PTR(result);
            nodes.push_back(result);

        }
        catch (RDViewParserError &e)
        {
            output_stream << e.what() << std::endl;
            this->synchronize_to(RDViewTokenType_Keyword);
        }

    }

    this->expect_keyword(RDViewKeywordType_WorldEnd, "script body (expected 'WorldEnd')");
    this->consume();

    // NOTE(Chris): We generate afterwards since we need to ensure there is a ending token,
    //              otherwise we need to handle memory cleanup on exception (gross).
    RDViewNodeWorld *world = this->create_node<RDViewNodeWorld>();
    world->children = nodes;

    return world;

}

RDViewNodeInterface* RDViewParser::
match_option_array()
{
    this->expect_keyword(RDViewKeywordType_OptionArray, "option (expected 'OptionArray')");
    this->consume();

    auto name  = this->fetch_type_and_consume(RDViewTokenType_String);
    auto count = this->fetch_type_and_consume(RDViewTokenType_Integer);

    RDViewNodeOptionArray *node = this->create_node<RDViewNodeOptionArray>();
    node->name = std::string(name.string.value);
    node->values.reserve((size_t)count.integer.value);
    for (int64_t i = 0; i < count.integer.value; ++i)
        node->values.push_back(this->fetch_numerical_and_consume());
    return node;
}

RDViewNodeInterface* RDViewParser::
match_option_bool()
{
    this->expect_keyword(RDViewKeywordType_OptionBool, "option (expected 'OptionBool')");
    this->consume();

    auto name = this->fetch_type_and_consume(RDViewTokenType_String);

    auto token = this->tokenizer->get_current_token();
    int32_t bool_value = 0;

    if (this->is_current_token(RDViewTokenType_Integer))
    {
        bool_value = (int32_t)token.integer.value;
        this->consume();
    }
    else if (this->is_current_token(RDViewTokenType_Identifier))
    {
        std::string_view id = token.identifier.value;
        if      (id == "true"  || id == "on")  bool_value = 1;
        else if (id == "false" || id == "off") bool_value = 0;
        else this->throw_error<RDViewParserErrorICF>(token, "expected boolean value (true/false/on/off/integer)");
        this->consume();
    }
    else
    {
        this->throw_error<RDViewParserErrorICF>(token, "expected boolean value");
    }

    RDViewNodeOptionBool *node = this->create_node<RDViewNodeOptionBool>();
    node->name  = std::string(name.string.value);
    node->value = bool_value;
    return node;
}

RDViewNodeInterface* RDViewParser::
match_option_list()
{
    this->expect_keyword(RDViewKeywordType_OptionList, "option (expected 'OptionList')");
    this->consume();

    auto name  = this->fetch_type_and_consume(RDViewTokenType_String);
    auto count = this->fetch_type_and_consume(RDViewTokenType_Integer);

    RDViewNodeOptionList *node = this->create_node<RDViewNodeOptionList>();
    node->name = std::string(name.string.value);
    node->values.reserve((size_t)count.integer.value);
    for (int64_t i = 0; i < count.integer.value; ++i)
    {
        auto str = this->fetch_type_and_consume(RDViewTokenType_String);
        node->values.push_back(std::string(str.string.value));
    }
    return node;
}

RDViewNodeInterface* RDViewParser::
match_option_real()
{
    this->expect_keyword(RDViewKeywordType_OptionReal, "option (expected 'OptionReal')");
    this->consume();

    auto name = this->fetch_type_and_consume(RDViewTokenType_String);

    RDViewNodeOptionReal *node = this->create_node<RDViewNodeOptionReal>();
    node->name  = std::string(name.string.value);
    node->value = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_option_string()
{
    this->expect_keyword(RDViewKeywordType_OptionString, "option (expected 'OptionString')");
    this->consume();

    auto name  = this->fetch_type_and_consume(RDViewTokenType_String);
    auto value = this->fetch_type_and_consume(RDViewTokenType_String);

    RDViewNodeOptionString *node = this->create_node<RDViewNodeOptionString>();
    node->name  = std::string(name.string.value);
    node->value = std::string(value.string.value);
    return node;
}

RDViewNodeInterface* RDViewParser::
match_background()
{
    this->expect_keyword(RDViewKeywordType_Background, "frame (expected 'Background')");
    this->consume();

    RDViewNodeBackground *node = this->create_node<RDViewNodeBackground>();
    node->red   = this->fetch_numerical_and_consume();
    node->green = this->fetch_numerical_and_consume();
    node->blue  = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_color()
{
    this->expect_keyword(RDViewKeywordType_Color, "expected 'Color'");
    this->consume();

    RDViewNodeColor *node = this->create_node<RDViewNodeColor>();
    node->red   = this->fetch_numerical_and_consume();
    node->green = this->fetch_numerical_and_consume();
    node->blue  = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_opacity()
{
    this->expect_keyword(RDViewKeywordType_Opacity, "expected 'Opacity'");
    this->consume();

    RDViewNodeOpacity *node = this->create_node<RDViewNodeOpacity>();
    node->opacity = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_camera_at()
{
    this->expect_keyword(RDViewKeywordType_CameraAt, "camera (expected 'CameraAt')");
    this->consume();

    RDViewNodeCameraAt *node = this->create_node<RDViewNodeCameraAt>();
    node->x = this->fetch_numerical_and_consume();
    node->y = this->fetch_numerical_and_consume();
    node->z = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_camera_eye()
{
    this->expect_keyword(RDViewKeywordType_CameraEye, "camera (expected 'CameraEye')");
    this->consume();

    RDViewNodeCameraEye *node = this->create_node<RDViewNodeCameraEye>();
    node->x = this->fetch_numerical_and_consume();
    node->y = this->fetch_numerical_and_consume();
    node->z = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_camera_fov()
{
    this->expect_keyword(RDViewKeywordType_CameraFOV, "camera (expected 'CameraFOV')");
    this->consume();

    RDViewNodeCameraFOV *node = this->create_node<RDViewNodeCameraFOV>();
    node->FOV = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_camera_up()
{
    this->expect_keyword(RDViewKeywordType_CameraUp, "camera (expected 'CameraUp')");
    this->consume();

    RDViewNodeCameraUp *node = this->create_node<RDViewNodeCameraUp>();
    node->x = this->fetch_numerical_and_consume();
    node->y = this->fetch_numerical_and_consume();
    node->z = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_clipping()
{
    this->expect_keyword(RDViewKeywordType_Clipping, "camera (expected 'Clipping')");
    this->consume();

    RDViewNodeClipping *node = this->create_node<RDViewNodeClipping>();
    node->near = this->fetch_numerical_and_consume();
    node->far  = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_point()
{
    this->expect_keyword(RDViewKeywordType_Point, "geometry (expected 'Point')");
    this->consume();

    RDViewNodePoint *node = this->create_node<RDViewNodePoint>();
    node->x = this->fetch_numerical_and_consume();
    node->y = this->fetch_numerical_and_consume();
    node->z = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_point_set()
{
    auto current_token = this->tokenizer->get_current_token();
    this->consume();
    throw RDViewParserErrorNI(current_token);
    SIMPLEX_NO_IMPLEMENTATION("");
    return nullptr;
}

RDViewNodeInterface* RDViewParser::
match_line()
{
    this->expect_keyword(RDViewKeywordType_Line, "geometry (expected 'Line')");
    this->consume();

    RDViewNodeLine *node = this->create_node<RDViewNodeLine>();
    node->x1 = this->fetch_numerical_and_consume();
    node->y1 = this->fetch_numerical_and_consume();
    node->z1 = this->fetch_numerical_and_consume();
    node->x2 = this->fetch_numerical_and_consume();
    node->y2 = this->fetch_numerical_and_consume();
    node->z2 = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_line_set()
{
    auto current_token = this->tokenizer->get_current_token();
    this->consume();
    throw RDViewParserErrorNI(current_token);
    SIMPLEX_NO_IMPLEMENTATION("");
    return nullptr;
}

RDViewNodeInterface* RDViewParser::
match_circle()
{
    this->expect_keyword(RDViewKeywordType_Circle, "geometry (expected 'Circle')");
    this->consume();

    RDViewNodeCircle *node = this->create_node<RDViewNodeCircle>();
    node->x      = this->fetch_numerical_and_consume();
    node->y      = this->fetch_numerical_and_consume();
    node->z      = this->fetch_numerical_and_consume();
    node->radius = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_fill()
{
    this->expect_keyword(RDViewKeywordType_Fill, "geometry (expected 'Fill')");
    this->consume();

    RDViewNodeFill *node = this->create_node<RDViewNodeFill>();
    node->x = this->fetch_numerical_and_consume();
    node->y = this->fetch_numerical_and_consume();
    node->z = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_cone()
{
    this->expect_keyword(RDViewKeywordType_Cone, "geometry (expected 'Cone')");
    this->consume();

    RDViewNodeCone *node = this->create_node<RDViewNodeCone>();
    node->height = this->fetch_numerical_and_consume();
    node->radius = this->fetch_numerical_and_consume();
    node->theta  = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_cube()
{
    this->expect_keyword(RDViewKeywordType_Cube, "geometry (expected 'Cube')");
    this->consume();
    return this->create_node<RDViewNodeCube>();
}

RDViewNodeInterface* RDViewParser::
match_curve()
{
    auto current_token = this->tokenizer->get_current_token();
    this->consume();
    throw RDViewParserErrorNI(current_token);
    SIMPLEX_NO_IMPLEMENTATION("");
    return nullptr;
}

RDViewNodeInterface* RDViewParser::
match_cylinder()
{
    this->expect_keyword(RDViewKeywordType_Cylinder, "geometry (expected 'Cylinder')");
    this->consume();

    RDViewNodeCylinder *node = this->create_node<RDViewNodeCylinder>();
    node->radius = this->fetch_numerical_and_consume();
    node->z_min  = this->fetch_numerical_and_consume();
    node->z_max  = this->fetch_numerical_and_consume();
    node->theta  = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_disk()
{
    this->expect_keyword(RDViewKeywordType_Disk, "geometry (expected 'Disk')");
    this->consume();

    RDViewNodeDisk *node = this->create_node<RDViewNodeDisk>();
    node->height = this->fetch_numerical_and_consume();
    node->radius = this->fetch_numerical_and_consume();
    node->theta  = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_hyperboloid()
{
    this->expect_keyword(RDViewKeywordType_Hyperboloid, "geometry (expected 'Hyperboloid')");
    this->consume();

    RDViewNodeHyperboloid *node = this->create_node<RDViewNodeHyperboloid>();
    node->x1    = this->fetch_numerical_and_consume();
    node->y1    = this->fetch_numerical_and_consume();
    node->z1    = this->fetch_numerical_and_consume();
    node->x2    = this->fetch_numerical_and_consume();
    node->y2    = this->fetch_numerical_and_consume();
    node->z2    = this->fetch_numerical_and_consume();
    node->theta = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_paraboloid()
{
    this->expect_keyword(RDViewKeywordType_Paraboloid, "geometry (expected 'Paraboloid')");
    this->consume();

    RDViewNodeParaboloid *node = this->create_node<RDViewNodeParaboloid>();
    node->radius = this->fetch_numerical_and_consume();
    node->z_min  = this->fetch_numerical_and_consume();
    node->z_max  = this->fetch_numerical_and_consume();
    node->theta  = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_patch()
{
    auto current_token = this->tokenizer->get_current_token();
    this->consume();
    throw RDViewParserErrorNI(current_token);
    SIMPLEX_NO_IMPLEMENTATION("");
    return nullptr;
}

RDViewNodeInterface* RDViewParser::
match_poly_set()
{
    auto current_token = this->tokenizer->get_current_token();
    this->consume();
    throw RDViewParserErrorNI(current_token);
    SIMPLEX_NO_IMPLEMENTATION("");
    return nullptr;
}

RDViewNodeInterface* RDViewParser::
match_sphere()
{
    this->expect_keyword(RDViewKeywordType_Sphere, "geometry (expected 'Sphere')");
    this->consume();

    RDViewNodeSphere *node = this->create_node<RDViewNodeSphere>();
    node->radius = this->fetch_numerical_and_consume();
    node->z_min  = this->fetch_numerical_and_consume();
    node->z_max  = this->fetch_numerical_and_consume();
    node->theta  = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_sq_sphere()
{
    this->expect_keyword(RDViewKeywordType_SqSphere, "geometry (expected 'SqSphere')");
    this->consume();

    auto n_token = this->tokenizer->get_current_token();

    RDViewNodeSqSphere *node = this->create_node<RDViewNodeSqSphere>();
    node->radius = this->fetch_numerical_and_consume();
    n_token      = this->fetch_type_and_consume(RDViewTokenType_Integer);
    node->n      = (real32_t)n_token.integer.value;
    node->e      = this->fetch_numerical_and_consume();
    node->z_min  = this->fetch_numerical_and_consume();
    node->z_max  = this->fetch_numerical_and_consume();
    node->theta  = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_sq_torus()
{
    this->expect_keyword(RDViewKeywordType_SqTorus, "geometry (expected 'SqTorus')");
    this->consume();

    RDViewNodeSqTorus *node = this->create_node<RDViewNodeSqTorus>();
    node->radius_a  = this->fetch_numerical_and_consume();
    node->radius_b  = this->fetch_numerical_and_consume();
    auto n_token    = this->fetch_type_and_consume(RDViewTokenType_Integer);
    node->n         = (real32_t)n_token.integer.value;
    node->e         = this->fetch_numerical_and_consume();
    node->phi_min   = this->fetch_numerical_and_consume();
    node->phi_max   = this->fetch_numerical_and_consume();
    node->theta_max = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_torus()
{
    this->expect_keyword(RDViewKeywordType_Torus, "geometry (expected 'Torus')");
    this->consume();

    RDViewNodeTorus *node = this->create_node<RDViewNodeTorus>();
    node->radius_a  = this->fetch_numerical_and_consume();
    node->radius_b  = this->fetch_numerical_and_consume();
    node->phi_min   = this->fetch_numerical_and_consume();
    node->phi_max   = this->fetch_numerical_and_consume();
    node->theta_max = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_tube()
{
    this->expect_keyword(RDViewKeywordType_Tube, "geometry (expected 'Tube')");
    this->consume();

    RDViewNodeTube *node = this->create_node<RDViewNodeTube>();
    node->x1     = this->fetch_numerical_and_consume();
    node->y1     = this->fetch_numerical_and_consume();
    node->z1     = this->fetch_numerical_and_consume();
    node->x2     = this->fetch_numerical_and_consume();
    node->y2     = this->fetch_numerical_and_consume();
    node->z2     = this->fetch_numerical_and_consume();
    node->radius = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_subdivision()
{
    auto current_token = this->tokenizer->get_current_token();
    this->consume();
    throw RDViewParserErrorNI(current_token);
    SIMPLEX_NO_IMPLEMENTATION("");
    return nullptr;
}

RDViewNodeInterface* RDViewParser::
match_object_instance()
{
    auto current_token = this->tokenizer->get_current_token();
    this->consume();
    throw RDViewParserErrorNI(current_token);
    SIMPLEX_NO_IMPLEMENTATION("");
    return nullptr;
}

RDViewNodeInterface* RDViewParser::
match_matrix()
{
    this->expect_keyword(RDViewKeywordType_Matrix, "transform (expected 'Matrix')");
    this->consume();

    RDViewNodeMatrix *node = this->create_node<RDViewNodeMatrix>();
    node->values.reserve(16);
    for (int i = 0; i < 16; ++i)
        node->values.push_back(this->fetch_numerical_and_consume());
    return node;
}

RDViewNodeInterface* RDViewParser::
match_rotate()
{
    this->expect_keyword(RDViewKeywordType_Rotate, "transform (expected 'Rotate')");
    this->consume();

    static const std::unordered_map<std::string_view, RDViewRotationAxis> axis_map =
    {
        { "X", RDViewRotationAxis_X },
        { "Y", RDViewRotationAxis_Y },
        { "Z", RDViewRotationAxis_Z },
    };

    auto axis_token = this->fetch_type_and_consume(RDViewTokenType_String);
    auto it = axis_map.find(axis_token.string.value);
    if (it == axis_map.end())
        this->throw_error<RDViewParserErrorICF>(axis_token, "expected rotation axis (\"X\", \"Y\", or \"Z\")");

    RDViewNodeRotate *node = this->create_node<RDViewNodeRotate>();
    node->axis  = it->second;
    node->angle = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_scale()
{
    this->expect_keyword(RDViewKeywordType_Scale, "transform (expected 'Scale')");
    this->consume();

    RDViewNodeScale *node = this->create_node<RDViewNodeScale>();
    node->x = this->fetch_numerical_and_consume();
    node->y = this->fetch_numerical_and_consume();
    node->z = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_translate()
{
    this->expect_keyword(RDViewKeywordType_Translate, "transform (expected 'Translate')");
    this->consume();

    RDViewNodeTranslate *node = this->create_node<RDViewNodeTranslate>();
    node->x = this->fetch_numerical_and_consume();
    node->y = this->fetch_numerical_and_consume();
    node->z = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_xformpush()
{
    this->expect_keyword(RDViewKeywordType_XformPush, "transform (expected 'XformPush')");
    this->consume();
    return this->create_node<RDViewNodeXformPush>();
}

RDViewNodeInterface* RDViewParser::
match_xformpop()
{
    this->expect_keyword(RDViewKeywordType_XformPop, "transform (expected 'XformPop')");
    this->consume();
    return this->create_node<RDViewNodeXformPop>();
}

RDViewNodeInterface* RDViewParser::
match_ambient_light()
{
    this->expect_keyword(RDViewKeywordType_AmbientLight, "lighting (expected 'AmbientLight')");
    this->consume();

    RDViewNodeAmbientLight *node = this->create_node<RDViewNodeAmbientLight>();
    node->r         = this->fetch_numerical_and_consume();
    node->g         = this->fetch_numerical_and_consume();
    node->b         = this->fetch_numerical_and_consume();
    node->intensity = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_far_light()
{
    this->expect_keyword(RDViewKeywordType_FarLight, "lighting (expected 'FarLight')");
    this->consume();

    RDViewNodeFarLight *node = this->create_node<RDViewNodeFarLight>();
    node->l_x       = this->fetch_numerical_and_consume();
    node->l_y       = this->fetch_numerical_and_consume();
    node->l_z       = this->fetch_numerical_and_consume();
    node->r         = this->fetch_numerical_and_consume();
    node->g         = this->fetch_numerical_and_consume();
    node->b         = this->fetch_numerical_and_consume();
    node->intensity = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_point_light()
{
    this->expect_keyword(RDViewKeywordType_PointLight, "lighting (expected 'PointLight')");
    this->consume();

    RDViewNodePointLight *node = this->create_node<RDViewNodePointLight>();
    node->p_x       = this->fetch_numerical_and_consume();
    node->p_y       = this->fetch_numerical_and_consume();
    node->p_z       = this->fetch_numerical_and_consume();
    node->r         = this->fetch_numerical_and_consume();
    node->g         = this->fetch_numerical_and_consume();
    node->b         = this->fetch_numerical_and_consume();
    node->intensity = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_cone_light()
{
    this->expect_keyword(RDViewKeywordType_ConeLight, "lighting (expected 'ConeLight')");
    this->consume();

    RDViewNodeConeLight *node = this->create_node<RDViewNodeConeLight>();
    node->p_x       = this->fetch_numerical_and_consume();
    node->p_y       = this->fetch_numerical_and_consume();
    node->p_z       = this->fetch_numerical_and_consume();
    node->a_x       = this->fetch_numerical_and_consume();
    node->a_y       = this->fetch_numerical_and_consume();
    node->a_z       = this->fetch_numerical_and_consume();
    node->theta_min = this->fetch_numerical_and_consume();
    node->theta_max = this->fetch_numerical_and_consume();
    node->r         = this->fetch_numerical_and_consume();
    node->g         = this->fetch_numerical_and_consume();
    node->b         = this->fetch_numerical_and_consume();
    node->intensity = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_ka()
{
    this->expect_keyword(RDViewKeywordType_Ka, "surface (expected 'Ka')");
    this->consume();

    RDViewNodeKa *node = this->create_node<RDViewNodeKa>();
    node->value = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_kd()
{
    this->expect_keyword(RDViewKeywordType_Kd, "surface (expected 'Kd')");
    this->consume();

    RDViewNodeKd *node = this->create_node<RDViewNodeKd>();
    node->value = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_ks()
{
    this->expect_keyword(RDViewKeywordType_Ks, "surface (expected 'Ks')");
    this->consume();

    RDViewNodeKs *node = this->create_node<RDViewNodeKs>();
    node->value = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_specular()
{
    this->expect_keyword(RDViewKeywordType_Specular, "surface (expected 'Specular')");
    this->consume();

    RDViewNodeSpecular *node = this->create_node<RDViewNodeSpecular>();
    node->r = this->fetch_numerical_and_consume();
    node->g = this->fetch_numerical_and_consume();
    node->b = this->fetch_numerical_and_consume();
    node->n = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_surface()
{
    this->expect_keyword(RDViewKeywordType_Surface, "surface (expected 'Surface')");
    this->consume();

    static const std::unordered_map<std::string_view, RDViewShaderType> shader_map =
    {
        { "Matte",          RDViewShaderType_Matte          },
        { "Metal",          RDViewShaderType_Metal          },
        { "Plastic",        RDViewShaderType_Plastic        },
        { "PaintedPlastic", RDViewShaderType_PaintedPlastic },
    };

    auto shader_token = this->fetch_type_and_consume(RDViewTokenType_String);
    auto it = shader_map.find(shader_token.string.value);
    if (it == shader_map.end())
        this->throw_error<RDViewParserErrorICF>(shader_token, "invalid shader type");

    RDViewNodeSurface *node = this->create_node<RDViewNodeSurface>();
    node->shader_type = it->second;
    return node;
}

RDViewNodeInterface* RDViewParser::
match_map_load()
{
    this->expect_keyword(RDViewKeywordType_MapLoad, "expected 'MapLoad'");
    this->consume();

    auto path_token  = this->fetch_type_and_consume(RDViewTokenType_String);
    auto label_token = this->fetch_type_and_consume(RDViewTokenType_String);

    std::string user_path(path_token.string.value);
    std::filesystem::path canonical_path = std::filesystem::weakly_canonical(user_path);

    RDViewNodeMapLoad *node = this->create_node<RDViewNodeMapLoad>();
    node->input_path    = user_path;
    node->canonical_path = canonical_path;
    node->label         = std::string(label_token.string.value);
    return node;
}

RDViewNodeInterface* RDViewParser::
match_map()
{
    this->expect_keyword(RDViewKeywordType_Map, "expected 'Map'");
    this->consume();

    static const std::unordered_map<std::string_view, RDViewMapType> map_type_map =
    {
        { "none",       RDViewMapType_None       },
        { "TextureMap", RDViewMapType_TextureMap },
        { "BumpMap",    RDViewMapType_BumpMap    },
    };

    auto type_token  = this->fetch_type_and_consume(RDViewTokenType_String);
    auto label_token = this->fetch_type_and_consume(RDViewTokenType_String);

    auto it = map_type_map.find(type_token.string.value);
    if (it == map_type_map.end())
        this->throw_error<RDViewParserErrorICF>(type_token, "invalid map type");

    RDViewNodeMap *node = this->create_node<RDViewNodeMap>();
    node->map_type = it->second;
    node->label    = std::string(label_token.string.value);
    return node;
}

RDViewNodeInterface* RDViewParser::
match_map_sample()
{
    this->expect_keyword(RDViewKeywordType_MapSample, "expected 'MapSample'");
    this->consume();

    static const std::unordered_map<std::string_view, RDViewMapType> map_type_map =
    {
        { "none",       RDViewMapType_None       },
        { "TextureMap", RDViewMapType_TextureMap },
        { "BumpMap",    RDViewMapType_BumpMap    },
    };

    static const std::unordered_map<std::string_view, RDViewMapLevelType> level_type_map =
    {
        { "Nearest", RDViewMapLevelType_Nearest },
        { "Linear",  RDViewMapLevelType_Linear  },
    };

    auto type_token  = this->fetch_type_and_consume(RDViewTokenType_String);
    auto intra_token = this->fetch_type_and_consume(RDViewTokenType_String);
    auto inter_token = this->fetch_type_and_consume(RDViewTokenType_String);

    auto type_it = map_type_map.find(type_token.string.value);
    if (type_it == map_type_map.end())
        this->throw_error<RDViewParserErrorICF>(type_token, "invalid map type");

    auto intra_it = level_type_map.find(intra_token.string.value);
    if (intra_it == level_type_map.end())
        this->throw_error<RDViewParserErrorICF>(intra_token, "invalid sample level type");

    auto inter_it = level_type_map.find(inter_token.string.value);
    if (inter_it == level_type_map.end())
        this->throw_error<RDViewParserErrorICF>(inter_token, "invalid sample level type");

    RDViewNodeMapSample *node = this->create_node<RDViewNodeMapSample>();
    node->map_type         = type_it->second;
    node->intra_level_type = intra_it->second;
    node->inter_level_type = inter_it->second;
    return node;
}

RDViewNodeInterface* RDViewParser::
match_map_bound()
{
    this->expect_keyword(RDViewKeywordType_MapBound, "expected 'MapBound'");
    this->consume();

    static const std::unordered_map<std::string_view, RDViewMapType> map_type_map =
    {
        { "none",       RDViewMapType_None       },
        { "TextureMap", RDViewMapType_TextureMap },
        { "BumpMap",    RDViewMapType_BumpMap    },
    };

    auto type_token = this->fetch_type_and_consume(RDViewTokenType_String);
    auto it = map_type_map.find(type_token.string.value);
    if (it == map_type_map.end())
        this->throw_error<RDViewParserErrorICF>(type_token, "invalid map type");

    RDViewNodeMapBound *node = this->create_node<RDViewNodeMapBound>();
    node->map_type = it->second;
    node->s_min    = this->fetch_numerical_and_consume();
    node->t_min    = this->fetch_numerical_and_consume();
    node->s_max    = this->fetch_numerical_and_consume();
    node->t_max    = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_map_border()
{
    this->expect_keyword(RDViewKeywordType_MapBorder, "expected 'MapBorder'");
    this->consume();

    static const std::unordered_map<std::string_view, RDViewMapType> map_type_map =
    {
        { "none",       RDViewMapType_None       },
        { "TextureMap", RDViewMapType_TextureMap },
        { "BumpMap",    RDViewMapType_BumpMap    },
    };

    static const std::unordered_map<std::string_view, RDViewBorderType> border_type_map =
    {
        { "none",   RDViewMapBorderType_None   },
        { "Clamp",  RDViewMapBorderType_Clamp  },
        { "Repeat", RDViewMapBorderType_Repeat },
    };

    auto type_token  = this->fetch_type_and_consume(RDViewTokenType_String);
    auto horiz_token = this->fetch_type_and_consume(RDViewTokenType_String);
    auto vert_token  = this->fetch_type_and_consume(RDViewTokenType_String);

    auto type_it = map_type_map.find(type_token.string.value);
    if (type_it == map_type_map.end())
        this->throw_error<RDViewParserErrorICF>(type_token, "invalid map type");

    auto horiz_it = border_type_map.find(horiz_token.string.value);
    if (horiz_it == border_type_map.end())
        this->throw_error<RDViewParserErrorICF>(horiz_token, "invalid border type");

    auto vert_it = border_type_map.find(vert_token.string.value);
    if (vert_it == border_type_map.end())
        this->throw_error<RDViewParserErrorICF>(vert_token, "invalid border type");

    RDViewNodeMapBorder *node = this->create_node<RDViewNodeMapBorder>();
    node->map_type                  = type_it->second;
    node->horizontal_border_type    = horiz_it->second;
    node->vertical_border_type      = vert_it->second;
    return node;
}

#include <utils/test_registry.hpp>
#if defined(SIMPLEX_ENABLE_TESTS) && SIMPLEX_ENABLE_TESTS == 1
#include <fstream>
#include <string>
#include <sstream>

struct RDVPT
{
    const char *path;
};

static inline bool _rdvpt(const RDVPT& parameter)
{

    const char *path = parameter.path;
    std::filesystem::path canon_path = std::filesystem::weakly_canonical(path);

    if (!std::filesystem::exists(path)) return false;
    size_t file_size = std::filesystem::file_size(path);
    std::string file_source(file_size, '\0');
    std::ifstream file_stream(path);
    if (!file_stream.is_open()) return false;
    file_stream.read(&file_source[0], file_size);
    file_stream.close();

    std::stringstream parse_results;
    RDViewParser parser(file_source, canon_path, parse_results);
    bool parse_result = parser.match_everything();
    return parse_result;

}

SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s01.rd", _rdvpt, RDVPT, "./tests/rdview/s01.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s02.rd", _rdvpt, RDVPT, "./tests/rdview/s02.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s03.rd", _rdvpt, RDVPT, "./tests/rdview/s03.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s04.rd", _rdvpt, RDVPT, "./tests/rdview/s04.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s05.rd", _rdvpt, RDVPT, "./tests/rdview/s05.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s06.rd", _rdvpt, RDVPT, "./tests/rdview/s06.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s07.rd", _rdvpt, RDVPT, "./tests/rdview/s07.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s08.rd", _rdvpt, RDVPT, "./tests/rdview/s08.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s09.rd", _rdvpt, RDVPT, "./tests/rdview/s09.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s10.rd", _rdvpt, RDVPT, "./tests/rdview/s10.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s11.rd", _rdvpt, RDVPT, "./tests/rdview/s11.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s12.rd", _rdvpt, RDVPT, "./tests/rdview/s12.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s13.rd", _rdvpt, RDVPT, "./tests/rdview/s13.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s14.rd", _rdvpt, RDVPT, "./tests/rdview/s14.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s15.rd", _rdvpt, RDVPT, "./tests/rdview/s15.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s16.rd", _rdvpt, RDVPT, "./tests/rdview/s16.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s17.rd", _rdvpt, RDVPT, "./tests/rdview/s17.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s18.rd", _rdvpt, RDVPT, "./tests/rdview/s18.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s19.rd", _rdvpt, RDVPT, "./tests/rdview/s19.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s20.rd", _rdvpt, RDVPT, "./tests/rdview/s20.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s21.rd", _rdvpt, RDVPT, "./tests/rdview/s21.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s22.rd", _rdvpt, RDVPT, "./tests/rdview/s22.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s23.rd", _rdvpt, RDVPT, "./tests/rdview/s23.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s24.rd", _rdvpt, RDVPT, "./tests/rdview/s24.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s25.rd", _rdvpt, RDVPT, "./tests/rdview/s25.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s26.rd", _rdvpt, RDVPT, "./tests/rdview/s26.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s27.rd", _rdvpt, RDVPT, "./tests/rdview/s27.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s28.rd", _rdvpt, RDVPT, "./tests/rdview/s28.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s29.rd", _rdvpt, RDVPT, "./tests/rdview/s29.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s30.rd", _rdvpt, RDVPT, "./tests/rdview/s30.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s31.rd", _rdvpt, RDVPT, "./tests/rdview/s31.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s32.rd", _rdvpt, RDVPT, "./tests/rdview/s32.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s33.rd", _rdvpt, RDVPT, "./tests/rdview/s33.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s34.rd", _rdvpt, RDVPT, "./tests/rdview/s34.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s35.rd", _rdvpt, RDVPT, "./tests/rdview/s35.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s36.rd", _rdvpt, RDVPT, "./tests/rdview/s36.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s37.rd", _rdvpt, RDVPT, "./tests/rdview/s37.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s38.rd", _rdvpt, RDVPT, "./tests/rdview/s38.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s39.rd", _rdvpt, RDVPT, "./tests/rdview/s39.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s40.rd", _rdvpt, RDVPT, "./tests/rdview/s40.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s41.rd", _rdvpt, RDVPT, "./tests/rdview/s41.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s42.rd", _rdvpt, RDVPT, "./tests/rdview/s42.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s43.rd", _rdvpt, RDVPT, "./tests/rdview/s43.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s44.rd", _rdvpt, RDVPT, "./tests/rdview/s44.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s45.rd", _rdvpt, RDVPT, "./tests/rdview/s45.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s46.rd", _rdvpt, RDVPT, "./tests/rdview/s46.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s47.rd", _rdvpt, RDVPT, "./tests/rdview/s47.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s48.rd", _rdvpt, RDVPT, "./tests/rdview/s48.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s49.rd", _rdvpt, RDVPT, "./tests/rdview/s49.rd");
SIMPLEX_REGISTER_GROUPED_TEST("RDView Parser", "RDView AST Parse s50.rd", _rdvpt, RDVPT, "./tests/rdview/s50.rd");

#endif