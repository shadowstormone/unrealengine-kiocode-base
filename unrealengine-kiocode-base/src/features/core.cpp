#include "core.h"

#include <iostream>
#include <Windows.h>

#include "../config.h"
#include "../features/main_loop.h"
#include "../utils/validity.h"
#include "../utils/general.h"
//#include "../lua/lua.h"
#include "../../libs/kiero/minhook/include/MinHook.h"
#include "../gui/core/gui.h"

void Core::Setup()
{
	if (!Core::InitVars()) assert("Unable to finish the first setup");
	EnableHooks();
}

void Core::EnableHooks() // if needed hooks are avaiable
{
	// EXAMPLE
	//// Health__TakeDamage
	//if (MH_CreateHook(reinterpret_cast<LPVOID*>(
	//	Offsets::Health__TakeDamage_Offset),
	//	&HooksFunctions::Health__TakeDamage_hook,
	//	(LPVOID*)&HooksFunctions::Health__TakeDamage) == MH_OK)
	//{
	//	MH_EnableHook(reinterpret_cast<LPVOID*>(Offsets::Health__TakeDamage_Offset));
	//}
}

bool Core::InitVars() {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x03);
	std::cout << "\n*******************************************************************************" << std::endl;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
	std::cout << R"(  
	 ______     __  __     ______     _____     ______     __     __    
	/\  ___\   /\ \_\ \   /\  __ \   /\  __-.  /\  __ \   /\ \  _ \ \    
	\ \___  \  \ \  __ \  \ \  __ \  \ \ \/\ \ \ \ \/\ \  \ \ \/ ".\ \   
	 \/\_____\  \ \_\ \_\  \ \_\ \_\  \ \____-  \ \_____\  \ \__/".~\_\    
	  \/_____/   \/_/\/_/   \/_/\/_/   \/____/   \/_____/   \/_/   \/_/
    )" << std::endl;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x03);
	std::cout << "\n*******************************************************************************\n" << std::endl;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x0F);

	// not rly usefull method call because is continuosly called in the main loop 
	return MainLoop::GetInstance().UpdateSDK(true);
}


void Core::HandleInputs() {

	// open/close menu
	if (GetAsyncKeyState(Config::Keys::m_cShowMenuKey) & 1)
	{
		Config::m_bShowMenu = !Config::m_bShowMenu;
	}

	// close all hooks (probably you will never use it)
	if (GetAsyncKeyState(Config::Keys::m_cDeAttachKey) & 1)
	{
		MH_DisableHook(MH_ALL_HOOKS);
		MH_Uninitialize();
		Config::m_bShowMenu = false;

		//if (LuaEditor::GetInstance().LuaState != NULL)
		//{
		//	lua_close(LuaEditor::GetInstance().LuaState);
		//}
	}
}

LRESULT __stdcall GUI::WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (Config::m_bShowMenu && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	if (Config::m_bShowMenu)
		return true;

	return CallWindowProcA(oWndProc, hWnd, uMsg, wParam, lParam);
}

// main hook thread (it's a loop)
HRESULT __stdcall GUI::hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	#pragma region SYSTEM THINGS

	if (!HkPresentInitialized)
	{
		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice)))
		{
			pDevice->GetImmediateContext(&pContext);
			DXGI_SWAP_CHAIN_DESC sd;
			pSwapChain->GetDesc(&sd);
			window = sd.OutputWindow;
			ID3D11Texture2D* pBackBuffer = nullptr;
			pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
			pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
			pBackBuffer->Release();
			oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProcWrapper);

			InitImGui();
			LoadFonts();

			HkPresentInitialized = true;
		}
		else return oPresent(pSwapChain, SyncInterval, Flags);
	}

	pContext->RSGetViewports(&Config::System::m_vps, &Config::System::m_Viewport);
	Config::System::m_ScreenSize = { Config::System::m_Viewport.Width, Config::System::m_Viewport.Height };
	Config::System::m_ScreenCenter = { Config::System::m_Viewport.Width / 2.0f, Config::System::m_Viewport.Height / 2.0f };


	DWORD currentTime = GetTickCount64();

	GetCursorPos(&Config::System::m_MousePos);
	ScreenToClient(window, &Config::System::m_MousePos);


	#pragma region IMGUI THINGS
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	#pragma endregion


	#pragma endregion

	#pragma region Our context
	// can cause crash if you try to get an unexisting material
	/*SDK::UMaterialInterface* material = SDK::UObject::FindObject<SDK::UMaterial>("Material WireframeMaterial.WireframeMaterial");
	if (!material) {
		std::cout << "Error: material not found" << std::endl;
	}
	else {
		Config::ChamsMaterial = SDK::UKismetMaterialLibrary::CreateDynamicMaterialInstance(Config::World, material, Utility::StrToName(L"ChamsVisibleMaterial"));
		if (!Config::ChamsMaterial) {
			std::cout << "Error: ChamsMaterial not found" << std::endl;
		} else {
			std::cout << "ChamsMaterial Found" << std::endl;
		}
	}*/

	Core::GetInstance().HandleInputs();

	// our ui
	RenderUI();

	// cheats
	try {
		MainLoop::GetInstance().Update(currentTime);
	}
	catch (const std::exception& e) {
		std::cout << e.what();
		MessageBoxA(NULL, e.what(), "Error", MB_OK | MB_ICONERROR);
	}

	#pragma endregion

	#pragma region SYSTEM THINGS

	#pragma region IMGUI THINGS

	ImGui::Render();

	#pragma endregion

	if (currentTime - Config::m_nLastTick > 5)
	{
		Config::m_nLastTick = currentTime;
	}

	pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	return oPresent(pSwapChain, SyncInterval, Flags);
	#pragma endregion
}