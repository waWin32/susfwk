// linkedlist.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/memory.h"
#include "include/susfwk/linkedlist.h"

// =======================================================================================

// -------------------------------------------------------------------

// Insert a node between the nodes
SUS_LIST_NODE SUSAPI susListNodeInsert(_Inout_opt_ SUS_LIST_NODE parent, _Inout_opt_ SUS_LIST_NODE next, _In_opt_ SUS_LPMEMORY value, _In_ SIZE_T size)
{
	SUS_LIST_NODE node = sus_calloc(1, sizeof(SUS_LIST_NODE_STRUCT) + size);
	if (!node) return NULL;
	if (parent) parent->next = node;
	node->prev = parent;
	if (next) next->prev = node;
	node->next = next;
	if (value) sus_memcpy(node->value, value, size);
	return node;
}
// Delete a node
SUS_LIST_NODE SUSAPI susListNodeErase(_Inout_ SUS_LIST_NODE node)
{
	SUS_ASSERT(node);
	if (node->prev) node->prev->next = node->next;
	if (node->next) node->next->prev = node->prev;
	SUS_LIST_NODE parent = node->prev ? node->prev : node->next;
	sus_free(node);
	return parent;
}

// -------------------------------------------------------------------

// =======================================================================================

// -------------------------------------------------------------------

// Create a list structure
SUS_LIST SUSAPI susListSetupEx(_In_ sus_size32_t typeSize)
{
	SUS_PRINTDL("Creating a list structure");
	SUS_LIST list = { 0 };
	list.valueSize = typeSize;
	return list;
}
// Clear the list
VOID SUSAPI susListCleanup(_Inout_ SUS_LPLIST list)
{
	SUS_PRINTDL("Clearing the list");
	SUS_ASSERT(list);
	while (list->head) {
		susListErase(list, list->head);
	}
}

// -------------------------------------------------------------------

// Insert a node in the list
SUS_LIST_NODE SUSAPI susListInsert(_Inout_ SUS_LPLIST list, _In_opt_ SUS_LIST_NODE before, _In_opt_ SUS_LPMEMORY value)
{
	SUS_ASSERT(list);
	SUS_LIST_NODE node = susListNodeInsert(before ? before->prev : list->tail, before, value, list->valueSize);
	if (!node) return NULL;
	if (!node->prev) list->head = node;
	if (!node->next) list->tail = node;
	list->count++;
	return node;
}
// Remove a node from the list
VOID SUSAPI susListErase(_Inout_ SUS_LPLIST list, _In_ SUS_LIST_NODE node)
{
	SUS_ASSERT(list && node);
	node = susListNodeErase(node);
	if (node) {
		if (!node->prev) list->head = node;
		if (!node->next) list->tail = node;
	}
	list->count--;
}
// Move the node by the specified number
VOID SUSAPI susListMove(_Inout_ SUS_LPLIST list, _In_ SUS_LIST_NODE node, _In_ INT move)
{
	SUS_ASSERT(list && node);
	if (list->count <= 1) return;
	if (node->prev) node->prev->next = node->next;
	if (node->next) node->next->prev = node->prev;
	if (list->head == node) list->head = node->next;
	if (list->tail == node) list->tail = node->prev;
	SUS_LIST_NODE next = node;
	SUS_LIST_NODE prev = node;
	move = move % (INT)(list->count - 1);
	if (move > 0) {
		next = (node->next) ? node->next : list->head;
		move--;
		for (int i = 1; i < move && next && next->next; i++) {
			next = next->next;
		}
		prev = (next) ? next->next : NULL;
	}
	else {
		prev = (node->prev) ? node->prev : list->tail;
		move--;
		for (int i = 1; i < move && prev && prev->prev; i++) {
			prev = prev->prev;
		}
		next = (prev) ? prev->prev : NULL;
	}
	node->prev = next;
	node->next = prev;
	if (next) next->next = node;
	else list->head = node;
	if (prev) prev->prev = node;
	else list->tail = node;
}

// The default list element comparison function
static BOOL SUSAPI susDefListSearcher(_In_ SUS_OBJECT v1, _In_ SUS_OBJECT v2, _In_ SIZE_T size) {
	return sus_memcmp(v1, v2, size);
}
// Search for an item from the list based on data
SUS_LIST_NODE SUSAPI susListFind(_In_ SUS_LIST list, _In_ SUS_LPMEMORY value, _In_opt_ SUS_LIST_ELEMENTS_COMPARE searcher)
{
	SUS_ASSERT(value);
	searcher = searcher ? searcher : susDefListSearcher;
	susListForeach(node, list) {
		if (searcher(node->value, value, list.valueSize)) return node;
	}
	return NULL;
}
// Searching for an element in a list based on data from the end
SUS_LIST_NODE SUSAPI susListFindLast(_In_ SUS_LIST list, _In_ SUS_LPMEMORY value, _In_opt_ SUS_LIST_ELEMENTS_COMPARE searcher)
{
	SUS_ASSERT(value);
	searcher = searcher ? searcher : susDefListSearcher;
	susListForeachReverse(node, list) {
		if (searcher(node->value, value, list.valueSize)) return node;
	}
	return NULL;
}
// Check the node for presence in the list
BOOL SUSAPI susListContains(_In_ SUS_LIST list, _In_opt_ SUS_LIST_NODE node)
{
	if (!list.head) return FALSE;
	for (; node; node = node->prev) {
		if (list.head == node) return TRUE;
	}
	return FALSE;
}
// -------------------------------------------------------------------

// =======================================================================================

