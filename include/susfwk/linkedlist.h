// linkedlist.h
//
#ifndef _SUS_LINKED_LIST_
#define _SUS_LINKED_LIST_

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

#pragma warning(push)
#pragma warning(disable: 4200)

// =======================================================================================

// -------------------------------------------------------------------

// A callback function for comparing elements
typedef BOOL(SUSAPI* SUS_LIST_ELEMENTS_COMPARE)(_In_ SUS_OBJECT a, _In_ SUS_OBJECT b, _In_ SIZE_T size);
// A node of two linked lists
typedef struct sus_list_node{
	struct sus_list_node*	next;	// The next node
	struct sus_list_node*	prev;	// Previous node
	sus_byte_t				value[];// Node Data
} SUS_LIST_NODE_STRUCT, *SUS_LIST_NODE;

// Two linked list
typedef struct sus_list {
	SUS_LIST_NODE	head;		// The first node of the list
	SUS_LIST_NODE	tail;		// The last node in the list
	sus_size32_t	valueSize;	// Default value size
	sus_uint_t		count;		// Number of list items
} SUS_LIST, *SUS_LPLIST;

// -------------------------------------------------------------------

// =======================================================================================

// -------------------------------------------------------------------

// Create a list structure
SUS_LIST SUSAPI susListSetupEx(
	_In_ sus_size32_t typeSize
);
// Create a list structure
#define susListSetup(type) susListSetupEx(sizeof(type))
// Clear the list
VOID SUSAPI susListCleanup(
	_Inout_ SUS_LPLIST list
);

// -------------------------------------------------------------------

// Insert a node in the list
SUS_LIST_NODE SUSAPI susListInsert(
	_Inout_ SUS_LPLIST list,
	_In_opt_ SUS_LIST_NODE before,
	_In_opt_ SUS_LPMEMORY value
);
// Remove a node from the list
VOID SUSAPI susListErase(
	_Inout_ SUS_LPLIST list,
	_In_ SUS_LIST_NODE node
);
// Move the node by the specified number
VOID SUSAPI susListMove(
	_Inout_ SUS_LPLIST list,
	_In_ SUS_LIST_NODE node,
	_In_ INT move
);

// -------------------------------------------------------------------

// Insert a node at the end of the list
SUS_INLINE SUS_LIST_NODE SUSAPI susListPush(_Inout_ SUS_LPLIST list, _In_opt_ SUS_LPMEMORY value) {
	SUS_ASSERT(list);
	return susListInsert(list, NULL, value);
}
// Pop the last node from the list
SUS_INLINE VOID SUSAPI susListPop(_Inout_ SUS_LPLIST list) {
	SUS_ASSERT(list && list->tail);
	susListErase(list, list->tail);
}
// Prepend a node to the list
SUS_INLINE SUS_LIST_NODE SUSAPI susListUnshift(_Inout_ SUS_LPLIST list, _In_opt_ SUS_LPMEMORY value) {
	SUS_ASSERT(list);
	return susListInsert(list, list->head, value);
}
// Remove the first node from the list
SUS_INLINE VOID SUSAPI susListShift(_Inout_ SUS_LPLIST list) {
	SUS_ASSERT(list && list->head);
	susListErase(list, list->head);
}

// -------------------------------------------------------------------

// Search for an item from the list based on data
SUS_LIST_NODE SUSAPI susListFind(
	_In_ SUS_LIST list,
	_In_ SUS_LPMEMORY value,
	_In_opt_ SUS_LIST_ELEMENTS_COMPARE searcher
);
// Searching for an element in a list based on data from the end
SUS_LIST_NODE SUSAPI susListFindLast(
	_In_ SUS_LIST list,
	_In_ SUS_LPMEMORY value,
	_In_opt_ SUS_LIST_ELEMENTS_COMPARE searcher
);
// Check the node for presence in the list
BOOL SUSAPI susListContains(
	_In_ SUS_LIST list,
	_In_opt_ SUS_LIST_NODE node
);

// -------------------------------------------------------------------

// Change the type size for new values
SUS_INLINE VOID SUSAPI susListSetValueSize(_Inout_ SUS_LPLIST list, _In_ sus_size32_t	newValueSize) {
	SUS_ASSERT(list);
	list->valueSize = newValueSize;
}
// Go through all the items in the list
#define susListForeach(node, list) for (SUS_LIST_NODE node = (list).head; node; node = node->next)
// Go through all the elements of the list
#define susListForeachReverse(i, vec) for (SUS_LIST_NODE node = (list).tail; node; node = node->prev)

#ifdef _DEBUG
SUS_INLINE VOID SUSAPI susListPrint(_In_ SUS_LIST list) {
	SUS_PRINTDL("List output {");
	susListForeach(node, list) {
		SUS_PRINTDL("'%s'", node->value);
	}
	SUS_PRINTDL("}");
}
#else
#define susListPrint(vec)
#endif // !_DEBUG

// -------------------------------------------------------------------

// =======================================================================================

// -------------------------------------------------------------------

// Insert a node between the nodes
SUS_LIST_NODE SUSAPI susListNodeInsert(
	_Inout_opt_ SUS_LIST_NODE parent,
	_Inout_opt_ SUS_LIST_NODE next,
	_In_opt_ SUS_LPMEMORY value,
	_In_ SIZE_T size
);
// Delete a node
SUS_LIST_NODE SUSAPI susListNodeErase(
	_Inout_ SUS_LIST_NODE node
);

// -------------------------------------------------------------------

// Insert a node at the end of the node order
SUS_INLINE SUS_LIST_NODE SUSAPI susListNodePush(_Inout_ SUS_LIST_NODE parent, _In_opt_ SUS_LPMEMORY value, _In_ SIZE_T size) {
	return parent && parent->next ? susListNodePush(parent->next, value, size) : susListNodeInsert(parent, NULL, value, size);
}
// Remove a node from the end of the node order
SUS_INLINE SUS_LIST_NODE SUSAPI susListNodePop(_Inout_ SUS_LIST_NODE parent) {
	SUS_ASSERT(parent);
	return parent->next ? susListNodePop(parent->next) : susListNodeErase(parent);
}

// -------------------------------------------------------------------

// =======================================================================================

#ifdef __cplusplus
}
#endif // !__cplusplus

#pragma warning(pop)

#endif /* !_SUS_LINKED_LIST_ */
