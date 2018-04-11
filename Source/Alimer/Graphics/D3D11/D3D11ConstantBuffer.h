// For conditions of distribution and use, see copyright notice in License.txt

#pragma once

#include "../../AlimerConfig.h"
#include "../GPUObject.h"
#include "../GraphicsDefs.h"
#include <memory>

namespace Alimer
{
	class JSONValue;

	/// GPU buffer for shader constant data.
	class ALIMER_API ConstantBuffer : public RefCounted, public GPUObject
	{
	public:
		/// Construct.
		ConstantBuffer();
		/// Destruct.
		~ConstantBuffer();

		/// Release the buffer.
		void Release() override;

		/// Load from JSON data. Return true on success.
		bool LoadJSON(const JSONValue& source);
		/// Save as JSON data.
		void SaveJSON(JSONValue& dest);
		/// Define the constants being used and create the GPU-side buffer. Return true on success.
		bool Define(ResourceUsage usage, const std::vector<Constant>& srcConstants);
		/// Define the constants being used and create the GPU-side buffer. Return true on success.
		bool Define(ResourceUsage usage, uint32_t numConstants, const Constant* srcConstants);
		/// Set a constant by index. Optionally specify how many elements to update, default all. Return true on success.
		bool SetConstant(uint32_t index, const void* data, uint32_t numElements = 0);
		/// Set a constant by name. Optionally specify how many elements to update, default all. Return true on success.
		bool SetConstant(const String& name, const void* data, uint32_t numElements = 0);
		/// Set a constant by name. Optionally specify how many elements to update, default all. Return true on success.
		bool SetConstant(const char* name, const void* data, uint32_t numElements = 0);
		/// Apply to the GPU-side buffer if has changes. Can only be used once on an immutable buffer. Return true on success.
		bool Apply();
		/// Set raw data directly to the GPU-side buffer. Optionally copy back to the shadow constants. Return true on success.
		bool SetData(const void* data, bool copyToShadow = false);
		/// Set a constant by index, template version.
		template <class T> bool SetConstant(uint32_t index, const T& data, uint32_t numElements = 0) { return SetConstant(index, (const void*)&data, numElements); }
		/// Set a constant by name, template version.
		template <class T> bool SetConstant(const String& name, const T& data, uint32_t numElements = 0) { return SetConstant(name, (const void*)&data, numElements); }
		/// Set a constant by name, template version.
		template <class T> bool SetConstant(const char* name, const T& data, uint32_t numElements = 0) { return SetConstant(name, (const void*)&data, numElements); }

		/// Return number of constants.
		uint32_t NumConstants() const { return  static_cast<uint32_t>(_constants.size()); }
		/// Return the constant descriptions.
		const std::vector<Constant>& Constants() const { return _constants; }
		/// Return the index of a constant, or NPOS if not found.
		size_t FindConstantIndex(const String& name) const;
		/// Return the index of a constant, or NPOS if not found.
		size_t FindConstantIndex(const char* name) const;
		/// Return pointer to the constant value, or null if not found.
		const void* GetConstantValue(size_t index, uint32_t elementIndex = 0) const;
		/// Return pointer to the constant value, or null if not found.
		const void* GetConstantValue(const String& name, uint32_t elementIndex = 0) const;
		/// Return pointer to the constant value, or null if not found.
		const void* GetConstantValue(const char* name, uint32_t elementIndex = 0) const;

		/// Return constant value, template version.
		template <class T> T GetConstantValue(uint32_t index, uint32_t elementIndex = 0) const
		{
			const void* value = GetConstantValue(index, elementIndex);
			return value ? *(reinterpret_cast<const T*>(value)) : T();
		}

		/// Return constant value, template version.
		template <class T> T ConstantValue(const String& name, size_t elementIndex = 0) const
		{
			const void* value = ConstantValue(name, elementIndex);
			return value ? *(reinterpret_cast<const T*>(value)) : T();
		}

		/// Return constant value, template version.
		template <class T> T ConstantValue(const char* name, size_t elementIndex = 0) const
		{
			const void* value = ConstantValue(name, elementIndex);
			return value ? *(reinterpret_cast<const T*>(value)) : T();
		}

		/// Return total byte size of the buffer.
		uint32_t ByteSize() const { return _byteSize; }
		/// Return whether buffer has unapplied changes.
		bool IsDirty() const { return dirty; }
		/// Return resource usage type.
		ResourceUsage Usage() const { return usage; }
		/// Return whether is dynamic.
		bool IsDynamic() const { return usage == USAGE_DYNAMIC; }
		/// Return whether is immutable.
		bool IsImmutable() const { return usage == USAGE_IMMUTABLE; }

		/// Return the D3D11 buffer. Used internally and should not be called by portable application code.
		void* D3DBuffer() const { return buffer; }

		/// Element sizes by type.
		static const size_t elementSize[];

		/// Index for "constant not found."
		static const size_t NPOS = (size_t)-1;

	private:
		/// Create the GPU-side constant buffer. Called on the first Apply() if the buffer is immutable. Return true on success.
		bool Create(const void* data = nullptr);

		/// D3D11 buffer.
		void* buffer;
		/// Constant definitions.
		std::vector<Constant> _constants;
		/// CPU-side data where updates are collected before applying.
		std::unique_ptr<uint8_t[]> _shadowData;
		/// Total byte size.
		uint32_t _byteSize{};
		/// Resource usage type.
		ResourceUsage usage;
		/// Dirty flag.
		bool dirty;
	};

}