#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <assert.h>

#include "list.h"

/**
 *  Represents the implmentation class for list.h.
 * 
 */
static List lists[LIST_MAX_NUM_HEADS];
static Node nodes[LIST_MAX_NUM_NODES];

static bool g_isInitialized = false;
static int g_numListHeads = 0;
static int g_numListNodes = 0;

static List* freeLists; // List pointer to track free lists
static Node* freeNodes; // Node pointer to track free nodes

// Setup lists and set fields
static void setupLists() {
    lists[0].first = NULL;
    lists[0].current = NULL;
    lists[0].last = NULL;
    lists[0].nextList = &lists[1];
    lists[0].size = 0;
    lists[0].currentPosition = 0;
    lists[0].lastList = &lists[LIST_MAX_NUM_HEADS - 1];

    for (int i = 1; i < LIST_MAX_NUM_HEADS - 1; i++) {
        lists[i].first = NULL;
        lists[i].current = NULL;
        lists[i].last = NULL;
        lists[i].nextList = &lists[i+1];
        lists[i].size = 0; 
        lists[i].currentPosition = 0;
        lists[i].lastList = &lists[LIST_MAX_NUM_HEADS - 1];
    }

    lists[LIST_MAX_NUM_HEADS - 1].first = NULL;
    lists[LIST_MAX_NUM_HEADS - 1].current = NULL;
    lists[LIST_MAX_NUM_HEADS - 1].last = NULL;
    lists[LIST_MAX_NUM_HEADS - 1].nextList = NULL;
    lists[LIST_MAX_NUM_HEADS - 1].size = 0; 
    lists[LIST_MAX_NUM_HEADS - 1].currentPosition = 0;
    lists[LIST_MAX_NUM_HEADS - 1].lastList = &lists[LIST_MAX_NUM_HEADS - 1];

    freeLists = lists;
}

// Setup nodes and set fields
static void setupNodes() {
    nodes[0].next = &nodes[1];
    nodes[0].previous = NULL;
    nodes[0].item = NULL;
    nodes[0].lastNode = &nodes[LIST_MAX_NUM_NODES - 1];

    for (int i = 1; i < LIST_MAX_NUM_NODES - 1; i++) {
        nodes[i].next = &nodes[i+1];
        nodes[i].previous = &nodes[i-1];
        nodes[i].item = NULL;
        nodes[i].lastNode = &nodes[LIST_MAX_NUM_NODES - 1];
    }

    nodes[LIST_MAX_NUM_NODES - 1].next = NULL;
    nodes[LIST_MAX_NUM_NODES - 1].previous = &nodes[LIST_MAX_NUM_NODES - 2];
    nodes[LIST_MAX_NUM_NODES - 1].item = NULL;
    nodes[LIST_MAX_NUM_NODES - 1].lastNode = &nodes[LIST_MAX_NUM_NODES - 1];

    freeNodes = nodes;
}

// Makes a new, empty list, and returns its reference on success. 
// Returns a NULL pointer on failure.
List* List_create() {
    if (!g_isInitialized) {
        setupLists();
        setupNodes();
        g_isInitialized = true;
    }

    if (freeLists == NULL) {
        return NULL;
    }

    if (g_numListHeads < LIST_MAX_NUM_HEADS) {
        List* newList = freeLists;
        freeLists = freeLists->nextList;
        
        // Ensure freeLists->lastList correctly points to the same lastList as the newList
        if(freeLists != NULL) {
            freeLists->lastList = newList->lastList;
            freeLists->lastList->nextList = newList->lastList->nextList;
        }

        newList->isFree = false;
        g_numListHeads++;

        return newList;
    }
    
    return NULL;
}

// Returns the number of items in pList.
int List_count(List* pList) {
    assert(pList != NULL);

    return pList->size;
}

// Returns a pointer to the first item in pList and makes the first item the current item.
// Returns NULL and sets current item to NULL if list is empty.
void* List_first(List* pList) {
    assert (pList != NULL);

    if (pList->size == 0) {
        return NULL;
    }

    pList->current = pList->first;
    pList->currentPosition = 0;

    return pList->first->item;
}

// Returns a pointer to the last item in pList and makes the last item the current item.
// Returns NULL and sets current item to NULL if list is empty.
void* List_last(List* pList) {
    assert (pList != NULL);

    if (pList->size == 0) {
        return NULL;
    }

    pList->current = pList->last; 
    pList->currentPosition = (pList->size) - 1;
  
    return pList->last->item;
}

// Advances pList's current item by one, and returns a pointer to the new current item.
// If this operation advances the current item beyond the end of the pList, a NULL pointer 
// is returned and the current item is set to be beyond end of pList.
void* List_next(List* pList) {
    assert (pList != NULL);

    if(pList->size == 0) {
        return NULL;
    }

    if(pList->currentPosition < 0) { // Before the start of the pList
        pList->current = pList->first;
        pList->currentPosition = 0;

        return pList->current->item;
    }

    if(pList->currentPosition <= (pList->size) - 1) {
        pList->current = pList->current->next;
        (pList->currentPosition)++;
    }

    if(pList->currentPosition > (pList->size) - 1) { // Beyond the end of the pList
        return NULL;
    }

    return pList->current->item;
}

// Backs up pList's current item by one, and returns a pointer to the new current item. 
// If this operation backs up the current item beyond the start of the pList, a NULL pointer 
// is returned and the current item is set to be before the start of pList.
void* List_prev(List* pList) {
    assert (pList != NULL);

    if(pList->size == 0) {
        return NULL;
    }

    if(pList->currentPosition > (pList->size) - 1) {
        pList->current = pList->last;
        pList->currentPosition = (pList->size) - 1;

        return pList->current->item;
    }

    if (pList->currentPosition >= 0) {
        pList->current = pList->current->previous;
        (pList->currentPosition)--;
    }

    if(pList->currentPosition < 0) {
        return NULL;
    }

    return pList->current->item;
}

// Returns a pointer to the current item in pList.
void* List_curr(List* pList) {
    assert (pList != NULL);

    if (pList->size == 0) {
        return NULL;
    }

    if (pList->currentPosition < 0 
    || pList->currentPosition > (pList->size) - 1) {
        return NULL;
    }

    return pList->current->item;
}

// Adds the new item to pList directly after the current item, and makes item the current item. 
// If the current pointer is before the start of the pList, the item is added at the start. If 
// the current pointer is beyond the end of the pList, the item is added at the end. 
// Returns 0 on success, -1 on failure.
int List_add(List* pList, void* pItem) {
    assert (pList!= NULL);

    if (freeNodes == NULL) {
        return -1;
    }

    // Ensure next free Node points to the last free Node
    if(freeNodes->next != NULL) {
        if(freeNodes->next->lastNode != freeNodes->lastNode) {
            freeNodes->next->lastNode = freeNodes->lastNode;
        }

        if(freeNodes->next->lastNode->next != freeNodes->lastNode->next) {
            freeNodes->next->lastNode->next = freeNodes->lastNode->next;
        }
    }

    if (pList->size == 0) {
        Node* newNode = freeNodes;
        freeNodes = freeNodes->next;
        freeNodes-> previous = NULL;

        newNode->item = pItem;
        newNode->next = NULL;
        newNode->previous = NULL;

        pList->first = newNode;
        pList->last = newNode;
        pList->current = newNode;
        pList->currentPosition = 0;
        (pList->size)++;

        g_numListNodes++;

        return 0;
    } 
    
    if (pList-> currentPosition < 0) {
        Node* newNode = freeNodes;
        freeNodes = freeNodes->next;

        newNode->previous = NULL;
        newNode->next = pList->first;
        newNode->item = pItem;

        pList->first->previous = newNode; // Instead of NULL
        pList->first = newNode;
        pList->current = newNode;

        (pList->size)++;
        pList->currentPosition = 0;
        g_numListNodes++;

        return 0;
    }

    if (pList->currentPosition > ((pList->size) - 1)) {
        Node* newNode = freeNodes;
        freeNodes = freeNodes->next;

        newNode->previous = pList->last;
        newNode->next = NULL;
        newNode->item = pItem;

        pList->last->next = newNode; // Instead of NULL
        pList->last = newNode;
        pList->current = newNode;

        (pList->size)++;
        pList->currentPosition =  (pList->size) - 1;
        g_numListNodes++;

        return 0;
    }

    Node* newNode = freeNodes;
    freeNodes = freeNodes->next;

    if (freeNodes != NULL) {
        freeNodes->previous = NULL;
    }

    newNode->previous = pList->current; 
    
    newNode->next = pList->current->next; 
    newNode->item = pItem;

    if (pList->current->next != NULL) {
        pList->current->next->previous = newNode;
    }

    pList->current->next = newNode;
    pList->current = newNode;

    if ((pList->currentPosition == (pList->size) - 1)) {
        pList->last = newNode;
    }

    (pList->size)++;
    (pList->currentPosition)++;

    g_numListNodes++;

    return 0;
}

// Adds item to pList directly before the current item, and makes the new item the current one. 
// If the current pointer is before the start of the pList, the item is added at the start. 
// If the current pointer is beyond the end of the pList, the item is added at the end. 
// Returns 0 on success, -1 on failure.
int List_insert(List* pList, void* pItem) {
    assert (pList!= NULL);

    if (freeNodes == NULL) {
        return -1;
    }

    List_prev(pList);

    return List_add(pList, pItem);
}

// Adds item to the end of pList, and makes the new item the current one. 
// Returns 0 on success, -1 on failure.
int List_append(List* pList, void* pItem) {
    assert (pList!= NULL);

    if (freeNodes == NULL) {
        return -1;
    }

    List_last(pList);
    List_next(pList);

    return List_add(pList, pItem);
}

// // Adds item to the front of pList, and makes the new item the current one. 
// // Returns 0 on success, -1 on failure.
int List_prepend(List* pList, void* pItem) {
    assert (pList!= NULL);

    if (freeNodes == NULL) {
        return -1;
    }

    List_first(pList);
    List_prev(pList);

    return List_add(pList, pItem);
}

// Return current item and take it out of pList. Make the next item the current one.
// If the current pointer is before the start of the pList, or beyond the end of the pList,
// then do not change the pList and return NULL.
void* List_remove(List* pList) {
    assert(pList != NULL);

    if (pList->currentPosition < 0
    || pList->currentPosition > (pList->size) - 1
    || pList->size == 0) {
        return NULL;
    }

    // If the current position is at the end of the list
    if (pList->currentPosition == (pList->size) - 1) {
        void* item  = pList->current->item;
        Node* removedNode = pList->current;

        if (pList->size == 1) {
            pList->first = NULL;
            pList->current = NULL;
            pList->last = NULL;
            pList->currentPosition = 0;

            // Add removed node to free pool
            if (freeNodes == NULL) {
                freeNodes = removedNode;
                freeNodes->lastNode = removedNode;
            } else {
                freeNodes->lastNode->next = removedNode;
                freeNodes->lastNode = removedNode;
            }

            (pList->size)--;
            g_numListNodes--;

            return item;
        }
        
        pList->last = pList->last->previous;
        pList->last->next = NULL;

        removedNode->next = NULL;
        removedNode->previous = NULL;

        pList->current = pList->last->next; //NULL
        (pList->size)--;
        pList->currentPosition = pList->size;

        // Add removed node to free pool
        if (freeNodes == NULL) {
            freeNodes = removedNode;
            freeNodes->lastNode = removedNode;
        } else {
            freeNodes->lastNode->next = removedNode;
            freeNodes->lastNode = removedNode;
        }

        g_numListNodes--;
        
        return item;
    }

    void* item  = pList->current->item;
    Node* removedNode = pList->current;
    
    // Rearranging pointers
    if (pList->currentPosition == 0) {
        pList->current = pList->current->next;
        pList->current->previous = NULL;
        pList->first = pList->current;
    } else {
        pList->current->previous->next = pList->current->next;
        pList->current->next->previous = pList->current->previous;
        pList->current = pList->current->next;
    }

    removedNode->next = NULL;    
    removedNode->previous = NULL;

    // Add removed node to free pool
    if (freeNodes == NULL) {
        freeNodes = removedNode;
        freeNodes->lastNode = removedNode;
    } else {
        freeNodes->lastNode->next = removedNode;
        freeNodes->lastNode = removedNode;
    }

    (pList->size)--;

    if (pList->currentPosition == (pList-> size - 1)) {
        pList->last = pList->current;
    }

    g_numListNodes--;
    
    return item;
}

// // Adds pList2 to the end of pList1. The current pointer is set to the current pointer of pList1. 
// // pList2 no longer exists after the operation; its head is available
// // for future operations.
void List_concat(List* pList1, List* pList2) {
    assert(pList1 != pList2); 

    if(freeLists != NULL) {
        assert(freeLists->lastList != pList2); // Cannot concat an already freed list
    } else {
        assert(freeLists != pList2); // Cannot concat an already freed list
    }

    if(pList1->size != 0 && pList2->size == 0) {
        pList2->first = NULL;
        pList2->last = NULL;
        pList2->currentPosition = 0;
        pList2->current = NULL;
        pList2->size = 0;
        pList2->nextList = NULL;
        pList2->isFree = true;

        if (freeLists == NULL) {
            freeLists = pList2;
            freeLists->lastList = pList2;
        } else {
            assert(freeLists->lastList->nextList == NULL);

            // Ensure next free Lists correctly points to the same lastList 
            if(freeLists->nextList != NULL) {
                if(freeLists->nextList->lastList != freeLists->lastList) {
                    freeLists->nextList->lastList = freeLists->lastList;
                }

                if(freeLists->nextList->lastList->nextList != freeLists->lastList->nextList) {
                    freeLists->nextList->lastList->nextList = freeLists->lastList->nextList;
                }
            }

            // Add removed node to free pool
            freeLists->lastList->nextList = pList2;
            freeLists->lastList = pList2;

            // Set next free list's last list as pList2
            if(freeLists->nextList != NULL) {
                freeLists->nextList->lastList->nextList = pList2;
                freeLists->nextList->lastList = pList2;
            }
        }

        g_numListHeads--;
        return;
    }

    // Concat empty pList1 with non-empty pList2
    if (pList1->size == 0 && pList2->size != 0) {
        pList1->first = pList2->first;
    } 

    // Link pList1 and pList2
    if (pList1->size != 0 && pList2->size != 0) {
        pList1->last->next = pList2->first;
        pList2->first->previous = pList1->last;
    }

    pList1->last = pList2->last;

    // Current position
    if (pList1->size == 0 && pList2->size != 0) {
        if (pList1->currentPosition > 0) {
            pList1->currentPosition = (pList1->size) + (pList2->size);
            pList1->current = NULL;
        } else {
            pList1->currentPosition = 0;
            pList1->current = pList2->first;
         }
    } else if(pList1->currentPosition > (pList1->size) - 1) {
        if (pList1->size != 0) {
            pList1->currentPosition = (pList1->size) + (pList2->size);
            pList1->current = NULL;
        }
    }

    // Size
    pList1->size = pList1->size + pList2->size;

    pList2->first = NULL;
    pList2->last = NULL;
    pList2->currentPosition = 0;
    pList2->current = NULL;
    pList2->size = 0;
    pList2->isFree = true;
    
    pList2->nextList = NULL;

    if (freeLists == NULL) {
        freeLists = pList2;
        freeLists->lastList = pList2;
    } else {
        assert(freeLists->lastList->nextList == NULL);

        // Ensure next free Lists correctly points to the same lastList 
        if(freeLists->nextList != NULL) {
            if(freeLists->nextList->lastList != freeLists->lastList) {
                freeLists->nextList->lastList = freeLists->lastList;
            }

            if(freeLists->nextList->lastList->nextList != freeLists->lastList->nextList) {
                freeLists->nextList->lastList->nextList = freeLists->lastList->nextList;
            }
        }

        // Add removed node to free pool
        freeLists->lastList->nextList = pList2;
        freeLists->lastList = pList2;

        // Set next free list's last list as pList2
        if(freeLists->nextList != NULL) {
            freeLists->nextList->lastList->nextList = pList2;
            freeLists->nextList->lastList = pList2;
        }
    }

    g_numListHeads--;
}

// Delete pList. pItemFreeFn is a pointer to a routine that frees an item. 
// It should be invoked (within List_free) as: (*pItemFreeFn)(itemToBeFreedFromNode);
// pList and all its nodes no longer exists after the operation; its head and nodes are 
// available for future operations.
// UPDATED: Changed function pointer type, May 19
typedef void (*FREE_FN)(void* pItem);
void List_free(List* pList, FREE_FN pItemFreeFn) {
    assert(pList != NULL);

    if(pList->isFree == true) {
        return;
    }

    if(freeLists != NULL) {
        assert(freeLists->lastList != pList); // Cannot free an already freed list
    } else {
        assert(freeLists != pList); // Cannot free an already freed list
    }

    List_first(pList);
    int size = pList->size;

    for (int i = 0; i < size; i++) {
        pItemFreeFn(List_remove(pList));
    }
    
    pList->first = NULL;
    pList->last = NULL;
    pList->currentPosition = 0;
    pList->current = NULL;
    pList->size = 0;

    pList->isFree = true;
    pList->nextList = NULL;

    if (freeLists == NULL) {
        freeLists = pList;
        freeLists->lastList = pList;
    } else {
        assert(freeLists->lastList->nextList == NULL);

        // Ensure next free Lists correctly points to the same lastList 
        if(freeLists->nextList != NULL) {
            if(freeLists->nextList->lastList != freeLists->lastList) {
                freeLists->nextList->lastList = freeLists->lastList;
            }

            if(freeLists->nextList->lastList->nextList != freeLists->lastList->nextList) {
                freeLists->nextList->lastList->nextList = freeLists->lastList->nextList;
            }
        }

        // Add removed node to free pool
        freeLists->lastList->nextList = pList;
        freeLists->lastList = pList;

        // Set next free list's last list as pList2
        if(freeLists->nextList != NULL) {
            freeLists->nextList->lastList->nextList = pList;
            freeLists->nextList->lastList = pList;     
        }
    }

    g_numListHeads--;
}

// Return last item and take it out of pList. Make the new last item the current one.
// Return NULL if pList is initially empty.
void* List_trim(List* pList) {
    assert(pList != NULL);

    if(pList->size == 0) {
        return NULL;
    }

    List_last(pList);
    void* item = List_remove(pList);
    List_prev(pList);

    return item;
}

// Search pList, starting at the current item, until the end is reached or a match is found. 
// In this context, a match is determined by the comparator parameter. This parameter is a
// pointer to a routine that takes as its first argument an item pointer, and as its second 
// argument pComparisonArg. Comparator returns 0 if the item and comparisonArg don't match, 
// or 1 if they do. Exactly what constitutes a match is up to the implementor of comparator. 
// 
// If a match is found, the current pointer is left at the matched item and the pointer to 
// that item is returned. If no match is found, the current pointer is left beyond the end of 
// the list and a NULL pointer is returned.
// 
// UPDATED: Added clarification of behaviour May 19
// If the current pointer is before the start of the pList, then start searching from
// the first node in the list (if any).
typedef bool (*COMPARATOR_FN)(void* pItem, void* pComparisonArg);
void* List_search(List* pList, COMPARATOR_FN pComparator, void* pComparisonArg) {
    assert(pList != NULL);

    if(pList->size == 0) {
        return NULL;
    }

    if (pList->currentPosition > (pList->size) - 1) {
        return NULL;
    }

    if(pList->currentPosition < 0) {
        List_first(pList);
    }

    int size = pList->size;

    for (int i = 0; i < size; i++) {
        if (pComparator(List_curr(pList), pComparisonArg) == true) {
            return List_curr(pList);
        } else {
            List_next(pList);
        }
    }

    return NULL;
}
