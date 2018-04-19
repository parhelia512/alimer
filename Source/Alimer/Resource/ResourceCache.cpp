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

#include "../Debug/Log.h"
#include "../Debug/Profiler.h"
#include "../IO/File.h"
#include "../IO/FileSystem.h"
#include "Image.h"
#include "JSONFile.h"
#include "ResourceCache.h"

using namespace std;

namespace Alimer
{
	ResourceCache::ResourceCache()
	{
		RegisterSubsystem(this);
	}

	ResourceCache::~ResourceCache()
	{
		UnloadAllResources(true);
		RemoveSubsystem(this);
	}

	bool ResourceCache::AddResourceDir(const string& pathName, bool addFirst)
	{
		ALIMER_PROFILE(AddResourceDir);

		if (!DirectoryExists(pathName))
		{
			ALIMER_LOGERROR("Could not open directory " + pathName);
			return false;
		}

		string fixedPath = SanitateResourceDirName(pathName);

		// Check that the same path does not already exist
		for (size_t i = 0; i < resourceDirs.size(); ++i)
		{
			if (!str::Compare(resourceDirs[i], fixedPath, false))
				return true;
		}

		if (addFirst)
			resourceDirs.insert(resourceDirs.begin(), fixedPath);
		else
			resourceDirs.push_back(fixedPath);

		ALIMER_LOGINFO("Added resource path '{}'", fixedPath);
		return true;
	}

	bool ResourceCache::AddManualResource(Resource* resource)
	{
		if (!resource)
		{
			ALIMER_LOGERROR("Null manual resource");
			return false;
		}

		if (resource->GetName().empty())
		{
			ALIMER_LOGERROR("Manual resource with empty name, can not add");
			return false;
		}

		resources[std::make_pair(resource->GetType(), StringHash(resource->GetName()))] = resource;
		return true;
	}

	void ResourceCache::RemoveResourceDir(const string& pathName)
	{
		// Convert path to absolute
		string fixedPath = SanitateResourceDirName(pathName);

		for (size_t i = 0; i < resourceDirs.size(); ++i)
		{
			if (!str::Compare(resourceDirs[i], fixedPath, false))
			{
				resourceDirs.erase(resourceDirs.begin() + i);
				ALIMER_LOGINFO("Removed resource path " + fixedPath);
				return;
			}
		}
	}

	void ResourceCache::UnloadResource(StringHash type, const string& name, bool force)
	{
		auto key = std::make_pair(type, StringHash(name));
		auto it = resources.find(key);
		if (it == resources.end())
			return;

		Resource* resource = it->second;
		if (resource->Refs() == 1 || force)
			resources.erase(key);
	}

	void ResourceCache::UnloadResources(StringHash type, bool force)
	{
		// In case resources refer to other resources, repeat until there are no further unloads
		for (;;)
		{
			size_t unloaded = 0;

			for (auto it = resources.begin(); it != resources.end();)
			{
				auto current = it++;
				if (current->first.first == type)
				{
					Resource* resource = current->second;
					if (resource->Refs() == 1 || force)
					{
						resources.erase(current);
						++unloaded;
					}
				}
			}

			if (!unloaded)
				break;
		}
	}

	void ResourceCache::UnloadResources(StringHash type, const string& partialName, bool force)
	{
		// In case resources refer to other resources, repeat until there are no further unloads
		for (;;)
		{
			size_t unloaded = 0;

			for (auto it = resources.begin(); it != resources.end();)
			{
				auto current = it++;
				if (current->first.first == type)
				{
					Resource* resource = current->second;
					if (str::StartsWith(resource->GetName(), partialName) && (resource->Refs() == 1 || force))
					{
						resources.erase(current);
						++unloaded;
					}
				}
			}

			if (!unloaded)
				break;
		}
	}

	void ResourceCache::UnloadResources(const string& partialName, bool force)
	{
		// In case resources refer to other resources, repeat until there are no further unloads
		for (;;)
		{
			size_t unloaded = 0;

			for (auto it = resources.begin(); it != resources.end();)
			{
				auto current = it++;
				Resource* resource = current->second;
				if (str::StartsWith(resource->GetName(), partialName) && (!resource->Refs() == 1 || force))
				{
					resources.erase(current);
					++unloaded;
				}
			}

			if (!unloaded)
				break;
		}
	}

	void ResourceCache::UnloadAllResources(bool force)
	{
		// In case resources refer to other resources, repeat until there are no further unloads
		for (;;)
		{
			size_t unloaded = 0;

			for (auto it = resources.begin(); it != resources.end();)
			{
				auto current = it++;
				Resource* resource = current->second;
				if (resource->Refs() == 1 || force)
				{
					resources.erase(current);
					++unloaded;
				}
			}

			if (!unloaded)
				break;
		}
	}

	bool ResourceCache::ReloadResource(Resource* resource)
	{
		if (!resource)
			return false;

		std::unique_ptr<Stream> stream = OpenResource(resource->GetName());
		return stream ? resource->Load(*stream) : false;
	}

	std::unique_ptr<Stream> ResourceCache::OpenResource(const string& nameIn)
	{
		string name = SanitateResourceName(nameIn);
		std::unique_ptr<Stream> ret;

		for (size_t i = 0; i < resourceDirs.size(); ++i)
		{
			if (FileExists(resourceDirs[i] + name))
			{
				// Construct the file first with full path, then rename it to not contain the resource path,
				// so that the file's name can be used in further OpenResource() calls (for example over the network)
				ret = std::make_unique<File>(resourceDirs[i] + name);
				break;
			}
		}

		// Fallback using absolute path
		if (!ret)
			ret = std::make_unique<File>(name);

		if (!ret->IsReadable())
		{
			ALIMER_LOGERROR("Could not open resource file " + name);
			ret.reset();
		}

		return ret;
	}

	Resource* ResourceCache::LoadResource(
		StringHash type,
		const std::string& name_)
	{
		std::string name = SanitateResourceName(name_);

		// If empty name, return null pointer immediately without logging an error
		if (name.empty())
			return nullptr;

		// Check for existing resource
		auto key = std::make_pair(type, StringHash(name));
		auto it = resources.find(key);
		if (it != resources.end())
			return it->second;

		SharedPtr<Object> newObject = Create(type);
		if (!newObject)
		{
			ALIMER_LOGERROR("Could not load unknown resource type {}", Object::GetTypeNameFromType(type));
			return nullptr;
		}
		Resource* newResource = dynamic_cast<Resource*>(newObject.Get());
		if (!newResource)
		{
			ALIMER_LOGERROR("Type {} is not a resource", Object::GetTypeNameFromType(type));
			return nullptr;
		}

		// Attempt to load the resource
		std::unique_ptr<Stream> stream = OpenResource(name);
		if (!stream)
			return nullptr;

		ALIMER_LOGDEBUG("Loading resource {}", name);
		newResource->SetName(name);
		if (!newResource->Load(*stream))
			return nullptr;

		// Store to cache
		resources[key] = newResource;
		return newResource;
	}

	void ResourceCache::ResourcesByType(std::vector<Resource*>& result, StringHash type) const
	{
		result.clear();

		for (auto it = resources.begin(); it != resources.end(); ++it)
		{
			if (it->second->GetType() == type)
			{
				result.push_back(it->second);
			}
		}
	}

	bool ResourceCache::Exists(const string& nameIn) const
	{
		string name = SanitateResourceName(nameIn);

		for (size_t i = 0; i < resourceDirs.size(); ++i)
		{
			if (FileExists(resourceDirs[i] + name))
				return true;
		}

		// Fallback using absolute path
		return FileExists(name);
	}

	string ResourceCache::ResourceFileName(const string& name) const
	{
		for (size_t i = 0; i < resourceDirs.size(); ++i)
		{
			if (FileExists(resourceDirs[i] + name))
				return resourceDirs[i] + name;
		}

		return string();
	}

	string ResourceCache::SanitateResourceName(const string& nameIn) const
	{
		// Sanitate unsupported constructs from the resource name
		string name = NormalizePath(nameIn);
		name = str::Replace(name, "../", "");
		name = str::Replace(name, "./", "");

		// If the path refers to one of the resource directories, normalize the resource name
		if (resourceDirs.size())
		{
			string namePath = GetPath(name);
			string exePath = GetExecutableDir();
			for (size_t i = 0; i < resourceDirs.size(); ++i)
			{
				string relativeResourcePath = resourceDirs[i];
				if (str::StartsWith(relativeResourcePath, exePath))
					relativeResourcePath = relativeResourcePath.substr(exePath.length());

				if (str::StartsWith(namePath, resourceDirs[i], false))
					namePath = namePath.substr(resourceDirs[i].length());
				else if (str::StartsWith(namePath, relativeResourcePath, false))
					namePath = namePath.substr(relativeResourcePath.length());
			}

			name = namePath + GetFileNameAndExtension(name);
		}

		return str::Trim(name);
	}

	string ResourceCache::SanitateResourceDirName(const string& nameIn) const
	{
		// Convert path to absolute
		string fixedPath = AddTrailingSlash(nameIn);
		if (!IsAbsolutePath(fixedPath))
		{
			fixedPath = GetCurrentDir() + fixedPath;
		}

		// Sanitate away /./ construct
		fixedPath = str::Replace(fixedPath, "/./", "/");

		return str::Trim(fixedPath);
	}

	void RegisterResourceLibrary()
	{
		static bool registered = false;
		if (registered)
			return;
		registered = true;

		Image::RegisterObject();
		JSONFile::RegisterObject();
	}

}
