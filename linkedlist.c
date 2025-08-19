// linkedlist.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/memory.h"
#include "include/susfwk/linkedlist.h"

// -------------------------------------------------------------------

// Create a new element of connectedness
SUS_NODE SUSAPI susNewNode(_Inout_ SUS_LPLIST list, _In_opt_ LPBYTE init)
{
	SUS_NODE node = sus_malloc(sizeof(SUS_NODE_STRUCT) + list->size);
	if (!node) return NULL;
	node->next = NULL;
	node->prev = NULL;
	if (init) sus_memcpy(node->data, init, list->size);
	else sus_zeromem(node->data, list->size);
	list->count++;
	return node;
}

// -------------------------------------------------------------------

// Add an item to the top of the list
SUS_NODE SUSAPI susListPushFront(_Inout_ SUS_LPLIST list, _In_opt_ LPBYTE init)
{
	SUS_ASSERT(list);
	SUS_NODE node = susNewNode(list, init);
	if (!node) return NULL;
	node->next = list->head;
	if (list->head) list->head->prev = node;
	else list->tail = node;
	list->head = node;
	return node;
}
// Add an item after the previous one
SUS_NODE SUSAPI susListPushAfter(_Inout_ SUS_LPLIST list, _Inout_ SUS_NODE prev, _In_opt_ LPBYTE init)
{
	SUS_ASSERT(list && prev);
	SUS_NODE node = susNewNode(list, init);
	if (!node) return NULL;
	node->next = prev->next;
	node->prev = prev;
	prev->next = node;
	if (prev == list->tail) list->tail = node;
	return node;
}
// Add an item to the end of the list
SUS_NODE SUSAPI susListPushBack(_Inout_ SUS_LPLIST list, _In_opt_ LPBYTE init)
{
	SUS_ASSERT(list);
	SUS_NODE node = susNewNode(list, init);
	if (!node) return NULL;
	node->prev = list->tail;
	if (list->tail) list->tail->next = node;
	else list->head = node;
	list->tail = node;
	return node;
}

// -------------------------------------------------------------------

// Delete a node from the list
VOID SUSAPI susListErase(_Inout_ SUS_LPLIST list, _Inout_ SUS_NODE node)
{
	SUS_ASSERT(list && node);
	if (!node) return;
	if (node->prev)
		node->prev->next = node->next;
	else list->head = node->next;
	if (node->next)
		node->next->prev = node->prev;
	else list->tail = node->prev;
	susNodeDestroy(node);
}

// -------------------------------------------------------------------

