/**
 * Copyright (c) [2019] maminjie <canpool@163.com>
 * Copyright (c) [2020] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#ifndef __VLIST_H__
#define __VLIST_H__

#include "vpool.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* double list */

typedef struct vdlist {
    struct vdlist *prev, *next;
} vdlist_t;

/**
 * @brief create a empty list head body
 */
#define VDLIST_INIT(h)          { &(h), &(h) }

/**
 * @brief define a empty list head
 */
#define VDLIST_DEF(name)        vdlist_t name = VDLIST_INIT(name)

/**
 * @brief get the first node
 */
#define VDLIST_FIRST(h)         ((h)->next)

/* inlines */

/* list operations */

/**
 * @brief initialize a list head
 *
 * @param head [IN] the list head to initialize
 */
v_always_inline void vdlist_init(vdlist_t *head) {
    head->prev = head->next = head;
}

/**
 * @brief insert a new entry between two known consecutive entries.
 *
 * @param prev [IN] the prev-node of node
 * @param next [IN] the next-node of node
 * @param node [IN] the new node
 *
 * This is only for internal list manipulation
 * where we know the prev/next entries already!
 */
v_always_inline void __vdlist_add_at(vdlist_t *prev, vdlist_t *next, vdlist_t *node) {
    node->prev = prev;
    node->next = next;
    prev->next = node;
    next->prev = node;
}

/**
 * @brief add a new entry at the front of the list
 *
 * @param head [IN] list head to add in
 * @param node [IN] the node to be added
 *
 * Insert a new entry after the specified head
 * This is good for implementing stacks
 */
v_always_inline void vdlist_add(vdlist_t *head, vdlist_t *node) {
    __vdlist_add_at(head, head->next, node);
}

/**
 * @brief add a new entry at the tail of the list
 *
 * @param head [IN] list head to add in
 * @param node [IN] the node to be added
 *
 * Insert a new entry before the specified head
 * This is useful for implementing queues
 */
v_always_inline void vdlist_add_tail(vdlist_t *head, vdlist_t *node) {
    __vdlist_add_at(head->prev, head, node);
}

/**
 * @brief get the first node
 *
 * @param head [IN] list head
 */
v_always_inline vdlist_t * vdlist_head(vdlist_t *head) {
    return head->next;
}

/**
 * @brief deletes entry from list
 *
 * @param node [IN] the node to be deleted
 *
 * Note: vdlist_empty() on the node does not return true after this,
 * the entry is in an undefined state
 */
v_always_inline void vdlist_del(vdlist_t *node) {
    node->prev->next = node->next;
    node->next->prev = node->prev;
}

/**
 * @brief deletes entry from list and init it
 *
 * @param node [IN] the node to be deleted
 *
 * Note: dlist_empty() on the node return true after this
 */
v_always_inline void vdlist_del_init(vdlist_t *node) {
    vdlist_del (node);
    vdlist_init(node);
}

/**
 * @brief check if a list is empty
 *
 * @param head [IN] the list head to check
 */
v_always_inline int vdlist_empty(vdlist_t *head) {
    return (head->prev == head);
}

/**
 * @brief check if a list is uninitilized
 *
 * @param head [IN] the list head to check
 */
v_always_inline int vdlist_uninitilized(vdlist_t *head) {
    return (head->prev == NULL || head->next == NULL);
}

/**
 * @brief get the length of list
 *
 * @param head [IN] the list head
 */
v_inline int vdlist_length(vdlist_t *head) {
    int num;
    vdlist_t *cur = head;
    for (num = 0; cur->next != head; cur = cur->next, ++num) {
        // do nothing
    }
    return num;
}

/**
 * @brief iterate over a list
 *
 * @param pos   [OUT] the &vdlist_t to use as a loop cursor
 * @param head  [IN]  the head for your list
 * @param dir   [IN]  iterate direction, next for forward, prev for backward
 */
#define __vdlist_foreach(pos, head, dir)                    \
    for (pos = (head)->dir; pos != (head); pos = pos->dir)

/**
 * @brief oterate over a list forward
 *
 * @param pos   [OUT] the &vdlist_t to use as a loop cursor
 * @param head  [IN]  the head for your list
 */
#define vdlist_foreach(pos, head)                           \
    __vdlist_foreach(pos, head, next)

/**
 * @brief iterate over a list backward
 *
 * @param pos   [OUT] the &vdlist_t to use as a loop cursor
 * @param head  [IN]  the head for your list
 */
#define vdlist_foreach_backward(pos, head)                  \
    __vdlist_foreach(pos, head, prev)

/**
 * @brief iterate over a list safe against removal of list entry
 *
 * @param pos   [OUT] the &struct list_head to use as a loop cursor
 * @param n     [IN]  another &struct list_head to use as temporary storage
 * @param head  [IN]  the head for your list
 * @param dir   [IN]  iterate direction, next for forward, prev for backward
 */
#define __vdlist_foreach_safe(pos, n, head, dir)            \
    for (pos = (head)->dir, n = pos->dir; pos != (head);    \
         pos = n, n = n->dir)

/**
 * @brief iterate over a list safe against removal of list entry forward
 *
 * @param pos   [OUT] the &struct list_head to use as a loop cursor
 * @param n     [IN]  another &struct list_head to use as temporary storage
 * @param head  [IN]  the head for your list
 */
#define vdlist_foreach_safe(pos, n, head)                   \
    __vdlist_foreach_safe(pos, n, head, next)

/**
 * @brief iterate over a list safe against removal of list entry backward
 *
 * @param pos   [OUT] the &struct list_head to use as a loop cursor
 * @param n     [IN]  another &struct list_head to use as temporary storage
 * @param head  [IN]  the head for your list
 */
#define vdlist_foreach_safe_backward(pos, n, head)           \
    __vdlist_foreach_safe(pos, n, head, prev)

/**
 * @brief iterate over a structure through a list
 *
 * @param item  [OUT] the &struct of @t to use as a loop cursor
 * @param head  [IN]  the head for your list
 * @param t     [IN]  the type of the container struct this is embedded in
 * @param m     [IN]  the name of the member within the struct
 */
#define vdlist_foreach_entry(item, head, t, m)                      \
    for (item = container_of((head)->next, t, m);                   \
         &item->m != (head);                                        \
         item = container_of(item->m.next, t, m))

/**
 * @brief iterate over a structure through a list, can delete list item safely
 *
 * @param item  [OUT] the &struct of @t to use as a loop cursor
 * @param head  [IN]  the head for your list
 * @param n     [IN]  another &struct of @t to use as temporary storage
 * @param t     [IN]  the type of the container struct this is embedded in
 * @param m     [IN]  the name of the member within the struct
 */
#define vdlist_foreach_entry_safe(item, n, head, t, m)              \
    for (item = container_of((head)->next, t, m),                   \
         n = container_of(item->m.next, t, m);                      \
         &item->m != (head);                                        \
         item = n, n = container_of(item->m.next, t, m))


/* single list */

typedef struct vslist {
    struct vslist *next;
} vslist_t;

/**
 * @brief init the list
 */
#define VSLIST_INIT(h)          {0}

/**
 * @brief define a empty list head
 */
#define VSLIST_DEF(name)        vslist_t name = VSLIST_INIT(name)

/* inlines */

v_inline void vslist_init(vslist_t *head) {
    head->next = 0;
}

v_inline void vslist_add(vslist_t *head, vslist_t *node) {
    node->next = head->next;
    head->next = node;
}

v_inline void vslist_add_tail(vslist_t *head, vslist_t *node) {
    while (head->next) {
        head = head->next;
    }
    vslist_add(head, node);
}

v_always_inline vslist_t * vslist_head(vslist_t *head) {
    return head->next;
}

v_inline void vslist_del(vslist_t *head, vslist_t *node) {
    while (head->next) {
        if (head->next == node) {
            head->next = node->next;
            break;
        }
        head = head->next;
    }
}

v_inline int vslist_empty(const vslist_t *head) {
    return !head->next;
}

v_inline int vslist_length(vslist_t *head) {
    int num;
    vslist_t *cur = head;
    for (num = 0; cur->next; cur = cur->next, ++num) {
        // do nothing
    }
    return num;
}

/**
 * @brief iterate over list of given type
 *
 * @param item  [OUT] the &struct of @t to use as a loop cursor
 * @param head  [IN]  the head for your list
 * @param t     [IN]  the type of the container struct this is embedded in
 * @param m     [IN]  the name of the member within the struct
 */
#define vslist_foreach_entry(item, head, t, m)              \
    for (item = container_of((head)->next, t, m);           \
         &item->m;                                          \
         item = container_of(item->m.next, t, m))

/**
 * @brief iterate over list of given type safe against removal of list entry
 *
 * @param item  [OUT] the &struct of @t to use as a loop cursor
 * @param head  [IN]  the head for your list
 * @param n     [IN]  another &struct of @t to use as temporary storage
 * @param t     [IN]  the type of the container struct this is embedded in
 * @param m     [IN]  the name of the member within the struct
 */
#define vslist_foreach_entry_safe(item, n, head, t, m)      \
    for (item = container_of((head)->next, t, m),           \
         n = container_of(item->m.next, t, m);              \
         &item->m;                                          \
         item = n, n = item ? container_of(item->m.next, t, m) : NULL)


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VLIST_H__ */
