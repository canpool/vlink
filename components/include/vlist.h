/**
 * Copyright (c) [2019] maminjie <canpool@163.com>
 *
 * vlink is licensed under the Mulan PSL v1.
 * You can use this software according to the terms and conditions of the Mulan PSL v1.
 * You may obtain a copy of Mulan PSL v1 at:
 *
 *    http://license.coscl.org.cn/MulanPSL
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
 * FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v1 for more details.
 */

#ifndef __VLIST_H__
#define __VLIST_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#ifndef __v_inline
#define __v_inline static inline
#endif

/* double list */
typedef struct vdlist {
    struct vdlist *prev, *next;
} vdlist_t;

#define V_DLIST_DEF(name) vdlist_t name = {&(name), &(name)}

__v_inline void vdlist_init(vdlist_t *list)
{
    list->next = list;
    list->prev = list;
}

__v_inline void vdlist_add(vdlist_t *head, vdlist_t *node)
{
    node->next = head->next;
    node->prev = head;
    head->next->prev = node;
    head->next = node;
}

__v_inline void vdlist_add_tail(vdlist_t *head, vdlist_t *node)
{
    vdlist_add(head->prev, node);
}

__v_inline vdlist_t *vdlist_head(vdlist_t *header)
{
    return header->next;
}

__v_inline void vdlist_del(vdlist_t *node)
{
    node->next->prev = node->prev;
    node->prev->next = node->next;
}

__v_inline void vdlist_del_init(vdlist_t *node)
{
    vdlist_del(node);
    vdlist_init(node);
}

__v_inline int vdlist_empty(vdlist_t *head)
{
    return (head->next == head);
}

#define V_DLIST_FIRST(object) ((object)->next)
#define V_OFFSET_OF(type, member) ((long)&((type *)0)->member)

#define V_DLIST_ENTRY(item, type, member) ((type *)((char *)(item)-V_OFFSET_OF(type, member)))

#define V_DLIST_FOR_EACH_ENTRY(item, list, type, member)                                                               \
    for (item = V_DLIST_ENTRY((list)->next, type, member); &item->member != (list);                                    \
         item = V_DLIST_ENTRY(item->member.next, type, member))

#define V_DLIST_FOR_EACH_ENTRY_SAFE(item, next, list, type, member)                                                    \
    for (item = V_DLIST_ENTRY((list)->next, type, member), next = V_DLIST_ENTRY(item->member->next, type, member);     \
         &item->member != (list); item = next, item = V_DLIST_ENTRY(item->member.next, type, member))

#define V_DLIST_FOR_EACH(item, list) for ((item) = (list)->next; (item) != (list); (item) = (item)->next)

#define V_DLIST_FOR_EACH_SAFE(item, next, list)                                                                        \
    for ((item) = (list)->next, (next) = (item)->next; (item) != (list); (item) = (next), (next) = (item)->next)

/* single list */
typedef struct vslist {
    struct vslist *next;
} vslist_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VLIST_H__ */
