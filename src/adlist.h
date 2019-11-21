/* adlist.h - A generic doubly linked list implementation
 *
 * Copyright (c) 2006-2012, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __ADLIST_H__
#define __ADLIST_H__

/* Node, List, and Iterator are the only data structures used currently. */

/*
 * 列表结构的节点定义
 */
typedef struct listNode {
    struct listNode *prev;  //前驱指针
    struct listNode *next;  //后继指针
    void *value;            //节点值
} listNode;

/**
 * 迭代器
 */
typedef struct listIter {
    listNode *next; //当前节点
    int direction;  //迭代方向
} listIter;

/*
 * 列表定义
 */
typedef struct list {
    listNode *head;                     //头节点指针
    listNode *tail;                     //尾节点指针
    void *(*dup)(void *ptr);            //复制链表节点所保存的值
    void (*free)(void *ptr);            //释放链表节点所保存的值
    int (*match)(void *ptr, void *key); //对比链表节点所保存的值和另一个输入值是否相等
    unsigned long len;                  //链表长度
} list;

/* Functions implemented as macros */
//返回链表的长度（包含了多少个节点）。
#define listLength(l) ((l)->len)
//返回链表的表头节点。
#define listFirst(l) ((l)->head)
//表尾节点可以通过链表的 tail 属性直接获得， O(1)
#define listLast(l) ((l)->tail)
//返回给定节点的前置节点。
#define listPrevNode(n) ((n)->prev)
//返回给定节点的后置节点。
#define listNextNode(n) ((n)->next)
//返回给定节点目前正在保存的值。
#define listNodeValue(n) ((n)->value)

//将给定的函数设置为链表的节点值复制函数。
#define listSetDupMethod(l,m) ((l)->dup = (m))
//将给定的函数设置为链表的节点值释放函数。
#define listSetFreeMethod(l,m) ((l)->free = (m))
//将给定的函数设置为链表的节点值对比函数。
#define listSetMatchMethod(l,m) ((l)->match = (m))

//返回链表当前正在使用的节点值复制函数。
#define listGetDupMethod(l) ((l)->dup)
//返回链表当前正在使用的节点值释放函数。
#define listGetFree(l) ((l)->free)
//返回链表当前正在使用的节点值对比函数。
#define listGetMatchMethod(l) ((l)->match)

/* Prototypes */
//创建一个不包含任何节点的新链表。
list *listCreate(void);
//释放给定链表，以及链表中的所有节点。
void listRelease(list *list);
//移除链表所有的节点，但是不销毁链表
void listEmpty(list *list);
//将一个包含给定值的新节点添加到给定链表的表头。
list *listAddNodeHead(list *list, void *value);
//将一个包含给定值的新节点添加到给定链表的表尾。
list *listAddNodeTail(list *list, void *value);
//将一个包含给定值的新节点添加到给定节点的之前或者之后。
list *listInsertNode(list *list, listNode *old_node, void *value, int after);
//从链表中删除给定节点。
void listDelNode(list *list, listNode *node);
//获取list的迭代器,初始化后,每次调用listNext()，返回链表的下一个元素
listIter *listGetIterator(list *list, int direction);
//获取链表的下一个元素, 入参迭代器由listGetIterator获得
listNode *listNext(listIter *iter);
//释放迭代器的内存
void listReleaseIterator(listIter *iter);
//复制一个给定链表的副本。
list *listDup(list *orig);
//查找并返回链表中包含给定值的节点。
listNode *listSearchKey(list *list, void *key);
//返回链表在给定索引上的节点。
listNode *listIndex(list *list, long index);
//从私有的迭代器中创建一个从头至尾顺序的迭代器
void listRewind(list *list, listIter *li);
//从私有的迭代器中创建一个从尾至头顺序的迭代器
void listRewindTail(list *list, listIter *li);
//将链表的表尾节点弹出，然后将被弹出的节点插入到链表的表头， 成为新的表头节点。
void listRotate(list *list);
//将列表o追加到列表l的尾部
void listJoin(list *l, list *o);

/* Directions for iterators */
#define AL_START_HEAD 0 //从表头向表尾进行迭代
#define AL_START_TAIL 1 //从表尾到表头进行迭代

#endif /* __ADLIST_H__ */
