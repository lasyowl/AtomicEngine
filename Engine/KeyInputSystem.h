#pragma once

#include "ECS.h"

enum EKeyType
{
	KeyType_A, KeyType_B, KeyType_C, KeyType_D, KeyType_E, KeyType_F, KeyType_G, KeyType_H, KeyType_I,
	KeyType_J, KeyType_K, KeyType_L, KeyType_M, KeyType_N, KeyType_O, KeyType_P, KeyType_Q, KeyType_R,
	KeyType_S, KeyType_T, KeyType_U, KeyType_V, KeyType_W, KeyType_X, KeyType_Y, KeyType_Z,
	KeyType_Unknown,
	KeyType_Count
};

struct KeyInputComponent : public IECSComponent
{
	DEFINE_ECS_COMPONENT( KeyInput );

	std::bitset<KeyType_Count> keyDown;
	std::bitset<KeyType_Count> keyPressed;
	std::bitset<KeyType_Count> keyUp;
};

class KeyInputSystem : public ISystem
{
public:
	DEFINE_ECS_SYSTEM( KeyInput );
public:
	virtual void RunSystem( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry ) override;
};
