//
// Alimer is based on the Turso3D codebase.
// Copyright (c) 2018 Amer Koleci and contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#include "../Math/Color.h"
#include "../Math/Size.h"
#include "../Object/Object.h"
#include "../Graphics/GraphicsDefs.h"
#include <mutex>

namespace Alimer
{
	struct GraphicsImpl;
	class BlendState;
	class ConstantBuffer;
	class DepthState;
	class GPUObject;
	class IndexBuffer;
	class RasterizerState;
	class ShaderVariation;
	class Texture;
	class VertexBuffer;
	class Window;

	// Handles.
	class BufferHandle;
	class TextureHandle;

	struct GraphicsSettings {
		Window*			window;
		bool			verticalSync{ false };
		uint32_t		multisample{ 1 };
		//PixelFormat		depthStencilFormat = PixelFormat::Undefined;
	};

	/// Screen mode set event.
	class ScreenModeEvent : public Event
	{
	public:
		/// New backbuffer size.
		Size size;
		/// Fullscreen flag.
		bool fullscreen;
		/// Window resizable flag.
		bool resizable;
		/// Multisample level.
		int multisample;
	};

	/// 3D graphics rendering context. Manages the rendering window and GPU objects.
	class ALIMER_API Graphics : public Object
	{
		ALIMER_OBJECT(Graphics, Object);

		friend class Buffer;

	protected:
		Graphics(GraphicsDeviceType deviceType, bool validation);

	public:
		/**
		* Gets a set of all available backend drivers.
		*/
		static std::vector<GraphicsDeviceType> GetAvailableDrivers();

		/**
		* Checks if given backend is supported.
		*
		* @param deviceType The graphics backend type to check
		* @return True if supported, false otherwise.
		*/
		static bool IsBackendSupported(GraphicsDeviceType deviceType);

		/**
		* Create graphics backend.
		*
		* @param deviceType The graphics device type.
		* @param validation Whether to enable debug/validation layer.
		* @param applicationName Optional application name.
		* @return Graphics instance or null if failed.
		*/
		static Graphics* Create(GraphicsDeviceType deviceType, bool validation = false, const std::string& applicationName = "Alimer");

		/// Destruct. Clean up the window, rendering context and GPU objects.
		~Graphics();

		/// Initializes the graphics.
		virtual bool Initialize(const GraphicsSettings& settings) = 0;

		/// Set new multisample level while retaining previous resolution. The initial graphics mode must have been set first. Return true on success.
		bool SetMultisample(uint32_t multisample);
		/// Set vertical sync on/off.
		void SetVSync(bool enable);

		/// Begin frame rendering
		virtual bool BeginFrame() = 0;
		/// Present the contents of the backbuffer.
		virtual void Present() = 0;
		/// Set the color rendertarget and depth stencil buffer.
		void SetRenderTarget(Texture* renderTarget, Texture* stencilBuffer);
		/// Set multiple color rendertargets and the depth stencil buffer.
		virtual void SetRenderTargets(const std::vector<Texture*>& renderTargets, Texture* stencilBuffer) = 0;
		/// Set the viewport rectangle. On window resize the viewport will automatically revert to the entire backbuffer.
		virtual void SetViewport(const IntRect& viewport) = 0;
		/// Bind a vertex buffer.
		virtual void SetVertexBuffer(
			uint32_t index,
			VertexBuffer* buffer,
			uint32_t vertexOffset = 0,
			VertexInputRate stepRate = VertexInputRate::Vertex) = 0;

		/// Bind an index buffer.
		void SetIndexBuffer(IndexBuffer* buffer);
		/// Bind a constant buffer.
		virtual void SetConstantBuffer(ShaderStage stage, uint32_t index, ConstantBuffer* buffer) = 0;
		/// Bind a texture.
		virtual void SetTexture(size_t index, Texture* texture) = 0;
		/// Bind vertex and pixel shaders.
		virtual void SetShaders(ShaderVariation* vs, ShaderVariation* ps) = 0;
		/// Set color write and blending related state using an arbitrary blend mode.
		void SetColorState(const BlendModeDesc& blendMode, bool alphaToCoverage = false, unsigned char colorWriteMask = COLORMASK_ALL);
		/// Set color write and blending related state using a predefined blend mode.
		void SetColorState(BlendMode blendMode, bool alphaToCoverage = false, unsigned char colorWriteMask = COLORMASK_ALL);
		/// Set depth buffer related state.
		void SetDepthState(CompareFunc depthFunc, bool depthWrite, bool depthClip = true, int depthBias = 0, float slopeScaledDepthBias = 0.0f);
		/// Set rasterizer related state.
		void SetRasterizerState(CullMode cullMode, FillMode fillMode);
		/// Set scissor test.
		virtual void SetScissorTest(bool scissorEnable, const IntRect& scissorRect = IntRect::ZERO) = 0;
		/// Set stencil test.
		void SetStencilTest(bool stencilEnable, const StencilTestDesc& stencilTest = StencilTestDesc(), unsigned char stencilRef = 0);
		/// Reset rendertarget and depth stencil buffer to the backbuffer.
		void ResetRenderTargets();
		/// Set the viewport to the entire rendertarget or backbuffer.
		void ResetViewport();
		/// Reset all bound vertex buffers.
		void ResetVertexBuffers();
		/// Reset all bound constant buffers.
		void ResetConstantBuffers();
		/// Reset all bound textures.
		void ResetTextures();
		/// Clear the current rendertarget. This is not affected by the defined viewport, but will always clear the whole target.
		virtual void Clear(ClearFlags clearFlags, const Color& clearColor = Color::BLACK, float clearDepth = 1.0f, uint8_t clearStencil = 0) = 0;
		/// Draw non-indexed geometry.
		virtual void Draw(PrimitiveType type, uint32_t vertexStart, uint32_t vertexCount) = 0;
		/// Draw indexed geometry.
		virtual void DrawIndexed(PrimitiveType type, uint32_t indexStart, uint32_t indexCount, uint32_t vertexStart) = 0;
		/// Draw instanced non-indexed geometry.
		virtual void DrawInstanced(PrimitiveType type, uint32_t vertexStart, uint32_t vertexCount, uint32_t instanceStart, uint32_t instanceCount) = 0;
		/// Draw instanced indexed geometry.
		virtual void DrawIndexedInstanced(PrimitiveType type, uint32_t indexStart, uint32_t indexCount, uint32_t vertexStart, uint32_t instanceStart, uint32_t instanceCount) = 0;

		/// Return whether has the rendering window and context.
		bool IsInitialized() const;
		/// Return backbuffer size, or 0,0 if not initialized.
		const Size& GetSize() const { return _backbufferSize; }
		/// Return backbuffer width, or 0 if not initialized.
		uint32_t GetWidth() const { return _backbufferSize.width; }
		/// Return backbuffer height, or 0 if not initialized.
		uint32_t GetHeight() const { return _backbufferSize.height; }
		/// Return multisample level, or 1 if not using multisampling.
		uint32_t GetMultisample() const { return _multisample; }
		/// Return current rendertarget width.
		uint32_t GetRenderTargetWidth() const { return _renderTargetSize.width; }
		/// Return current rendertarget height.
		uint32_t GetRenderTargetHeight() const { return _renderTargetSize.height; }
		/// Return whether is using vertical sync.
		bool VSync() const { return vsync; }
		/// Return the current color rendertarget by index, or null if rendering to the backbuffer.
		Texture* GetRenderTarget(size_t index) const;
		/// Return the current depth-stencil buffer, or null if rendering to the backbuffer.
		Texture* GetDepthStencil() const { return _depthStencil; }
		/// Return the current viewport rectangle.
		const IntRect& Viewport() const { return viewport; }
		
		/// Return currently bound constant buffer by shader stage and index.
		ConstantBuffer* GetConstantBuffer(ShaderStage stage, size_t index) const;
		/// Return currently bound texture by texture unit.
		Texture* GetTexture(size_t index) const;
		/// Return currently bound vertex shader.
		ShaderVariation* GetVertexShader() const { return vertexShader; }
		/// Return currently bound pixel shader.
		ShaderVariation* GetPixelShader() const { return pixelShader; }
		/// Return the current renderstate.
		const RenderState& GetRenderState() const { return renderState; }

		/// Register a GPU object to keep track of.
		void AddGPUObject(GPUObject* object);
		/// Remove a GPU object.
		void RemoveGPUObject(GPUObject* object);
		/// Remove all framebuffers except the currently bound one. No-op on Direct3D but provided for compatibility.
		void CleanupFramebuffers() {}

		/// Screen mode changed event.
		ScreenModeEvent screenModeEvent;
		/// %Graphics context lost event. Will not be called on Direct3D but provided for compatibility.
		Event contextLossEvent;
		/// %Graphics context restored event. Will not be called on Direct3D but provided for compatibility.
		Event contextRestoreEvent;

	private:
		// Backend methods
		virtual BufferHandle* CreateBuffer(BufferUsage usage, uint32_t size, uint32_t stride, ResourceUsage resourceUsage, const void* initialData) = 0;

		virtual void SetIndexBufferCore(BufferHandle* handle, IndexType type) = 0;

	protected:
		virtual void Finalize();

		/// Implementation for holding OS-specific API objects.
		GraphicsImpl* impl = nullptr;

	protected:
		
		GraphicsDeviceType _deviceType;
		bool _validation{};
		bool _initialized{};

		/// Current size of the backbuffer.
		Size _backbufferSize{ Size::Empty };
		/// Current size of the active rendertarget.
		Size _renderTargetSize{ Size::Empty };
		/// Bound vertex buffers.
		VertexBuffer* _vertexBuffers[MaxVertexBuffers];
		
		/// Bound constant buffers by shader stage.
		ConstantBuffer* constantBuffers[MAX_SHADER_STAGES][MAX_CONSTANT_BUFFERS];
		/// Bound textures by texture unit.
		Texture* textures[MAX_TEXTURE_UNITS];
		/// Bound rendertarget textures.
		Texture* _renderTargets[MAX_RENDERTARGETS] = {};
		/// Bound depth-stencil texture.
		Texture* _depthStencil = nullptr;
		/// Bound vertex shader.
		ShaderVariation* vertexShader;
		/// Bound pixel shader.
		ShaderVariation* pixelShader;
		/// Current renderstate.
		RenderState renderState;
		/// Textures dirty flag.
		bool texturesDirty;
		
		/// Blend state dirty flag.
		bool blendStateDirty;
		/// Depth state dirty flag.
		bool depthStateDirty;
		/// Rasterizer state dirty flag.
		bool rasterizerStateDirty;
		/// Scissor rect dirty flag.
		bool scissorRectDirty;
		/// Current primitive type.
		PrimitiveType primitiveType;
		
		/// Current viewport rectangle.
		IntRect viewport;
		/// GPU objects mutex.
		std::mutex _gpuResourceMutex;
		/// GPU objects.
		std::vector<GPUObject*> gpuObjects;
		
		/// Multisample level.
		uint32_t _multisample{ 1 };
		/// Vertical sync flag.
		bool vsync{ false };
	};

	/// Register Graphics related object factories and attributes.
	ALIMER_API void RegisterGraphicsLibrary();
}