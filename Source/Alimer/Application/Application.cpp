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

#include "Debug/Log.h"
#include "Debug/Profiler.h"
#include "Application.h"
#include "Time.h"
#include "Window/Input.h"
#include "Resource/ResourceCache.h"
#include "IO/FileSystem.h"
#include "Graphics/Graphics.h"
#include "Renderer/Renderer.h"
using namespace std;

#if defined(__i386__)
// From http://stereopsis.com/FPU.html

#define FPU_CW_PREC_MASK        0x0300
#define FPU_CW_PREC_SINGLE      0x0000
#define FPU_CW_PREC_DOUBLE      0x0200
#define FPU_CW_PREC_EXTENDED    0x0300
#define FPU_CW_ROUND_MASK       0x0c00
#define FPU_CW_ROUND_NEAR       0x0000
#define FPU_CW_ROUND_DOWN       0x0400
#define FPU_CW_ROUND_UP         0x0800
#define FPU_CW_ROUND_CHOP       0x0c00

inline unsigned GetFPUState()
{
	unsigned control = 0;
	__asm__ __volatile__("fnstcw %0" : "=m" (control));
	return control;
}

inline void SetFPUState(unsigned control)
{
	__asm__ __volatile__("fldcw %0" : : "m" (control));
}
#endif


namespace Alimer
{
	static Application* __appInstance = nullptr;

	void InitFPU()
	{
		// Make sure FPU is in round-to-nearest, single precision mode
		// This ensures Direct3D and OpenGL behave similarly, and all threads behave similarly
#if defined(_MSC_VER) && defined(_M_IX86)
		_controlfp(_RC_NEAR | _PC_24, _MCW_RC | _MCW_PC);
#elif defined(__i386__)
		unsigned control = GetFPUState();
		control &= ~(FPU_CW_PREC_MASK | FPU_CW_ROUND_MASK);
		control |= (FPU_CW_PREC_SINGLE | FPU_CW_ROUND_NEAR);
		SetFPUState(control);
#endif
	}

	Application::Application()
		: _initialized(false)
		, _exiting(false)
		, _headless(false)
		, _exitCode(EXIT_SUCCESS)
	{
		// Init modules.
		_log = make_unique<Log>();
#ifdef ALIMER_PROFILING
		_profiler = make_unique<Profiler>();
#endif

		_time = make_unique<Time>();
		_cache = make_unique<ResourceCache>();
		_renderer = make_unique<Renderer>();

		InitFPU();

		__appInstance = this;
		RegisterSubsystem(this);
	}

	Application::~Application()
	{
		_window.reset();
		RemoveSubsystem(this);
		__appInstance = nullptr;
	}

	Application* Application::GetInstance()
	{
		return __appInstance;
	}

	bool Application::InitializeBeforeRun()
	{
		if (_initialized)
			return true;

		ALIMER_PROFILE(Application);

		if (!_headless)
		{
			// Create main window.
			_window.reset(new Window(_settings.title, _settings.width, _settings.height, _settings.resizable, _settings.fullscreen));
			SubscribeToEvent(_window->resizeEvent, &Application::HandleResize);
			SubscribeToEvent(_window->closeRequestEvent, &Application::HandleCloseRequest);

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

		ALIMER_LOGINFO("Application initialized.");
		//_time.Reset();
		_initialized = true;
		return true;
	}

	void Application::RunFrame()
	{
		_time->Update();
		Render();
	}

	void Application::Render()
	{
		if (_headless)
			return;

		ALIMER_PROFILE(Render);

		if (!_graphics->BeginFrame())
			return;

		OnRender();
		_graphics->Present();
	}

	int Application::Run()
	{
#if !defined(__GNUC__) || __EXCEPTIONS
		try
		{
#endif
			OnSetup();
			if (_exitCode)
				return _exitCode;

			return PlatformRun();
#if !defined(__GNUC__) || __EXCEPTIONS
		}
		catch (std::bad_alloc&)
		{
			//ErrorDialog(GetTypeName(), "An out-of-memory error occurred. The application will now exit.");
			return EXIT_FAILURE;
		}
#endif
	}

	void Application::Exit()
	{
		PlatformExit();
	}

	void Application::ErrorExit(const std::string& /*message*/)
	{
		// Exit the engine.
		Exit(); 
		_exitCode = EXIT_FAILURE;

		// TODO: Error message.
	}

	void Application::HandleResize(WindowResizeEvent&)
	{

	}

	void Application::HandleCloseRequest(Event&)
	{

	}
}
