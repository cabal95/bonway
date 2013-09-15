#ifndef __MDNS_LIST_H__
#define __MDNS_LIST_H__


typedef struct g_mdns_list_item mdns_list_item;
struct g_mdns_list_item {
    mdns_list_item	*prev, *next;
    void		*object;
};

typedef struct g_mdns_list {
    mdns_list_item	*head, *tail;

    void		(* free_func)(void *);
    void		*(* copy_func)(void *);
} mdns_list;

mdns_list	*mdns_list_new(void *free_func, void *copy_func);
void		mdns_list_free(mdns_list *list);

mdns_list	*mdns_list_copy(const mdns_list *list);

mdns_list_item	*mdns_list_prepend(mdns_list *list, void *object);
mdns_list_item	*mdns_list_append(mdns_list *list, void *object);
mdns_list_item	*mdns_list_find_object(const mdns_list *list, void *object);
void		mdns_list_remove_item(mdns_list *list, mdns_list_item *item);
void		mdns_list_remove(mdns_list *list, void *object);

#define		mdns_list_first(list)		list->head
#define		mdns_list_last(list)		list->tail
#define		mdns_list_item_next(item)	item->next
#define		mdns_list_item_prev(item)	item->prev
#define		mdns_list_item_object(item)	item->object

#endif /* __MDNS_LIST_H__ */

