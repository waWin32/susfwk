// ecs.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/memory.h"
#include "include/susfwk/ecs.h"

// Create a new ecs world
ECS_LPWORLD SUSAPI susNewWorld()
{
	SUS_PRINTDL("Creating the world");
	ECS_LPWORLD world = sus_malloc(sizeof(ECS_WORLD));
	if (!world) return NULL;
	world->componentTypes = susNewArray(ECS_COMPONENT_POOL);
	world->next = 0;
	return world;
}
VOID SUSAPI susWorldDestroy(ECS_LPWORLD world)
{
	for (DWORD i = 0; i < susArrayCount(world->componentTypes); i++) {
		susArrayDestroy(&((ECS_LPCOMPONENT_POOL)susArrayAt(world->componentTypes, i))->components);
	}
	susArrayDestroy(&world->componentTypes);
	sus_free(world);
}

// ----------------------------------------------

ECS_ENTITY SUSAPI susNewEntity(ECS_LPWORLD world) {
	return world->next++;;
}
VOID SUSAPI susEntityDestroy(ECS_LPWORLD world, ECS_ENTITY entity)
{
	for (ECS_COMPONENT_TYPE i = 0; i < susArrayCount(world->componentTypes); i++) {
		susComponentRemove(world, entity, i);
	}
}

// ----------------------------------------------

ECS_COMPONENT_TYPE SUSAPI susRegisterComponent(ECS_LPWORLD world, SIZE_T componentSize)
{
	ECS_COMPONENT_POOL pool = {
		.components = susNewArrayEx(sizeof(ECS_COMPONENT) + componentSize),
	};
	susArrayPushBack(&world->componentTypes, &pool);
	return susArrayCount(world->componentTypes) - 1;
}
SUS_LPMEMORY susComponentGet(ECS_LPWORLD world, ECS_ENTITY entity, ECS_COMPONENT_TYPE type)
{
	ECS_LPCOMPONENT_POOL pool = (ECS_LPCOMPONENT_POOL)susArrayAt(world->componentTypes, type);
	if (!pool) return NULL;
	for (DWORD i = 0; i < susArrayCount(pool->components); i++) {
		ECS_LPCOMPONENT component = (ECS_LPCOMPONENT)susArrayAt(pool->components, i);
		if (component->entity == entity) {
			return (SUS_LPMEMORY)component->data;
		}
	}
	return NULL;
}
VOID susComponentAdd(ECS_LPWORLD world, ECS_ENTITY entity, ECS_COMPONENT_TYPE type, CONST LPBYTE InitializingConstructor)
{
	if (susComponentGet(world, entity, type)) return;
	ECS_LPCOMPONENT_POOL pool = (ECS_LPCOMPONENT_POOL)susArrayAt(world->componentTypes, type);
	if (!pool) return;
	ECS_LPCOMPONENT component = sus_fmalloc(susArrayTypeSize(pool->components));
	if (!component) return;
	component->entity = entity;
	sus_memcpy(component->data, InitializingConstructor, susArrayTypeSize(pool->components));
	susArrayPushBack(&pool->components, component);
	sus_free(component);
}
VOID susComponentRemove(ECS_LPWORLD world, ECS_ENTITY entity, ECS_COMPONENT_TYPE type)
{
	ECS_LPCOMPONENT_POOL pool = (ECS_LPCOMPONENT_POOL)susArrayAt(world->componentTypes, type);
	if (!pool) return;
	for (DWORD i = 0; i < susArrayCount(pool->components); i++) {
		ECS_LPCOMPONENT component = (ECS_LPCOMPONENT)susArrayAt(pool->components, i);
		if (component->entity == entity) {
			susArrayErase(&pool->components, i);
			return;
		}
	}
}