#pragma once
#include <string>
#include <sstream>
#include <parsers/rdview/rdview_node_types.hpp>

class RDViewReferenceVisitor : public RDViewNodeVisitor
{

    private:
        int32_t tabs = 0;
        int32_t tab_size = 4;
        std::stringstream output;

        inline void 
        push_tabs() 
        { 
            this->tabs += this->tab_size; 
        }

        inline void 
        pop_tabs() 
        { 
            this->tabs -= this->tab_size; SIMPLEX_ASSERT(this->tabs >= 0); 
        }

        inline void 
        print_tabs() 
        { 
            for (int32_t i = 0; i < tabs; ++i) 
            { 
                output << " "; 
            }
        }

    public:
        inline std::string_view get_output() const
        {
            return this->output.view();
        }

    public:
        inline virtual void 
        accept(RDViewNodeRoot *node) override 
        { 
            output << "# This was reference-generated using from an existing script.\n";
            output << "# The output may not reflect the original script's format.\n\n";
            node->display->visit(this);
            node->format->visit(this);

            output << "\n";
            node->body->visit(this);
        };

        inline virtual void 
        accept(RDViewNodeBody *node) override 
        { 
            for (auto n : node->children) n->visit(this);
        };

        inline virtual void 
        accept(RDViewNodeInclude *node) override 
        { 
            // TODO(Chris): We haven't supported includes yet, so we don't care to print it.
            SIMPLEX_NO_IMPLEMENTATION("");
        };

        inline virtual void 
        accept(RDViewNodeDisplay *node) override 
        { 

            this->print_tabs();
            output << "Display " << "\"" << node->name << "\" ";

            switch (node->format)
            {
                case RDViewDisplayType_Invalid:     { output << "\"Invalid\" ";     } break;
                case RDViewDisplayType_Screen:      { output << "\"Screen\" ";      } break;
                case RDViewDisplayType_PNM:         { output << "\"PNM\" ";         } break;
                case RDViewDisplayType_PNG:         { output << "\"PNG\" ";         } break;
                default: SIMPLEX_NO_REACH("Oops, not implemented!");
            }

            switch (node->mode)
            {
                case RDViewModeType_Invalid:        { output << "\"Invalid\"";      } break;
                case RDViewModeType_RGB:            { output << "\"RGB\"";          } break;
                case RDViewModeType_RGBSingle:      { output << "\"RGBSingle\"";    } break;
                case RDViewModeType_RGBDouble:      { output << "\"RGBDouble\"";    } break;
                case RDViewModeType_RGBObject:      { output << "\"RGBObject\"";    } break;
                default: SIMPLEX_NO_REACH("Oops, not implemented!");
            }

            output << "\n";

        };

        inline virtual void 
        accept(RDViewNodeFormat *node) override 
        { 
            this->print_tabs();
            output << "Format " << node->width << " " << node->height << "\n";
        };

        inline virtual void 
        accept(RDViewNodeObject *node) override 
        { 
            this->print_tabs();
            output << "ObjectBegin " << "\"" << node->name << "\"\n";

            this->push_tabs();
                for (auto child : node->children) child->visit(this);
            this->pop_tabs();

            this->print_tabs();
            output << "ObjectEnd # " << node->name << "\n";
        };

        inline virtual void 
        accept(RDViewNodeFrame *node) override 
        { 

            output << "\n";
            this->print_tabs();
            output << "FrameBegin " << node->frame_number << "\n";

            this->push_tabs();
                for (auto n : node->children) n->visit(this);
                node->world->visit(this);
            this->pop_tabs();

            this->print_tabs();
            output << "FrameEnd" << "\n";

        };

        inline virtual void 
        accept(RDViewNodeWorld *node) override 
        { 
            output << "\n";
            this->print_tabs();
            output << "WorldBegin" << "\n";

            this->push_tabs();
                for (auto n : node->children) n->visit(this);
            this->pop_tabs();

            this->print_tabs();
            output << "WorldEnd" << "\n";
        };

        inline virtual void
        accept(RDViewNodeOptionArray *node) override
        {
            this->print_tabs();
            output << "OptionArray \"" << node->name << "\" " << node->values.size();
            for (auto v : node->values) output << " " << v;
            output << "\n";
        };

        inline virtual void
        accept(RDViewNodeOptionBool *node) override
        {
            this->print_tabs();
            output << "OptionBool \"" << node->name << "\" " << node->value << "\n";
        };

        inline virtual void
        accept(RDViewNodeOptionList *node) override
        {
            this->print_tabs();
            output << "OptionList \"" << node->name << "\" " << node->values.size();
            for (auto &v : node->values) output << " \"" << v << "\"";
            output << "\n";
        };

        inline virtual void
        accept(RDViewNodeOptionReal *node) override
        {
            this->print_tabs();
            output << "OptionReal \"" << node->name << "\" " << node->value << "\n";
        };

        inline virtual void
        accept(RDViewNodeOptionString *node) override
        {
            this->print_tabs();
            output << "OptionString \"" << node->name << "\" \"" << node->value << "\"\n";
        };

        inline virtual void
        accept(RDViewNodeBackground *node) override
        {
            this->print_tabs();
            output << "Background " << node->red << " " << node->green << " " << node->blue << "\n";
        };

        inline virtual void
        accept(RDViewNodeColor *node) override
        {
            this->print_tabs();
            output << "Color " << node->red << " " << node->green << " " << node->blue << "\n";
        };

        inline virtual void
        accept(RDViewNodeOpacity *node) override
        {
            this->print_tabs();
            output << "Opacity " << node->opacity << "\n";
        };

        inline virtual void
        accept(RDViewNodeCameraAt *node) override
        {
            this->print_tabs();
            output << "CameraAt " << node->x << " " << node->y << " " << node->z << "\n";
        };

        inline virtual void
        accept(RDViewNodeCameraEye *node) override
        {
            this->print_tabs();
            output << "CameraEye " << node->x << " " << node->y << " " << node->z << "\n";
        };

        inline virtual void
        accept(RDViewNodeCameraFOV *node) override
        {
            this->print_tabs();
            output << "CameraFOV " << node->FOV << "\n";
        };

        inline virtual void
        accept(RDViewNodeCameraUp *node) override
        {
            this->print_tabs();
            output << "CameraUp " << node->x << " " << node->y << " " << node->z << "\n";
        };

        inline virtual void
        accept(RDViewNodeClipping *node) override
        {
            this->print_tabs();
            output << "Clipping " << node->near << " " << node->far << "\n";
        };

        inline virtual void
        accept(RDViewNodePoint *node) override
        {
            this->print_tabs();
            output << "Point " << node->x << " " << node->y << " " << node->z << "\n";
        };

        inline virtual void
        accept(RDViewNodePointSet *node) override
        {
            this->print_tabs();
            output << "PointSet \"" << node->format << "\" " << node->vertices.size();
            for (auto v : node->vertices) output << " " << v;
            output << "\n";
        };

        inline virtual void
        accept(RDViewNodeLine *node) override
        {
            this->print_tabs();
            output << "Line " << node->x1 << " " << node->y1 << " " << node->z1 << " "
                              << node->x2 << " " << node->y2 << " " << node->z2 << "\n";
        };

        inline virtual void
        accept(RDViewNodeLineSet *node) override
        {
            this->print_tabs();
            output << "LineSet \"" << node->format << "\" " 
                << node->vertex_values.size() << " " << node->index_values.size();
            for (auto v : node->vertex_values) output << " " << v;
            for (auto iv : node->index_values) 
                for (auto i : iv) output << " " << i;
            output << "\n";
        };

        inline virtual void
        accept(RDViewNodeCircle *node) override
        {
            this->print_tabs();
            output << "Circle " << node->x << " " << node->y << " " << node->z << " " << node->radius << "\n";
        };

        inline virtual void
        accept(RDViewNodeFill *node) override
        {
            this->print_tabs();
            output << "Fill " << node->x << " " << node->y << " " << node->z << "\n";
        };

        inline virtual void
        accept(RDViewNodeCone *node) override
        {
            this->print_tabs();
            output << "Cone " << node->height << " " << node->radius << " " << node->theta << "\n";
        };

        inline virtual void
        accept(RDViewNodeCube *node) override
        {
            this->print_tabs();
            output << "Cube\n";
        };

        inline virtual void
        accept(RDViewNodeCurve *node) override
        {
            this->print_tabs();
            output << "Curve \"Bezier\" \"" << node->format << "\" " << node->degree;
            for (auto v : node->vertices) output << " " << v;
            output << "\n";
        };

        inline virtual void
        accept(RDViewNodeCylinder *node) override
        {
            this->print_tabs();
            output << "Cylinder " << node->radius << " " << node->z_min << " " << node->z_max << " " << node->theta << "\n";
        };

        inline virtual void
        accept(RDViewNodeDisk *node) override
        {
            this->print_tabs();
            output << "Disk " << node->height << " " << node->radius << " " << node->theta << "\n";
        };

        inline virtual void
        accept(RDViewNodeHyperboloid *node) override
        {
            this->print_tabs();
            output << "Hyperboloid " << node->x1 << " " << node->y1 << " " << node->z1 << " "
                                     << node->x2 << " " << node->y2 << " " << node->z2 << " "
                                     << node->theta << "\n";
        };

        inline virtual void
        accept(RDViewNodeParaboloid *node) override
        {
            this->print_tabs();
            output << "Paraboloid " << node->radius << " " << node->z_min << " " << node->z_max << " " << node->theta << "\n";
        };

        inline virtual void
        accept(RDViewNodePatch *node) override
        {
            this->print_tabs();
            output << "Patch ";
            switch (node->patch_type)
            {
                case RDViewPatchType_Invalid: { SIMPLEX_NO_REACH("Invalid patch type encountered!");  } break;
                case RDViewPatchType_Bezier:  { output << "\"Bezier\"";                               } break;
                default: SIMPLEX_NO_REACH("Oops, not implemented!");
            }
            output << " \"" << node->format << "\" " << node->degree_m << " " << node->degree_n;
            for (auto v : node->vertices) output << " " << v;
            output << "\n";
        };

        inline virtual void
        accept(RDViewNodePolySet *node) override
        {
            this->print_tabs();
            output << "PolySet \"" << node->format << "\" " 
                << node->vertex_values.size() << " " << node->index_values.size();
            for (auto v : node->vertex_values) output << " " << v;
            for (auto iv : node->index_values) 
                for (auto i : iv) output << " " << i;
            output << "\n";
        };

        inline virtual void
        accept(RDViewNodeSphere *node) override
        {
            this->print_tabs();
            output << "Sphere " << node->radius << " " << node->z_min << " " << node->z_max << " " << node->theta << "\n";
        };

        inline virtual void
        accept(RDViewNodeSqSphere *node) override
        {
            this->print_tabs();
            output << "SqSphere " << node->radius << " " << node->n << " " << node->e << " "
                                  << node->z_min << " " << node->z_max << " " << node->theta << "\n";
        };

        inline virtual void
        accept(RDViewNodeSqTorus *node) override
        {
            this->print_tabs();
            output << "SqTorus " << node->radius_a << " " << node->radius_b << " "
                                 << node->n << " " << node->e << " "
                                 << node->phi_min << " " << node->phi_max << " " << node->theta_max << "\n";
        };

        inline virtual void
        accept(RDViewNodeTorus *node) override
        {
            this->print_tabs();
            output << "Torus " << node->radius_a << " " << node->radius_b << " "
                               << node->phi_min << " " << node->phi_max << " " << node->theta_max << "\n";
        };

        inline virtual void
        accept(RDViewNodeTube *node) override
        {
            this->print_tabs();
            output << "Tube " << node->x1 << " " << node->y1 << " " << node->z1 << " "
                              << node->x2 << " " << node->y2 << " " << node->z2 << " "
                              << node->radius << "\n";
        };

        inline virtual void
        accept(RDViewNodeSubdivision *node) override
        {
            this->print_tabs();
            output << "Subdivision\n";
        };

        inline virtual void
        accept(RDViewNodeObjectInstance *node) override
        {
            this->print_tabs();
            output << "ObjectInstance \"" << node->name << "\"";
            for (auto p : node->parameters) output << " " << p;
            output << "\n";
        };

        inline virtual void
        accept(RDViewNodeMatrix *node) override
        {
            this->print_tabs();
            output << "Matrix";
            for (auto v : node->values) output << " " << v;
            output << "\n";
        };

        inline virtual void
        accept(RDViewNodeRotate *node) override
        {
            this->print_tabs();
            output << "Rotate ";
            switch (node->axis)
            {
                case RDViewRotationAxis_X: { output << "\"X\""; } break;
                case RDViewRotationAxis_Y: { output << "\"Y\""; } break;
                case RDViewRotationAxis_Z: { output << "\"Z\""; } break;
                default: SIMPLEX_NO_REACH("Oops, not implemented!");
            }
            output << " " << node->angle << "\n";
        };

        inline virtual void
        accept(RDViewNodeScale *node) override
        {
            this->print_tabs();
            output << "Scale " << node->x << " " << node->y << " " << node->z << "\n";
        };

        inline virtual void
        accept(RDViewNodeTranslate *node) override
        {
            this->print_tabs();
            output << "Translate " << node->x << " " << node->y << " " << node->z << "\n";
        };

        inline virtual void
        accept(RDViewNodeXformPush *node) override
        {
            this->print_tabs();
            output << "XformPush\n";
        };

        inline virtual void
        accept(RDViewNodeXformPop *node) override
        {
            this->print_tabs();
            output << "XformPop\n";
        };

        inline virtual void
        accept(RDViewNodeAmbientLight *node) override
        {
            this->print_tabs();
            output << "AmbientLight " << node->r << " " << node->g << " " << node->b << " " << node->intensity << "\n";
        };

        inline virtual void
        accept(RDViewNodeFarLight *node) override
        {
            this->print_tabs();
            output << "FarLight " << node->l_x << " " << node->l_y << " " << node->l_z << " "
                                  << node->r << " " << node->g << " " << node->b << " "
                                  << node->intensity << "\n";
        };

        inline virtual void
        accept(RDViewNodePointLight *node) override
        {
            this->print_tabs();
            output << "PointLight " << node->p_x << " " << node->p_y << " " << node->p_z << " "
                                    << node->r << " " << node->g << " " << node->b << " "
                                    << node->intensity << "\n";
        };

        inline virtual void
        accept(RDViewNodeConeLight *node) override
        {
            this->print_tabs();
            output << "ConeLight " << node->p_x << " " << node->p_y << " " << node->p_z << " "
                                   << node->a_x << " " << node->a_y << " " << node->a_z << " "
                                   << node->theta_min << " " << node->theta_max << " "
                                   << node->r << " " << node->g << " " << node->b << " "
                                   << node->intensity << "\n";
        };

        inline virtual void
        accept(RDViewNodeKa *node) override
        {
            this->print_tabs();
            output << "Ka " << node->value << "\n";
        };

        inline virtual void
        accept(RDViewNodeKd *node) override
        {
            this->print_tabs();
            output << "Kd " << node->value << "\n";
        };

        inline virtual void
        accept(RDViewNodeKs *node) override
        {
            this->print_tabs();
            output << "Ks " << node->value << "\n";
        };

        inline virtual void
        accept(RDViewNodeSpecular *node) override
        {
            this->print_tabs();
            output << "Specular " << node->r << " " << node->g << " " << node->b << " " << node->n << "\n";
        };

        inline virtual void
        accept(RDViewNodeSurface *node) override
        {
            this->print_tabs();
            output << "Surface ";
            switch (node->shader_type)
            {
                case RDViewShaderType_Matte:            { output << "\"matte\"";            } break;
                case RDViewShaderType_Metal:            { output << "\"metal\"";            } break;
                case RDViewShaderType_Plastic:          { output << "\"plastic\"";          } break;
                case RDViewShaderType_PaintedPlastic:   { output << "\"paintedplastic\"";   } break;
                default: SIMPLEX_NO_REACH("Oops, not implemented!");
            }
            output << "\n";
        };

        inline virtual void
        accept(RDViewNodeMapLoad *node) override
        {
            this->print_tabs();
            output << "MapLoad \"" << node->input_path << "\" \"" << node->label << "\"\n";
        };

        inline virtual void
        accept(RDViewNodeMap *node) override
        {
            this->print_tabs();
            output << "Map ";
            switch (node->map_type)
            {
                case RDViewMapType_None:        { output << "\"none\"";    } break;
                case RDViewMapType_TextureMap:  { output << "\"texture\""; } break;
                case RDViewMapType_BumpMap:     { output << "\"bump\"";    } break;
                default: SIMPLEX_NO_REACH("Oops, not implemented!");
            }
            output << " \"" << node->label << "\"\n";
        };

        inline virtual void
        accept(RDViewNodeMapSample *node) override
        {
            this->print_tabs();
            output << "MapSample ";
            switch (node->map_type)
            {
                case RDViewMapType_None:        { output << "\"none\"";    } break;
                case RDViewMapType_TextureMap:  { output << "\"texture\""; } break;
                case RDViewMapType_BumpMap:     { output << "\"bump\"";    } break;
                default: SIMPLEX_NO_REACH("Oops, not implemented!");
            }
            output << " ";
            switch (node->intra_level_type)
            {
                case RDViewMapLevelType_Nearest: { output << "\"nearest\""; } break;
                case RDViewMapLevelType_Linear:  { output << "\"linear\"";  } break;
                default: SIMPLEX_NO_REACH("Oops, not implemented!");
            }
            output << " ";
            switch (node->inter_level_type)
            {
                case RDViewMapLevelType_Nearest: { output << "\"nearest\""; } break;
                case RDViewMapLevelType_Linear:  { output << "\"linear\"";  } break;
                default: SIMPLEX_NO_REACH("Oops, not implemented!");
            }
            output << "\n";
        };

        inline virtual void
        accept(RDViewNodeMapBound *node) override
        {
            this->print_tabs();
            output << "MapBound ";
            switch (node->map_type)
            {
                case RDViewMapType_None:        { output << "\"none\"";    } break;
                case RDViewMapType_TextureMap:  { output << "\"texture\""; } break;
                case RDViewMapType_BumpMap:     { output << "\"bump\"";    } break;
                default: SIMPLEX_NO_REACH("Oops, not implemented!");
            }
            output << " " << node->s_min << " " << node->t_min << " "
                          << node->s_max << " " << node->t_max << "\n";
        };

        inline virtual void
        accept(RDViewNodeMapBorder *node) override
        {
            this->print_tabs();
            output << "MapBorder ";
            switch (node->map_type)
            {
                case RDViewMapType_None:        { output << "\"none\"";    } break;
                case RDViewMapType_TextureMap:  { output << "\"texture\""; } break;
                case RDViewMapType_BumpMap:     { output << "\"bump\"";    } break;
                default: SIMPLEX_NO_REACH("Oops, not implemented!");
            }
            output << " ";
            switch (node->horizontal_border_type)
            {
                case RDViewMapBorderType_None:   { output << "\"none\"";   } break;
                case RDViewMapBorderType_Clamp:  { output << "\"clamp\"";  } break;
                case RDViewMapBorderType_Repeat: { output << "\"repeat\""; } break;
                default: SIMPLEX_NO_REACH("Oops, not implemented!");
            }
            output << " ";
            switch (node->vertical_border_type)
            {
                case RDViewMapBorderType_None:   { output << "\"none\"";   } break;
                case RDViewMapBorderType_Clamp:  { output << "\"clamp\"";  } break;
                case RDViewMapBorderType_Repeat: { output << "\"repeat\""; } break;
                default: SIMPLEX_NO_REACH("Oops, not implemented!");
            }
            output << "\n";
        };

};
