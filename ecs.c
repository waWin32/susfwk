// ecs.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/memory.h"
#include "include/susfwk/bitset.h"
#include "include/susfwk/vector.h"
#include "include/susfwk/hashtable.h"
#include "include/susfwk/ecs.h"

//////////////////////////////////////////////////////////////////////////////////////////
//									Archetypes											//
//////////////////////////////////////////////////////////////////////////////////////////

// --------------------------------------------------------------------------------------

// Find the archetype
static inline SUS_ARCHETYPE SUSAPI susFindArchetype(_Inout_ SUS_WORLD world, _In_ SUS_COMPONENTMASK mask) {
	SUS_ASSERT(world && world->archetypes);
	return (SUS_ARCHETYPE)susMapGet(world->archetypes, &mask);
}
// Create a new archetype
static inline SUS_ARCHETYPE SUSAPI susNewArchetype(_Inout_ SUS_WORLD world, _In_ SUS_COMPONENTMASK mask) {
	SUS_ASSERT(world && world->archetypes && !susFindArchetype(world, mask));
	SUS_ARCHETYPE_STRUCT archetype = {
		.componentPools = susNewMap(SUS_COMPONENT_TYPE, SUS_VECTOR),
		.entities = susNewVector(SUS_ENTITY),
		.mask = mask
	};
	susVecForeach(i, world->registeredComponents) {
		if (susBitmask256Test(mask, i)) {
			SUS_LPREGISTERED_COMPONENT component = (SUS_LPREGISTERED_COMPONENT)susVectorGet(world->registeredComponents, i);
			SUS_VECTOR pool = susNewVectorSized(component->size);
			susMapAdd(&archetype.componentPools, &i, &pool);
		}
	}
	return (SUS_ARCHETYPE)susMapAdd(&world->archetypes, &mask, &archetype);
}
// Create an archetype
static inline SUS_ARCHETYPE SUSAPI susCreateArchetype(_Inout_ SUS_WORLD world, _In_ SUS_COMPONENTMASK mask) {
	SUS_ASSERT(world && world->archetypes);
	SUS_ARCHETYPE archetype = susFindArchetype(world, mask);
	return archetype ? archetype : susNewArchetype(world, mask);
}

// --------------------------------------------------------------------------------------

// Add an entity to an archetype
static inline sus_u32 SUSAPI susArchetypeAddEntityEx(_In_ SUS_WORLD world, _Inout_ SUS_ARCHETYPE archetype, _In_ SUS_ENTITY entity, _In_opt_ SUS_ENTITY_LOCATION constructor) {
	SUS_ASSERT(world && archetype);
	susVectorPushBack(&archetype->entities, &entity);
	SUS_COMPONENTMASK mask = archetype->mask;
	SUS_COMPONENTMASK intersection = constructor.archetype ? susBitmask256op(mask, &, constructor.archetype->mask) : (SUS_COMPONENTMASK) { 0 };
	SUS_OBJECT lpComponent;
	susVecForeach(i, world->registeredComponents) {
		if (constructor.archetype && susBitmask256Test(intersection, i)) {
			lpComponent = susVectorPushBack((SUS_LPVECTOR)susMapGet(archetype->componentPools, &i), susVectorGet(*(SUS_LPVECTOR)susMapGet(constructor.archetype->componentPools, &i), constructor.index));
		}
		else if (susBitmask256Test(mask, i)) {
			lpComponent = susVectorPushBack((SUS_LPVECTOR)susMapGet(archetype->componentPools, &i), NULL);
			if (((SUS_LPREGISTERED_COMPONENT)susVectorGet(world->registeredComponents, i))->constructor) ((SUS_LPREGISTERED_COMPONENT)susVectorGet(world->registeredComponents, i))->constructor(lpComponent);
		}
	}
	return archetype->entities->length - 1;
}
// Add an entity to an archetype
static inline SUS_ENTITY_LOCATION SUSAPI susArchetypeAddEntity(_Inout_ SUS_WORLD world, _In_ SUS_ENTITY entity, _In_ SUS_COMPONENTMASK mask) {
	SUS_ENTITY_LOCATION location = { .archetype = susCreateArchetype(world, mask) };
	location.index = susArchetypeAddEntityEx(world, location.archetype, entity, (SUS_ENTITY_LOCATION) { 0 });
	return location;
}

// --------------------------------------------------------------------------------------

// Destroy the archetype in the absence of connections
static inline VOID SUSAPI susArchetypeCull(_Inout_ SUS_WORLD world, _In_ SUS_ARCHETYPE archetype) {
	SUS_ASSERT(world && world->archetypes && archetype);
	if (!archetype->entities->length) {
		susVectorDestroy(archetype->entities);
		susMapDestroy(archetype->componentPools);
		susMapRemove(&world->archetypes, &archetype->mask);
	}
}
// Remove an entity component from an archetype
static inline VOID SUSAPI susArchetypeRemoveEntity(_In_ SUS_WORLD world, _In_ SUS_ENTITY_LOCATION location) {
	SUS_ASSERT(world && location.archetype && world->registeredComponents && location.archetype->componentPools);
	((SUS_LPENTITY_LOCATION)susMapGet(world->entities, (SUS_ENTITY*)susVectorSwapErase(&location.archetype->entities, location.index)))->index = location.index;
	susVecForeach(i, world->registeredComponents) {
		if (susBitmask256Test(location.archetype->mask, i)) {
			SUS_LPVECTOR pool = susMapGet(location.archetype->componentPools, &i);
			if (((SUS_LPREGISTERED_COMPONENT)susVectorGet(world->registeredComponents, i))->destructor) ((SUS_LPREGISTERED_COMPONENT)susVectorGet(world->registeredComponents, i))->destructor(susVectorGet(*pool, location.index));
			susVectorSwapErase(pool, location.index);
		}
	}
	susArchetypeCull(world, location.archetype);
}

// --------------------------------------------------------------------------------------

// Move an entity from one archetype to another
static inline VOID SUSAPI susArchetypeMoveEntityEx(_Inout_ SUS_WORLD world, _In_ SUS_ENTITY_LOCATION oldLoc, SUS_ARCHETYPE newArchetype) {
	SUS_ASSERT(world && world->archetypes && susVectorGet(oldLoc.archetype->entities, oldLoc.index));
	SUS_ENTITY* entity = (SUS_ENTITY*)susVectorGet(oldLoc.archetype->entities, oldLoc.index);
	susArchetypeAddEntityEx(world, newArchetype, *entity, oldLoc);
	((SUS_LPENTITY_LOCATION)susMapGet(world->entities, entity))->index = newArchetype->entities->length - 1;
	susArchetypeRemoveEntity(world, oldLoc);
}
// Move an entity from one archetype to another
static inline SUS_ARCHETYPE SUSAPI susArchetypeMoveEntity(_Inout_ SUS_WORLD world, _In_ SUS_ENTITY_LOCATION oldLoc, SUS_COMPONENTMASK newMask) {
	SUS_ASSERT(world && world->archetypes && susVectorGet(oldLoc.archetype->entities, oldLoc.index));
	SUS_ARCHETYPE newArchetype = susCreateArchetype(world, newMask);
	susArchetypeMoveEntityEx(world, oldLoc, newArchetype);
	return newArchetype;
}

// --------------------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////////////////
//									Manager of Worlds									//
//////////////////////////////////////////////////////////////////////////////////////////

// --------------------------------------------------------------------------------------

// Create a new world of entities, components, and systems
SUS_WORLD_STRUCT SUSAPI susWorldSetup()
{
	SUS_PRINTDL("Creating the world");
	return (SUS_WORLD_STRUCT) {
		.archetypes = susNewMap(SUS_COMPONENTMASK, SUS_ARCHETYPE_STRUCT),
		.entities = susNewMap(SUS_ENTITY, SUS_ENTITY_LOCATION),
		.freeEntities = susNewVector(SUS_ENTITY),
		.next = 0,
		.registeredComponents = susNewVector(SUS_REGISTERED_COMPONENT),
		.systems = susNewVector(SUS_SYSTEM)
	};
}
// Create a new world of entities, components, and systems
SUS_WORLD SUSAPI susNewWorld()
{
	SUS_PRINTDL("Creating the world");
	SUS_WORLD world = sus_malloc(sizeof(SUS_WORLD_STRUCT));
	if (!world) return NULL;
	SUS_WORLD_STRUCT worldStruct = susWorldSetup();
	sus_memcpy((LPBYTE)world, (LPBYTE)&worldStruct, sizeof(SUS_WORLD_STRUCT));
	return world;
}
// Destroy the world
VOID SUSAPI susWorldCleanup(_In_ SUS_WORLD_STRUCT* world)
{
	SUS_PRINTDL("The destruction of the world");
	susVectorDestroy(world->systems);
	SUS_VECTOR rootEntities = susNewVector(SUS_ENTITY);
	susMapForeach(world->entities, i) {
		SUS_ENTITY* entity = (SUS_ENTITY*)susMapIterKey(i);
		SUS_LPENTITY_LOCATION location = susMapGet(world->entities, entity);
		if (location->parent == SUS_INVALID_ENTITY) susVectorPushBack(&rootEntities, entity);
	}
	susVecForeach(i, rootEntities) {
		susEntityDestroy(world, *(SUS_ENTITY*)susVectorGet(rootEntities, i));
	}
	susVectorDestroy(rootEntities);
	susMapDestroy(world->archetypes);
	susMapDestroy(world->entities);
	susVectorDestroy(world->registeredComponents);
}
// Destroy the world
VOID SUSAPI susWorldDestroy(_In_ SUS_WORLD world)
{
	SUS_PRINTDL("The destruction of the world");
	SUS_ASSERT(world);
	susWorldCleanup(world);
	sus_free(world);
}
// Update the state of the world
VOID SUSAPI susWorldUpdate(_In_ SUS_WORLD world, _In_ sus_float deltaTime)
{
	SUS_ASSERT(world);
	susVecForeach(i, world->systems) {
		susSystemRun(world, i, deltaTime);
	}
}

// --------------------------------------------------------------------------------------

// Register a new component
VOID SUSAPI susWorldRegisterComponentEx(_Inout_ SUS_WORLD world, _In_ sus_size_t componentSize, _In_opt_ SUS_COMPONENT_CONSTRUCTOR constructor, _In_opt_ SUS_COMPONENT_DESTRUCTOR destructor, _Out_ SUS_LPCOMPONENT_TYPE type)
{
	SUS_PRINTDL("Component registration");
	SUS_ASSERT(world && world->registeredComponents && world->registeredComponents->length < SUS_MAX_COMPONENTS);
	SUS_REGISTERED_COMPONENT component = { .size = componentSize, .constructor = constructor, .destructor = destructor };
	*type = world->registeredComponents->length;
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
	return world->systems->length - 1;
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
	return world->systems->length - 1;
}

// --------------------------------------------------------------------------------------

// Get all entities with a mask
SUS_VECTOR SUSAPI susWorldGetEntitiesWith(_Inout_ SUS_WORLD world, _In_ SUS_COMPONENTMASK mask)
{
	SUS_ASSERT(world);
	SUS_VECTOR entities = susNewVector(SUS_ENTITY);
	if (!entities) return NULL;
	susMapForeach(world->archetypes, i) {
		SUS_BITMASK256 curmask = *(SUS_LPBITMASK256)susMapIterKey(i);
		if (susBitmask256Contains(curmask, mask)) {
			SUS_ARCHETYPE archetype = (SUS_ARCHETYPE)susMapIterValue(i);
			susVectorAppend(&entities, archetype->entities->data, archetype->entities->size);
		}
	}
	return entities;
}

// --------------------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////////////////
//									Entity Manager										//
//////////////////////////////////////////////////////////////////////////////////////////

// --------------------------------------------------------------------------------------

// Create a new entity
SUS_ENTITY SUSAPI susNewEntity(_Inout_ SUS_WORLD world, _In_ SUS_COMPONENTMASK initMask, _In_opt_ SUS_ENTITY parent)
{
	SUS_ASSERT(world && world->entities && world->freeEntities);
	SUS_ENTITY entity;
	if (world->freeEntities->length) {
		entity = *(SUS_ENTITY*)susVectorBack(world->freeEntities);
		susVectorPopBack(&world->freeEntities);
	}
	else entity = world->next++;
	SUS_ENTITY_LOCATION location = susArchetypeAddEntity(world, entity, initMask);
	location.parent = SUS_INVALID_ENTITY;
	location.children = susNewSet(SUS_ENTITY);
	susMapAdd(&world->entities, &entity, &location);
	susEntitySetParent(world, entity, parent);
	return entity;
}
// Destroy the entity
VOID SUSAPI susEntityDestroy(_Inout_ SUS_WORLD world, _In_ SUS_ENTITY entity)
{
	SUS_ASSERT(world && world->entities && world->freeEntities && susEntityExists(world, entity));
	SUS_LPENTITY_LOCATION location = susMapGet(world->entities, &entity);
	if (location->parent != SUS_INVALID_ENTITY) {
		SUS_LPENTITY_LOCATION parentLocation = susMapGet(world->entities, &entity);
		susSetRemove(&parentLocation->children, &entity);
	}
	susSetForeach(location->children, i) {
		susEntityDestroy(world, *(SUS_ENTITY*)susMapIterKey(i));
	}
	susArchetypeRemoveEntity(world, *location);
	susMapRemove(&world->entities, &entity);
	susVectorPushBack(&world->freeEntities, &entity);
}

// --------------------------------------------------------------------------------------

// Add a component to an entity
VOID SUSAPI susEntityAddComponent(_Inout_ SUS_WORLD world, _In_ SUS_ENTITY entity, _In_ SUS_COMPONENT_TYPE type)
{
	SUS_ASSERT(world && world->entities && susMapGet(world->entities, &entity) && !susEntityHasComponent(world, entity, type));
	SUS_ENTITY_LOCATION oldLocation = *(SUS_LPENTITY_LOCATION)susMapGet(world->entities, &entity);
	SUS_COMPONENTMASK mask = oldLocation.archetype->mask;
	susBitmask256Set(&mask, type);
	SUS_ENTITY_LOCATION newLocation = { .archetype = susCreateArchetype(world, mask) };
	newLocation.index = newLocation.archetype->entities->length;
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
	newLocation.index = newLocation.archetype->entities->length;
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

//////////////////////////////////////////////////////////////////////////////////////////
//								Hierarchy of entities									//
//////////////////////////////////////////////////////////////////////////////////////////

// --------------------------------------------------------------------------------------

// Check for cyclical dependence
static inline BOOL SUSAPI susHierarchyDetectCycle(_Inout_ SUS_WORLD world, _In_ SUS_ENTITY entity, _In_ SUS_ENTITY potentialParent) {
	while (potentialParent != SUS_INVALID_ENTITY) {
		if (potentialParent == entity) return TRUE;
		SUS_LPENTITY_LOCATION location = susMapGet(world->entities, &potentialParent);
		potentialParent = location ? location->parent : SUS_INVALID_ENTITY;
	}
	return FALSE;
}
// Set the parent
VOID SUSAPI susEntitySetParent(_Inout_ SUS_WORLD world, _In_ SUS_ENTITY entity, _In_opt_ SUS_ENTITY parent)
{
	SUS_ASSERT(entity != parent && !susHierarchyDetectCycle(world, entity, parent));
	SUS_LPENTITY_LOCATION location = susMapGet(world->entities, &entity);
	if (location->parent != SUS_INVALID_ENTITY) {
		SUS_LPENTITY_LOCATION oldParentLocation = susMapGet(world->entities, &location->parent);
		susSetRemove(&oldParentLocation->children, &entity);
		SUS_LPENTITY_LOCATION parentLocation = susMapGet(world->entities, &parent);
		susSetAdd(&parentLocation->children, &entity);
	}
	location->parent = parent;
}
// Get a parent
SUS_ENTITY SUSAPI susEntityGetParent(_In_ SUS_WORLD world, _In_ SUS_ENTITY entity)
{
	SUS_ASSERT(world && susEntityExists(world, entity));
	SUS_LPENTITY_LOCATION location = susMapGet(world->entities, &entity);
	return location->parent;
}

// --------------------------------------------------------------------------------------

// Get kids
SUS_HASHSET SUSAPI susEntityGetChildren(_In_ SUS_WORLD world, _In_ SUS_ENTITY entity)
{
	SUS_ASSERT(world && susEntityExists(world, entity));
	SUS_LPENTITY_LOCATION location = susMapGet(world->entities, &entity);
	return location->children;
}
// Check the affiliation
BOOL SUSAPI susEntityIsDescendantOf(_In_ SUS_WORLD world, _In_ SUS_ENTITY entity, _In_ SUS_ENTITY ancestor)
{
	do {
		entity = susEntityGetParent(world, entity);
		if (entity == ancestor) return TRUE;
	} while (entity != SUS_INVALID_ENTITY);
	return FALSE;
}

// --------------------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////////////////
//								Systems Manager											//
//////////////////////////////////////////////////////////////////////////////////////////

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
	susMapForeach(world->archetypes, i) {
		SUS_BITMASK256 mask = *(SUS_LPBITMASK256)susMapIterKey(i);
		if (susBitmask256Contains(mask, system->mask)) {
			SUS_ARCHETYPE archetype = (SUS_ARCHETYPE)susMapIterValue(i);
			susVecForeach(j, archetype->entities) {
				system->callbackEntity(world, *(SUS_ENTITY*)susVectorGet(archetype->entities, j), deltaTime, system->userData);
			}
		}
	}
}

// --------------------------------------------------------------------------------------
