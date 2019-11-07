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

#ifndef __JFFS2_LIST_H__
#define __JFFS2_LIST_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* TYPES */

struct list_head {
    struct list_head *next;
    struct list_head *prev;
};

/* MACROS */

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
        struct list_head name = LIST_HEAD_INIT(name)

#define INIT_LIST_HEAD( _list_ )              \
            do{                               \
                (_list_)->next = (_list_)->prev = (_list_);   \
            }while(0)

/* FUNCTIONS */

/* Insert an entry _after_ the specified entry */
static inline void list_add( struct list_head *newent, struct list_head *afterthisent )
{
    struct list_head *next = afterthisent->next;
    newent->next = next;
    newent->prev = afterthisent;
    afterthisent->next = newent;
    next->prev = newent;
} /* list_add() */

/* Insert an entry _before_ the specified entry */
static inline void list_add_tail( struct list_head *newent, struct list_head *beforethisent )
{
    struct list_head *prev = beforethisent->prev;
    newent->prev = prev;
    newent->next = beforethisent;
    beforethisent->prev = newent;
    prev->next = newent;
} /* list_add_tail() */

/* Delete the specified entry */
static inline void list_del( struct list_head *ent )
{
    ent->prev->next = ent->next;
    ent->next->prev = ent->prev;
} /* list_del() */

/* Is this list empty? */
static inline int list_empty( struct list_head *list )
{
    return ( list->next == list );
} /* list_empty() */

/* list_entry - Assuming you have a struct of type _type_ that contains a
   list which has the name _member_ in that struct type, then given the
   address of that list in the struct, _list_, this returns the address
   of the container structure */

#define list_entry( _list_, _type_, _member_ ) \
    ((_type_ *)((char *)(_list_)-(char *)(offsetof(_type_,_member_))))

/* list_for_each - using _ent_, iterate through list _list_ */

#define list_for_each( _ent_, _list_ )   \
    for ( (_ent_) = (_list_)->next;      \
    (_ent_) != (_list_);                 \
    (_ent_) = (_ent_)->next )

/*
 * list_for_each_entry - this function can be use to iterate over all
 * items in a list* _list_ with it's head at _head_ and link _item_
 */
//#define list_for_each_entry(_list_, _head_, _item_)                   \
//for ((_list_) = list_entry((_head_)->next, typeof(*_list_), _item_); \
//     &((_list_)->_item_) != (_head_);                                 \
//     (_list_) = list_entry((_list_)->_item_.next, typeof(*_list_), _item_))

#define list_for_each_entry(_list_, _head_, _type_, _item_)                   \
for ((_list_) = list_entry((_head_)->next, _type_, _item_); \
     &((_list_)->_item_) != (_head_);                                 \
     (_list_) = list_entry((_list_)->_item_.next, _type_, _item_))


#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif /* __cplusplus */


#endif /* __JFFS2_LIST_H__ */

