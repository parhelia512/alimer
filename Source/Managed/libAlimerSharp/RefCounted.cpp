/*
** Alimer - Copyright (C) 2016-2018 Amer Koleci.
**
** This file is subject to the terms and conditions defined in
** file 'LICENSE', which is part of this source code package.
*/

#include "Alimer.h"
using namespace Alimer;

extern "C"
{
	ALIMER_DLL_EXPORT uint32_t RefCounted_Refs(RefCounted* _this)
	{
		return _this->Refs();
	}

	ALIMER_DLL_EXPORT uint32_t RefCounted_WeakRefs(RefCounted* _this)
	{
		return _this->WeakRefs();
	}

	ALIMER_DLL_EXPORT void Ref_TryDelete(RefCounted* _this)
	{
		if (_this && _this->RefCountPtr() && !_this->Refs()) {
			delete _this;
		}
	}
}
