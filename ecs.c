// ecs.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/memory.h"
#include "include/susfwk/bitset.h"
#include "include/susfwk/vector.h"
#include "include/susfwk/hashtable.h"
#include "include/susfwk/ecs.h"

// --------------------------------------------------------------------------------------

// Create a new world of entities, components, and systems
SUS_WORLD SUSAPI susNewWorld()
{
	SUS_PRINTDL("Creating the world");
	SUS_WORLD world = sus_malloc(sizeof(SUS_WORLD_STRUCT));
	if (!world) return NULL;
	world->entities = susNewMap(SUS_ENTITY, SUS_COMPONENTMASK);
	world->systems = susNewVector(SUS_SYSTEM);
	world->componentPools = susNewVector(SUS_HASHMAP);
	world->nextEntity = 0;
	return world;
}
// Destroy the world
VOID SUSAPI susWorldDestroy(_Inout_ SUS_WORLD world)
{
	SUS_PRINTDL("The destruction of the world");
	SUS_ASSERT(world);
	susMapDestroy(world->entities);
	susVectorDestroy(world->systems);
	susVecForeach(i, world->systems) {
		susVectorDestroy(((SUS_LPSYSTEM)susVectorGet(world->systems, i))->cachedEntities);
	}
	susVecForeach(i, world->componentPools) {
		susMapDestroy(*(SUS_LPHASHMAP)susVectorGet(world->componentPools, i));
	}
	susVectorDestroy(world->componentPools);
	sus_free(world);
}
// Update the state of the world
VOID SUSAPI susWorldUpdate(_In_ SUS_WORLD world, _In_ FLOAT deltaTime)
{
	susVecForeach(i, world->systems) {
		SUS_LPSYSTEM sys = (SUS_LPSYSTEM)susVectorGet(world->systems, i);
		if (sys && sys->enabled && sys->callback)
		susSystemRun(world, i, deltaTime);
	}
}

// --------------------------------------------------------------------------------------

// Get all entities with a mask
SUS_VECTOR SUSAPI susGetEntitiesWithMask(_In_ SUS_WORLD world, _In_opt_ SUS_COMPONENTMASK mask)
{
	SUS_VECTOR entities = susNewVector(SUS_ENTITY);
	susMapForeach(world->entities, entry) {
		SUS_LPCOMPONENTMASK lpEntityMask = (SUS_LPCOMPONENTMASK)susMapValue(world->entities, entry);
		if (susBitmask256Cmp(susBitmask256op(*lpEntityMask, &, mask), mask)) {
			susVectorPushBack(&entities, susMapKey(world->entities, entry));
		}
	}
	return entities;
}
// Get all entities with this system
SUS_VECTOR SUSAPI susGetEntitiesWithSystem(_In_ SUS_WORLD world, _In_opt_ SUS_SYSTEM_ID id)
{
	SUS_LPSYSTEM system = susSystemGetStruct(world, id);
	if (!system) return susNewVector(SUS_ENTITY);
	return susGetEntitiesWithMask(world, system->mask);
}


// --------------------------------------------------------------------------------------

// Create a new entity
SUS_ENTITY SUSAPI susNewEntity(_Inout_ SUS_WORLD world)
{
	SUS_ASSERT(world);
	SUS_ENTITY entity = SUS_INVALID_ENTITY;
	if (susVectorCount(world->freeEntities) > 0) {
		entity = *(SUS_ENTITY*)susVectorBack(world->freeEntities);
		susVectorPopBack(world->freeEntities);
	}
	else {
		SUS_ASSERT(world->nextEntity < SUS_INVALID_ENTITY);
		entity = world->nextEntity++;
	}
	susMapAdd(&world->entities, (LPBYTE)&entity, NULL);
	return entity;
}
// Destroy the entity
VOID SUSAPI susEntityDestroy(_Inout_ SUS_WORLD world, _In_ SUS_ENTITY entity)
{
	SUS_ASSERT(world && susEntityExists(world, entity));
	SUS_LPCOMPONENTMASK mask = susEntityGetMask(world, entity);
	susVecForeach(i, world->componentPools) {
		if (susBitmask256Test(mask, i)) {
			susMapRemove((SUS_LPHASHMAP)susVectorGet(world->componentPools, i), (LPBYTE)&entity);
		}
	}
	susMapRemove(&world->entities, (LPBYTE)&entity);
	susVectorPushBack(&world->freeEntities, &entity);
}

// --------------------------------------------------------------------------------------

// Register a component
SUS_COMPONENT_TYPE SUSAPI susRegisterComponent(_Inout_ SUS_WORLD world, _In_ SIZE_T ComponentSize)
{
	SUS_HASHMAP pool = susNewMapSized(sizeof(SUS_ENTITY), ComponentSize);
	susVectorPushBack(&world->componentPools, &pool);
	return susVectorCount(world->componentPools) - 1;
}
// Add a component to an entity
LPBYTE SUSAPI susEntityAddComponent(_Inout_ SUS_WORLD world, _In_ SUS_ENTITY entity, _In_ SUS_COMPONENT_TYPE type, _In_opt_ LPBYTE init)
{
	SUS_ASSERT(!susEntityHasComponent(world, entity, type));
	SUS_LPCOMPONENTMASK mask = susEntityGetMask(world, entity);
	SUS_ASSERT(!susBitmask256Test(mask, type));
	susBitmask256Set(mask, type);
	return susMapValue(*(SUS_HASHMAP*)susVectorGet(world->componentPools, type), susMapAdd(susVectorGet(world->componentPools, type), (LPBYTE)&entity, init));
}
// Remove a component from an entity
VOID SUSAPI susEntityRemoveComponent(_Inout_ SUS_WORLD world, _In_ SUS_ENTITY entity, _In_ SUS_COMPONENT_TYPE type)
{
	SUS_ASSERT(susEntityHasComponent(world, entity, type));
	SUS_LPCOMPONENTMASK mask = susEntityGetMask(world, entity);
	SUS_ASSERT(susBitmask256Test(mask, type));
	susBitmask256Reset(mask, type);
	susMapRemove((SUS_LPHASHMAP)susVectorGet(world->componentPools, type), (LPBYTE)&entity);
}

// --------------------------------------------------------------------------------------

//
SUS_STATIC VOID _susRebuildSystemEntities(_In_ SUS_WORLD world, _In_ SUS_SYSTEM_ID id) {
	SUS_LPSYSTEM system = (SUS_LPSYSTEM)susVectorGet(world->systems, id);
	if (!system) return;
	susVectorClear(system->cachedEntities);
	susMapForeach(world->entities, entry) {
		SUS_LPCOMPONENTMASK lpEntityMask = (SUS_LPCOMPONENTMASK)susMapValue(world->entities, entry);
		if (susBitmask256Cmp(susBitmask256op(*lpEntityMask, &, system->mask), system->mask)) {
			susVectorPushBack(&system->cachedEntities, susMapKey(world->entities, entry));
		}
	}
}
// Register the system
SUS_SYSTEM_ID SUSAPI susRegisterSystem(_Inout_ SUS_WORLD world, _In_ SUS_SYSTEM_CALLBACK callback, _In_ SUS_COMPONENTMASK mask)
{
	SUS_SYSTEM system = {
		.callback = callback,
		.mask = mask,
		.enabled = TRUE,
		.cachedEntities = susNewVector();
	};
	susVectorPushBack(&world->systems, &system);
	return susVectorCount(world ->systems) - 1;
}
// Launching the system
VOID SUSAPI susSystemRun(_In_ SUS_WORLD world, _In_ SUS_SYSTEM_ID id, _In_ FLOAT deltaTime)
{
	SUS_LPSYSTEM system = susSystemGetStruct(world, id);
	SUS_ASSERT(system && system->enabled && system->callback);
	SUS_VECTOR entities = susGetEntitiesWithSystem(world, id);
	susVecForeach(i, entities) {
		SUS_ENTITY* entity = susVectorGet(entities, i);
		if (entity) system->callback(*entity, deltaTime);
	}
	susVectorDestroy(entities);
}
