#include <parsers/rdview/rdview_node_types.hpp>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <string>

const char *
to_string(RDViewNodeType type)
{
    switch (type)
    {
        case RDViewNodeType_NodeInterface:      { return "RDViewNodeType_NodeInterface";     } break;
        case RDViewNodeType_Root:               { return "RDViewNodeType_Root";              } break;
        case RDViewNodeType_Body:               { return "RDViewNodeType_Body";              } break;
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
    }

    SIMPLEX_NO_REACH("");
    return "";
}

const char *
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

size_t RDViewNodeInterface::
get_attribute_size(RDViewVertexAttributeType attribute_type)
{
    switch (attribute_type)
    {
        case RDViewVertexAttributeType_Invalid:     { return 0; } break;
        case RDViewVertexAttributeType_Position:    { return 3; } break;
        case RDViewVertexAttributeType_Direction:   { return 3; } break;
        case RDViewVertexAttributeType_Color:       { return 3; } break;
        case RDViewVertexAttributeType_Weight:      { return 1; } break;
        case RDViewVertexAttributeType_Texture:     { return 2; } break;
        case RDViewVertexAttributeType_Opacity:     { return 1; } break;
    }

    return 0;
}

RDViewVertexAttributeType RDViewNodeInterface::
classify_attribute_type(char c)
{
    switch(c)
    {
        case 'P': { return RDViewVertexAttributeType_Position; } break;
        case 'D':
        case 'N': { return RDViewVertexAttributeType_Direction; } break;
        case 'C': { return RDViewVertexAttributeType_Color; } break;
        case 'W': { return RDViewVertexAttributeType_Weight; } break;
        case 'T': { return RDViewVertexAttributeType_Texture; } break;
        case 'O': { return RDViewVertexAttributeType_Opacity; } break;
    }

    return RDViewVertexAttributeType_Invalid;
}

RDViewDisplayType RDViewNodeDisplay::
map_display_type(std::string_view parameter)
{
    static const std::unordered_map<std::string_view, RDViewDisplayType> map =
    {
        { "Screen",     RDViewDisplayType_Screen    },
        { "PBM",        RDViewDisplayType_PNM       },
        { "PNM",        RDViewDisplayType_PNM       },
        { "BMP",        RDViewDisplayType_BMP       },
        { "PNG",        RDViewDisplayType_PNG       },
    };

    const auto result = map.find(parameter);
    if (result == map.end()) return RDViewDisplayType_Invalid;
    return result->second;
}

RDViewModeType RDViewNodeDisplay::
map_mode_type(std::string_view parameter)
{

    static const std::unordered_map<std::string_view, RDViewModeType> map =
    {
        { "rgb",            RDViewModeType_RGB          },
        { "rgbsingle",      RDViewModeType_RGBSingle    },
        { "rgbobject",      RDViewModeType_RGBObject    },
        { "rgbdouble",      RDViewModeType_RGBDouble    },
    };

    // Really, no dedicated string tolower method?
    std::string input(parameter);
    std::transform(input.begin(), input.end(), input.begin(), [](char c) { return std::tolower(c); });
    const auto result = map.find(input);
    if (result == map.end()) return RDViewModeType_Invalid;
    return result->second;

}
