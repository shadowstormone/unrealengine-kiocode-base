#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>

#include "../libs/kiero/kiero.h"
#include "../libs/imgui/imgui.h"
#include "../libs/imgui/imgui_impl_win32.h"
#include "../libs/imgui/imgui_impl_dx11.h"
#include "sdk/Remnant_From_the_Ashes/SDK.hpp"

typedef HRESULT(__stdcall* Present) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);
typedef uintptr_t PTR;

/* Basic.cpp -> Engine_functions.cpp -> CoreUObject_functions.cpp */