// linkedlist.h
//
#ifndef _SUS_LINKED_LIST_
#define _SUS_LINKED_LIST_

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

#pragma warning(push)
#pragma warning(disable: 4200)

typedef struct sus_node{
	struct sus_node*	next;
	struct sus_node*	prev;
	SIZE_T				size;
	BYTE				data[];
} SUS_NODE_STRUCT, *SUS_NODE, **SUS_LPNODE;

typedef struct sus_list {
	SUS_NODE	head;
	SUS_NODE	tail;
} SUS_LIST, *SUS_PLIST, *SUS_LPLIST;

// Create a new element of connectedness
SUS_NODE SUSAPI susNewNode(_In_ SIZE_T size);

// Add an item to the top of the list
SUS_NODE SUSAPI susListPushFront(_Inout_ SUS_LPLIST list, _In_ SIZE_T size);
// Add an item after the previous one
SUS_NODE SUSAPI susListPushAfter(_Inout_ SUS_LPLIST list, _Inout_ SUS_NODE prev, _In_ SIZE_T size);
// Add an item to the end of the list
SUS_NODE SUSAPI susListPushBack(_Inout_ SUS_LPLIST list, _In_ SIZE_T size);

// Delete a node from the list
VOID SUSAPI susListErase(_Inout_ SUS_LPLIST list, _Inout_ SUS_NODE node);
// Delete the first item in the list
SUS_INLINE VOID SUSAPI susListPopFront(_Inout_ SUS_LPLIST list) {
	SUS_ASSERT(list);
	susListErase(list, list->head);
}
// Delete the last item in the list
SUS_INLINE VOID SUSAPI susListPopBack(_Inout_ SUS_LPLIST list) {
	SUS_ASSERT(list);
	susListErase(list, list->tail);
}

// Create a new list
SUS_INLINE SUS_LIST SUSAPI susNewList() { return (SUS_LIST) { 0 }; }
// Destroy the list
SUS_INLINE VOID SUSAPI susListDestroy(_Inout_ SUS_LIST list) { while (list.head) susListPopFront(&list); }
// Destroy the element of connectedness
SUS_INLINE VOID SUSAPI susNodeDestroy(_In_ SUS_NODE node) { sus_free(node); }

#define susListForeach(i, list) for (SUS_NODE i = list.head; i; i = i->next)

// Get the number of items in the list
SUS_INLINE DWORD SUSAPI susListCount(_In_ SUS_LIST list) {
	DWORD count = 0;
	susListForeach(node, list) count++;
	return count;
}


// ---------------------------- Tests debug ----------------------------

#ifdef _DEBUG
SUS_INLINE VOID SUSAPI susListPrint(_In_ SUS_LIST list) {
	SUS_PRINTDL("List output {");
	for (SUS_NODE node = list.head; node; node = node->next) {
		SUS_PRINTDL("\t[0x%.13p] 0x%.13p <- Node (%d bytes) -> 0x%.13p", node, node->prev, node->size, node->next);
	}
	SUS_PRINTDL("}");
}
#else
#define susListPrint(list)
#endif // !_DEBUG

#ifdef __cplusplus
}
#endif // !__cplusplus

#pragma warning(pop)

#endif /* !_SUS_LINKED_LIST_ */