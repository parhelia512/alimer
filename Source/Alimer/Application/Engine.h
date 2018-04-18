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
	class Input;
	class Graphics;

	struct EngineSettings
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

	/// Alimer Engine. Creates the other subsystems and modules.
	class ALIMER_API Engine final : public Object
	{
		ALIMER_OBJECT(Engine, Object);

	public:
		/// Constructor.
		Engine();
		/// Destructor.
		~Engine() override;

		/// Gets the single instance of the Engine.
		static Engine* GetInstance();

		/// Initialize engine using given settings. Return true if successful.
		bool Initialize(const EngineSettings& settings);

		/// Run one frame.
		void RunFrame();

		/// Exits the engine main loop.
		void Exit();

		/// Render after frame update.
		void Render();

		/// Return whether engine has been initialized.
		bool IsInitialized() const { return _initialized; }

		/// Return whether exit has been requested.
		bool IsExiting() const { return _exiting; }

		/// Return whether the engine has been created in headless mode.
		bool IsHeadless() const { return _headless; }

		/// Gets main OS window.
		inline Window* GetWindow() { return _window.get(); }

		/// Time module.
		inline Time* GetTime() { return _time.get(); }

		/// ResourceCache module.
		inline ResourceCache* GetCache() { return _cache.get(); }

		/// Input module.
		inline Input* GetInput() { return _input.get(); }

		/// Graphics module.
		inline Graphics* GetGraphics() { return _graphics.get(); }

	private:
		/// Handle window resize event.
		void HandleResize(WindowResizeEvent&);
		void HandleCloseRequest(Event&);

		EngineSettings _settings{};

		bool _initialized{};
		bool _exiting{};
		bool _headless{};

		/// OS-level rendering window.
		std::unique_ptr<Window> _window;

		/// Log module.
		std::unique_ptr<Log> _log;

		/// Time module.
		std::unique_ptr<Time> _time;

		/// ResourceCache module.
		std::unique_ptr<ResourceCache> _cache;

		/// Input module.
		std::unique_ptr<Input> _input;

		/// Graphics module.
		std::unique_ptr<Graphics> _graphics;
	};
}
