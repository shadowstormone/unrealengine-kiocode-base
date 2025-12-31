#include "aimbot.h"

#include "../config.h"
#include "../utils/general.h"

void Aimbot::DebugPrintBonePairs(SDK::ACharacter* pawn)
{
	if (!pawn || !pawn->Mesh)
	{
		std::cout << "Invalid pawn or mesh!" << std::endl;
		return;
	}

	SDK::USkeletalMeshComponent* mesh = pawn->Mesh;

	std::cout << "=== BONE PAIRS DEBUG ===" << std::endl;
	std::cout << "Total pairs: " << Config::m_BonePairs.size() << std::endl;
	std::cout << std::endl;

	for (size_t i = 0; i < Config::m_BonePairs.size(); i++)
	{
		int fromIdx = Config::m_BonePairs[i].first;
		int toIdx = Config::m_BonePairs[i].second;

		SDK::FName fromBoneName = mesh->GetBoneName(fromIdx);
		SDK::FName toBoneName = mesh->GetBoneName(toIdx);

		// Получи позицию через GetSocketLocation (работает для костей)
		SDK::FVector fromPos = mesh->GetSocketLocation(fromBoneName);
		SDK::FVector toPos = mesh->GetSocketLocation(toBoneName);

		std::cout << "Pair [" << i << "]: ";
		std::cout << fromIdx << " -> " << toIdx << std::endl;

		std::cout << "  From Pos: (" << fromPos.X << ", " << fromPos.Y << ", " << fromPos.Z << ")" << std::endl;
		std::cout << "  To Pos:   (" << toPos.X << ", " << toPos.Y << ", " << toPos.Z << ")" << std::endl;
	}

	std::cout << "=== END DEBUG ===" << std::endl;
}

void Aimbot::RegularAimbot(SDK::ACharacter* pawn) 
{
	if (!pawn || !Config::m_pMyController || Config::m_BonePairs.empty())
		return;

	SDK::USkeletalMeshComponent* mesh = pawn->Mesh;
	if (!mesh)
		return;

	SDK::FVector head = mesh->GetSocketLocation(mesh->GetBoneName(Config::m_BonePairs[10].second));
	SDK::FVector2D headScreen;

	if (
		!Config::m_pMyController->ProjectWorldLocationToScreen(head, &headScreen, false)
	) return;

	if (Config::m_bAimbotFOVCheck)
	{
		if (headScreen.X > (Config::System::m_ScreenCenter.X + Config::m_fAimbotFOV))
			return;
		if (headScreen.X < (Config::System::m_ScreenCenter.X - Config::m_fAimbotFOV))
			return;
		if (headScreen.Y > (Config::System::m_ScreenCenter.Y + Config::m_fAimbotFOV))
			return;
		if (headScreen.Y < (Config::System::m_ScreenCenter.Y - Config::m_fAimbotFOV))
			return;
	}

	if (GetAsyncKeyState(VK_RBUTTON)) 
	{
		if (Config::m_pCurrentTarget == nullptr)
		{
			Config::m_pCurrentTarget = pawn;
		}

		if (Config::m_pCurrentTarget == pawn) 
		{
			Utility::MouseMove(headScreen.X, headScreen.Y, Config::System::m_ScreenSize.X, Config::System::m_ScreenSize.Y, Config::m_fAimbotSmoothness);
		}
	}
	else 
	{
		Config::m_pCurrentTarget = nullptr;
	}
	

}