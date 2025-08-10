// ecs.h
//
#ifndef _SUS_ECS_
#define _SUS_ECS_

typedef sus_u32 SUS_ENTITY;
typedef SUS_BITMASK64 SUS_COMPONENTMASK;
typedef sus_u32 SUS_COMPONENT_TYPE;
typedef VOID(SUSAPI* SUS_SYSTEMS_CALLBACK)(SUS_ENTITY entity);

typedef struct sus_world {
	SUS_HASHMAP	entities;		// SUS_ENTITY -> SUS_COMPONENTMASK
	SUS_HASHMAP	componentPools[64];
	SUS_ENTITY	nextId;
} SUS_WORLD, *SUS_PWORLD, *SUS_LPWORLD;

// Create a new world of entities, components, and systems
SUS_LPWORLD SUSAPI susNewWorld();

#endif /* !_SUS_ECS_ */
