#include "main_loop.h"

#include "../config.h"
#include "../utils/validity.h"
#include "../utils/general.h"
#include "../utils/imgui/imgui_helper.h"
#include "esp.h"
#include "aimbot.h"

#include <set>
#include <cstdio>
#include <fstream>

void MainLoop::DrawCrosshair() 
{
	ImColor color = Config::m_bRainbowCrosshair ? Config::m_cRainbow : Config::m_cCrosshairColor;
	switch (Config::m_nCrosshairType)
	{
	case 0:
		ImGui::GetForegroundDrawList()->AddLine(ImVec2(Config::System::m_ScreenCenter.X - Config::m_fCrosshairSize, Config::System::m_ScreenCenter.Y), ImVec2((Config::System::m_ScreenCenter.X - Config::m_fCrosshairSize) + (Config::m_fCrosshairSize * 2), Config::System::m_ScreenCenter.Y), color, 1.2f);
		ImGui::GetForegroundDrawList()->AddLine(ImVec2(Config::System::m_ScreenCenter.X, Config::System::m_ScreenCenter.Y - Config::m_fCrosshairSize), ImVec2(Config::System::m_ScreenCenter.X, (Config::System::m_ScreenCenter.Y - Config::m_fCrosshairSize) + (Config::m_fCrosshairSize * 2)), color, 1.2f);
		break;
	case 1:
		ImGui::GetForegroundDrawList()->AddCircle(ImVec2(Config::System::m_ScreenCenter.X, Config::System::m_ScreenCenter.Y), Config::m_fCrosshairSize, color, 100, 1.2f);
		break;
	}
}

//void MainLoop::FetchFromObjects(std::vector<SDK::ACharacter*>* list)
//{
//
//	list->clear();
//
//	for (int i = 0; i < SDK::UObject::GObjects->Num(); i++)
//	{
//		SDK::UObject* obj = SDK::UObject::GObjects->GetByIndex(i);
//
//		if (!obj || obj->IsDefaultObject())
//			continue;
//				
//		// THIS PART CAN BE VARIABLE BELOW
//
//		// Crabs Champions
//		//if (!obj->IsA(SDK::ACrabEnemyC::StaticClass()))
//		//	continue;
//		
//		// Destroy All Humans
//		//if (!obj->IsA(SDK::UBFGAnimationInstance_Human::StaticClass()))
//		//	continue;		
//		
//		// oar
//		/*if (!obj->IsA(SDK::ANPCBase_C::StaticClass()))
//			continue;
//
//		SDK::ANPCBase_C* npc = static_cast<SDK::ANPCBase_C*>(obj);
//
//		if (!npc || Validity::IsBadPoint(npc) || npc->Health <= 0 || npc->Dead_)
//			continue;*/
//
//		//if(Config::m_bKillAll) npc->TakeDamage(999, npc);
//
//		//list->push_back(npc);
//
//	}
//}

//void MainLoop::FetchFromObjects(std::vector<SDK::ACharacter*>* list)
//{
//	if (!list) return;
//	list->clear();
//
//	for (int i = 0; i < SDK::UObject::GObjects->Num(); i++)
//	{
//		SDK::UObject* obj = SDK::UObject::GObjects->GetByIndex(i);
//		if (!obj || obj->IsDefaultObject())
//			continue;
//
//		// Ищем только монстров
//		if (!obj->IsA(SDK::ABP_Monster_C::StaticClass()))
//			continue;
//
//		SDK::ABP_Monster_C* monster = static_cast<SDK::ABP_Monster_C*>(obj);
//		if (!monster || !monster->RootComponent)
//			continue;
//
//		list->push_back(monster);
//	}
//}

// Глобальный кеш врагов с временем обновления
static std::vector<SDK::ABP_Monster_C*> g_CachedMonsters;
static double g_LastUpdateTime = 0.0;
static const double UPDATE_INTERVAL = 0.5; // Обновлять каждые 0.5 секунды

void MainLoop::UpdateMonstersCache()
{
	// Обновляем кеш врагов только с интервалом, не каждый кадр
	double currentTime = GetCurrentTime();
	if (currentTime - g_LastUpdateTime < UPDATE_INTERVAL)
		return;

	g_LastUpdateTime = currentTime;
	g_CachedMonsters.clear();

	if (!Config::m_pWorld || Config::m_pWorld->Levels.Num() == 0)
		return;

	// Итерируем по ВСЕ уровнем (враги могут быть в разных уровнях)
	for (int l = 0; l < Config::m_pWorld->Levels.Num(); l++)
	{
		SDK::ULevel* currLevel = Config::m_pWorld->Levels[l];
		if (!currLevel)
			continue;

		// Итерируем по Actors в каждом уровне
		for (int j = 0; j < currLevel->Actors.Num(); j++)
		{
			SDK::AActor* actor = currLevel->Actors[j];
			if (!actor || !actor->RootComponent)
				continue;

			if (!actor->IsA(SDK::ABP_Monster_C::StaticClass()))
				continue;

			SDK::ABP_Monster_C* monster = static_cast<SDK::ABP_Monster_C*>(actor);
			if (monster)
			{
				g_CachedMonsters.push_back(monster);
			}
		}
	}
}

void MainLoop::FetchFromObjects(std::vector<SDK::ACharacter*>* list)
{
	if (!list) return;

	UpdateMonstersCache();

	// Копируем кеш вместо пересканирования GObjects
	list->clear();

	for (const auto& monster : g_CachedMonsters)
	{
		if (monster && monster->RootComponent)
		{
			list->push_back(monster);
		}
	}
}

void MainLoop::FetchFromActors(std::vector<SDK::AActor*>* list)
{
	if (Config::m_pWorld->Levels.Num() == 0)
		return;
	SDK::ULevel* currLevel = Config::m_pWorld->Levels[0];
	if (!currLevel)
		return;
	list->clear();

	std::set<std::string> uniqueActors;

	for (int j = 0; j < currLevel->Actors.Num(); j++)
	{
		SDK::AActor* currActor = currLevel->Actors[j];

		// Сохраняем названия акторов до всех проверок
		if (currActor)
			uniqueActors.insert(currActor->GetFullName());

		if (!currActor)
			continue;
		if (!currActor->RootComponent)
			continue;

		std::string fullName = currActor->GetFullName();
		// Ищем врагов (Character_*), исключаем игрока
		if (fullName.find("Character_") != std::string::npos &&
			fullName.find("Character_Master_Player") == std::string::npos)
		{
			list->push_back(currActor);
		}
	}

	// Сохраняем уникальные актеры в файл
	std::ofstream file("actor_name.list");
	if (file.is_open())
	{
		for (const auto& actorName : uniqueActors)
		{
			file << actorName << '\n';
		}
		file.close();
	}
}

void MainLoop::FetchFromPlayers(std::vector<SDK::ACharacter*>* list)
{
	list->clear();

	SDK::TSubclassOf<SDK::ACharacter> PlayerBaseCharacterReference = SDK::ACharacter::StaticClass();
	SDK::TArray<SDK::AActor*> PlayerCharacters;
	SDK::UGameplayStatics::GetAllActorsOfClass(Config::m_pWorld, PlayerBaseCharacterReference, &PlayerCharacters);

	for (SDK::AActor* actor : PlayerCharacters)
	{
		if (!actor || Validity::IsBadPoint(actor) || !actor->IsA(PlayerBaseCharacterReference)) continue;

		SDK::ACharacter* PlayerCharacter = reinterpret_cast<SDK::ACharacter*>(actor);
		if (!PlayerCharacter || !PlayerCharacter->PlayerState || !PlayerCharacter->PlayerState->GetPlayerName() || !PlayerCharacter->PlayerState->GetPlayerName().IsValid())
			continue;

		list->push_back(PlayerCharacter);
	}
}

void MainLoop::FetchEntities()
{
	do {
		if (!Config::System::m_bUpdateTargets)
		{
			std::lock_guard<std::mutex> lock(list_mutex);
			if (!Config::m_TargetsList.empty())
			{
				Config::m_TargetsList.clear();
			}

			if (Config::System::m_bUpdateTargetsInDifferentThread)
			{
				Sleep(500);
				continue;
			}
			else
			{
				return;
			}
		}

		if (!Config::m_pWorld || Validity::IsBadPoint(Config::m_pWorld) ||
			!Config::m_pEngine || Validity::IsBadPoint(Config::m_pEngine) ||
			!Config::m_pMyController || Validity::IsBadPoint(Config::m_pMyController) ||
			!Config::m_pMyPawn || Validity::IsBadPoint(Config::m_pMyPawn))
		{
			if (Config::System::m_bUpdateTargetsInDifferentThread)
			{
				Sleep(500);
				continue;
			}
			else
			{
				return;
			}
		}

		if (!Config::m_pWorld->GameState || Validity::IsBadPoint(Config::m_pWorld->GameState) ||
			Validity::IsBadPoint(Config::m_pWorld->OwningGameInstance))
		{
			if (Config::System::m_bUpdateTargetsInDifferentThread)
			{
				Sleep(500);
				continue;
			}
			else
			{
				return;
			}
		}

		std::vector<SDK::ACharacter*> ACharNewTargets;

		switch (Config::m_nTargetFetch)
		{
			case 0:
				FetchFromObjects(&ACharNewTargets);
				break;

			case 1:
				//FetchFromActors(&AActorNewTargets);
				break;

			case 2:
				FetchFromPlayers(&ACharNewTargets);
				break;
		}

		{
			std::lock_guard<std::mutex> lock(list_mutex);
			Config::m_TargetsList = std::move(ACharNewTargets);
		}

		if (Config::System::m_bUpdateTargetsInDifferentThread)
		{
			Sleep(10);
		}

	// if its in a thread run it continuously
	} while (Config::System::m_bUpdateTargetsInDifferentThread);
}

bool MainLoop::UpdateSDK(bool log) 
{
	Config::m_pWorld = SDK::UWorld::GetWorld();
	if (Validity::IsBadPoint(Config::m_pWorld) || Validity::IsBadPoint(Config::m_pWorld->GameState))
	{
		std::cerr << "Error: World not found" << std::endl;
		return false;
	}
	if (log) {
		std::cout << "World address: 0x" << std::hex << reinterpret_cast<uintptr_t>(Config::m_pWorld) << std::dec << std::endl;
	}

	Config::m_pEngine = SDK::UEngine::GetEngine();
	if (Validity::IsBadPoint(Config::m_pEngine)) 
	{
		std::cerr << "Error: Engine not found" << std::endl;
		return false;
	}
	if (log) {
		std::cout << "Engine address: 0x" << std::hex << reinterpret_cast<uintptr_t>(Config::m_pEngine) << std::dec << std::endl;
	}

	// Init PlayerController
	if (Validity::IsBadPoint(Config::m_pWorld->OwningGameInstance)) 
	{
		std::cerr << "Error: OwningGameInstance not found" << std::endl;
		return false;
	}
	if (Validity::IsBadPoint(Config::m_pWorld->OwningGameInstance->LocalPlayers[0])) 
	{
		std::cerr << "Error: LocalPlayers[0] not found" << std::endl;
		return false;
	}
	Config::m_pMyController = Config::m_pWorld->OwningGameInstance->LocalPlayers[0]->PlayerController;
	if (Validity::IsBadPoint(Config::m_pMyController)) 
	{
		std::cerr << "Error: MyController not found" << std::endl;
		return false;
	}
	if (log) {
		std::cout << "PlayerController address: 0x" << std::hex << reinterpret_cast<uintptr_t>(Config::m_pMyController) << std::dec << std::endl;
	}

	// Init Pawn
	Config::m_pMyPawn = Config::m_pMyController->AcknowledgedPawn;
	if (Config::m_pMyPawn == nullptr) 
	{
		std::cerr << "Error: MyPawn not found" << std::endl;
		return false;
	}	
	if (log) {
		std::cout << "MyPawn address: 0x" << std::hex << reinterpret_cast<uintptr_t>(Config::m_pMyPawn) << std::dec << std::endl;
	}

	Config::m_pMyCharacter = Config::m_pMyController->Character;
	if (Config::m_pMyCharacter == nullptr)
	{
		std::cerr << "Error: MyCharacter not found" << std::endl;
		return false;
	}
	if (log) {
		std::cout << "MyCharacter address: 0x" << std::hex << reinterpret_cast<uintptr_t>(Config::m_pMyCharacter) << std::dec << std::endl;
	}

	return true;
}

void MainLoop::Update(DWORD tick) 
{
	// important update of the sdk, bc if we inject the dll in the menu for example, 
	// after in game we will not have the access to some obejct or the player controller
	if (!UpdateSDK(false)) return;

	// thats a check because we can start that in a thread to avoid lag, but in some game
	// it must be in the main loop to avoid game freezing (like in OHD)
	if (!Config::System::m_bUpdateTargetsInDifferentThread)
	{
		FetchEntities();
	}
	
	#pragma region EXPLOIT CHEATS

	if (Config::m_bGodMode)
	{
		// Cast to the Remnant character type and call GodMode function
		if (Config::m_pMyCharacter && !Validity::IsBadPoint(Config::m_pMyCharacter))
		{
			SDK::ACharacter_Master_Player_C* PlayerCharacter = static_cast<SDK::ACharacter_Master_Player_C*>(Config::m_pMyCharacter);
			if (PlayerCharacter && !Validity::IsBadPoint(PlayerCharacter))
			{
				PlayerCharacter->God_Mode = true;
			}
		}
	}
	else
	{
		// Disable godmode when the checkbox is unchecked
		if (Config::m_pMyCharacter && !Validity::IsBadPoint(Config::m_pMyCharacter))
		{
			SDK::ACharacter_Master_Player_C* PlayerCharacter = static_cast<SDK::ACharacter_Master_Player_C*>(Config::m_pMyCharacter);
			if (PlayerCharacter && !Validity::IsBadPoint(PlayerCharacter))
			{
				PlayerCharacter->God_Mode = false;
			}
		}
	}

	if (Config::m_bInfiniteStamina)
	{
		if (Config::m_pMyCharacter && !Validity::IsBadPoint(Config::m_pMyCharacter))
		{
			SDK::ACharacter_Master_Player_C* PlayerCharacter = static_cast<SDK::ACharacter_Master_Player_C*>(Config::m_pMyCharacter);
			if (PlayerCharacter && !Validity::IsBadPoint(PlayerCharacter) && PlayerCharacter->Stamina)
			{
				PlayerCharacter->Stamina->Value = 99999.f;
			}
		}
	}

	if (Config::m_bInfiniteSpecialAmmo)
	{
		if (Config::m_pMyCharacter && !Validity::IsBadPoint(Config::m_pMyCharacter))
		{
			SDK::ACharacter_Master_Player_C* PlayerCharacter = static_cast<SDK::ACharacter_Master_Player_C*>(Config::m_pMyCharacter);
			if (PlayerCharacter && !Validity::IsBadPoint(PlayerCharacter) && PlayerCharacter->SpecialAmmo)
			{
				PlayerCharacter->SpecialAmmo->Value = 200.f;
			}
		}
	}

	if (Config::m_bInfiniteLongGunAmmo)
	{
		if (Config::m_pMyCharacter && !Validity::IsBadPoint(Config::m_pMyCharacter))
		{
			SDK::ACharacter_Master_Player_C* PlayerCharacter = static_cast<SDK::ACharacter_Master_Player_C*>(Config::m_pMyCharacter);
			if (PlayerCharacter && !Validity::IsBadPoint(PlayerCharacter) && PlayerCharacter->LongGunAmmo)
			{
				PlayerCharacter->LongGunAmmo->Value = 200.f;
			}
		}
	}

	if (Config::m_bInfiniteHandGunAmmo)
	{
		if (Config::m_pMyCharacter && !Validity::IsBadPoint(Config::m_pMyCharacter))
		{
			SDK::ACharacter_Master_Player_C* PlayerCharacter = static_cast<SDK::ACharacter_Master_Player_C*>(Config::m_pMyCharacter);
			if (PlayerCharacter && !Validity::IsBadPoint(PlayerCharacter) && PlayerCharacter->HandGunAmmo)
			{
				PlayerCharacter->HandGunAmmo->Value = 200.f;
			}
		}
	}

	if (Config::m_bShowAdvanceStats)
	{
		if (Config::m_pMyCharacter && !Validity::IsBadPoint(Config::m_pMyCharacter))
		{
			SDK::ACharacter_Master_Player_C* PlayerCharacter = static_cast<SDK::ACharacter_Master_Player_C*>(Config::m_pMyCharacter);
			if (PlayerCharacter && !Validity::IsBadPoint(PlayerCharacter))
			{
				PlayerCharacter->ShowAdvanceStats = true;
			}
		}
	}
	else
	{
		if (Config::m_pMyCharacter && !Validity::IsBadPoint(Config::m_pMyCharacter))
		{
			SDK::ACharacter_Master_Player_C* PlayerCharacter = static_cast<SDK::ACharacter_Master_Player_C*>(Config::m_pMyCharacter);
			if (PlayerCharacter && !Validity::IsBadPoint(PlayerCharacter))
			{
				PlayerCharacter->ShowAdvanceStats = false;
			}
		}
	}

	if (Config::m_bNoSpread)
	{
		if (Config::m_pMyCharacter && !Validity::IsBadPoint(Config::m_pMyCharacter))
		{
			auto* pc = static_cast<SDK::ACharacter_Master_Player_C*>(Config::m_pMyCharacter);
			auto* inventory = pc->Inventory;
			if (!inventory)
				return;

			for (int i = 0; i < inventory->EquipmentSlots.Num(); ++i)
			{
				const SDK::FEquipmentSlot& slot = inventory->EquipmentSlots[i];
				SDK::AEquipment* item = inventory->GetEquippedItemBySlotName(slot.NameID);
				if (!item || Validity::IsBadPoint(item))
					continue;

				// Проверяем, что это дальнобойное оружие
				if (!item->IsA(SDK::ARangedWeapon::StaticClass()))
					continue;

				SDK::ARangedWeapon* rangedWeapon = static_cast<SDK::ARangedWeapon*>(item);
				if (!rangedWeapon->Profile)
					continue;

				// Получаем объект профиля через ClassDefaultObject
				SDK::URangedWeaponProfile* profile = static_cast<SDK::URangedWeaponProfile*>(rangedWeapon->Profile->ClassDefaultObject);
				if (!profile)
					continue;

				profile->FiringSpreadIncrement = 0.f;
				profile->InitialFiringSpread = 0.f;
				profile->FiringSpreadDecay = 0.f;
				profile->FiringSpreadDecayDelay = 0.f;
				profile->FiringSpreadMin = 0.f;
				profile->FiringSpreadMax = 0.f;
				profile->FiringSpreadMoveRef = 0.f;
				profile->FiringSpreadAimMin = 0.f;
				profile->FiringSpreadAimMax = 0.f;

				profile->CameraShake = nullptr;
				profile->CameraShakeAimScale = 0.f;
				profile->CameraShakeScale = 0.f;
				profile->CameraShakeScopeScale = 0.f;

				rangedWeapon->CustomSpread = {};
				rangedWeapon->ApplySpread(0.f);
			}
		}
	}

	if (Config::m_bNoRecoil)
	{
		if (Config::m_pMyCharacter && !Validity::IsBadPoint(Config::m_pMyCharacter))
		{
			auto* pc = static_cast<SDK::ACharacter_Master_Player_C*>(Config::m_pMyCharacter);
			auto* inventory = pc->Inventory;
			if (!inventory)
				return;

			for (int i = 0; i < inventory->EquipmentSlots.Num(); ++i)
			{
				const SDK::FEquipmentSlot& slot = inventory->EquipmentSlots[i];
				SDK::AEquipment* item = inventory->GetEquippedItemBySlotName(slot.NameID);
				if (!item || Validity::IsBadPoint(item))
					continue;

				// Проверяем, что это дальнобойное оружие
				if (!item->IsA(SDK::ARangedWeapon::StaticClass()))
					continue;

				SDK::ARangedWeapon* rangedWeapon = static_cast<SDK::ARangedWeapon*>(item);
				if (!rangedWeapon->Profile)
					continue;

				// Получаем объект профиля через ClassDefaultObject
				SDK::URangedWeaponProfile* profile = static_cast<SDK::URangedWeaponProfile*>(rangedWeapon->Profile->ClassDefaultObject);
				if (!profile)
					continue;

				profile->RecoilHorizontalMin = 0.f;
				profile->RecoilHorizontalMax = 0.f;
				profile->RecoilVertical = 0.f;
				profile->RecoilSpeed = 0.f;
				profile->RecoilRecover = 0.f;
				profile->RecoilRecoverTime = 0.f;
				profile->RecoilHorizontalScopeScale = 0.f;
				profile->RecoilVerticalScopeScale = 0.f;
				profile->RecoilSpeedScopeScale = 0.f;

				profile->CameraShake = nullptr;
				profile->CameraShakeAimScale = 0.f;
				profile->CameraShakeScale = 0.f;
				profile->CameraShakeScopeScale = 0.f;

				rangedWeapon->ApplyRecoil(0.f);
			}
		}
	}

	if (Config::m_bNoReload)
	{
		if (Config::m_pMyCharacter && !Validity::IsBadPoint(Config::m_pMyCharacter))
		{
			auto* pc = static_cast<SDK::ACharacter_Master_Player_C*>(Config::m_pMyCharacter);
			auto* inventory = pc->Inventory;
			if (!inventory)
				return;

			for (int i = 0; i < inventory->EquipmentSlots.Num(); ++i)
			{
				const SDK::FEquipmentSlot& slot = inventory->EquipmentSlots[i];
				SDK::AEquipment* item = inventory->GetEquippedItemBySlotName(slot.NameID);
				if (!item || Validity::IsBadPoint(item))
					continue;

				if (!item->IsA(SDK::ARangedWeapon::StaticClass()))
					continue;

				SDK::ARangedWeapon* rangedWeapon = static_cast<SDK::ARangedWeapon*>(item);
				if (!rangedWeapon->Profile)
					continue;

				// Проверяем InstanceData
				SDK::UItemInstanceData* baseData = item->InstanceData;
				if (!baseData || Validity::IsBadPoint(baseData))
					continue;

				// Cast к URangedWeaponInstanceData
				SDK::URangedWeaponInstanceData* weaponData = static_cast<SDK::URangedWeaponInstanceData*>(baseData);
				if (!weaponData || Validity::IsBadPoint(weaponData))
					continue;

				// Заполняем обойму полностью (AmmoInClip = максимум на клип)
				weaponData->AmmoInClip = rangedWeapon->GetAmmoPerClip();
			}
		}
	}

	if (Config::m_bAutoFire)
	{
		if (Config::m_pMyCharacter && !Validity::IsBadPoint(Config::m_pMyCharacter))
		{
			auto* pc = static_cast<SDK::ACharacter_Master_Player_C*>(Config::m_pMyCharacter);
			auto* inventory = pc->Inventory;
			if (!inventory)
				return;

			for (int i = 0; i < inventory->EquipmentSlots.Num(); ++i)
			{
				const SDK::FEquipmentSlot& slot = inventory->EquipmentSlots[i];
				SDK::AEquipment* item = inventory->GetEquippedItemBySlotName(slot.NameID);
				if (!item || Validity::IsBadPoint(item))
					continue;

				if (!item->IsA(SDK::ARangedWeapon::StaticClass()))
					continue;

				SDK::ARangedWeapon* rangedWeapon = static_cast<SDK::ARangedWeapon*>(item);
				if (!rangedWeapon)
					continue;

				 SDK::FRangedWeaponMode& defaultMode = rangedWeapon->DefaultWeaponMode;
				 defaultMode.BurstCount = 0.0f;
			}
		}
	}

	if (Config::m_bRapidFire)
	{
		if (Config::m_pMyCharacter && !Validity::IsBadPoint(Config::m_pMyCharacter))
		{
			auto* pc = static_cast<SDK::ACharacter_Master_Player_C*>(Config::m_pMyCharacter);
			auto* inventory = pc->Inventory;
			if (!inventory)
				return;

			for (int i = 0; i < inventory->EquipmentSlots.Num(); ++i)
			{
				const SDK::FEquipmentSlot& slot = inventory->EquipmentSlots[i];
				SDK::AEquipment* item = inventory->GetEquippedItemBySlotName(slot.NameID);
				if (!item || Validity::IsBadPoint(item))
					continue;

				if (!item->IsA(SDK::ARangedWeapon::StaticClass()))
					continue;

				SDK::ARangedWeapon* rangedWeapon = static_cast<SDK::ARangedWeapon*>(item);
				if (!rangedWeapon)
					continue;

				SDK::FRangedWeaponMode& defaultMode = rangedWeapon->DefaultWeaponMode;
				defaultMode.RateOfFire = Config::m_fRapidSpeedValue;
				defaultMode.BurstRateOfFire = Config::m_fRapidSpeedValue;
			}
		}
	}

	if (Config::m_bInstantReload)
	{
		if (Config::m_pMyCharacter && !Validity::IsBadPoint(Config::m_pMyCharacter))
		{
			auto* pc = static_cast<SDK::ACharacter_Master_Player_C*>(Config::m_pMyCharacter);
			auto* inventory = pc->Inventory;
			if (!inventory)
				return;

			for (int i = 0; i < inventory->EquipmentSlots.Num(); ++i)
			{
				const SDK::FEquipmentSlot& slot = inventory->EquipmentSlots[i];
				SDK::AEquipment* item = inventory->GetEquippedItemBySlotName(slot.NameID);
				if (!item || Validity::IsBadPoint(item))
					continue;

				if (!item->IsA(SDK::ARangedWeapon::StaticClass()))
					continue;

				SDK::ARangedWeapon* rangedWeapon = static_cast<SDK::ARangedWeapon*>(item);
				if (!rangedWeapon)
					continue;

				rangedWeapon->ReloadSpeed = 9999.0f;
			}
		}
	}

	/* ==================== Weapon Hack ==================== */

	/*if (Config::)
	{
		if (Config::m_pMyWeapon && !Validity::IsBadPoint(Config::m_pMyWeapon))
		{
			SDK::ARangedWeapon* CurrentWeapon = static_cast<SDK::ARangedWeapon*>(Config::m_pMyWeapon);
			if (CurrentWeapon && !Validity::IsBadPoint(CurrentWeapon))
			{
				CurrentWeapon->
			}
		}
	}*/

	// seems universal (use it with the fly or you will fall under the map forever :D)
	//if (Config::m_bNoClip)
	//{
	//	Config::m_pMyPawn->SetActorEnableCollision(false);
	//}
	//else if (!Config::m_bNoClip && Config::m_pMyPawn->GetActorEnableCollision())
	//{
	//	Config::m_pMyPawn->SetActorEnableCollision(true);
	//}

	// seems universal
	if (Config::m_bCameraFovChanger)
	{
		Config::m_pMyController->FOV(Config::m_fCameraCustomFOV);
	}

	// seems universal
	if (Config::m_bTimeScaleChanger)
	{
		Config::m_pWorld->K2_GetWorldSettings()->TimeDilation = Config::m_fTimeScale;
	}

	// seems universal (in multiplayer it may not work if player pos is server sided)
	//if (Config::m_bFly)
	//{
	//	Config::m_pMyCharacter->CharacterMovement->MaxFlySpeed = 20000.f;
	//	Config::m_pMyCharacter->CharacterMovement->MovementMode = SDK::EMovementMode::MOVE_Flying;
	//	if (GetAsyncKeyState(VK_SPACE))
	//	{
	//		SDK::FVector posUP = { 0.f, 0.f, 10.f };
	//		Config::m_pMyCharacter->CharacterMovement->AddInputVector(posUP, true);
	//	}
	//	if (GetAsyncKeyState(VK_LCONTROL))
	//	{
	//		SDK::FVector posDOWN = { 0.f, 0.f, -10.f };
	//		Config::m_pMyCharacter->CharacterMovement->AddInputVector(posDOWN, true);
	//	}
	//}
	//else if (Config::m_pMyCharacter->CharacterMovement->MovementMode == SDK::EMovementMode::MOVE_Flying)
	//{
	//	Config::m_pMyCharacter->CharacterMovement->MovementMode = SDK::EMovementMode::MOVE_Falling;
	//}

	//// seems universal
	//if (Config::m_bNoGravity)
	//{
	//	Config::m_pMyCharacter->CharacterMovement->GravityScale = 0.2f;
	//}
	//else if (Config::m_pMyCharacter->CharacterMovement->GravityScale != 1.f)
	//{
	//	Config::m_pMyCharacter->CharacterMovement->GravityScale = 1.f;
	//}

	// seems universal
	if (Config::m_bSpeedHack)
	{
		Config::m_pMyCharacter->CharacterMovement->MaxWalkSpeed = Config::m_fSpeedValue;
		Config::m_pMyCharacter->CharacterMovement->MaxAcceleration = Config::m_fSpeedValue;
	}

	#pragma endregion

	std::shared_ptr<std::vector<SDK::ACharacter*>> currentTargets;

	{
		std::lock_guard<std::mutex> lock(list_mutex);
		currentTargets = std::make_shared<std::vector<SDK::ACharacter*>>(Config::m_TargetsList);
	}

	// looping our targets (in online games it will be prob a ACharacter vector, in offline games for npc can be AActor vector)
	for (auto* currTarget : *currentTargets) 
	{

		if (!currTarget || Validity::IsBadPoint(currTarget))
			continue;

		// skip local player
		if (currTarget->Controller && currTarget->Controller->IsLocalPlayerController())
			continue;

		// raycast to check if targets are behind walls
		bool isVisible = Config::m_pMyController->LineOfSightTo(currTarget, Config::m_pMyController->PlayerCameraManager->CameraCachePrivate.POV.Location, false);

		if (Config::m_bPlayerChams && Config::m_pChamsMaterial) 
		{
			SDK::ASkeletalMeshActor* mesh = reinterpret_cast<SDK::ASkeletalMeshActor*>(currTarget);
			Utility::ApplyChams(mesh->SkeletalMeshComponent, true);
		}

		ImColor color = ImColor(255.0f / 255, 255.0f / 255, 255.0f / 255);

		#pragma region CHEATS FOR TARGETS

		// NOTE: Config::CurrentTarget is a pointer to the current target (of the aimbot)

		if (Config::m_bPlayersSnapline)
		{

			if (currTarget == Config::m_pCurrentTarget)
			{
				color = Config::m_bRainbowAimbotTargetColor ? Config::m_cRainbow : Config::m_cAimbotTargetColor;
			}
			else
			{

				if (isVisible)
				{
					color = Config::m_bRainbowPlayersSnapline ? Config::m_cRainbow : Config::m_cPlayersSnaplineColor;
				}
				else
				{
					color = Config::m_bRainbowTargetNotVisibleColor ? Config::m_cRainbow : Config::m_cTargetNotVisibleColor;
				}

			}

			ESP::GetInstance().RenderSnapline(currTarget, color);
		}

		if (Config::m_bPlayerSkeleton)
		{

			if (currTarget == Config::m_pCurrentTarget)
			{
				color = Config::m_bRainbowAimbotTargetColor ? Config::m_cRainbow : Config::m_cAimbotTargetColor;
			}
			else
			{

				if (isVisible)
				{
					color = Config::m_bRainbowPlayerSkeleton ? Config::m_cRainbow : Config::m_cPlayerSkeletonColor;
				}
				else
				{
					color = Config::m_bRainbowTargetNotVisibleColor ? Config::m_cRainbow : Config::m_cTargetNotVisibleColor;
				}

			}

			ESP::GetInstance().RenderSkeleton(currTarget, color);
		}

		if (Config::m_bPlayersBox)
		{

			if (currTarget == Config::m_pCurrentTarget)
			{
				color = Config::m_bRainbowAimbotTargetColor ? Config::m_cRainbow : Config::m_cAimbotTargetColor;
			}
			else
			{

				if (isVisible)
				{
					color = Config::m_bRainbowPlayersBox ? Config::m_cRainbow : Config::m_cPlayersBoxColor;
				}
				else
				{
					color = Config::m_bRainbowTargetNotVisibleColor ? Config::m_cRainbow : Config::m_cTargetNotVisibleColor;
				}
			}

			ESP::GetInstance().RenderBox(currTarget, color);
		}

		if (Config::m_bPlayersCornerBox)
		{
			ImColor color;

			if (currTarget == Config::m_pCurrentTarget)
			{
				color = Config::m_bRainbowAimbotTargetColor ? Config::m_cRainbow : Config::m_cAimbotTargetColor;
			}
			else
			{
				if (isVisible)
				{
					color = Config::m_cPlayersCornerBoxColor;
				}
				else
				{
					color = Config::m_bRainbowTargetNotVisibleColor ? Config::m_cRainbow : Config::m_cTargetNotVisibleColor;
				}
			}

			ESP::GetInstance().RenderCornerBox(currTarget, color);
		}

		if (Config::m_bPlayersBox3D)
		{

			if (currTarget == Config::m_pCurrentTarget)
			{
				color = Config::m_bRainbowAimbotTargetColor ? Config::m_cRainbow : Config::m_cAimbotTargetColor;
			}
			else
			{

				if (isVisible)
				{
					color = Config::m_bRainbowPlayersBox ? Config::m_cRainbow : Config::m_cPlayersBoxColor;
				}
				else
				{
					color = Config::m_bRainbowTargetNotVisibleColor ? Config::m_cRainbow : Config::m_cTargetNotVisibleColor;
				}
			}

			ESP::GetInstance().Render3DBox(currTarget, color);
		}

		if (Config::m_bEnableAimbot && isVisible)
		{
			Aimbot::GetInstance().RegularAimbot(currTarget);
		}

		#pragma endregion
	}
}