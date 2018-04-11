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

#include "Alimer.h"
#include "Debug/DebugNew.h"

#ifdef _MSC_VER
#include <crtdbg.h>
#endif

#include <cstdio>
#include <cstdlib>

#include "Windows.h"

using namespace Alimer;

class RendererTest : public Object
{
	OBJECT(RendererTest);

public:
	void Run()
	{
		RegisterGraphicsLibrary();
		RegisterResourceLibrary();
		RegisterRendererLibrary();

		cache = std::make_unique<ResourceCache>();
		cache->AddResourceDir(ExecutableDir() + "Data");

		log = std::make_unique<Log>();
		input = std::make_unique<Input>();
		profiler = std::make_unique<Profiler>();
		graphics = std::make_unique<Graphics>();
		renderer = std::make_unique<Renderer>();

		graphics->GetRenderWindow()->SetTitle("Renderer test");
		graphics->GetRenderWindow()->SetMouseVisible(false);
		if (!graphics->SetMode(IntVector2(800, 600), false, true))
			return;

		renderer->SetupShadowMaps(1, 2048, FMT_D16);

		SubscribeToEvent(graphics->GetRenderWindow()->closeRequestEvent, &RendererTest::HandleCloseRequest);

		SharedPtr<Scene> scene = new Scene();
		scene->CreateChild<Octree>();
		Camera* camera = scene->CreateChild<Camera>();
		camera->SetPosition(Vector3(0.0f, 20.0f, -75.0f));
		camera->SetAmbientColor(Color(0.1f, 0.1f, 0.1f));

		for (int y = -5; y <= 5; ++y)
		{
			for (int x = -5; x <= 5; ++x)
			{
				StaticModel* object = scene->CreateChild<StaticModel>();
				object->SetPosition(Vector3(10.5f * x, -0.1f, 10.5f * y));
				object->SetScale(Vector3(10.0f, 0.1f, 10.0f));
				object->SetModel(cache->LoadResource<Model>("Box.mdl"));
				object->SetMaterial(cache->LoadResource<Material>("Stone.json"));
			}
		}

		for (unsigned i = 0; i < 435; ++i)
		{
			StaticModel* object = scene->CreateChild<StaticModel>();
			object->SetPosition(Vector3(Random() * 100.0f - 50.0f, 1.0f, Random() * 100.0f - 50.0f));
			object->SetScale(1.5f);
			object->SetModel(cache->LoadResource<Model>("Mushroom.mdl"));
			object->SetMaterial(cache->LoadResource<Material>("Mushroom.json"));
			object->SetCastShadows(true);
			object->SetLodBias(2.0f);
		}

		for (unsigned i = 0; i < 10; ++i)
		{
			Light* light = scene->CreateChild<Light>();
			light->SetLightType(LIGHT_POINT);
			light->SetCastShadows(true);
			Vector3 colorVec = 2.0f * Vector3(Random(), Random(), Random()).Normalized();
			light->SetColor(Color(colorVec.x, colorVec.y, colorVec.z));
			light->SetFov(90.0f);
			light->SetRange(20.0f);
			light->SetPosition(Vector3(Random() * 120.0f - 60.0f, 7.0f, Random() * 120.0f - 60.0f));
			light->SetDirection(Vector3(0.0f, -1.0f, 0.0f));
			light->SetShadowMapSize(256);
		}

		float yaw = 0.0f, pitch = 20.0f;
		HiresTimer frameTimer;
		Timer profilerTimer;
		float dt = 0.0f;
		String profilerOutput;

		for (;;)
		{
			frameTimer.Reset();
			if (profilerTimer.ElapsedMSec() >= 1000)
			{
				profilerOutput = profiler->OutputResults();
				profilerTimer.Reset();
				profiler->BeginInterval();
			}

			profiler->BeginFrame();

			input->Update();
			if (input->IsKeyPress(27))
				graphics->Close();

			// Break if window closed; Graphics drawing functions are not safe to any more
			if (!graphics->IsInitialized())
				break;

			if (input->IsKeyPress('F'))
				graphics->SetFullscreen(!graphics->IsFullscreen());

			pitch += input->MouseMove().y * 0.25f;
			yaw += input->MouseMove().x * 0.25f;
			pitch = Clamp(pitch, -90.0f, 90.0f);

			float moveSpeed = input->IsKeyDown(VK_SHIFT) ? 50.0f : 10.0f;

			camera->SetRotation(Quaternion(pitch, yaw, 0.0f));
			if (input->IsKeyDown('W'))
				camera->Translate(Vector3::FORWARD * dt * moveSpeed);
			if (input->IsKeyDown('S'))
				camera->Translate(Vector3::BACK * dt * moveSpeed);
			if (input->IsKeyDown('A'))
				camera->Translate(Vector3::LEFT * dt * moveSpeed);
			if (input->IsKeyDown('D'))
				camera->Translate(Vector3::RIGHT * dt * moveSpeed);

			// Update camera aspect ratio based on window size
			camera->SetAspectRatio((float)graphics->Width() / (float)graphics->Height());

			{
				ALIMER_PROFILE(RenderScene);
				std::vector<PassDesc> passes;
				passes.emplace_back("opaque", SORT_STATE, true);
				passes.emplace_back("alpha", SORT_BACK_TO_FRONT, true);
				renderer->PrepareView(scene, camera, passes);

				renderer->RenderShadowMaps();
				graphics->ResetRenderTargets();
				graphics->ResetViewport();
				graphics->Clear(ClearFlags::Color | ClearFlags::Depth, Color::BLACK);
				renderer->RenderBatches(passes);
			}
			graphics->Present();

			profiler->EndFrame();
			dt = frameTimer.ElapsedUSec() * 0.000001f;
		}

		LOGRAW(profilerOutput);
	}

	void HandleCloseRequest(Event& /* event */)
	{
		graphics->Close();
	}

	std::unique_ptr<ResourceCache> cache;
	std::unique_ptr<Graphics> graphics;
	std::unique_ptr<Renderer> renderer;
	std::unique_ptr<Input> input;
	std::unique_ptr<Log> log;
	std::unique_ptr<Profiler> profiler;
};

int main()
{
#ifdef _MSC_VER
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	RendererTest test;
	test.Run();

	return 0;
}