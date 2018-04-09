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

using namespace Turso3D;

class GraphicsTest : public Object
{
    OBJECT(GraphicsTest);

public:
    void Run()
    {
        RegisterGraphicsLibrary();
        RegisterResourceLibrary();

        cache = new ResourceCache();
        cache->AddResourceDir(ExecutableDir() + "Data");

        log = new Log();
        input = new Input();

        graphics = new Graphics();
        graphics->RenderWindow()->SetTitle("Graphics test");
        if (!graphics->SetMode(IntVector2(800, 600), false, true))
            return;

        SubscribeToEvent(graphics->RenderWindow()->closeRequestEvent, &GraphicsTest::HandleCloseRequest);
        
        const size_t NUM_OBJECTS = 1000;

        float vertexData[] = {
            // Position             // Texcoord
            0.0f, 0.05f, 0.0f,      0.5f, 0.0f,
            0.05f, -0.05f, 0.0f,    1.0f, 1.0f,
            -0.05f, -0.05f, 0.0f,   0.0f, 1.0f
        };

        Vector<VertexElement> vertexDeclaration;
        vertexDeclaration.Push(VertexElement(ELEM_VECTOR3, SEM_POSITION));
        vertexDeclaration.Push(VertexElement(ELEM_VECTOR2, SEM_TEXCOORD));
        AutoPtr<VertexBuffer> vb = new VertexBuffer();
        vb->Define(USAGE_IMMUTABLE, 3, vertexDeclaration, true, vertexData);
        
        Vector<VertexElement> instanceVertexDeclaration;
        instanceVertexDeclaration.Push(VertexElement(ELEM_VECTOR3, SEM_TEXCOORD, 1, true));
        SharedPtr<VertexBuffer> ivb = new VertexBuffer();
        ivb->Define(USAGE_DYNAMIC, NUM_OBJECTS, instanceVertexDeclaration, true);

        unsigned short indexData[] = {
            0,
            1,
            2
        };

        AutoPtr<IndexBuffer> ib = new IndexBuffer();
        ib->Define(USAGE_IMMUTABLE, 3, sizeof(unsigned short), true, indexData);
        
        Constant pc(ELEM_VECTOR4, "Color");
        AutoPtr<ConstantBuffer> pcb = new ConstantBuffer();
        pcb->Define(USAGE_IMMUTABLE, 1, &pc);
        pcb->SetConstant("Color", Color::WHITE);
        pcb->Apply();
        
        String vsCode =
#ifndef TURSO3D_OPENGL
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

        AutoPtr<Shader> vs = new Shader();
        vs->SetName("Test.vs");
        vs->Define(SHADER_VS, vsCode);
        ShaderVariation* vsv = vs->CreateVariation();

        String psCode =
#ifndef TURSO3D_OPENGL
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

        AutoPtr<Shader> ps = new Shader();
        ps->SetName("Test.ps");
        ps->Define(SHADER_PS, psCode);
        ShaderVariation* psv = ps->CreateVariation();

        Texture* tex = cache->LoadResource<Texture>("Test.png");
        
        for (;;)
        {
            input->Update();
            if (input->IsKeyPress('F'))
                graphics->SetFullscreen(!graphics->IsFullscreen());
            if (input->IsKeyPress('M'))
                graphics->SetMultisample(graphics->Multisample() > 1 ? 1 : 4);
            if (input->IsKeyPress(27))
                graphics->Close();

            // Break if window closed; Graphics drawing functions are not safe to any more
            if (!graphics->IsInitialized())
                break;

            Vector3 instanceData[NUM_OBJECTS];
            for (size_t i = 0; i < NUM_OBJECTS; ++i)
                instanceData[i] = Vector3(Random() * 2.0f - 1.0f, Random() * 2.0f - 1.0f, 0.0f);
            ivb->SetData(0, NUM_OBJECTS, instanceData);

            graphics->Clear(CLEAR_COLOR | CLEAR_DEPTH, Color(0.0f, 0.0f, 0.5f));
            graphics->SetVertexBuffer(0, vb);
            graphics->SetVertexBuffer(1, ivb);
            graphics->SetIndexBuffer(ib);
            graphics->SetConstantBuffer(SHADER_PS, 0, pcb);
            graphics->SetShaders(vsv, psv);
            graphics->SetTexture(0, tex);
            graphics->SetDepthState(CMP_LESS_EQUAL, true);
            graphics->SetColorState(BLEND_MODE_REPLACE);
            graphics->SetRasterizerState(CULL_BACK, FILL_SOLID);
            graphics->DrawIndexedInstanced(TRIANGLE_LIST, 0, 3, 0, 0, NUM_OBJECTS);

            graphics->Present();
        }
    }

    void HandleCloseRequest(Event& /* event */)
    {
        graphics->Close();
    }

    AutoPtr<ResourceCache> cache;
    AutoPtr<Graphics> graphics;
    AutoPtr<Input> input;
    AutoPtr<Log> log;
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