#include "stdafx.h"
#include "ECS.h"

Entity ECSCreateEntity()
{
	return ECS::GetInstance().CreateEntity();
}

void ECSRunSystems()
{
	ECS::GetInstance().RunSystems();
}