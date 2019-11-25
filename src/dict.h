/* Hash Tables Implementation.
 *
 * This file implements in-memory hash tables with insert/del/replace/find/
 * get-random-element operations. Hash tables will auto-resize if needed
 * tables of power of two in size are used, collisions are handled by
 * chaining. See the source code for more information... :)
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

#include <stdint.h>

#ifndef __DICT_H
#define __DICT_H

#define DICT_OK 0
#define DICT_ERR 1

/* Unused arguments generate annoying warnings... */
#define DICT_NOTUSED(V) ((void) V)

/*
 * 哈希表节点的数据结构
 * 整理 : 张德满
 * 邮箱 : go_developer@163.com
 * QQ : 2215508028
 */
typedef struct dictEntry {
    void *key;  //字典key
    union {     //数据值
        void *val;
        uint64_t u64;
        int64_t s64;
        double d;
    } v;
    struct dictEntry *next; //指向下个哈希表节点，形成链表，hash键冲突时才被用到
} dictEntry;

typedef struct dictType {
    uint64_t (*hashFunction)(const void *key);
    void *(*keyDup)(void *privdata, const void *key);
    void *(*valDup)(void *privdata, const void *obj);
    int (*keyCompare)(void *privdata, const void *key1, const void *key2);
    void (*keyDestructor)(void *privdata, void *key);
    void (*valDestructor)(void *privdata, void *obj);
} dictType;

/*
 * 哈希表的数据结构
 * 一个字典拥有两个哈希表, 用于实现数据增长后的rehash
 * 整理 : 张德满
 * 邮箱 : go_developer@163.com
 * QQ : 2215508028
 */
typedef struct dictht {
    dictEntry **table;      //哈希表数组
    unsigned long size;     //哈希表大小
    unsigned long sizemask; //哈希表大小掩码，用于计算索引值 总是等于 size - 1
    unsigned long used;     //该哈希表已有节点的数量
} dictht;

/*
 * 字典的数据结构
 * 一个字典拥有两个哈希表, 用于实现数据增长后的rehash
 * 整理 : 张德满
 * 邮箱 : go_developer@163.com
 * QQ : 2215508028
 */
typedef struct dict {
    dictType *type;             //类型特定函数
    void *privdata;             //私有数据
    dictht ht[2];               //哈希表
    long rehashidx;             //rehash 索引 当 rehash 不在进行时，值为 -1
    unsigned long iterators;    //目前正在运行的安全迭代器的数量
} dict;

/* If safe is set to 1 this is a safe iterator, that means, you can call
 * dictAdd, dictFind, and other functions against the dictionary even while
 * iterating. Otherwise it is a non safe iterator, and only dictNext()
 * should be called while iterating. */
typedef struct dictIterator {
    dict *d;                        //迭代器对应的表
    long index;                     //table中的下标，标记table[index]
    int table, safe;                //table 判断是表th[0] 还是th[1] safe 判断是是否为安全
    dictEntry *entry, *nextEntry;   //当前拉链的元素与下一个元素
    long long fingerprint;          //不安全的迭代器，即正在更新中的表的迭代器，使用指纹来标记。
} dictIterator;

typedef void (dictScanFunction)(void *privdata, const dictEntry *de);
typedef void (dictScanBucketFunction)(void *privdata, dictEntry **bucketref);

/*
 * 新建一个空的哈希表时的默认大小
 * 整理 : 张德满
 * 邮箱 : go_developer@163.com
 * QQ : 2215508028
 */
#define DICT_HT_INITIAL_SIZE     4

/* ------------------------------- Macros ------------------------------------*/
#define dictFreeVal(d, entry) \
    if ((d)->type->valDestructor) \
        (d)->type->valDestructor((d)->privdata, (entry)->v.val)

#define dictSetVal(d, entry, _val_) do { \
    if ((d)->type->valDup) \
        (entry)->v.val = (d)->type->valDup((d)->privdata, _val_); \
    else \
        (entry)->v.val = (_val_); \
} while(0)

#define dictSetSignedIntegerVal(entry, _val_) \
    do { (entry)->v.s64 = _val_; } while(0)

#define dictSetUnsignedIntegerVal(entry, _val_) \
    do { (entry)->v.u64 = _val_; } while(0)

#define dictSetDoubleVal(entry, _val_) \
    do { (entry)->v.d = _val_; } while(0)

#define dictFreeKey(d, entry) \
    if ((d)->type->keyDestructor) \
        (d)->type->keyDestructor((d)->privdata, (entry)->key)

#define dictSetKey(d, entry, _key_) do { \
    if ((d)->type->keyDup) \
        (entry)->key = (d)->type->keyDup((d)->privdata, _key_); \
    else \
        (entry)->key = (_key_); \
} while(0)

#define dictCompareKeys(d, key1, key2) \
    (((d)->type->keyCompare) ? \
        (d)->type->keyCompare((d)->privdata, key1, key2) : \
        (key1) == (key2))

#define dictHashKey(d, key) (d)->type->hashFunction(key)
#define dictGetKey(he) ((he)->key)
#define dictGetVal(he) ((he)->v.val)
#define dictGetSignedIntegerVal(he) ((he)->v.s64)
#define dictGetUnsignedIntegerVal(he) ((he)->v.u64)
#define dictGetDoubleVal(he) ((he)->v.d)
#define dictSlots(d) ((d)->ht[0].size+(d)->ht[1].size)
#define dictSize(d) ((d)->ht[0].used+(d)->ht[1].used)
#define dictIsRehashing(d) ((d)->rehashidx != -1)

/* API */
//创建一个新的字典
dict *dictCreate(dictType *type, void *privDataPtr);
//扩张或者创建一个字典
int dictExpand(dict *d, unsigned long size);
//将指定的键值对添加到字典里
int dictAdd(dict *d, void *key, void *val);
//基础的向hash表中新增一个键值对的方法
dictEntry *dictAddRaw(dict *d, void *key, dictEntry **existing);
dictEntry *dictAddOrFind(dict *d, void *key);
//将给定的键值对添加到字典里面， 如果键已经存在于字典，那么用新值取代原有的值。
int dictReplace(dict *d, void *key, void *val);
//从字典中删除给定键所对应的键值对
int dictDelete(dict *d, const void *key);
dictEntry *dictUnlink(dict *ht, const void *key);
void dictFreeUnlinkedEntry(dict *d, dictEntry *he);
//释放给定字典，以及字典中包含的所有键值对。
void dictRelease(dict *d);
//查找指定的key
dictEntry * dictFind(dict *d, const void *key);
//返回指定的key的值
void *dictFetchValue(dict *d, const void *key);
//重计算字典大小
int dictResize(dict *d);
//获取迭代器, 不安全, safe = 0
dictIterator *dictGetIterator(dict *d);
//获取迭代器, 安全, safe = 1
dictIterator *dictGetSafeIterator(dict *d);
//依据迭代器获取下一个键值对
dictEntry *dictNext(dictIterator *iter);
//释放迭代器
void dictReleaseIterator(dictIterator *iter);
//随机获取一个键值对
dictEntry *dictGetRandomKey(dict *d);
unsigned int dictGetSomeKeys(dict *d, dictEntry **des, unsigned int count);
void dictGetStats(char *buf, size_t bufsize, dict *d);
uint64_t dictGenHashFunction(const void *key, int len);
uint64_t dictGenCaseHashFunction(const unsigned char *buf, int len);
//清空字典数据但不释放空间
void dictEmpty(dict *d, void(callback)(void*));
//启用redis空间再次分配
void dictEnableResize(void);
//禁用redis空间再次分配
void dictDisableResize(void);
//对字典进行rehash
int dictRehash(dict *d, int n);
//执行rehash操作，指定运行时间，此时间内rehash可能未最终完成
int dictRehashMilliseconds(dict *d, int ms);
void dictSetHashFunctionSeed(uint8_t *seed);
uint8_t *dictGetHashFunctionSeed(void);
unsigned long dictScan(dict *d, unsigned long v, dictScanFunction *fn, dictScanBucketFunction *bucketfn, void *privdata);
uint64_t dictGetHash(dict *d, const void *key);
dictEntry **dictFindEntryRefByPtrAndHash(dict *d, const void *oldptr, uint64_t hash);

/* Hash table types */
extern dictType dictTypeHeapStringCopyKey;
extern dictType dictTypeHeapStrings;
extern dictType dictTypeHeapStringCopyKeyValue;

#endif /* __DICT_H */
