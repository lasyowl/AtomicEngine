#include "stdafx.h"
#include "ShaderCompiler.h"
#include "EngineEssential.h"

#include <d3dcompiler.h>

#define CHECK_HRESULT( hr ) \
	if (FAILED (hr)) {\
		throw std::runtime_error( "Device creation failed." );\
	}

