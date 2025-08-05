// ecs.h
//
#ifndef _SUS_ECS_
#define _SUS_ECS_

typedef sus_u32 ECS_ENTITY;
typedef sus_u32 ECS_COMPONENT_TYPE;

typedef struct ecs_component {
	ECS_ENTITY entity;
	BYTE data[];
} ECS_COMPONENT, *ECS_LPCOMPONENT;

typedef struct ecs_component_pool {
	SUS_ARRAY components;
} ECS_COMPONENT_POOL, *ECS_LPCOMPONENT_POOL;

typedef struct ecs_world {
	SUS_ARRAY componentTypes; // ECS_COMPONENT_POOL
	ECS_ENTITY next;
} ECS_WORLD, *ECS_PWORLD, *ECS_LPWORLD;

#endif /* !_SUS_ECS_ */
