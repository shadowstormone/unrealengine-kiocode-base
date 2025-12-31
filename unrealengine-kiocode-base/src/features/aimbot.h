#pragma once

#include "../includes.h"

class Aimbot {
public:
	static Aimbot& GetInstance() {
		static Aimbot instance;
		return instance;
	}

	void RegularAimbot(SDK::ACharacter* pawn);

private:
	void DebugPrintBonePairs(SDK::ACharacter* pawn);
	Aimbot() = default;
	~Aimbot() = default;

	Aimbot(const Aimbot&) = delete;
	Aimbot& operator=(const Aimbot&) = delete;
};