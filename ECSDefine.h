#pragma once

#include "EngineDefine.h"

using Entity = uint32;

constexpr uint32 NUM_ENTITY_MAX = 8192;
constexpr uint32 NUM_COMPONENT_MAX = 256;
constexpr uint32 NUM_SYSTEM_MAX = 256;

#define DEFINE_ECS_COMPONENT(name) static const ECSComponentType type = ECSComponentType_##name
#define DEFINE_ECS_SYSTEM(name) static const ECSSystemType type = ECSSystemType_##name

enum ECSComponentType
{
	ECSComponentType_MetaData,
	ECSComponentType_KeyInput,
	ECSComponentType_SceneView,
	ECSComponentType_Primitive,
	ECSComponentType_Count
};

enum ECSSystemType
{
	ECSSystemType_EntityInitialize,
	ECSSystemType_KeyInput,
	ECSSystemType_SceneView,
	ECSSystemType_Render,
	ECSSystemType_Count
};
