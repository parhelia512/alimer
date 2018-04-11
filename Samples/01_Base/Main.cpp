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

using namespace Alimer;

class Test
{
public:
    Test()
    {
        printf("Test constructed\n");
    }

    ~Test()
    {
        printf("Test destroyed\n");
    }
};

class TestRefCounted: public RefCounted
{
public:
    TestRefCounted()
    {
        printf("TestRefCounted constructed\n");
    }

    ~TestRefCounted()
    {
        printf("TestRefCounted destroyed\n");
    }
};

const size_t NUM_ITEMS = 10000;

int main()
{
    #ifdef _MSC_VER
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    #endif
    
    printf("Size of String: %d\n", sizeof(String));
    printf("Size of Vector: %d\n", sizeof(Vector<int>));
    printf("Size of RefCounted: %d\n", sizeof(RefCounted));

    {
        printf("\nTesting AutoPtr assignment\n");
        AutoPtr<Test> ptr1(new Test);
        AutoPtr<Test> ptr2;
        ptr2 = ptr1;
    }

    {
        printf("\nTesting AutoPtr copy construction\n");
        AutoPtr<Test> ptr1(new Test);
        AutoPtr<Test> ptr2(ptr1);
    }

    {
        printf("\nTesting AutoPtr detaching\n");
        AutoPtr<Test> ptr1(new Test);
        // We would now have a memory leak if we don't manually delete the object
        Test* object = ptr1.Detach();
        delete object;
    }

    {
        printf("\nTesting AutoPtr inside a vector\n");
        Vector<AutoPtr<Test> > vec;
        printf("Filling vector\n");
        for (size_t i = 0; i < 4; ++i)
            vec.Push(new Test());
        printf("Clearing vector\n");
        vec.Clear();
    }
    
    {
        printf("\nTesting SharedPtr\n");
        SharedPtr<TestRefCounted> ptr1(new TestRefCounted);
        SharedPtr<TestRefCounted> ptr2(ptr1);
        printf("Number of refs: %d\n", ptr1.Refs());
    }
    
    {
        printf("\nTesting WeakPtr\n");
        TestRefCounted* object = new TestRefCounted;
        WeakPtr<TestRefCounted> ptr1(object);
        WeakPtr<TestRefCounted> ptr2(ptr1);
        printf("Number of weak refs: %d expired: %d\n", ptr1.WeakRefs(), ptr1.IsExpired());
        ptr2.Reset();
        delete object;
        printf("Number of weak refs: %d expired: %d\n", ptr1.WeakRefs(), ptr1.IsExpired());
    }
    
    {
        printf("\nTesting Vector\n");
        HiresTimer t;
        Vector<int> vec;
        SetRandomSeed(0);
        for (size_t i = 0; i < NUM_ITEMS; ++i)
            vec.Push(Rand());
        int sum = 0;
        int count = 0;
        for (auto it = vec.Begin(); it != vec.End(); ++it)
        {
            sum += *it;
            ++count;
        }
        int usec = (int)t.ElapsedUSec();
        printf("Size: %d capacity: %d\n", vec.Size(), vec.Capacity());
        printf("Counted vector items %d, sum: %d\n", count, sum);
        printf("Processing took %d usec\n", usec);
    }
    
    {
        printf("\nTesting String\n");
        HiresTimer t;
        String test;
        for (size_t i = 0; i < NUM_ITEMS/4; ++i)
            test += "Test";
        String test2;
        test2.AppendWithFormat("Size: %d capacity: %d\n", test.Length(), test.Capacity());
        printf(test2.CString());
        test2 = test2.ToUpper();
        printf(test2.CString());
        test2.Replace("SIZE:", "LENGTH:");
        printf(test2.CString());
        int usec = (int)t.ElapsedUSec();
        printf("Processing took %d usec\n", usec);
    }

    return 0;
}