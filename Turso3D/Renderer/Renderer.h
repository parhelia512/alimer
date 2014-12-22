// For conditions of distribution and use, see copyright notice in License.txt

#pragma once

#include "../Base/AutoPtr.h"
#include "../Graphics/GraphicsDefs.h"
#include "../Math/Frustum.h"
#include "../Object/Object.h"

namespace Turso3D
{

class Camera;
class ConstantBuffer;
class Light;
class Octree;
class Scene;
class VertexBuffer;
struct SourceBatch;

typedef Vector<Light*> LightQueue;

/// Shader constant buffers used by high-level rendering.
enum RendererConstantBuffer
{
    CB_FRAME = 0,
    CB_OBJECT,
    CB_MATERIAL
};

/// Batch sorting modes.
enum BatchSortMode
{
    SORT_STATE = 0,
    SORT_BACKTOFRONT
};

/// Parameter indices in constant buffers used by high-level rendering.
static const size_t VS_SCENE_VIEW_MATRIX = 0;
static const size_t VS_SCENE_PROJECTION_MATRIX = 1;
static const size_t VS_SCENE_VIEWPROJ_MATRIX = 2;
static const size_t VS_OBJECT_WORLD_MATRIX = 0;

static const unsigned char INSTANCE_TEXCOORD = 4;

/// Description of a render-time draw call.
struct Batch
{
    /// Pointer to source data.
    const SourceBatch* source;
    /// Material pass.
    Pass* pass;
    /// Pointer to light queue.
    LightQueue* lights;
    /// Shader variations.
    ShaderVariation* shaders[MAX_SHADER_STAGES];
    /// Instance count. If non-zero, the batches following this (which must have same pass, light queue & shaders) will be drawn instanced.
    unsigned instanceCount;
    /// Instance buffer start index.
    unsigned instanceStart;

    union
    {
        /// Sort key for state sorting.
        unsigned long long sortKey;
        /// Distance for sorting.
        float distance;
    };

    /// Calculate sort key from state.
    void CalculateSortKey();
};

/// High-level rendering subsystem. Performs rendering of 3D scenes.
class Renderer : public Object
{
    OBJECT(Renderer);

public:
    /// Construct and register subsystem. 
    Renderer();
    /// Destruct.
    ~Renderer();

    /// Initialize rendering of a new view and collect visible objects from the camera's point of view.
    void CollectObjects(Scene* scene, Camera* camera);
    /// Collect and sort batches from a pass.
    void CollectBatches(const String& pass, BatchSortMode sort = SORT_STATE);
    /// Draw the collected batches.
    void DrawBatches(const String& pass);

private:
    /// Collect batches and setup sorting by state.
    void CollectBatchesByState(size_t passIndex, Vector<Batch>& batchQueue);
    /// Collect batches and setup sorting by distance.
    void CollectBatchesByDistance(size_t passIndex, Vector<Batch>& batchQueue);
    /// Load shaders for a pass.
    void LoadPassShaders(Pass* pass);
    /// Set shader variations for a batch after the final geometry type is known.
    void SetBatchShaders(Batch& batch);

    /// Current scene.
    Scene* scene;
    /// Current camera.
    Camera* camera;
    /// Current octree.
    Octree* octree;
    /// Camera's view frustum.
    Frustum frustum;
    /// Camera's view matrix.
    Matrix3x4 viewMatrix;
    /// Camera's projection matrix.
    Matrix4 projectionMatrix;
    /// Combined view-projection matrix.
    Matrix4 viewProjMatrix;
    /// Found geometry objects.
    Vector<GeometryNode*> geometries;
    /// Batches per pass.
    HashMap<String, Vector<Batch> > batches;
    /// Sorted batches per pass.
    HashMap<String, Vector<Batch*> > sortedBatches;
    /// Per-frame vertex shader constant buffer.
    AutoPtr<ConstantBuffer> vsFrameConstantBuffer;
    /// Per-fame pixel shader constant buffer.
    AutoPtr<ConstantBuffer> psFrameConstantBuffer;
    /// Per-object vertex shader constant buffer.
    AutoPtr<ConstantBuffer> vsObjectConstantBuffer;
    /// Instance transform vertex buffer.
    AutoPtr<VertexBuffer> instanceVertexBuffer;
    /// Vertex elements for the instance vertex buffer.
    Vector<VertexElement> instanceVertexElements;
    /// Instance transforms for uploading to the instnce vertex buffer.
    Vector<Matrix3x4> instanceTransforms;
};

/// Register Renderer related object factories and attributes.
TURSO3D_API void RegisterRendererLibrary();

}

