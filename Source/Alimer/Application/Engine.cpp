//
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

#include "Engine.h"
#include "Time.h"
#include "Debug/Log.h"
#include "Debug/Profiler.h"
#include "Window/Input.h"
#include "Resource/ResourceCache.h"
#include "IO/FileSystem.h"
#include "Graphics/Graphics.h"
#include "Renderer/Renderer.h"
#include "Application/Application.h"
using namespace std;

namespace Alimer
{
	static Engine* __engineInstance = nullptr;

	Engine::Engine()
	{
		// Register self as a subsystem.
		RegisterSubsystem(this);

		// Init modules.
		_log = make_unique<Log>();
#ifdef ALIMER_PROFILING
		_profiler = make_unique<Profiler>();
#endif

		_time = make_unique<Time>();
		_cache = make_unique<ResourceCache>();
		_renderer = make_unique<Renderer>();
		__engineInstance = this;
	}

	Engine::~Engine()
	{
		_window.reset();

		RemoveSubsystem(this);
		__engineInstance = nullptr;
	}

	Engine* Engine::GetInstance()
	{
		return __engineInstance;
	}

	bool Engine::Initialize(const EngineSettings& settings)
	{
		if (_initialized)
			return true;

		ALIMER_PROFILE(InitEngine);

		_settings = settings;

		if (!_headless)
		{
			// Create main window.
			_window.reset(new Window(_settings.title, _settings.width, _settings.height, _settings.resizable, _settings.fullscreen));
			SubscribeToEvent(_window->resizeEvent, &Engine::HandleResize);
			SubscribeToEvent(_window->closeRequestEvent, &Engine::HandleCloseRequest);

			// Create and init graphics.
			_graphics.reset(Graphics::Create(GraphicsDeviceType::Direct3D11));

			// Initialize graphics backend.
			GraphicsSettings graphicsSettings = {};
			graphicsSettings.window = _window.get();
			graphicsSettings.multisample = _settings.multisample;
			graphicsSettings.verticalSync = _settings.verticalSync;
			//graphicsSettings.depthStencilFormat = _graphics->GetDefaultDepthStencilFormat();
			if (!_graphics->Initialize(graphicsSettings))
			{
				ALIMER_LOGERROR("Error while initializing graphics system");
				return false;
			}
		}

		RegisterGraphicsLibrary();
		RegisterResourceLibrary();
		RegisterRendererLibrary();

		// Init ResourceCache.
		const String executableDir = GetExecutableDir();
		if (DirectoryExists(executableDir + "Data"))
			_cache->AddResourceDir(GetExecutableDir() + "Data");

		if (DirectoryExists(GetParentPath(executableDir) + "Data"))
			_cache->AddResourceDir(GetParentPath(executableDir) + "Data");

		// Initialize Renderer
		_renderer->SetupShadowMaps(1, 2048, PixelFormat::Depth16UNorm);

		ALIMER_LOGINFO("Engine initialized");
		//_time.Reset();
		_initialized = true;
		return true;
	}

	void Engine::RunFrame()
	{
		_time->Update();
		Render();
	}

	void Engine::Exit()
	{
		_window->Close();
		_exiting = true;
	}

	void Engine::Render()
	{
		if (_headless)
			return;

		ALIMER_PROFILE(Render);

		if (!_graphics->BeginFrame())
			return;

		// Call render on Application.
		Application::GetInstance()->Render();
		_graphics->Present();
	}

	void Engine::HandleResize(WindowResizeEvent&)
	{

	}

	void Engine::HandleCloseRequest(Event&)
	{

	}
}
