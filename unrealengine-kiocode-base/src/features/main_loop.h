#pragma once

#include <vector>
#include <Windows.h>
#include <mutex>

#include "../includes.h"

class MainLoop
{
public:
	static MainLoop& GetInstance() {
		static MainLoop instance;
		return instance;
	}

	void DrawCrosshair();
	void FetchEntities();
	void FetchFromObjects(std::vector<SDK::AActor*>* list);
	void FetchFromActors(std::vector<SDK::AActor*>* list);
	void FetchFromPlayers(std::vector<SDK::AActor*>* list);	
	void FetchFromObjects(std::vector<SDK::ACharacter*>* list);
	void FetchFromActors(std::vector<SDK::ACharacter*>* list);
	void FetchFromPlayers(std::vector<SDK::ACharacter*>* list);
	void Update(DWORD tick);
	bool UpdateSDK(bool log);
	void UpdateMonstersCache();

private:

	MainLoop() = default;
	~MainLoop() = default;

	std::mutex list_mutex;


	MainLoop(const MainLoop&) = delete;
	MainLoop& operator=(const MainLoop&) = delete;
};