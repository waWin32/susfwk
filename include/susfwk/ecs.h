// ecs.h
//
#ifndef _SUS_ECS_
#define _SUS_ECS_

typedef sus_u32 SUS_ENTITY;
typedef SUS_BITMASK256 SUS_COMPONENTMASK, *SUS_LPCOMPONENTMASK;
typedef DWORD SUS_COMPONENT_TYPE;
typedef DWORD SUS_SYSTEM_ID;
typedef VOID(SUSAPI* SUS_SYSTEM_CALLBACK)(SUS_ENTITY entity, FLOAT deltaTime);

#define SUS_MAX_COMPONENTS 256
#define SUS_INVALID_COMPONENT_TYPE	INFINITE
#define SUS_INVALID_ENTITY			INFINITE

// --------------------------------------------------------------------------------------

typedef struct sus_system {
	SUS_SYSTEM_CALLBACK callback;
	SUS_COMPONENTMASK	mask;
	SUS_VECTOR			cachedEntities; // Not used
	BOOL				enabled;
} SUS_SYSTEM, *SUS_LPSYSTEM;

typedef struct sus_world {
	SUS_HASHMAP	entities;		// SUS_ENTITY -> SUS_COMPONENTMASK
	SUS_VECTOR	componentPools;	// SUS_HASHMAP SUS_ENTITY -> COMPONENT
	SUS_VECTOR	systems;		// SUS_SYSTEM
	SUS_ENTITY	nextEntity;
	SUS_VECTOR	freeEntities;
} SUS_WORLD_STRUCT, *SUS_WORLD, **SUS_LPWORLD;

#define SUS_DECL_COMPONENT(componentName) extern SUS_COMPONENT_TYPE componentName##Type; struct componentName##Component

// --------------------------------------------------------------------------------------

// Create a new world of entities, components, and systems
SUS_WORLD SUSAPI susNewWorld();
// Destroy the world
VOID SUSAPI susWorldDestroy(_Inout_ SUS_WORLD world);
// Update the state of the world
VOID SUSAPI susWorldUpdate(_In_ SUS_WORLD world, _In_ FLOAT deltaTime);

// --------------------------------------------------------------------------------------

// Get all entities with a mask
SUS_VECTOR SUSAPI susGetEntitiesWithMask(_In_ SUS_WORLD world, _In_opt_ SUS_COMPONENTMASK mask);
// Get all entities with this system
SUS_VECTOR SUSAPI susGetEntitiesWithSystem(_In_ SUS_WORLD world, _In_opt_ SUS_SYSTEM_ID id);

// --------------------------------------------------------------------------------------

// Create a new entity
SUS_ENTITY SUSAPI susNewEntity(_Inout_ SUS_WORLD world);
// Destroy the entity
VOID SUSAPI susEntityDestroy(_Inout_ SUS_WORLD world, _In_ SUS_ENTITY entity);

// Verify the existence of an entity
SUS_INLINE BOOL SUSAPI susEntityExists(_In_ SUS_WORLD world, _In_ SUS_ENTITY entity) {
	return (BOOL)(INT_PTR)susMapGet(world->entities, (LPBYTE)&entity);
}
// Get an entity mask
SUS_INLINE SUS_LPCOMPONENTMASK SUSAPI susEntityGetMask(_In_ SUS_WORLD world, _In_ SUS_ENTITY entity) {
	SUS_ASSERT(susEntityExists(world, entity));
	return (SUS_LPCOMPONENTMASK)susMapGetValue(world->entities, (LPBYTE)&entity);
}

// --------------------------------------------------------------------------------------

// Register a component
SUS_COMPONENT_TYPE SUSAPI susRegisterComponent(_Inout_ SUS_WORLD world, _In_ SIZE_T ComponentSize);
// Add a component to an entity
LPBYTE SUSAPI susEntityAddComponent(_Inout_ SUS_WORLD world, _In_ SUS_ENTITY entity, _In_ SUS_COMPONENT_TYPE type, _In_opt_ LPBYTE init);
// Remove a component from an entity
VOID SUSAPI susEntityRemoveComponent(_Inout_ SUS_WORLD world, _In_ SUS_ENTITY entity, _In_ SUS_COMPONENT_TYPE type);

// Verify the component's existence
SUS_INLINE BOOL SUSAPI susEntityHasComponent(_In_ SUS_WORLD world, _In_ SUS_ENTITY entity, _In_ SUS_COMPONENT_TYPE type) {
	return (BOOL)(INT_PTR)susBitmask256Test(susEntityGetMask(world, entity), type);
}
// Get an Entity component
SUS_INLINE LPBYTE SUSAPI susEntityGetComponent(_In_ SUS_WORLD world, _In_ SUS_ENTITY entity, _In_ SUS_COMPONENT_TYPE type) {
	SUS_ASSERT(susEntityHasComponent(world, entity, type));
	return susMapGetValue(*(SUS_HASHMAP*)susVectorGet(world->componentPools, type), (LPBYTE)&entity);
}

// --------------------------------------------------------------------------------------

// Register the system
SUS_SYSTEM_ID SUSAPI susRegisterSystem(_Inout_ SUS_WORLD world, _In_ SUS_SYSTEM_CALLBACK callback, _In_ SUS_COMPONENTMASK mask);
// Launching the system
VOID SUSAPI susSystemRun(_In_ SUS_WORLD world, _In_ SUS_SYSTEM_ID id, _In_ FLOAT deltaTime);

// Get system structure data
SUS_INLINE SUS_LPSYSTEM SUSAPI susSystemGetStruct(_In_ SUS_WORLD world, _In_ SUS_SYSTEM_ID id) {
	return (SUS_LPSYSTEM)susVectorGet(world->systems, id);
}
// Set the signature for the system
SUS_INLINE VOID SUSAPI susSystemSetSignature(_Inout_ SUS_WORLD world, _In_ SUS_SYSTEM_ID id, _In_ SUS_COMPONENTMASK mask) {
	SUS_LPSYSTEM system = susSystemGetStruct(world, id);
	if (!system) return;
	system->mask = mask;
}
// Set the signature for the system
SUS_INLINE VOID SUSAPI susSystemSetEnable(_Inout_ SUS_WORLD world, _In_ SUS_SYSTEM_ID id, _In_ BOOL enabled) {
	SUS_LPSYSTEM system = susSystemGetStruct(world, id);
	if (!system) return;
	system->enabled = enabled;
}

// --------------------------------------------------------------------------------------

#endif /* !_SUS_ECS_ */
