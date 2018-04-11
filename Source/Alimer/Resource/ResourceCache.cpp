// For conditions of distribution and use, see copyright notice in License.txt

#include "../Debug/Log.h"
#include "../Debug/Profiler.h"
#include "../IO/File.h"
#include "../IO/FileSystem.h"
#include "Image.h"
#include "JSONFile.h"
#include "ResourceCache.h"

#include "../Debug/DebugNew.h"

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

	bool ResourceCache::AddResourceDir(const String& pathName, bool addFirst)
	{
		ALIMER_PROFILE(AddResourceDir);

		if (!DirExists(pathName))
		{
			LOGERROR("Could not open directory " + pathName);
			return false;
		}

		String fixedPath = SanitateResourceDirName(pathName);

		// Check that the same path does not already exist
		for (size_t i = 0; i < resourceDirs.size(); ++i)
		{
			if (!resourceDirs[i].Compare(fixedPath, false))
				return true;
		}

		if (addFirst)
			resourceDirs.insert(resourceDirs.begin(), fixedPath);
		else
			resourceDirs.push_back(fixedPath);

		LOGINFO("Added resource path " + fixedPath);
		return true;
	}

	bool ResourceCache::AddManualResource(Resource* resource)
	{
		if (!resource)
		{
			LOGERROR("Null manual resource");
			return false;
		}

		if (resource->Name().IsEmpty())
		{
			LOGERROR("Manual resource with empty name, can not add");
			return false;
		}

		resources[std::make_pair(resource->Type(), StringHash(resource->Name()))] = resource;
		return true;
	}

	void ResourceCache::RemoveResourceDir(const String& pathName)
	{
		// Convert path to absolute
		String fixedPath = SanitateResourceDirName(pathName);

		for (size_t i = 0; i < resourceDirs.size(); ++i)
		{
			if (!resourceDirs[i].Compare(fixedPath, false))
			{
				resourceDirs.erase(resourceDirs.begin() + i);
				LOGINFO("Removed resource path " + fixedPath);
				return;
			}
		}
	}

	void ResourceCache::UnloadResource(StringHash type, const String& name, bool force)
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

	void ResourceCache::UnloadResources(StringHash type, const String& partialName, bool force)
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
					if (resource->Name().StartsWith(partialName) && (resource->Refs() == 1 || force))
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

	void ResourceCache::UnloadResources(const String& partialName, bool force)
	{
		// In case resources refer to other resources, repeat until there are no further unloads
		for (;;)
		{
			size_t unloaded = 0;

			for (auto it = resources.begin(); it != resources.end();)
			{
				auto current = it++;
				Resource* resource = current->second;
				if (resource->Name().StartsWith(partialName) && (!resource->Refs() == 1 || force))
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

		std::unique_ptr<Stream> stream = OpenResource(resource->Name());
		return stream ? resource->Load(*stream) : false;
	}

	std::unique_ptr<Stream> ResourceCache::OpenResource(const String& nameIn)
	{
		String name = SanitateResourceName(nameIn);
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
			LOGERROR("Could not open resource file " + name);
			ret.reset();
		}

		return ret;
	}

	Resource* ResourceCache::LoadResource(StringHash type, const String& nameIn)
	{
		String name = SanitateResourceName(nameIn);

		// If empty name, return null pointer immediately without logging an error
		if (name.IsEmpty())
			return nullptr;

		// Check for existing resource
		auto key = std::make_pair(type, StringHash(name));
		auto it = resources.find(key);
		if (it != resources.end())
			return it->second;

		SharedPtr<Object> newObject = Create(type);
		if (!newObject)
		{
			LOGERRORF("Could not load unknown resource type %s", type.ToString().c_str());
			return nullptr;
		}
		Resource* newResource = dynamic_cast<Resource*>(newObject.Get());
		if (!newResource)
		{
			LOGERRORF("Type %s is not a resource", type.ToString().c_str());
			return nullptr;
		}

		// Attempt to load the resource
		std::unique_ptr<Stream> stream = OpenResource(name);
		if (!stream)
			return nullptr;

		LOGDEBUG("Loading resource " + name);
		newResource->SetName(name);
		if (!newResource->Load(*stream))
			return nullptr;

		// Store to cache
		resources[key] = newResource;
		return newResource;
	}

	void ResourceCache::ResourcesByType(Vector<Resource*>& result, StringHash type) const
	{
		result.Clear();

		for (auto it = resources.begin(); it != resources.end(); ++it)
		{
			if (it->second->Type() == type)
				result.Push(it->second);
		}
	}

	bool ResourceCache::Exists(const String& nameIn) const
	{
		String name = SanitateResourceName(nameIn);

		for (size_t i = 0; i < resourceDirs.size(); ++i)
		{
			if (FileExists(resourceDirs[i] + name))
				return true;
		}

		// Fallback using absolute path
		return FileExists(name);
	}

	String ResourceCache::ResourceFileName(const String& name) const
	{
		for (size_t i = 0; i < resourceDirs.size(); ++i)
		{
			if (FileExists(resourceDirs[i] + name))
				return resourceDirs[i] + name;
		}

		return String();
	}

	String ResourceCache::SanitateResourceName(const String& nameIn) const
	{
		// Sanitate unsupported constructs from the resource name
		String name = NormalizePath(nameIn);
		name.Replace("../", "");
		name.Replace("./", "");

		// If the path refers to one of the resource directories, normalize the resource name
		if (resourceDirs.size())
		{
			String namePath = Path(name);
			String exePath = ExecutableDir();
			for (size_t i = 0; i < resourceDirs.size(); ++i)
			{
				String relativeResourcePath = resourceDirs[i];
				if (relativeResourcePath.StartsWith(exePath))
					relativeResourcePath = relativeResourcePath.Substring(exePath.Length());

				if (namePath.StartsWith(resourceDirs[i], false))
					namePath = namePath.Substring(resourceDirs[i].Length());
				else if (namePath.StartsWith(relativeResourcePath, false))
					namePath = namePath.Substring(relativeResourcePath.Length());
			}

			name = namePath + FileNameAndExtension(name);
		}

		return name.Trimmed();
	}

	String ResourceCache::SanitateResourceDirName(const String& nameIn) const
	{
		// Convert path to absolute
		String fixedPath = AddTrailingSlash(nameIn);
		if (!IsAbsolutePath(fixedPath))
			fixedPath = CurrentDir() + fixedPath;

		// Sanitate away /./ construct
		fixedPath.Replace("/./", "/");

		return fixedPath.Trimmed();
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