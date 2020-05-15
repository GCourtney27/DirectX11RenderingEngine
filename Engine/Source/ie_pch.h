#pragma once

// === Standard Library === //
#include <memory>
#include <utility>
#include <iostream>
#include <algorithm>
#include <functional>

#include <array>
#include <map>
#include <vector>
#include <string>
#include <thread>
#include <queue>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

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

// === Insight Specific === //
#include "Insight/Core/Log.h"
#include "Insight/Core/Interfaces.h"

// === Windows Library === //
#ifdef IE_PLATFORM_WINDOWS

	// Windows API	
	#include <Windows.h>
	#include <wrl/client.h>

	// Direct3D 12
	#include <SimpleMath.h>
	#include <DirectXMath.h>
	#include <d3d12.h>
	#include <dxgi1_4.h>
	#include <D3Dcompiler.h>
	#include <dxgi1_2.h>
	#include <d3dx12.h>
	#include <wincodec.h>

#endif // IE_PLATFORM_WINDOWS