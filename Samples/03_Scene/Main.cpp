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
#	include <crtdbg.h>
#endif

#include <cstdio>
#include <cstdlib>

using namespace Alimer;

int main()
{
    #ifdef _MSC_VER
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    #endif
    
    RegisterSceneLibrary();
    printf("\nTesting scene serialization\n");
    
    Log log;
    //log.Open("03_Scene.log");
    
    {
        Profiler profiler;
        profiler.BeginFrame();

        Scene scene;
        scene.DefineLayer(1, "TestLayer");
        scene.DefineTag(1, "TestTag");
        for (size_t i = 0; i < 10; ++i)
        {
            SpatialNode* node = scene.CreateChild<SpatialNode>("Child" + std::to_string(i));
            node->SetPosition(Vector3(Random(-100.0f, 100.0f), Random(-100.0f, 100.0f), Random(-100.0f, 100.0f)));
            node->SetLayerName("TestLayer");
            node->SetTagName("TestTag");
        }

        {
            File binaryFile("Scene.bin", FileMode::Write);
            scene.Save(binaryFile);
        }

        {
            File jsonFile("Scene.json", FileMode::Write);
            scene.SaveJSON(jsonFile);
        }

        {
            File loadFile("Scene.bin", FileMode::Read);
            Scene loadScene;
            if (loadScene.Load(loadFile))
            {
                printf("Scene loaded successfully from binary data\n");
                for (size_t i = 0; i < loadScene.NumChildren(); ++i)
                {
                    Node* child = loadScene.GetChild(i);
                    printf("Child name: %s layer: %d tag: %d\n", child->GetName().c_str(), child->GetLayer(), child->GetTag());
                }
            }
            else
                printf("Failed to load scene from binary data\n");
        }

        profiler.EndFrame();
        ALIMER_LOGINFO(profiler.OutputResults(false, false, 16));
    }
    
    return 0;
}