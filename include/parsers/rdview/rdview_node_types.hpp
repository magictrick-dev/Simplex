#pragma once
#include <utils/defs.hpp>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

enum RDViewNodeType
{
    RDViewNodeType_NodeInterface,
    RDViewNodeType_Root,
    RDViewNodeType_Body,
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
};

const char *to_string(RDViewNodeType type);

enum RDViewPrimitiveType
{
    RDViewPrimitiveType_Integer,
    RDViewPrimitiveType_Real,
    RDViewPrimitiveType_String,
    RDViewPrimitiveType_Boolean,
};

const char *to_string(RDViewPrimitiveType type);

enum RDViewVertexAttributeType
{
    RDViewVertexAttributeType_Invalid,
    RDViewVertexAttributeType_Position,
    RDViewVertexAttributeType_Direction,
    RDViewVertexAttributeType_Color,
    RDViewVertexAttributeType_Weight,
    RDViewVertexAttributeType_Texture,
    RDViewVertexAttributeType_Opacity,
};

class RDViewNodeVisitor;
class RDViewNodeInterface
{
    public:
        inline          RDViewNodeInterface() { };
        virtual inline ~RDViewNodeInterface() { };

        virtual void    visit(RDViewNodeVisitor *visitor) = 0;

        static size_t get_attribute_size(RDViewVertexAttributeType attribute_type);
        static RDViewVertexAttributeType classify_attribute_type(char c);

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
struct RDViewNodeInclude;
struct RDViewNodeDisplay;
struct RDViewNodeFormat;
struct RDViewNodeObject;
struct RDViewNodeFrame;
struct RDViewNodeWorld;
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

class RDViewNodeVisitor
{
    public:
        virtual void accept(RDViewNodeInterface *node) { SIMPLEX_NO_REACH("Base node should not be traversed!"); }
        virtual void accept(RDViewNodeRoot *node) { };
        virtual void accept(RDViewNodeBody *node) { };
        virtual void accept(RDViewNodeInclude *node) { };
        virtual void accept(RDViewNodeDisplay *node) { };
        virtual void accept(RDViewNodeFormat *node) { };
        virtual void accept(RDViewNodeObject *node) { };
        virtual void accept(RDViewNodeFrame *node) { };
        virtual void accept(RDViewNodeWorld *node) { };
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
    RDViewDisplayType_Invalid,  // Invalid type.
    RDViewDisplayType_Screen,   // Screen via GUI.
    RDViewDisplayType_PNM,      // Output directly as PNM.
    RDViewDisplayType_BMP,      // Output directly as BMP.
    RDViewDisplayType_PNG,      // Output directly as PNG.
};

enum RDViewModeType
{
    RDViewModeType_Invalid,     // Invalid type.
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

        static RDViewDisplayType map_display_type(std::string_view parameter);
        static RDViewModeType map_mode_type(std::string_view parameter);

        std::string name;
        RDViewDisplayType format;
        RDViewModeType mode;

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
        std::vector<int32_t> index_values;
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
