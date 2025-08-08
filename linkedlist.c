// linkedlist.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/memory.h"
#include "include/susfwk/linkedlist.h"

// Create a new element of connectedness
SUS_NODE SUSAPI susNewNode(_In_ SIZE_T size)
{
	SUS_NODE node = sus_malloc(sizeof(SUS_NODE_STRUCT) + size);
	if (!node) return NULL;
	node->size = size;
	node->next = NULL;
	node->prev = NULL;
	return node;
}

// Add an item to the top of the list
SUS_NODE SUSAPI susListPushFront(_Inout_ SUS_LPLIST list, _In_ SIZE_T size)
{
	SUS_ASSERT(list);
	SUS_NODE node = susNewNode(size);
	if (!node) return NULL;
	node->next = list->head;
	if (list->head) list->head->prev = node;
	else list->tail = node;
	list->head = node;
	return node;
}
// Add an item after the previous one
SUS_NODE SUSAPI susListPushAfter(_Inout_ SUS_LPLIST list, _Inout_ SUS_NODE prev, _In_ SIZE_T size)
{
	SUS_ASSERT(list);
	SUS_NODE node = susNewNode(size);
	if (!node) return NULL;
	node->next = prev->next;
	node->prev = prev;
	prev->next = node;
	if (prev == list->tail) list->tail = node;
	return node;
}
// Add an item to the end of the list
SUS_NODE SUSAPI susListPushBack(_Inout_ SUS_LPLIST list, SIZE_T _In_ size)
{
	SUS_ASSERT(list);
	SUS_NODE node = susNewNode(size);
	if (!node) return NULL;
	node->prev = list->tail;
	if (list->tail) list->tail->next = node;
	else list->head = node;
	list->tail = node;
	return node;
}
// Delete a node from the list
VOID SUSAPI susListErase(_Inout_ SUS_LPLIST list, _Inout_ SUS_NODE node)
{
	SUS_ASSERT(list);
	if (!node) return;
	if (node->prev)
		node->prev->next = node->next;
	else list->head = node->next;
	if (node->next)
		node->next->prev = node->prev;
	else list->tail = node->prev;
	susNodeDestroy(node);
}

