#ifndef GGPUDRIVER_H
#define GGPUDRIVER_H

#include <Ultralight/Ultralight.h>
#include <Ultralight/platform/GPUDriver.h>

namespace ultralight
{
    class GodotGPUDriver : public GPUDriver
    {
    public:
        GodotGPUDriver() { }
        ~GodotGPUDriver() { }
        void BeginSynchronize() override {}
        void EndSynchronize() override {}
        uint32_t NextTextureId() override { return 0; }
        void CreateTexture(uint32_t texture_id, RefPtr<Bitmap> bitmap) override {}
        void UpdateTexture(uint32_t texture_id, RefPtr<Bitmap> bitmap) override {}
        void DestroyTexture(uint32_t texture_id) override {}
        uint32_t NextRenderBufferId() override { return 0; }
        void CreateRenderBuffer(uint32_t render_buffer_id, const RenderBuffer &buffer) override {}
        void DestroyRenderBuffer(uint32_t render_buffer_id) override {}
        uint32_t NextGeometryId() override { return 0; }
        void CreateGeometry(uint32_t geometry_id, const VertexBuffer &vertices, const IndexBuffer &indices) override {}
        void UpdateGeometry(uint32_t geometry_id, const VertexBuffer &vertices, const IndexBuffer &indices) override {}
        void DestroyGeometry(uint32_t geometry_id) override {}
        void UpdateCommandList(const CommandList &list) override {}
    };
};

#endif