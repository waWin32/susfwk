// ecs.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/memory.h"
#include "include/susfwk/vector.h"
#include "include/susfwk/hashmap.h"
#include "include/susfwk/ecs.h"

// Create a new world of entities, components, and systems
SUS_LPWORLD SUSAPI susNewWorld()
{
	SUS_LPWORLD world = sus_malloc(sizeof(SUS_WORLD));
	if (!world) return NULL;
	world->entities = susNewMap();
	world->nextId = 0;
	return world;
}
//
SUS_ENTITY susNewEntity(_Inout_ SUS_LPWORLD world) {
	SUS_ENTITY entity = world->nextId++;
	susMapAdd(&world->entities, &entity, sizeof(SUS_ENTITY), NULL, sizeof(SUS_COMPONENTMASK));
	return entity;
}
//
VOID susEntityDestroy(_Inout_ SUS_LPWORLD world, _In_ SUS_ENTITY entity) {
	susVecForeach(i, count, world->entities) {
		susVectorErase(&world->entities, i);
	}
}
