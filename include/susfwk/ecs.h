// ecs.h
//
#ifndef _SUS_ECS_
#define _SUS_ECS_

typedef SUS_BITMASK256 SUS_COMPONENTMASK, *SUS_LPCOMPONENTMASK;
typedef sus_u32 SUS_COMPONENT_TYPE;
typedef sus_u32 SUS_ENTITY;
typedef sus_u32 SUS_SYSTEM_ID;
// The system's callback function
typedef VOID(SUSAPI* SUS_SYSTEM_CALLBACK)(SUS_ENTITY entity, FLOAT deltaTime, SUS_OBJECT userData);

#define SUS_MAX_COMPONENTS	256

// --------------------------------------------------------------------------------------

// The archetype of entities
typedef struct sus_archetype {
	SUS_COMPONENTMASK mask;		// The mask of the archetype components
	SUS_VECTOR entities;		// SUS_ENTITY
	SUS_HASHMAP componentPools; // SUS_COMPONENT_TYPE -> SUS_VECTOR(of components)
} SUS_ARCHETYPE_STRUCT, *SUS_ARCHETYPE;
// The position of the entity in the archetype
typedef struct sus_entity_location {
	SUS_ARCHETYPE archetype;	// The Archetype
	sus_u32 index;				// The index of the entity in the archetype
} SUS_ENTITY_LOCATION, *SUS_LPENTITY_LOCATION;
// ECS system structure
typedef struct sus_system {
	SUS_SYSTEM_CALLBACK callback;	// System function
	SUS_COMPONENTMASK	mask;		// System Mask
	BOOL				enabled;	// System status - TRUE/FALSE
	SUS_OBJECT			userData;	// User data
} SUS_SYSTEM, *SUS_LPSYSTEM;
// Registered components in the world
typedef struct sus_registered_component {
	sus_size_t size;					// Size of the registered component
	CONST SUS_OBJECT constructor;	// Component Template
} SUS_REGISTERED_COMPONENT, *SUS_LPREGISTERED_COMPONENT;
// A pool for storing all the world's data
typedef struct sus_world {
	SUS_HASHMAP archetypes;				// SUS_COMPONENTMASK -> SUS_ARCHETYPE_STRUCT
	SUS_HASHMAP entities;				// SUS_ENTITY -> SUS_ENTITY_LOCATION
	SUS_VECTOR  systems;				// SUS_SYSTEM
	SUS_VECTOR registeredComponents;	// SUS_REGISTERED_COMPONENT
	SUS_VECTOR freeEntities;			// SUS_ENTITY
	SUS_ENTITY next;					// The following entity id
} SUS_WORLD_STRUCT, *SUS_WORLD;
// Create a component
#define SUS_DECLARE_COMPONENT(componentName, data) SUS_COMPONENT_TYPE componentName##ComponentType; struct componentName##Component { data } componentName##Constructor

// --------------------------------------------------------------------------------------

// Create a new world of entities, components, and systems
SUS_WORLD SUSAPI susNewWorld();
// Destroy the world
VOID SUSAPI susWorldDestroy(
	_Inout_ SUS_WORLD world
);
// Update the state of the world
VOID SUSAPI susWorldUpdate(
	_In_ SUS_WORLD world,
	_In_ FLOAT deltaTime
);
// Register a new component
SUS_COMPONENT_TYPE SUSAPI susWorldRegisterComponentEx(
	_Inout_ SUS_WORLD world,
	_In_ sus_size_t componentSize,
	_In_opt_ SUS_OBJECT constructor
);
#define susWorldRegisterComponent(world, componentName, constructor) componentName##ComponentType = susWorldRegisterComponentEx(world, sizeof(componentName##Component), constructor)
// Register the system
SUS_SYSTEM_ID SUSAPI susWorldRegisterSystem(
	_Inout_ SUS_WORLD world,
	_In_ SUS_SYSTEM_CALLBACK callback,
	_In_ SUS_COMPONENTMASK mask,
	_In_opt_ SUS_OBJECT userData
);

// --------------------------------------------------------------------------------------

// Create a new entity
SUS_ENTITY SUSAPI susNewEntity(
	_Inout_ SUS_WORLD world,
	_In_ SUS_COMPONENTMASK initMask
);
// Destroy the entity
VOID SUSAPI susEntityDestroy(
	_Inout_ SUS_WORLD world,
	_In_ SUS_ENTITY entity
);
// Add a component to an entity
VOID SUSAPI susEntityAddComponent(
	_Inout_ SUS_WORLD world,
	_In_ SUS_ENTITY entity,
	_In_ SUS_COMPONENT_TYPE type
);
// Remove a component from an entity
VOID SUSAPI susEntityRemoveComponent(
	_Inout_ SUS_WORLD world,
	_In_ SUS_ENTITY entity,
	_In_ SUS_COMPONENT_TYPE type
);
// Replace the component
VOID SUSAPI susEntityReplaceComponent(
	_Inout_ SUS_WORLD world,
	_In_ SUS_ENTITY entity,
	_In_ SUS_COMPONENT_TYPE srcType,
	_In_ SUS_COMPONENT_TYPE newType
);

// Check the entity for existence
SUS_INLINE BOOL SUSAPI susEntityExists(_Inout_ SUS_WORLD world, _In_ SUS_ENTITY entity) {
	SUS_ASSERT(world && world->entities);
	return (BOOL)((ULONGLONG)susMapGet(world->entities, &entity));
}
// Check the component for the existence of the component
SUS_INLINE BOOL SUSAPI susEntityHasComponent(_Inout_ SUS_WORLD world, _In_ SUS_ENTITY entity, _In_ SUS_COMPONENT_TYPE type) {
	SUS_ASSERT(world && world->entities);
	SUS_ENTITY_LOCATION location = *(SUS_LPENTITY_LOCATION)susMapGet(world->entities, &entity);
	return susBitmask256Test(location.archetype->mask, type);
}
// Replace the component
SUS_INLINE SUS_OBJECT SUSAPI susEntityGetComponent(_Inout_ SUS_WORLD world, _In_ SUS_ENTITY entity, _In_ SUS_COMPONENT_TYPE type) {
	SUS_ASSERT(world && world->entities && susEntityHasComponent(world, entity, type));
	SUS_ENTITY_LOCATION location = *(SUS_LPENTITY_LOCATION)susMapGet(world->entities, &entity);
	return susVectorGet(*(SUS_LPVECTOR)susMapGet(location.archetype->componentPools, &type), location.index);
}

// --------------------------------------------------------------------------------------

// Launch the system
VOID SUSAPI susSystemRun(
	_Inout_ SUS_WORLD world,
	_In_ SUS_SYSTEM_ID index,
	_In_ FLOAT deltaTime
);

// Get a system
SUS_INLINE SUS_STATIC SUS_LPSYSTEM SUSAPI susWorldGetSystem(_In_ SUS_WORLD world, _In_ SUS_SYSTEM_ID index) {
	SUS_ASSERT(world);
	return (SUS_LPSYSTEM)susVectorGet(world->systems, index);
}
// Check the system for existence
SUS_INLINE SUS_STATIC BOOL SUSAPI susSystemExists(_In_ SUS_WORLD world, _In_ SUS_SYSTEM_ID index) {
	return (BOOL)((ULONGLONG)susWorldGetSystem(world, index));
}
// Set the operating status of the system
SUS_INLINE VOID SUSAPI susSystemSetEnabled(_Inout_ SUS_WORLD world, _In_ SUS_SYSTEM_ID index, _In_ BOOL enabled) {
	SUS_ASSERT(world && susSystemExists(world, index));
	susWorldGetSystem(world, index)->enabled = enabled;
}
// Install the user data of the system
SUS_INLINE VOID SUSAPI susSystemSetUserdata(_Inout_ SUS_WORLD world, _In_ SUS_SYSTEM_ID index, _In_ SUS_OBJECT userData) {
	SUS_ASSERT(world && susSystemExists(world, index));
	susWorldGetSystem(world, index)->userData = userData;
}

#endif /* !_SUS_ECS_ */
