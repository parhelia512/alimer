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
#include "../Object/Object.h"
#include "../Graphics/GraphicsDefs.h"
#include <vector>
#include <map>
#include <mutex>

#ifdef ALIMER_D3D11
struct ID3D11InputLayout;
#endif

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
	class WindowResizeEvent;

	// Handles.
	class BufferHandle;

	using InputLayoutDesc = std::pair<uint64_t, uint32_t>;
	using InputLayoutMap = std::map<InputLayoutDesc, ID3D11InputLayout*>;
	using StateObjectMap = std::map<uint64_t, void*>;

	/// Screen mode set event.
	class ScreenModeEvent : public Event
	{
	public:
		/// New backbuffer size.
		IntVector2 size;
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
		OBJECT(Graphics);

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

		/// Set graphics mode. Create the window and rendering context if not created yet. Return true on success.
		bool SetMode(const IntVector2& size, bool fullscreen = false, bool resizable = false, int multisample = 1);
		/// Set fullscreen mode on/off while retaining previous resolution. The initial graphics mode must have been set first. Return true on success.
		bool SetFullscreen(bool enable);
		/// Set new multisample level while retaining previous resolution. The initial graphics mode must have been set first. Return true on success.
		bool SetMultisample(int multisample);
		/// Set vertical sync on/off.
		void SetVSync(bool enable);
		/// Present the contents of the backbuffer.
		void Present();
		/// Set the color rendertarget and depth stencil buffer.
		void SetRenderTarget(Texture* renderTarget, Texture* stencilBuffer);
		/// Set multiple color rendertargets and the depth stencil buffer.
		void SetRenderTargets(const std::vector<Texture*>& renderTargets, Texture* stencilBuffer);
		/// Set the viewport rectangle. On window resize the viewport will automatically revert to the entire backbuffer.
		void SetViewport(const IntRect& viewport);
		/// Bind a vertex buffer.
		void SetVertexBuffer(uint32_t index, VertexBuffer* buffer);
		/// Bind an index buffer.
		void SetIndexBuffer(IndexBuffer* buffer);
		/// Bind a constant buffer.
		void SetConstantBuffer(ShaderStage stage, uint32_t index, ConstantBuffer* buffer);
		/// Bind a texture.
		void SetTexture(size_t index, Texture* texture);
		/// Bind vertex and pixel shaders.
		void SetShaders(ShaderVariation* vs, ShaderVariation* ps);
		/// Set color write and blending related state using an arbitrary blend mode.
		void SetColorState(const BlendModeDesc& blendMode, bool alphaToCoverage = false, unsigned char colorWriteMask = COLORMASK_ALL);
		/// Set color write and blending related state using a predefined blend mode.
		void SetColorState(BlendMode blendMode, bool alphaToCoverage = false, unsigned char colorWriteMask = COLORMASK_ALL);
		/// Set depth buffer related state.
		void SetDepthState(CompareFunc depthFunc, bool depthWrite, bool depthClip = true, int depthBias = 0, float slopeScaledDepthBias = 0.0f);
		/// Set rasterizer related state.
		void SetRasterizerState(CullMode cullMode, FillMode fillMode);
		/// Set scissor test.
		void SetScissorTest(bool scissorEnable, const IntRect& scissorRect = IntRect::ZERO);
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
		void Clear(ClearFlags clearFlags, const Color& clearColor = Color::BLACK, float clearDepth = 1.0f, unsigned char clearStencil = 0);
		/// Draw non-indexed geometry.
		void Draw(PrimitiveType type, size_t vertexStart, size_t vertexCount);
		/// Draw indexed geometry.
		void DrawIndexed(PrimitiveType type, size_t indexStart, size_t indexCount, size_t vertexStart);
		/// Draw instanced non-indexed geometry.
		void DrawInstanced(PrimitiveType type, size_t vertexStart, size_t vertexCount, size_t instanceStart, size_t instanceCount);
		/// Draw instanced indexed geometry.
		void DrawIndexedInstanced(PrimitiveType type, size_t indexStart, size_t indexCount, size_t vertexStart, size_t instanceStart, size_t instanceCount);

		/// Return whether has the rendering window and context.
		bool IsInitialized() const;
		/// Return backbuffer size, or 0,0 if not initialized.
		const IntVector2& Size() const { return backbufferSize; }
		/// Return backbuffer width, or 0 if not initialized.
		int Width() const { return backbufferSize.x; }
		/// Return backbuffer height, or 0 if not initialized.
		int Height() const { return backbufferSize.y; }
		/// Return multisample level, or 1 if not using multisampling.
		int Multisample() const { return multisample; }
		/// Return current rendertarget width.
		int RenderTargetWidth() const { return renderTargetSize.x; }
		/// Return current rendertarget height.
		int RenderTargetHeight() const { return renderTargetSize.y; }
		/// Return whether is using fullscreen mode.
		bool IsFullscreen() const;
		/// Return whether the window is resizable.
		bool IsResizable() const;
		/// Return whether is using vertical sync.
		bool VSync() const { return vsync; }
		/// Return the rendering window.
		Window* GetRenderWindow() const { return window.get(); }
		/// Return the current color rendertarget by index, or null if rendering to the backbuffer.
		Texture* GetRenderTarget(size_t index) const;
		/// Return the current depth-stencil buffer, or null if rendering to the backbuffer.
		Texture* GetDepthStencil() const { return _depthStencil; }
		/// Return the current viewport rectangle.
		const IntRect& Viewport() const { return viewport; }
		/// Return currently bound vertex buffer by index.
		VertexBuffer* GetVertexBuffer(size_t index) const;
		
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
		/// Return the D3D11 device. Used internally and should not be called by portable application code.
		void* D3DDevice() const;
		/// Return the D3D11 immediate device context. Used internally and should not be called by portable application code.
		void* D3DDeviceContext() const;

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

	private:
		/// Create the D3D11 device and swap chain. Requires an open window. Can also be called again to recrease swap chain. Return true on success.
		bool CreateD3DDevice(int multisample);
		/// Update swap chain state for a new mode and create views for the backbuffer & default depth buffer.
		bool UpdateSwapChain(int width, int height);
		/// Handle window resize event.
		void HandleResize(WindowResizeEvent& event);
		/// Set texture state for the next draw call. PrepareDraw() calls this.
		void PrepareTextures();
		/// Set state for the next draw call. Return false if the draw call should not be attempted.
		bool PrepareDraw(PrimitiveType type);

	protected:
		GraphicsDeviceType _deviceType;
		bool _validation;
		bool _initialized{};

		/// OS-level rendering window.
		std::unique_ptr<Window> window;
		/// Current size of the backbuffer.
		IntVector2 backbufferSize{ IntVector2::ZERO };
		/// Current size of the active rendertarget.
		IntVector2 renderTargetSize{ IntVector2::ZERO };
		/// Bound vertex buffers.
		VertexBuffer* vertexBuffers[MAX_VERTEX_STREAMS];
		
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
		/// Input layout dirty flag.
		bool inputLayoutDirty;
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
		/// Current input layout: vertex buffers' element mask and vertex shader's element mask combined.
		InputLayoutDesc inputLayout;
		/// Current viewport rectangle.
		IntRect viewport;
		/// GPU objects mutex.
		std::mutex _gpuResourceMutex;
		/// GPU objects.
		std::vector<GPUObject*> gpuObjects;
		/// Input layouts.
		InputLayoutMap _inputLayouts;
		/// Blend state objects.
		StateObjectMap blendStates;
		/// Depth state objects.
		StateObjectMap depthStates;
		/// Rasterizer state objects.
		StateObjectMap rasterizerStates;
		/// Multisample level.
		int multisample{ 1 };
		/// Vertical sync flag.
		bool vsync{ false };
	};

	/// Register Graphics related object factories and attributes.
	ALIMER_API void RegisterGraphicsLibrary();

}