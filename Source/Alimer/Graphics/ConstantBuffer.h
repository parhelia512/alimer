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

#pragma once

#include "../Graphics/Buffer.h"
#include "../nlohmann/json.hpp"
using json = nlohmann::json;

namespace Alimer
{
	/// GPU buffer for shader constant data.
	class ALIMER_API ConstantBuffer final : public Buffer
	{
	public:
		/// Construct.
		ConstantBuffer();
		/// Destruct.
		~ConstantBuffer() override;

		/// Load from JSON data. Return true on success.
		bool LoadJSON(const json& source);
		/// Save as JSON data.
		void SaveJSON(json& dest);
		/// Define the constants being used and create the GPU-side buffer. Return true on success.
		bool Define(const std::vector<Constant>& srcConstants, bool hostVisible);
		/// Define the constants being used and create the GPU-side buffer. Return true on success.
		bool Define(uint32_t numConstants, const Constant* srcConstants, bool hostVisible);
		/// Set a constant by index. Optionally specify how many elements to update, default all. Return true on success.
		bool SetConstant(uint32_t index, const void* data, uint32_t numElements = 0);
		/// Set a constant by name. Optionally specify how many elements to update, default all. Return true on success.
		bool SetConstant(const std::string& name, const void* data, uint32_t numElements = 0);
		/// Apply to the GPU-side buffer if has changes. Can only be used once on an immutable buffer. Return true on success.
		bool Apply();
		/// Set raw data directly to the GPU-side buffer. Optionally copy back to the shadow constants. Return true on success.
		bool SetData(const void* data, bool copyToShadow = false);
		/// Set a constant by index, template version.
		template <class T> bool SetConstant(uint32_t index, const T& data, uint32_t numElements = 0) { return SetConstant(index, (const void*)&data, numElements); }
		/// Set a constant by name, template version.
		template <class T> bool SetConstant(const std::string& name, const T& data, uint32_t numElements = 0) { return SetConstant(name, (const void*)&data, numElements); }

		/// Return number of constants.
		uint32_t NumConstants() const { return  static_cast<uint32_t>(_constants.size()); }
		/// Return the constant descriptions.
		const std::vector<Constant>& Constants() const { return _constants; }
		/// Return the index of a constant, or NPOS if not found.
		uint32_t FindConstantIndex(const std::string& name) const;
		/// Return pointer to the constant value, or null if not found.
		const void* GetConstantValue(uint32_t index, uint32_t elementIndex = 0) const;
		/// Return pointer to the constant value, or null if not found.
		const void* GetConstantValue(const std::string& name, uint32_t elementIndex = 0) const;

		/// Return constant value, template version.
		template <class T> T GetConstantValue(uint32_t index, uint32_t elementIndex = 0) const
		{
			const void* value = GetConstantValue(index, elementIndex);
			return value ? *(reinterpret_cast<const T*>(value)) : T();
		}

		/// Return constant value, template version.
		template <class T> T GetConstantValue(const std::string& name, uint32_t elementIndex = 0) const
		{
			const void* value = GetConstantValue(name, elementIndex);
			return value ? *(reinterpret_cast<const T*>(value)) : T();
		}

		/// Return whether buffer has unapplied changes.
		bool IsDirty() const { return _dirty; }

		/// Index for "constant not found."
		static const uint32_t NPOS = (uint32_t)-1;

	private:
		/// Constant definitions.
		std::vector<Constant> _constants;
		/// Dirty flag.
		bool _dirty{ false };
	};
}