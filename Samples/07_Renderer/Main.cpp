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

#ifdef _MSC_VER
#include <crtdbg.h>
#endif

#include <cstdio>
#include <cstdlib>

#include "Windows.h"

using namespace Alimer;

class RendererTest final : public Application
{
	ALIMER_OBJECT(RendererTest, Application);

private:
	void Start() override;
	void Stop() override;
	void Render() override;

private:
	std::unique_ptr<Scene> _scene;
	Camera* _camera = nullptr;
};

void RendererTest::Start()
{
	_scene = std::make_unique<Scene>();

	_scene->CreateChild<Octree>();
	_camera = _scene->CreateChild<Camera>();
	_camera->SetPosition(Vector3(0.0f, 20.0f, -75.0f));
	_camera->SetAmbientColor(Color(0.1f, 0.1f, 0.1f));

	for (int y = -5; y <= 5; ++y)
	{
		for (int x = -5; x <= 5; ++x)
		{
			StaticModel* object = _scene->CreateChild<StaticModel>();
			object->SetPosition(Vector3(10.5f * x, -0.1f, 10.5f * y));
			object->SetScale(Vector3(10.0f, 0.1f, 10.0f));
			object->SetModel(_engine->GetCache()->LoadResource<Model>("Box.mdl"));
			object->SetMaterial(_engine->GetCache()->LoadResource<Material>("Stone.json"));
		}
	}

	for (unsigned i = 0; i < 435; ++i)
	{
		StaticModel* object = _scene->CreateChild<StaticModel>();
		object->SetPosition(Vector3(Random() * 100.0f - 50.0f, 1.0f, Random() * 100.0f - 50.0f));
		object->SetScale(1.5f);
		object->SetModel(_engine->GetCache()->LoadResource<Model>("Mushroom.mdl"));
		object->SetMaterial(_engine->GetCache()->LoadResource<Material>("Mushroom.json"));
		object->SetCastShadows(true);
		object->SetLodBias(2.0f);
	}

	for (unsigned i = 0; i < 10; ++i)
	{
		Light* light = _scene->CreateChild<Light>();
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
}

void RendererTest::Stop()
{
	_scene.reset();
}


void RendererTest::Render()
{
	auto input = _engine->GetInput();
	auto graphics = _engine->GetGraphics();
	const float deltaTime = (float)_engine->GetTime()->GetElapsedSeconds();
	auto renderer = _engine->GetRenderer();

	if (input->IsKeyPress(27))
		Exit();

	if (input->IsKeyPress('F'))
	{
		//graphics->SetFullscreen(!graphics->GetRenderWindow()->IsFullscreen());
	}

	static float yaw = 0.0f;
	static float pitch = 20.0f;
	pitch += input->MouseMove().y * 0.25f;
	yaw += input->MouseMove().x * 0.25f;
	pitch = Clamp(pitch, -90.0f, 90.0f);

	float moveSpeed = input->IsKeyDown(VK_SHIFT) ? 50.0f : 10.0f;

	_camera->SetRotation(Quaternion(pitch, yaw, 0.0f));
	if (input->IsKeyDown('W'))
		_camera->Translate(Vector3::FORWARD * deltaTime * moveSpeed);
	if (input->IsKeyDown('S'))
		_camera->Translate(Vector3::BACK * deltaTime * moveSpeed);
	if (input->IsKeyDown('A'))
		_camera->Translate(Vector3::LEFT * deltaTime * moveSpeed);
	if (input->IsKeyDown('D'))
		_camera->Translate(Vector3::RIGHT * deltaTime * moveSpeed);

	// Update camera aspect ratio based on window size
	_camera->SetAspectRatio((float)graphics->GetWidth() / (float)graphics->GetHeight());

	{
		ALIMER_PROFILE(RenderScene);
		std::vector<PassDesc> passes;
		passes.emplace_back("opaque", SORT_STATE, true);
		passes.emplace_back("alpha", SORT_BACK_TO_FRONT, true);
		renderer->PrepareView(_scene.get(), _camera, passes);

		renderer->RenderShadowMaps();
		graphics->ResetRenderTargets();
		graphics->ResetViewport();
		graphics->Clear(ClearFlags::Color | ClearFlags::Depth, Color::BLACK);
		renderer->RenderBatches(passes);
	}
}

int main()
{
#ifdef _MSC_VER
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	RendererTest test;
	return test.Run();
}