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

using namespace Alimer;

class GraphicsTest final : public Application
{
	ALIMER_OBJECT(GraphicsTest, Application);

public:
	static constexpr uint32_t ObjectsCount = 1000;
	std::unique_ptr<VertexBuffer> _vertexBuffer;
	std::unique_ptr<VertexBuffer> _instanceVertexBuffer;
	std::unique_ptr<IndexBuffer> _indexBuffer;
	std::unique_ptr<Shader> _vertexShader;
	std::unique_ptr<Shader> _fragmentShader;
	std::unique_ptr<ConstantBuffer> _constantBuffer;

	Texture* _texture;

	void Start() override
	{
		float vertexData[] = {
			// Position             // Texcoord
			0.0f, 0.05f, 0.0f,      0.5f, 0.0f,
			0.05f, -0.05f, 0.0f,    1.0f, 1.0f,
			-0.05f, -0.05f, 0.0f,   0.0f, 1.0f
		};

		std::vector<VertexElement> vertexDeclaration;
		vertexDeclaration.push_back(VertexElement(ELEM_VECTOR3, SEM_POSITION));
		vertexDeclaration.push_back(VertexElement(ELEM_VECTOR2, SEM_TEXCOORD));

		_vertexBuffer = std::make_unique<VertexBuffer>();
		_vertexBuffer->Define(ResourceUsage::Immutable, 3, vertexDeclaration, true, vertexData);

		std::vector<VertexElement> instanceVertexDeclaration;
		instanceVertexDeclaration.push_back(VertexElement(ELEM_VECTOR3, SEM_TEXCOORD, 1, true));
		_instanceVertexBuffer = std::make_unique<VertexBuffer>();
		_instanceVertexBuffer->Define(ResourceUsage::Dynamic, ObjectsCount, instanceVertexDeclaration, true);

		uint16_t indexData[] = {
			0,
			1,
			2
		};

		_indexBuffer = std::make_unique<IndexBuffer>();
		_indexBuffer->Define(ResourceUsage::Immutable, 3, IndexType::UInt16, true, indexData);

		Constant pc(ELEM_VECTOR4, "Color");

		_constantBuffer = std::make_unique<ConstantBuffer>();
		_constantBuffer->Define(1, &pc, true);
		_constantBuffer->SetConstant("Color", Color::WHITE);
		_constantBuffer->Apply();

		std::string vsCode =
#ifndef ALIMER_OPENGL
			"struct VOut\n"
			"{\n"
			"    float4 position : SV_POSITION;\n"
			"    float2 texCoord : TEXCOORD0;\n"
			"};\n"
			"\n"
			"VOut main(float3 position : POSITION, float2 texCoord : TEXCOORD0, float3 objectPosition : TEXCOORD1)\n"
			"{\n"
			"    VOut output;\n"
			"    output.position = float4(position + objectPosition, 1.0);\n"
			"    output.texCoord = texCoord;\n"
			"    return output;\n"
			"}";
#else
			"#version 150\n"
			"\n"
			"in vec3 position;\n"
			"in vec2 texCoord;\n"
			"in vec3 texCoord1; // objectPosition\n"
			"\n"
			"out vec2 vTexCoord;\n"
			"\n"
			"void main()\n"
			"{\n"
			"    gl_Position = vec4(position + texCoord1, 1.0);\n"
			"    vTexCoord = texCoord;\n"
			"}\n";
#endif

		_vertexShader = std::make_unique<Shader>();
		_vertexShader->SetName("Test.vs");
		_vertexShader->Define(SHADER_VS, vsCode);

		std::string psCode =
#ifndef ALIMER_OPENGL
			"cbuffer ConstantBuffer : register(b0)\n"
			"{\n"
			"    float4 color;\n"
			"}\n"
			"\n"
			"Texture2D Texture : register(t0);\n"
			"SamplerState Sampler : register(s0);\n"
			"\n"
			"float4 main(float4 position : SV_POSITION, float2 texCoord : TEXCOORD0) : SV_TARGET\n"
			"{\n"
			"    return color * Texture.Sample(Sampler, texCoord);\n"
			"}\n";
#else
			"#version 150\n"
			"\n"
			"layout(std140) uniform ConstantBuffer0\n"
			"{\n"
			"    vec4 color;\n"
			"};\n"
			"\n"
			"uniform sampler2D Texture0;\n"
			"in vec2 vTexCoord;\n"
			"out vec4 fragColor;\n"
			"\n"
			"void main()\n"
			"{\n"
			"    fragColor = color * texture(Texture0, vTexCoord);\n"
			"}\n";
#endif

		_fragmentShader = std::make_unique<Shader>();
		_fragmentShader->SetName("Test.ps");
		_fragmentShader->Define(SHADER_PS, psCode);
		
		_texture = _engine->GetCache()->LoadResource<Texture>("Test.png");
	}

	void Render() override
	{
		//if (_engine->GetInput()->IsKeyPress('F'))
		//{
			//graphics->SetFullscreen(!graphics->GetRenderWindow()->IsFullscreen());
		//}

		//if (_engine->GetInput()->IsKeyPress('M'))
		//{
			//graphics->SetMultisample(graphics->GetMultisample() > 1 ? 1 : 4);
		//}

		// Escape.
		if (Input::GetInput()->IsKeyDown(Key::Escape))
		{
			Exit();
		}

		Vector3 instanceData[ObjectsCount];
		for (uint32_t i = 0; i < ObjectsCount; ++i)
		{
			instanceData[i] = Vector3(Random() * 2.0f - 1.0f, Random() * 2.0f - 1.0f, 0.0f);
		}
		_instanceVertexBuffer->SetData(0, ObjectsCount, instanceData);

		auto* vertexVariation = _vertexShader->CreateVariation();
		auto* fragmentVariation = _fragmentShader->CreateVariation();

		auto graphics = _engine->GetGraphics();
		graphics->Clear(ClearFlags::Color | ClearFlags::Depth, Color(0.0f, 0.0f, 0.5f));
		graphics->SetVertexBuffer(0, _vertexBuffer.get());
		graphics->SetVertexBuffer(1, _instanceVertexBuffer.get());
		graphics->SetIndexBuffer(_indexBuffer.get());
		graphics->SetConstantBuffer(SHADER_PS, 0, _constantBuffer.get());
		graphics->SetShaders(vertexVariation, fragmentVariation);
		graphics->SetTexture(0, _texture);
		graphics->SetDepthState(CMP_LESS_EQUAL, true);
		graphics->SetColorState(BLEND_MODE_REPLACE);
		graphics->SetRasterizerState(CULL_BACK, FILL_SOLID);
		graphics->DrawIndexedInstanced(TRIANGLE_LIST, 0, 3, 0, 0, ObjectsCount);
	}
};

int main()
{
#ifdef _MSC_VER
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	GraphicsTest test;
	test.Run();

	return 0;
}