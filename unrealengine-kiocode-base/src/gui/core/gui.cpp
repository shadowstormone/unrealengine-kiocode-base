#include "gui.h"

#include "../modmenu.h"
#include "../../config.h"
#include "../../includes.h"
#include "../../features/core.h"
#include "../../features/main_loop.h"
#include "../../utils/general.h"
#include "../../utils/imgui/fonts/font.h"
#include "../../utils/imgui/imgui_helper.h"
#include "../../../resource.h"

/* +++++++++++++++++++++++++++++++++++ UTILS FONT +++++++++++++++++++++++++++++++++++ */

extern HMODULE g_hModule;
#define LOG(x) std::cout << x << std::endl

void* LoadFontFromResource(int id, DWORD& size)
{
    HRSRC res = FindResource(g_hModule, MAKEINTRESOURCE(id), RT_RCDATA);

    if (!res)
        return nullptr;

    HGLOBAL data = LoadResource(g_hModule, res);
    if (!data)
        return nullptr;

    size = SizeofResource(g_hModule, res);
    return LockResource(data);
}

/* +++++++++++++++++++++++++++++++++++ UTILS FONT +++++++++++++++++++++++++++++++++++ */

void GUI::InitImGui()
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(pDevice, pContext);
}

void GUI::LoadFonts()
{
    ImGuiIO& io = ImGui::GetIO();

    //LOG("[Fonts] ===== Font loading started =====");
    //LOG("[Fonts] Module handle: " << g_hModule);

    DWORD fontSize = 0;
    void* fontData = LoadFontFromResource(IDR_RCDATA1, fontSize);

    if (!fontData)
    {
        LOG("[Fonts] ERROR: Resource IDR_RCDATA1 not found");
        return;
    }

    //LOG("[Fonts] Resource loaded");
    //LOG("[Fonts] Font data ptr : " << fontData);
    //LOG("[Fonts] Font size     : " << fontSize << " bytes");

    if (fontSize < 1024)
    {
        LOG("[Fonts] WARNING: Font size is suspiciously small");
    }

    ImFontConfig cfg{};
    cfg.FontDataOwnedByAtlas = false;
    cfg.OversampleH = 3;
    cfg.OversampleV = 1;
    cfg.PixelSnapH = true;

    //LOG("[Fonts] Creating ImFont...");
    //LOG("[Fonts] Font size (px) : 20.0");
    //LOG("[Fonts] Cyrillic range : enabled");

	float sizePixels = 18.0f;

    Config::m_pGameFont = io.Fonts->AddFontFromMemoryTTF(
        fontData,
        fontSize,
        sizePixels,
        &cfg,
        io.Fonts->GetGlyphRangesCyrillic()
    );

    if (!Config::m_pGameFont)
    {
        LOG("[Fonts] ERROR: AddFontFromMemoryTTF failed");
        return;
    }

    //LOG("[Fonts] ImFont created successfully");
    //LOG("[Fonts] Glyphs count  : " << Config::m_pGameFont->Glyphs.Size);
    //LOG("[Fonts] Ascent        : " << Config::m_pGameFont->Ascent);
    //LOG("[Fonts] Descent       : " << Config::m_pGameFont->Descent);

    io.Fonts->Build();

    //LOG("[Fonts] Font atlas build finished");
    //LOG("[Fonts] Atlas texture size : "
    //    << io.Fonts->TexWidth << "x" << io.Fonts->TexHeight);

    //LOG("[Fonts] ===== Font loading finished =====");
}

void GUI::RenderMouse() 
{
	if (!Config::m_bShowMouse) return;

	ImColor color = Config::m_bRainbowMouse ? Config::m_cRainbow : Config::m_cMouseColor;

	switch (Config::m_nMouseType) {
	case 0:
		ImGui::GetForegroundDrawList()->AddCircleFilled(ImGui::GetMousePos(), 4, color);
		break;
	case 1:
		ImGuiHelper::DrawOutlinedTextForeground(Config::m_pGameFont, ImVec2(Config::System::m_MousePos.x, Config::System::m_MousePos.y), 13.0f, color, false, "X");
		break;
	case 2:
		if (!ImGui::GetIO().MouseDrawCursor) {
			ImGui::GetIO().MouseDrawCursor = true;
		}
		break;
	}
}

void GUI::RenderUI() 
{

	if (Config::m_bShowMenu)
	{
		RenderMouse();

		// our menu
		ModMenu::Window();
	}

	if (Config::m_bWatermark)
	{
		ImGuiHelper::DrawOutlinedText(Config::m_pGameFont, ImVec2(Config::System::m_ScreenCenter.X, Config::System::m_ScreenSize.Y - 20), 13.0f, Config::m_cRainbow, true, Config::System::m_cAuthor);
		ImGuiHelper::DrawOutlinedText(Config::m_pGameFont, ImVec2(Config::System::m_ScreenCenter.X, 5), 13.0f, Config::m_cRainbow, true, "[ %.1f FPS ]", ImGui::GetIO().Framerate);
	}

	if (Config::m_bCrosshair) {
		MainLoop::GetInstance().DrawCrosshair();
	}

	if (Config::m_bAimbotFOVCheck) {
		ImGui::GetForegroundDrawList()->AddCircle(ImVec2(Config::System::m_ScreenCenter.X, Config::System::m_ScreenCenter.Y), Config::m_fAimbotFOV, ImColor(255, 255, 255), 360);
	}

}

