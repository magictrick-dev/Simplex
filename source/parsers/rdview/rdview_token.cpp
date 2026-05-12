#include <parsers/rdview/rdview_token.hpp>
#include <iostream>

const char *
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

const char *
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

std::ostream&
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
