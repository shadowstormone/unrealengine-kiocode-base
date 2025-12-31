#include "esp.h"

#include "../config.h"
#include "../utils/validity.h"
#include <algorithm>

void ESP::RenderSkeleton(SDK::ACharacter* pawn, ImColor color)
{
	if (!pawn || !Config::m_pMyController || Validity::IsBadPoint(Config::m_pMyController) || Config::m_BonePairs.empty())
		return;

	SDK::USkeletalMeshComponent* mesh = pawn->Mesh;
	if (!mesh)
		return;

	for (const std::pair<int, int>& pair : Config::m_BonePairs)
	{
		const int bone1Index = pair.first;
		const int bone2Index = pair.second;

		const SDK::FVector boneLoc1 = pawn->Mesh->GetSocketLocation(pawn->Mesh->GetBoneName(bone1Index));
		const SDK::FVector boneLoc2 = pawn->Mesh->GetSocketLocation(pawn->Mesh->GetBoneName(bone2Index));

		SDK::FVector2D boneScreen;
		SDK::FVector2D prevBoneScreen;

		if (
			!Config::m_pMyController->ProjectWorldLocationToScreen(boneLoc1, &boneScreen, false) ||
			!Config::m_pMyController->ProjectWorldLocationToScreen(boneLoc2, &prevBoneScreen, false)
		) continue;

		if (pawn == Config::m_pCurrentTarget) color = Config::m_bRainbowAimbotTargetColor ? Config::m_cRainbow : Config::m_cAimbotTargetColor;

		ImGui::GetForegroundDrawList()->AddLine(
			ImVec2(boneScreen.X, boneScreen.Y),
			ImVec2(prevBoneScreen.X, prevBoneScreen.Y),
			color,
			1.0f
		);
	}
}

void ESP::RenderSnapline(SDK::ACharacter* pawn, ImColor color)
{
	if (!pawn || !Config::m_pMyController || Validity::IsBadPoint(Config::m_pMyController))
		return;

	SDK::FVector pawnLoc = pawn->K2_GetActorLocation();
	SDK::FVector2D pawnScreen;

	if (Config::m_pMyController->ProjectWorldLocationToScreen(pawnLoc, &pawnScreen, false))
	{
		ImVec2 origin;

		switch (Config::m_nPlayersSnaplineType) {
			case 0: // from top
				origin = ImVec2(Config::System::m_ScreenCenter.X, 0);
				break;
			case 1: // from center
				origin = ImVec2(Config::System::m_ScreenCenter.X, Config::System::m_ScreenCenter.Y);
				break;
			case 2: // from bottom
				origin = ImVec2(Config::System::m_ScreenCenter.X, Config::System::m_ScreenSize.Y);
				break;
		}

		if (pawn == Config::m_pCurrentTarget) color = Config::m_bRainbowAimbotTargetColor ? Config::m_cRainbow : Config::m_cAimbotTargetColor;

		ImGui::GetBackgroundDrawList()->AddLine(
			origin,
			ImVec2(pawnScreen.X, pawnScreen.Y),
			color,
			1.0f
		);
	}
}	

void ESP::RenderBox(SDK::ACharacter* pawn, ImColor color)
{
	if (!pawn || !Config::m_pMyController || Validity::IsBadPoint(Config::m_pMyController) || Config::m_BonePairs.empty())
		return;

	SDK::USkeletalMeshComponent* mesh = pawn->Mesh;
	if (!mesh)
		return;

	SDK::FVector head = mesh->GetSocketLocation(mesh->GetBoneName(Config::m_BonePairs[4].second));
	SDK::FVector feet = mesh->GetSocketLocation(mesh->GetBoneName(Config::m_BonePairs[Config::m_BonePairs.size()-1].second));

	SDK::FVector2D headScreen;
	SDK::FVector2D feetScreen;

	if (
		!Config::m_pMyController->ProjectWorldLocationToScreen(head, &headScreen, false) ||
		!Config::m_pMyController->ProjectWorldLocationToScreen(feet, &feetScreen, false)
	) return;

	const float height = feetScreen.Y - headScreen.Y;
	const float width = height / 3;

	color.Value.w = 0.3f;

	// filled
	if (Config::m_bPlayersBoxFilled) {
		ImGui::GetForegroundDrawList()->AddRectFilled(
			ImVec2(headScreen.X - width, headScreen.Y),
			ImVec2(headScreen.X + width, feetScreen.Y),
			color,
			0.0f,
			0
		);
	}

	color.Value.w = 1.0f;

	ImGui::GetForegroundDrawList()->AddRect(
		ImVec2(headScreen.X - width, headScreen.Y),
		ImVec2(headScreen.X + width, feetScreen.Y),
		color,
		0.0f,
		0,
		1.0f
	);
	
}

void ESP::Render3DBox(SDK::ACharacter* pawn, ImColor color)
{
	if (!pawn || !Config::m_pMyController || Validity::IsBadPoint(Config::m_pMyController) || Config::m_BonePairs.empty())
		return;

	SDK::USkeletalMeshComponent* mesh = pawn->Mesh;
	if (!mesh)
		return;

	SDK::FVector head = mesh->GetSocketLocation(mesh->GetBoneName(Config::m_BonePairs[4].second));
	SDK::FVector feet = mesh->GetSocketLocation(mesh->GetBoneName(Config::m_BonePairs[Config::m_BonePairs.size() - 1].second));

	SDK::FVector2D headScreen;
	SDK::FVector2D feetScreen;

	if (
		!Config::m_pMyController->ProjectWorldLocationToScreen(head, &headScreen, false) ||
		!Config::m_pMyController->ProjectWorldLocationToScreen(feet, &feetScreen, false)
	) return;

	if (pawn == Config::m_pCurrentTarget) color = Config::m_bRainbowAimbotTargetColor ? Config::m_cRainbow : Config::m_cAimbotTargetColor;

	SDK::FVector center = pawn->K2_GetActorLocation();
	const float height = feetScreen.Y - headScreen.Y;
	const float width = height / 3;
	const float depth = width;

	SDK::FVector corners[] = {
		{ center.X - width / 2, center.Y + depth / 2, center.Z - height / 2 }, // a1
		{ center.X + width / 2, center.Y + depth / 2, center.Z - height / 2 }, // a2
		{ center.X - width / 2, center.Y - depth / 2, center.Z - height / 2 }, // a3
		{ center.X + width / 2, center.Y - depth / 2, center.Z - height / 2 }, // a4
		{ center.X - width / 2, center.Y + depth / 2, center.Z + height / 2 }, // b1
		{ center.X + width / 2, center.Y + depth / 2, center.Z + height / 2 }, // b2
		{ center.X - width / 2, center.Y - depth / 2, center.Z + height / 2 }, // b3
		{ center.X + width / 2, center.Y - depth / 2, center.Z + height / 2 }  // b4
	};

	SDK::FVector2D screenCorners[8];
	bool isVisible = true;

	for (int i = 0; i < 8; i++) {
		if (!Config::m_pMyController->ProjectWorldLocationToScreen(corners[i], &screenCorners[i], false)) {
			isVisible = false;
			break;
		}
	}

	if (isVisible) {
		ImVec2 s[8];
		for (int i = 0; i < 8; i++)
			s[i] = ImVec2(screenCorners[i].X, screenCorners[i].Y);

		ImGui::GetForegroundDrawList()->AddLine(s[0], s[1], color, 1.f);
		ImGui::GetForegroundDrawList()->AddLine(s[1], s[3], color, 1.f);
		ImGui::GetForegroundDrawList()->AddLine(s[3], s[2], color, 1.f);
		ImGui::GetForegroundDrawList()->AddLine(s[2], s[0], color, 1.f);

		ImGui::GetForegroundDrawList()->AddLine(s[4], s[5], color, 1.f);
		ImGui::GetForegroundDrawList()->AddLine(s[5], s[7], color, 1.f);
		ImGui::GetForegroundDrawList()->AddLine(s[7], s[6], color, 1.f);
		ImGui::GetForegroundDrawList()->AddLine(s[6], s[4], color, 1.f);

		ImGui::GetForegroundDrawList()->AddLine(s[0], s[4], color, 1.f);
		ImGui::GetForegroundDrawList()->AddLine(s[1], s[5], color, 1.f);
		ImGui::GetForegroundDrawList()->AddLine(s[2], s[6], color, 1.f);
		ImGui::GetForegroundDrawList()->AddLine(s[3], s[7], color, 1.f);
	}
}

void ESP::RenderCornerBox(SDK::ACharacter* pawn, ImColor color)
{
	if (!pawn || !Config::m_pMyController || Validity::IsBadPoint(Config::m_pMyController) || Config::m_BonePairs.empty())
		return;

	SDK::USkeletalMeshComponent* mesh = pawn->Mesh;
	if (!mesh)
		return;

	SDK::FVector head = mesh->GetSocketLocation(
		mesh->GetBoneName(Config::m_BonePairs[10].second)
	);

	SDK::FVector feet = mesh->GetSocketLocation(
		mesh->GetBoneName(Config::m_BonePairs.back().second)
	);

	SDK::FVector2D headScreen, feetScreen;

	if (
		!Config::m_pMyController->ProjectWorldLocationToScreen(head, &headScreen, false) ||
		!Config::m_pMyController->ProjectWorldLocationToScreen(feet, &feetScreen, false)
		)
		return;

	float height = feetScreen.Y - headScreen.Y;
	if (height <= 0.0f)
		return;

	float width = height / 2.0f;
	float corner = width * 0.25f;

	ImDrawList* draw = ImGui::GetForegroundDrawList();

	float left = headScreen.X - width;
	float right = headScreen.X + width;
	float top = headScreen.Y;
	float bottom = feetScreen.Y;

	// Alpha for filled corner background (optional)
	if (Config::m_bPlayersBoxFilled)
	{
		ImColor fill = color;
		fill.Value.w = 0.15f;

		draw->AddRectFilled(
			ImVec2(left, top),
			ImVec2(right, bottom),
			fill
		);
	}

	color.Value.w = 1.0f;

	// ===== TOP LEFT =====
	draw->AddLine(ImVec2(left, top), ImVec2(left + corner, top), color, 1.0f);
	draw->AddLine(ImVec2(left, top), ImVec2(left, top + corner), color, 1.0f);

	// ===== TOP RIGHT =====
	draw->AddLine(ImVec2(right, top), ImVec2(right - corner, top), color, 1.0f);
	draw->AddLine(ImVec2(right, top), ImVec2(right, top + corner), color, 1.0f);

	// ===== BOTTOM LEFT =====
	draw->AddLine(ImVec2(left, bottom), ImVec2(left + corner, bottom), color, 1.0f);
	draw->AddLine(ImVec2(left, bottom), ImVec2(left, bottom - corner), color, 1.0f);

	// ===== BOTTOM RIGHT =====
	draw->AddLine(ImVec2(right, bottom), ImVec2(right - corner, bottom), color, 1.0f);
	draw->AddLine(ImVec2(right, bottom), ImVec2(right, bottom - corner), color, 1.0f);
}
