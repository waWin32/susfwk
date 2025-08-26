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

// Find the archetype
SUS_STATIC SUS_INLINE SUS_ARCHETYPE SUSAPI susFindArchetype(_Inout_ SUS_WORLD world, _In_ SUS_COMPONENTMASK mask) {
	SUS_ASSERT(world && world->archetypes);
	return (SUS_ARCHETYPE)susMapGet(world->archetypes, &mask);
}
// Create a new archetype
SUS_STATIC SUS_INLINE SUS_ARCHETYPE SUSAPI susNewArchetype(_Inout_ SUS_WORLD world, _In_ SUS_COMPONENTMASK mask) {
	SUS_ASSERT(world && world->archetypes && !susFindArchetype(world, mask));
	SUS_ARCHETYPE_STRUCT archetype = {
		.componentPools = susNewMap(SUS_COMPONENT_TYPE, SUS_VECTOR),
		.entities = susNewVector(SUS_ENTITY),
		.mask = mask
	};
	susVecForeach(0, i, _count, world->registeredComponents) {
		if (susBitmask256Test(mask, i)) {
			SUS_LPREGISTERED_COMPONENT component = (SUS_LPREGISTERED_COMPONENT)susVectorGet(world->registeredComponents, i);
			SUS_VECTOR pool = susNewVectorSized(component->size);
			susMapAdd(&archetype.componentPools, &i, &pool);
		}
	}
	return (SUS_ARCHETYPE)susMapAdd(&world->archetypes, &mask, &archetype);
}
// Create an archetype
SUS_STATIC SUS_INLINE SUS_ARCHETYPE SUSAPI susCreateArchetype(_Inout_ SUS_WORLD world, _In_ SUS_COMPONENTMASK mask) {
	SUS_ASSERT(world && world->archetypes);
	SUS_ARCHETYPE archetype = susFindArchetype(world, mask);
	return archetype ? archetype : susNewArchetype(world, mask);
}
// Destroy the archetype
SUS_STATIC SUS_INLINE VOID SUSAPI susArchetypeDestroy(_Inout_ SUS_WORLD world, _In_ SUS_ARCHETYPE archetype) {
	SUS_ASSERT(world && world->archetypes && archetype);
	if (susVectorCount(archetype->entities)) {
		susMapForeach(archetype->componentPools, _count, entry) {
			susVectorDestroy(*(SUS_LPVECTOR)susMapValue(archetype->componentPools, entry));
		}
	}
	susVectorDestroy(archetype->entities);
	susMapDestroy(archetype->componentPools);
	susMapRemove(&world->archetypes, &archetype->mask);
}
// Destroy the archetype in the absence of connections
SUS_STATIC SUS_INLINE VOID SUSAPI susArchetypeCull(_Inout_ SUS_WORLD world, _In_ SUS_ARCHETYPE archetype) {
	SUS_ASSERT(world && world->archetypes && archetype);
	if (!susVectorCount(archetype->entities)) susArchetypeDestroy(world, archetype);
}
// Remove an entity component from an archetype
SUS_STATIC SUS_INLINE VOID SUSAPI susArchetypeRemoveEntity(_In_ SUS_WORLD world, _In_ SUS_ENTITY_LOCATION location) {
	SUS_ASSERT(world && location.archetype && world->registeredComponents && location.archetype->componentPools);
	((SUS_LPENTITY_LOCATION)susMapGet(world->entities, (SUS_ENTITY*)susVectorSwapErase(&location.archetype->entities, location.index)))->index = location.index;
	susVecForeach(0, i, _count, world->registeredComponents) {
		if (susBitmask256Test(location.archetype->mask, i)) {
			SUS_LPVECTOR pool = susMapGet(location.archetype->componentPools, &i);
			susVectorSwapErase(pool, location.index);
		}
	}
	susArchetypeCull(world, location.archetype);
}
// Add an entity to an archetype
SUS_STATIC SUS_INLINE sus_u32 SUSAPI susArchetypeAddEntityEx(_In_ SUS_WORLD world, _Inout_ SUS_ARCHETYPE archetype, _In_ SUS_ENTITY entity, _In_opt_ SUS_ENTITY_LOCATION constructor) {
	SUS_ASSERT(world && archetype);
	susVectorPushBack(&archetype->entities, &entity);
	SUS_COMPONENTMASK mask = archetype->mask;
	SUS_COMPONENTMASK intersection = constructor.archetype ? susBitmask256op(mask, &, constructor.archetype->mask) : (SUS_COMPONENTMASK) { 0 };
	susVecForeach(0, i, _count, world->registeredComponents) {
		if (susBitmask256Test(intersection, i)) {
			SUS_ASSERT(constructor.archetype);
			susVectorPushBack((SUS_LPVECTOR)susMapGet(archetype->componentPools, &i), susVectorGet(*(SUS_LPVECTOR)susMapGet(constructor.archetype->componentPools, &i), constructor.index));
		}
		else if (susBitmask256Test(mask, i)) {
			susVectorPushBack((SUS_LPVECTOR)susMapGet(archetype->componentPools, &i), (((SUS_LPREGISTERED_COMPONENT)susVectorGet(world->registeredComponents, i))->constructor));
		}
	}
	return susVectorCount(archetype->entities) - 1;
}
// Add an entity to an archetype
SUS_STATIC SUS_INLINE SUS_ENTITY_LOCATION SUSAPI susArchetypeAddEntity(_Inout_ SUS_WORLD world, _In_ SUS_ENTITY entity, _In_ SUS_COMPONENTMASK mask) {
	SUS_ENTITY_LOCATION location = { .archetype = susCreateArchetype(world, mask) };
	location.index = susArchetypeAddEntityEx(world, location.archetype, entity, (SUS_ENTITY_LOCATION) { 0 });
	return location;
}
// Move an entity from one archetype to another
SUS_STATIC SUS_INLINE VOID SUSAPI susArchetypeMoveEntityEx(_Inout_ SUS_WORLD world, _In_ SUS_ENTITY_LOCATION oldLoc, SUS_ARCHETYPE newArchetype) {
	SUS_ASSERT(world && world->archetypes && susVectorGet(oldLoc.archetype->entities, oldLoc.index));
	SUS_ENTITY* entity = (SUS_ENTITY*)susVectorGet(oldLoc.archetype->entities, oldLoc.index);
	susArchetypeAddEntityEx(world, newArchetype, *entity, oldLoc);
	((SUS_LPENTITY_LOCATION)susMapGet(world->entities, entity))->index = susVectorCount(newArchetype->entities) - 1;
	susArchetypeRemoveEntity(world, oldLoc);
}
// Move an entity from one archetype to another
SUS_STATIC SUS_INLINE SUS_ARCHETYPE SUSAPI susArchetypeMoveEntity(_Inout_ SUS_WORLD world, _In_ SUS_ENTITY_LOCATION oldLoc, SUS_COMPONENTMASK newMask) {
	SUS_ASSERT(world && world->archetypes && susVectorGet(oldLoc.archetype->entities, oldLoc.index));
	SUS_ARCHETYPE newArchetype = susCreateArchetype(world, newMask);
	susArchetypeMoveEntityEx(world, oldLoc, newArchetype);
	return newArchetype;
}

// --------------------------------------------------------------------------------------

// Create a new world of entities, components, and systems
SUS_WORLD SUSAPI susNewWorld()
{
	SUS_PRINTDL("Creating the world");
	SUS_WORLD world = sus_malloc(sizeof(SUS_WORLD_STRUCT));
	if (!world) return NULL;
	world->archetypes = susNewMap(SUS_COMPONENTMASK, SUS_ARCHETYPE_STRUCT);
	world->entities = susNewMap(SUS_ENTITY, SUS_ENTITY_LOCATION);
	world->freeEntities = susNewVector(SUS_ENTITY);
	world->registeredComponents = susNewVector(SUS_REGISTERED_COMPONENT);
	world->systems = susNewVector(SUS_SYSTEM);
	return world;
}
// Destroy the world
VOID SUSAPI susWorldDestroy(_Inout_ SUS_WORLD world)
{
	SUS_PRINTDL("The destruction of the world");
	SUS_ASSERT(world);
	susVectorDestroy(world->systems);
	susMapForeach(world->archetypes, _count, entry) {
		susArchetypeDestroy(world, susMapValue(world->archetypes, entry));
	}
	susMapDestroy(world->archetypes);
	susMapDestroy(world->entities);
	susVectorDestroy(world->registeredComponents);
	sus_free(world);
}
// Update the state of the world
VOID SUSAPI susWorldUpdate(_In_ SUS_WORLD world, _In_ FLOAT deltaTime)
{
	SUS_ASSERT(world);
	susVecForeach(0, i, _count, world->systems) {
		susSystemRun(world, i, deltaTime);
	}
}
// Register a new component
VOID SUSAPI susWorldRegisterComponent(_Inout_ SUS_WORLD world, _In_ sus_size_t componentSize, _In_opt_ SUS_OBJECT constructor, _Out_ SUS_LPCOMPONENT_TYPE type)
{
	SUS_PRINTDL("Component registration");
	SUS_ASSERT(world && world->registeredComponents && susVectorCount(world->registeredComponents) < SUS_MAX_COMPONENTS);
	SUS_REGISTERED_COMPONENT component = { .size = componentSize, .constructor = constructor };
	*type = susVectorCount(world->registeredComponents);
	susVectorPushBack(&world->registeredComponents, &component);
}
// Register an entity processing system
SUS_SYSTEM_ID SUSAPI susWorldRegisterEntitySystem(_Inout_ SUS_WORLD world, _In_ SUS_SYSTEM_ENTITY_CALLBACK callback, _In_ SUS_COMPONENTMASK mask, _In_opt_ SUS_OBJECT userData)
{
	SUS_ASSERT(world && callback);
	SUS_SYSTEM system = {
		.callbackEntity = callback,
		.enabled = TRUE,
		.type = SUS_SYSTEM_TYPE_ENTITY,
		.mask = mask,
		.userData = userData
	};
	susVectorPushBack(&world->systems, &system);
	return susVectorCount(world->systems) - 1;
}
// Register a free system
SUS_SYSTEM_ID SUSAPI susWorldRegisterFreeSystem(_Inout_ SUS_WORLD world, _In_ SUS_SYSTEM_FREE_CALLBACK callback, _In_opt_ SUS_OBJECT userData)
{
	SUS_ASSERT(world && callback);
	SUS_SYSTEM system = {
		.callbackFree = callback,
		.enabled = TRUE,
		.type = SUS_SYSTEM_TYPE_FREE,
		.mask = (SUS_COMPONENTMASK) { 0 },
		.userData = userData
	};
	susVectorPushBack(&world->systems, &system);
	return susVectorCount(world->systems) - 1;
}
// Get all entities with a mask
SUS_VECTOR SUSAPI susWorldGetEntitiesWith(_Inout_ SUS_WORLD world, _In_ SUS_COMPONENTMASK mask)
{
	SUS_ASSERT(world);
	SUS_VECTOR entities = susNewVector(SUS_ENTITY);
	if (!entities) return NULL;
	susMapForeach(world->archetypes, _count, entry) {
		SUS_BITMASK256 curmask = *(SUS_LPBITMASK256)susMapKey(world->archetypes, entry);
		if (susBitmask256Contains(curmask, mask)) {
			SUS_ARCHETYPE archetype = (SUS_ARCHETYPE)susMapValue(world->archetypes, entry);
			susVectorAppend(&entities, archetype->entities->data, archetype->entities->size);
		}
	}
	return entities;
}

// --------------------------------------------------------------------------------------

// Create a new entity
SUS_ENTITY SUSAPI susNewEntity(_Inout_ SUS_WORLD world, _In_ SUS_COMPONENTMASK initMask)
{
	SUS_ASSERT(world && world->entities && world->freeEntities);
	SUS_ENTITY entity;
	if (susVectorCount(world->freeEntities)) {
		entity = *(SUS_ENTITY*)susVectorBack(world->freeEntities);
		susVectorPopBack(&world->freeEntities);
	}
	else {
		entity = world->next++;
	}
	SUS_ENTITY_LOCATION location = susArchetypeAddEntity(world, entity, initMask);
	susMapAdd(&world->entities, &entity, &location);
	return entity;
}
// Destroy the entity
VOID SUSAPI susEntityDestroy(_Inout_ SUS_WORLD world, _In_ SUS_ENTITY entity)
{
	SUS_ASSERT(world && world->entities && world->freeEntities && susEntityExists(world, entity));
	SUS_ENTITY_LOCATION location = *(SUS_LPENTITY_LOCATION)susMapGet(world->entities, &entity);
	susArchetypeRemoveEntity(world, location);
	susMapRemove(&world->entities, &entity);
	susVectorPushBack(&world->freeEntities, &entity);
}
// Add a component to an entity
VOID SUSAPI susEntityAddComponent(_Inout_ SUS_WORLD world, _In_ SUS_ENTITY entity, _In_ SUS_COMPONENT_TYPE type)
{
	SUS_ASSERT(world && world->entities && susMapGet(world->entities, &entity) && !susEntityHasComponent(world, entity, type));
	SUS_ENTITY_LOCATION oldLocation = *(SUS_LPENTITY_LOCATION)susMapGet(world->entities, &entity);
	SUS_COMPONENTMASK mask = oldLocation.archetype->mask;
	susBitmask256Set(&mask, type);
	SUS_ENTITY_LOCATION newLocation = { .archetype = susCreateArchetype(world, mask) };
	newLocation.index = susVectorCount(newLocation.archetype->entities);
	susArchetypeMoveEntity(world, oldLocation, mask);
}
// Remove a component from an entity
VOID SUSAPI susEntityRemoveComponent(_Inout_ SUS_WORLD world, _In_ SUS_ENTITY entity, _In_ SUS_COMPONENT_TYPE type)
{
	SUS_ASSERT(world && world->entities && susMapGet(world->entities, &entity) && susEntityHasComponent(world, entity, type));
	SUS_ENTITY_LOCATION oldLocation = *(SUS_LPENTITY_LOCATION)susMapGet(world->entities, &entity);
	SUS_COMPONENTMASK mask = oldLocation.archetype->mask;
	susBitmask256Reset(&mask, type);
	SUS_ENTITY_LOCATION newLocation = { .archetype = susCreateArchetype(world, mask) };
	newLocation.index = susVectorCount(newLocation.archetype->entities);
	susArchetypeMoveEntity(world, oldLocation, mask);
}
// Replace the component
VOID SUSAPI susEntityReplaceComponent(_Inout_ SUS_WORLD world, _In_ SUS_ENTITY entity, _In_ SUS_COMPONENT_TYPE srcType, _In_ SUS_COMPONENT_TYPE newType)
{
	SUS_ASSERT(world && world->entities && susEntityHasComponent(world, entity, srcType));
	SUS_ENTITY_LOCATION oldLocation = *(SUS_LPENTITY_LOCATION)susMapGet(world->entities, &entity);
	SUS_COMPONENTMASK mask = oldLocation.archetype->mask;
	susBitmask256Reset(&mask, srcType);
	susBitmask256Set(&mask, newType);
	susArchetypeMoveEntity(world, oldLocation, mask);
}

// --------------------------------------------------------------------------------------

// Launch the system
VOID SUSAPI susSystemRun(_Inout_ SUS_WORLD world, _In_ SUS_SYSTEM_ID index, _In_ FLOAT deltaTime)
{
	SUS_ASSERT(world && susSystemExists(world, index));
	SUS_LPSYSTEM system = susWorldGetSystem(world, index);
	if (!system->enabled) return;
	if (system->type == SUS_SYSTEM_TYPE_FREE) {
		system->callbackFree(world, deltaTime, system->userData);
		return;
	}
	susMapForeach(world->archetypes, _count, entry) {
		SUS_BITMASK256 mask = *(SUS_LPBITMASK256)susMapKey(world->archetypes, entry);
		if (susBitmask256Contains(mask, system->mask)) {
			SUS_ARCHETYPE archetype = (SUS_ARCHETYPE)susMapValue(world->archetypes, entry);
			susVecForeach(0, i, entityCount, archetype->entities) {
				system->callbackEntity(*(SUS_ENTITY*)susVectorGet(archetype->entities, i), deltaTime, system->userData);
			}
		}
	}
}