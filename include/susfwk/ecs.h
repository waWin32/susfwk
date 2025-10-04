// ecs.h
//
#ifndef _SUS_ECS_
#define _SUS_ECS_

//////////////////////////////////////////////////////////////////////////////////////////
//								Dealing with time differences							//
//////////////////////////////////////////////////////////////////////////////////////////

// --------------------------------------------------------------------------------------

// The structure of the time delta
typedef struct sus_delta_time {
	LARGE_INTEGER	lastTime;
	LARGE_INTEGER	frequency;
	FLOAT			invFrequency;
} SUS_DTIMER, *SUS_LPDTIMER;
// Initialize the time delta
SUS_INLINE SUS_DTIMER SUSAPI susDTimerInit() {
	SUS_DTIMER timer = { 0 };
	QueryPerformanceFrequency(&timer.frequency);
	QueryPerformanceCounter(&timer.lastTime);
	timer.invFrequency = 1.0f / (FLOAT)timer.frequency.QuadPart;
	return timer;
}
// Update the delta from time to time
SUS_INLINE FLOAT SUSAPI susDTimeGet(SUS_LPDTIMER timer) {
	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);
	LONGLONG delta = currentTime.QuadPart - timer->lastTime.QuadPart;
	timer->lastTime = currentTime;
	return delta * timer->invFrequency;
}

// --------------------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////////////////
//										ECS structures									//
//////////////////////////////////////////////////////////////////////////////////////////

// --------------------------------------------------------------------------------------

typedef SUS_BITMASK256 SUS_COMPONENTMASK, *SUS_LPCOMPONENTMASK;
typedef sus_u32 SUS_COMPONENT_TYPE, *SUS_LPCOMPONENT_TYPE;
typedef sus_u32 SUS_ENTITY;
typedef sus_u32 SUS_SYSTEM_ID;
// Maximum number of possible registered components (SUS_BITMASK256 limit - 256 bits)
#define SUS_MAX_COMPONENTS	256
#define SUS_INVALID_ENTITY	INFINITE
#define SUS_COMPONENT SUS_STRUCT
// Declare the component
#define SUS_DECLARE_COMPONENT(ComponentName) extern SUS_COMPONENT_TYPE ComponentName##Type; SUS_STRUCT ComponentName
// Define the component
#define SUS_DEFINE_COMPONENT(ComponentName) SUS_COMPONENT_TYPE ComponentName##Type	

// --------------------------------------------------------------------------------------

// Type of system
typedef enum sus_system_type {
	SUS_SYSTEM_TYPE_ENTITY,
	SUS_SYSTEM_TYPE_FREE
} SUS_SYSTEM_TYPE;
// The archetype of entities
typedef struct sus_archetype {
	SUS_COMPONENTMASK mask;		// The mask of the archetype components
	SUS_VECTOR entities;		// SUS_ENTITY
	SUS_HASHMAP componentPools; // SUS_COMPONENT_TYPE -> SUS_VECTOR(of components)
} SUS_ARCHETYPE_STRUCT, *SUS_ARCHETYPE;
// The position of the entity in the archetype
typedef struct sus_entity_location {
	SUS_ARCHETYPE	archetype;	// The Archetype
	sus_u32			index;		// The index of the entity in the archetype
	SUS_ENTITY		parent;		// Parent Entity
	SUS_HASHSET		children;	// Children of the entity
} SUS_ENTITY_LOCATION, *SUS_LPENTITY_LOCATION;
// The system's callback function
typedef VOID(SUSAPI* SUS_SYSTEM_ENTITY_CALLBACK)(SUS_OBJECT world, SUS_ENTITY entity, FLOAT deltaTime, SUS_OBJECT userData);
// The system's callback function
typedef VOID(SUSAPI* SUS_SYSTEM_FREE_CALLBACK)(SUS_OBJECT world, FLOAT deltaTime, SUS_OBJECT userData);
// ECS system structure
typedef struct sus_system {
#pragma warning(push)
#pragma warning(disable: 4201)
	union sus_system_callback_union {
		SUS_SYSTEM_ENTITY_CALLBACK	callbackEntity;	// System function
		SUS_SYSTEM_FREE_CALLBACK	callbackFree;	// System function
	};
#pragma warning(pop)
	SUS_COMPONENTMASK	mask;		// System Mask
	sus_bool			enabled;	// System status
	SUS_SYSTEM_TYPE		type;		// Type of system
	SUS_OBJECT			userData;	// User data
} SUS_SYSTEM, *SUS_LPSYSTEM;

// ECS Component Constructor
typedef VOID(SUSAPI* SUS_COMPONENT_CONSTRUCTOR)(SUS_OBJECT component);
// The ECS component destructor
typedef VOID(SUSAPI* SUS_COMPONENT_DESTRUCTOR)(SUS_OBJECT component);

// Registered components in the world
typedef struct sus_registered_component {
	sus_size_t					size;		// Size of the registered component
	SUS_COMPONENT_CONSTRUCTOR	constructor;// The component Constructor
	SUS_COMPONENT_DESTRUCTOR	destructor;	// The component's destructor
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

// --------------------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////////////////
//									Manager of Worlds									//
//////////////////////////////////////////////////////////////////////////////////////////

// --------------------------------------------------------------------------------------

// Create a new world of entities, components, and systems
SUS_WORLD_STRUCT SUSAPI susWorldSetup();
// Create a new world of entities, components, and systems
SUS_WORLD SUSAPI susNewWorld();
// Destroy the world
VOID SUSAPI susWorldCleanup(
	_In_ SUS_WORLD_STRUCT* world
);
// Destroy the world
VOID SUSAPI susWorldDestroy(
	_In_ SUS_WORLD world
);
// Update the state of the world
VOID SUSAPI susWorldUpdate(
	_In_ SUS_WORLD world,
	_In_ sus_float deltaTime
);

// --------------------------------------------------------------------------------------

// Register a new component
VOID SUSAPI susWorldRegisterComponentEx(
	_Inout_ SUS_WORLD world,
	_In_ sus_size_t componentSize,
	_In_opt_ SUS_COMPONENT_CONSTRUCTOR constructor,
	_In_opt_ SUS_COMPONENT_DESTRUCTOR destructor,
	_Out_ SUS_LPCOMPONENT_TYPE type
);
#define susWorldRegisterComponent(world, componentName, constructor, destructor) susWorldRegisterComponentEx(world, sizeof(SUS_COMPONENT componentName), constructor, destructor, &componentName##Type)
// Register an entity processing system
SUS_SYSTEM_ID SUSAPI susWorldRegisterEntitySystem(
	_Inout_ SUS_WORLD world,
	_In_ SUS_SYSTEM_ENTITY_CALLBACK callback,
	_In_ SUS_COMPONENTMASK mask,
	_In_opt_ SUS_OBJECT userData
);
// Register a free system
SUS_SYSTEM_ID SUSAPI susWorldRegisterFreeSystem(
	_Inout_ SUS_WORLD world,
	_In_ SUS_SYSTEM_FREE_CALLBACK callback,
	_In_opt_ SUS_OBJECT userData
);
// Get all entities with a mask
SUS_VECTOR SUSAPI susWorldGetEntitiesWith(
	_Inout_ SUS_WORLD world,
	_In_ SUS_COMPONENTMASK mask
);

// --------------------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////////////////
//									Entity Manager										//
//////////////////////////////////////////////////////////////////////////////////////////

// --------------------------------------------------------------------------------------

// Create a new entity
SUS_ENTITY SUSAPI susNewEntity(
	_Inout_ SUS_WORLD world,
	_In_ SUS_COMPONENTMASK initMask,
	_In_opt_ SUS_ENTITY parent
);
// Destroy the entity
VOID SUSAPI susEntityDestroy(
	_Inout_ SUS_WORLD world,
	_In_ SUS_ENTITY entity
);

// --------------------------------------------------------------------------------------

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

// --------------------------------------------------------------------------------------

// Check the entity for existence
SUS_INLINE BOOL SUSAPI susEntityExists(_Inout_ SUS_WORLD world, _In_ SUS_ENTITY entity) {
	SUS_ASSERT(world && world->entities && entity != SUS_INVALID_ENTITY);
	return (BOOL)((ULONGLONG)susMapGet(world->entities, &entity));
}
// Check the component for the existence of the component
SUS_INLINE BOOL SUSAPI susEntityHasComponent(_Inout_ SUS_WORLD world, _In_ SUS_ENTITY entity, _In_ SUS_COMPONENT_TYPE type) {
	SUS_ASSERT(world && susEntityExists(world, entity));
	SUS_ENTITY_LOCATION location = *(SUS_LPENTITY_LOCATION)susMapGet(world->entities, &entity);
	return susBitmask256Test(location.archetype->mask, type);
}
// Get a component
SUS_INLINE SUS_OBJECT SUSAPI susEntityGetComponent(_Inout_ SUS_WORLD world, _In_ SUS_ENTITY entity, _In_ SUS_COMPONENT_TYPE type) {
	SUS_ASSERT(world && susEntityExists(world, entity));
	SUS_ENTITY_LOCATION location = *(SUS_LPENTITY_LOCATION)susMapGet(world->entities, &entity);
	SUS_LPVECTOR pool = (SUS_LPVECTOR)susMapGet(location.archetype->componentPools, &type);
	return pool ? susVectorGet(*pool, location.index) : NULL;
}
// Get an entity mask
SUS_INLINE SUS_COMPONENTMASK SUSAPI susEntityGetMask(_Inout_ SUS_WORLD world, _In_ SUS_ENTITY entity) {
	SUS_ASSERT(world && susEntityExists(world, entity));
	SUS_LPENTITY_LOCATION location = (SUS_LPENTITY_LOCATION)susMapGet(world->entities, &entity);
	return location->archetype->mask;
}

// --------------------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////////////////
//								Hierarchy of entities									//
//////////////////////////////////////////////////////////////////////////////////////////

// --------------------------------------------------------------------------------------

VOID SUSAPI susEntitySetParent(
	_Inout_ SUS_WORLD world,
	_In_ SUS_ENTITY entity,
	_In_opt_ SUS_ENTITY parent
);
// Get a parent
SUS_ENTITY SUSAPI susEntityGetParent(
	_In_ SUS_WORLD world,
	_In_ SUS_ENTITY entity
);

// --------------------------------------------------------------------------------------

// Get kids
SUS_HASHSET SUSAPI susEntityGetChildren(
	_In_ SUS_WORLD world,
	_In_ SUS_ENTITY entity
);
// Check the affiliation
BOOL SUSAPI susEntityIsDescendantOf(
	_In_ SUS_WORLD world,
	_In_ SUS_ENTITY entity,
	_In_ SUS_ENTITY ancestor
);

// --------------------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////////////////
//								Systems Manager											//
//////////////////////////////////////////////////////////////////////////////////////////

// --------------------------------------------------------------------------------------

// Launch the system
VOID SUSAPI susSystemRun(
	_Inout_ SUS_WORLD world,
	_In_ SUS_SYSTEM_ID index,
	_In_ FLOAT deltaTime
);

// --------------------------------------------------------------------------------------

// Get a system
SUS_INLINE SUS_LPSYSTEM SUSAPI susWorldGetSystem(_In_ SUS_WORLD world, _In_ SUS_SYSTEM_ID index) {
	SUS_ASSERT(world);
	return (SUS_LPSYSTEM)susVectorGet(world->systems, index);
}
// Check the system for existence
SUS_INLINE BOOL SUSAPI susSystemExists(_In_ SUS_WORLD world, _In_ SUS_SYSTEM_ID index) {
	return (BOOL)((ULONGLONG)susWorldGetSystem(world, index));
}

// --------------------------------------------------------------------------------------

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

// -------------------------------------------------------------------

#endif /* !_SUS_ECS_ */