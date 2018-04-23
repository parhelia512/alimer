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

#pragma once

#include "../Window/Window.h"

namespace Alimer
{
	class Log;
	class Time;
	class ResourceCache;
	class Graphics;
	class Renderer;
	class Profiler;

	struct ApplicationSettings
	{
		std::string title = "Alimer";
		std::string applicationName = "Alimer";
		uint32_t width = 800;
		uint32_t height = 600;
		bool resizable = true;
		bool fullscreen = false;
		uint32_t multisample = 1;
		bool verticalSync = true;
#ifdef _DEBUG
		bool validation = true;
#else
		bool validation = false;
#endif
	};

	/// Application subsystem for main loop and all modules and OS setup.
	class ALIMER_API Application : public Object
	{
		ALIMER_OBJECT(Application, Object);

	protected:
		/// Construct and register subsystem.
		Application();

	public:
		/// Destructor.
		virtual ~Application();

		/// Gets the single instance of the Application.
		static Application* GetInstance();

		/// Run the application and enters in main loop until main window is closed or exit is requested.
		int Run();

		/// Run one frame.
		void RunFrame();

		/// Request application to exit.
		void Exit();

		/// Show an error message (last log message if empty), terminate the main loop, and set failure exit code.
		void ErrorExit(const std::string& message = "");

		/// Return whether application has been initialized.
		bool IsInitialized() const { return _initialized; }

		/// Return whether exit has been requested.
		bool IsExiting() const { return _exiting; }

		/// Return whether the application has been created in headless mode.
		bool IsHeadless() const { return _headless; }

		/// Gets main OS window.
		inline Window* GetWindow() { return _window.get(); }

		/// Time module.
		inline Time* GetTime() { return _time.get(); }

		/// ResourceCache module.
		inline ResourceCache* GetCache() { return _cache.get(); }

		/// Graphics module.
		inline Graphics* GetGraphics() { return _graphics.get(); }

		/// Renderer module.
		inline Renderer* GetRenderer() { return _renderer.get(); }

	protected:
		/// Setup before engine initialization. This is a chance to eg. modify the engine parameters. Call ErrorExit() to terminate without initializing the engine. 
		virtual void OnSetup() { }

		/// Setup after engine initialization and before running the main loop. Call ErrorExit() to terminate without running the main loop.
		virtual void OnStart() { }

		/// Cleanup after the main loop. 
		virtual void OnStop() { }

		/// Perform rendering logic. Called from Engine::Render.
		virtual void OnRender() { }

	private:
		bool InitializeBeforeRun();

		/// Render after frame update.
		void Render();

		int PlatformRun();
		void PlatformExit();

	protected:
		bool _initialized;
		bool _exiting;
		bool _headless;

		/// Application exit code.
		int _exitCode;

		ApplicationSettings _settings{};

	private:
		/// Handle window resize event.
		void HandleResize(WindowResizeEvent&);
		void HandleCloseRequest(Event&);

		/// OS-level rendering window.
		std::unique_ptr<Window> _window;

		/// Log module.
		std::unique_ptr<Log> _log;

		/// Profiler module [optional].
		std::unique_ptr<Profiler> _profiler;

		/// Time module.
		std::unique_ptr<Time> _time;

		/// ResourceCache module.
		std::unique_ptr<ResourceCache> _cache;

		/// Graphics module.
		std::unique_ptr<Graphics> _graphics;

		/// Renderer module.
		std::unique_ptr<Renderer> _renderer;
	};

}
