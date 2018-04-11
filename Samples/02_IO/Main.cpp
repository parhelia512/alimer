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
#include "Object/ObjectResolver.h"
#include "Debug/DebugNew.h"

#ifdef _MSC_VER
#include <crtdbg.h>
#endif

#include <cstdio>
#include <cstdlib>

using namespace Alimer;

class TestEvent : public Event
{
public:
	int data;
};

class TestEventSender : public Object
{
	OBJECT(TestEventSender);
	
public:
	void SendTestEvent(int value)
	{
		testEvent.data = value;
		SendEvent(testEvent);
	}
	
	TestEvent testEvent;
};

class TestEventReceiver : public Object
{
	OBJECT(TestEventReceiver);
	
public:
	void SubscribeToTestEvent(TestEventSender* sender)
	{
		SubscribeToEvent(sender->testEvent, &TestEventReceiver::HandleTestEvent);
	}
	
	void HandleTestEvent(TestEvent& event)
	{
		printf("Receiver %08x got TestEvent from %08x with data %d\n", *(int*)this, *(int*)event.Sender(), event.data);
	}
};

class TestSerializable : public Serializable
{
	OBJECT(TestSerializable);

public:
	TestSerializable() :
		intVariable(0)
	{
	}

	static void RegisterObject()
	{
		RegisterFactory<TestSerializable>();
		RegisterAttribute("intVariable", &TestSerializable::IntVariable, &TestSerializable::SetIntVariable);
		RegisterRefAttribute("stringVariable", &TestSerializable::StringVariable, &TestSerializable::SetStringVariable);
	}

	void SetIntVariable(int newValue) { intVariable = newValue; }
	int IntVariable() const { return intVariable; }

	void SetStringVariable(const String& newValue) { stringVariable = newValue; }
	const String& StringVariable() const { return stringVariable; }

private:
	int intVariable;
	String stringVariable;
};

int main()
{
	#ifdef _MSC_VER
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	#endif

	printf("Size of Event: %d\n", sizeof(Event));
	printf("Size of File: %d\n", sizeof(File));
	printf("Size of JSONValue: %d\n", sizeof(JSONValue));
	printf("Size of Serializable: %d\n", sizeof(Serializable));

	{
		printf("\nTesting objects & events\n");
		Object::RegisterFactory<TestEventSender>();
		Object::RegisterFactory<TestEventReceiver>();
		
		TestEventSender* sender = Object::Create<TestEventSender>();
		TestEventReceiver* receiver1 = Object::Create<TestEventReceiver>();
		TestEventReceiver* receiver2 = Object::Create<TestEventReceiver>();
		printf("Type of sender is %s\n", sender->TypeName().CString());
		receiver1->SubscribeToTestEvent(sender);
		receiver2->SubscribeToTestEvent(sender);
		sender->SendTestEvent(1);
		delete receiver2;
		sender->SendTestEvent(2);
		delete receiver1;
		delete sender;
	}
	
	{
		printf("\nTesting logging and profiling\n");
		Log log;
		Profiler profiler;
		
		profiler.BeginFrame();
		
		{
			ALIMER_PROFILE(OpenLog);
			log.Open("02_IO.log");
		}
		
		{
			ALIMER_PROFILE(WriteMessages);
			LOGDEBUG("Debug message");
			LOGINFO("Info message");
			LOGERROR("Error message");
			LOGINFOF("Formatted message: %d", 100);
		}
		
		profiler.EndFrame();
		
		printf("%s\n", profiler.OutputResults().CString());
	}
	
	{
		printf("\nTesting JSONValue\n");
		JSONValue org;
		org["name"] = "S.C.E.P.T.R.E";
		org["longName"] = "Sectarian Chosen Elite Privileged To Rule & Exterminate";
		org["isEvil"] = true;
		org["members"] = 218;
		org["honor"] = JSONValue();
		JSONValue officers;
		officers.Push("Ahriman");
		officers.Push("Lilith");
		officers.Push("Suhrim");
		org["officers"] = officers;
		org["allies"].SetEmptyArray();
		org["sightings"].SetEmptyObject();
		
		String jsonString = org.ToString();
		printf("%s\n", jsonString.CString());
		printf("JSON text size: %d\n", jsonString.Length());
		
		JSONValue parsed;
		if (parsed.FromString(jsonString))
		{
			printf("JSON parse successful\n");
			if (parsed == org)
				printf("Parsed data equals original\n");
			else
				printf("Parsed data does not equal original\n");
		}
		else
			printf("Failed to parse JSON from text\n");
		
		VectorBuffer buffer;
		buffer.Write(org);
		printf("JSON binary size: %d\n", buffer.Size());
		buffer.Seek(0);
		JSONValue binaryParsed = buffer.Read<JSONValue>();
		if (binaryParsed == org)
			printf("Binary parsed data equals original\n");
		else
			printf("Binary parsed data does not equal original\n");
	}

	{
		printf("\nTesting Serializable\n");

		TestSerializable::RegisterObject();

		std::unique_ptr<TestSerializable> instance(new TestSerializable());
		instance->SetIntVariable(100);
		instance->SetStringVariable("Test!");

		JSONValue saveData;
		instance->SaveJSON(saveData);
		printf("Object JSON data: %s\n", saveData.ToString().CString());

		VectorBuffer binarySaveData;
		instance->Save(binarySaveData);
		printf("Object binary data size: %d\n", binarySaveData.Size());

		std::unique_ptr<TestSerializable> instance2(new TestSerializable());
		ObjectResolver res;
		instance2->LoadJSON(saveData, res);
		printf("Loaded variables (JSON): int %d string: %s\n", instance2->IntVariable(), instance2->StringVariable().CString());
		
		std::unique_ptr<TestSerializable> instance3(new TestSerializable());
		ObjectResolver res2;
		binarySaveData.Seek(0);
		instance3->Load(binarySaveData, res2);
		printf("Loaded variables (binary): int %d string: %s\n", instance3->IntVariable(), instance3->StringVariable().CString());
	}

	return 0;
}