#include <stdlib.h>
#include <strings.h>
#include <assert.h>
#include "mdns_list.h"


//
// Allocate a new list structure.
//
mdns_list *mdns_list_new(void *free_func, void *copy_func)
{
    mdns_list *list;


    list = (mdns_list *)malloc(sizeof(mdns_list));
    assert(list != NULL);
    bzero(list, sizeof(mdns_list));

    list->head = NULL;
    list->tail = NULL;

    list->free_func = free_func;
    list->copy_func = copy_func;

    return list;
}


//
// Free a list. If free_func is NULL then the objects themselves
// are not freed. Otherwise free_func is called for each object
// in the list.
//
void mdns_list_free(mdns_list *list)
{
    assert(list != NULL);

    while (list->head != NULL) {
	if (list->free_func != NULL)
	    list->free_func(list->head->object);
	mdns_list_remove_item(list, list->head);
    }

    assert(list->head == NULL);
    assert(list->tail == NULL);

    free(list);
}


//
// Copy a list and all the items in the list. If the copy_func
// is NULL then the object pointers are simply copied, otherwise
// copy_func is used to copy the objects themselves.
//
mdns_list *mdns_list_copy(const mdns_list *list)
{
    mdns_list		*copy;
    mdns_list_item	*item;


    assert(list != NULL);

    copy = mdns_list_new(list->free_func, list->copy_func);

    for (item = list->head; item != NULL; item = item->next) {
	if (list->copy_func != NULL)
	    mdns_list_append(copy, list->copy_func(item->object));
	else
	    mdns_list_append(copy, item->object);
    }

    return copy;
}


//
// Prepend an object to the beginning of the list. Return value can
// safely be ignored.
//
mdns_list_item *mdns_list_prepend(mdns_list *list, void *object)
{
    mdns_list_item	*item;


    assert(list != NULL);
    assert(object != NULL);

    item = (mdns_list_item *)malloc(sizeof(mdns_list_item));
    assert(item != NULL);
    bzero(item, sizeof(mdns_list_item));

    item->next = list->head;
    if (list->head != NULL)
	list->head->prev = item;
    list->head = item;

    if (list->tail == NULL)
	list->tail = item;
    else if (list->tail->prev == NULL)
	list->tail->prev = item;

    item->object = object;

    return item;
}


//
// Append an object to the end of the list. Return value can
// safely be ignored.
//
mdns_list_item *mdns_list_append(mdns_list *list, void *object)
{
    mdns_list_item	*item;


    assert(list != NULL);
    assert(object != NULL);

    item = (mdns_list_item *)malloc(sizeof(mdns_list_item));
    assert(item != NULL);
    bzero(item, sizeof(mdns_list_item));

    item->prev = list->tail;
    if (list->tail != NULL)
	list->tail->next = item;
    list->tail = item;

    if (list->head == NULL)
	list->head = item;
    else if (list->head->next == NULL)
	list->head->next = item;

    item->object = object;

    return item;
}


//
// Find an object in the list of items.
//
mdns_list_item *mdns_list_find_object(const mdns_list *list, void *object)
{
    mdns_list_item *item;


    for (item = list->head; item != NULL; item = item->next) {
        if (item->object == object)
	    return item;
    }

    return NULL;
}


//
// Remove a linked-list item from the list. 'item' is freed before this
// method returns.
//
void mdns_list_remove_item(mdns_list *list, mdns_list_item *item)
{
    assert(list != NULL);
    assert(item != NULL);

    if (item->prev == NULL)
	list->head = item->next;
    else
	item->prev->next = item->next;

    if (item->next == NULL)
	list->tail = item->prev;
    else
	item->next->prev = item->prev;

    free(item);
}


//
// Find the object in the linked list and then remove it. 'object' itself
// is NOT freed.
//
void mdns_list_remove(mdns_list *list, void *object)
{
    mdns_list_item *item;


    item = mdns_list_find_object(list, object);
    assert(item != NULL);
    mdns_list_remove_item(list, item);
}


