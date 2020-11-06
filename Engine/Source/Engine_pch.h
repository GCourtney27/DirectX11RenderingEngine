#pragma once

// === Standard Library === //
#include <map>
#include <array>
#include <stack>
#include <queue>
#include <vector>
#include <future>
#include <string>
#include <math.h>
#include <thread>
#include <memory>
#include <sstream>
#include <utility>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <type_traits>

#if _MSC_VER >= 1700
	using std::shared_ptr;
	using std::weak_ptr;
	using std::unique_ptr;
	using std::static_pointer_cast;
	using std::dynamic_pointer_cast;
	using std::make_shared;
	using std::make_unique;
#elif _MSC_VER == 1600
	using std::tr1::shared_ptr;
	using std::tr1::weak_ptr;
	using std::tr1::unique_ptr;
	using std::tr1::static_pointer_cast;
	using std::tr1::dynamic_pointer_cast;
#endif

// === Third Party === //
// Rapid Json
#include <rapidjson/json.h>
#include <rapidjson/document.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>

// === Retina Specific === //
#include "Insight/Core/Log.h"
#include "Insight/Core/Interfaces.h"
#include "Insight/Math/ie_Vectors.h"
#include "Insight/Utilities/Profiling.h"
#include "Insight/Utilities/String_Helper.h"
#include "Insight/Systems/File_System.h"

// Parameters
	static const uint8_t s_NumRenderContexts = 2;

// -----------
// Platforms |
// -----------

// === Windows === //
#if defined IE_PLATFORM_WINDOWS

	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers.
	#endif

	// Windows API	
	#include <Windows.h>
	#include <wrl/client.h>
	#include <windowsx.h>
	#include <shlobj.h>
	#include "Shlwapi.h"
	#include <strsafe.h>

	// Direct3D 12
	#include <d3d12.h>
	#include <DirectX12/d3dx12.h> 
	#include <WinPixEventRuntime/pix3.h>
	#include <dxcapi.h>

	// Direct3D 11
	#include <d3d11.h>

	// DirectX
	#include <dxgi1_2.h>
	#include <dxgi1_4.h>
	#include <wincodec.h>
	#include <DirectXMath.h>
	#include <D3Dcompiler.h>
	
	using Microsoft::WRL::ComPtr;


#endif // IE_PLATFORM_WINDOWS

// === Mac === //
#ifdef RN_PLATFORM_MAC



#endif // RN_PLATFORM_MAC