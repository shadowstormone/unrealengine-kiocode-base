#pragma once

#include "includes.h"

#define RAINBOW_BORDERS TRUE;

class Config 
{
public:

	class System
	{
	public:
		inline static constexpr const char* m_cAppName = "RemnantHack";
		inline static constexpr const char* m_cAuthor = "@Remnant From the Ashes Hack";

		inline static bool m_bInit = false;
		inline static bool m_bUpdateTargetsInDifferentThread = false; // some game is freezing if true
		inline static bool m_bUpdateTargets = true;

		inline static POINT m_MousePos = { 0, 0 };

		inline static UINT m_vps = 1;
		inline static SDK::FVector2D m_ScreenSize = { 0, 0 };
		inline static SDK::FVector2D m_ScreenCenter = { 0, 0 };
		inline static D3D11_VIEWPORT m_Viewport;
	};

	class Keys
	{
	public:
		inline static constexpr const int m_cShowMenuKey = VK_INSERT;
		inline static constexpr const int m_cDeAttachKey = VK_F9;
	};

	class Offsets // get it with https://github.com/Spuckwaffel/UEDumper maybe
	{
	public:
		//uintptr_t Health__TakeDamage_Offset = 0x0;
	};

	inline static ImFont* m_pGameFont = NULL;

	inline static enum MenuTab
	{
		TAB_VISUALS,
		TAB_AIM,
		TAB_EXPLOITS,
		TAB_MISC,
		TAB_DEV
	};

	inline static int m_nTargetFetch = 0;
	
	inline static SDK::UEngine* m_pEngine;
	inline static SDK::UWorld* m_pWorld;
	inline static SDK::APlayerController* m_pMyController;
	inline static SDK::APawn* m_pMyPawn;
	inline static SDK::ACharacter* m_pMyCharacter;
	inline static SDK::ARangedWeapon* m_pMyWeapon;

	inline static std::vector<SDK::AActor*> ATargetsList{};
	inline static std::vector<SDK::ACharacter*> m_TargetsList{};
	inline static SDK::AActor* m_pCurrentTarget;

	inline static ImColor m_TargetPlayerColor = ImColor(255, 0, 0);

	// dumped with Dumper::DumpUBones() 
	// Remnant: From the Ashes - Actual skeleton structure
	inline static std::vector<std::pair<int, int>> m_BonePairs = {
	{2, 30},
	{30, 31},
	{31, 32},
	{32, 33},
	{33, 34},
	{34, 35},
	{35, 36},
	{2, 3},
	{3, 4},
	{4, 5},
	{5, 6},
	{2, 11},
	{11, 12},
	{12, 13},
	{13, 14}
	};

// Global rainbow color
	inline static ImVec4 m_v4Rainbow = ImVec4(0.0f, 0.0f, 0.0f, 0.0f); 
 // Global rainbow color
	inline static ImColor m_cRainbow = ImColor(255.0f / 255, 255.0f / 255, 255.0f / 255); 

	inline static DWORD m_nLastShotTime = 0;
	inline static DWORD m_nLastTick = 0;

	inline static bool m_bShowMenu = false;
	inline static bool m_bWatermark = false;

	inline static bool m_bCameraFovChanger = false;
	inline static float m_fCameraCustomFOV = 80.0f;

	inline static bool m_bTimeScaleChanger = false;
	inline static float m_fTimeScale = 1.0f;

	inline static bool m_bEnableDeveloperOptions = true;

	inline static bool m_bShowInspector = false;

	inline static bool m_bShowMouse = true;
	inline static bool m_bRainbowMouse = false;
	inline static ImColor m_cMouseColor = ImColor(255.0f / 255, 255.0f / 255, 255.0f / 255);
	inline static int m_nMouseType = 0;

	//inline static bool m_bFly = false;

	//inline static bool m_bNoGravity = false;

	inline static bool m_bCrosshair = false;
	inline static bool m_bRainbowCrosshair = false;
	inline static ImColor m_cCrosshairColor = ImColor(255.0f / 255, 255.0f / 255, 255.0f / 255);
	inline static float m_fCrosshairSize = 5.0f;
	inline static int m_nCrosshairType = 0;

	inline static bool m_bPlayersSnapline = false;
	inline static bool m_bRainbowPlayersSnapline = false;
	inline static ImColor m_cPlayersSnaplineColor = ImColor(255.0f / 255, 255.0f / 255, 255.0f / 255);
	inline static int m_nPlayersSnaplineType = 2;

	inline static bool m_bPlayerChams = false;
	inline static ImColor m_cChamsColorTargetVisible = ImColor(255.0f / 255, 255.0f / 255, 255.0f / 255);
	inline static ImColor m_cChamsColorTargetHidden = ImColor(255.0f / 255, 255.0f / 255, 255.0f / 255);
	inline static bool m_bRainbowPlayerChams = false;
	inline static SDK::UMaterialInstanceDynamic* m_pChamsMaterial;

	inline static bool m_bPlayerSkeleton = false;
	inline static bool m_bRainbowPlayerSkeleton = false;
	inline static ImColor m_cPlayerSkeletonColor = ImColor(255.0f / 255, 255.0f / 255, 255.0f / 255);

	inline static bool m_bPlayersBox = false;
	inline static bool m_bRainbowPlayersBox = false;
	inline static ImColor m_cPlayersBoxColor = ImColor(255.0f / 255, 255.0f / 255, 255.0f / 255);
	inline static bool m_bPlayersBoxFilled = false;

	inline static bool m_bPlayersCornerBox = false;
	inline static ImColor m_cPlayersCornerBoxColor = ImColor(0.0f / 255.0f, 255.0f / 255.0f, 0.0f / 255);

	inline static bool m_bRainbowAimbotTargetColor = false;
	inline static ImColor m_cAimbotTargetColor = ImColor(255.0f / 255, 255.0f / 255, 255.0f / 255);

	inline static ImColor m_cTargetNotVisibleColor = ImColor(255.0f / 255, 255.0f / 255, 255.0f / 255);
	inline static bool m_bRainbowTargetNotVisibleColor = false;
	
	inline static bool m_bPlayersBox3D = false;

	inline static bool m_bBotChecker = false;
	inline static bool m_bRainbowBotChecker = false;
	inline static ImColor m_cBotCheckerColor = ImColor(0, 0, 255);
	inline static bool m_bBotCheckerText = true;

	inline static bool m_bPlayersHealth = false;

	inline static bool m_bGodMode = false;

	inline static bool m_bOneShot = false;

	inline static bool m_bSpeedHack = false;
	inline static float m_fSpeedValue = 1.0f;

	//inline static bool m_bNoClip = false;

	inline static bool m_bEnableAimbot = false;
	inline static bool m_bAimbotFOVCheck = false;
	inline static float m_fAimbotFOV = 80.0f;
	inline static float m_fAimbotSmoothness = 0.5f;
	inline static float m_fHeadPosOffset = 2;
	inline static float m_fFeetPosOffset = 0;



	//Custom Exploits
	inline static bool m_bInfiniteStamina = false;
	inline static bool m_bInfiniteSpecialAmmo = false;
	inline static bool m_bInfiniteLongGunAmmo = false;
	inline static bool m_bInfiniteHandGunAmmo = false;

	inline static bool m_bShowAdvanceStats = false;

	//inline static bool m_bPlayerAttributes = false;


	//Weapon Exploits
	inline static bool m_bAutoFire = false;
	inline static bool m_bNoSpread = false;
	inline static bool m_bNoRecoil = false;
	inline static bool m_bNoReload = false;

	inline static bool m_bRapidFire = false;
	inline static float m_fRapidSpeedValue = 5.0f;

	inline static bool m_bInstantReload = false;
};