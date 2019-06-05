#ifndef __STUB_LIST_H__
#define __STUB_LIST_H__

#define __inline__
 
typedef struct list_head
{
    struct list_head *next, *prev;
 
}list_head_t;
 
#define LIST_HEAD_INIT(name)    { &(name),&(name) }
#define LIST_HEAD(name)         struct list_head name = LIST_HEAD_INIT(name)
#define INIT_LIST_HEAD(ptr)     do {\
    (ptr)->next = (ptr);(ptr)->prev = (ptr);\
} while (0);
 
static __inline__ void __list_add(struct list_head *new_, struct list_head *prev, struct list_head *next)
{
    next->prev = new_;
    new_->next = next;
    new_->prev = prev;
    prev->next = new_;
}
 
static __inline__ void list_add(struct list_head *new_, struct list_head *head)
{
    __list_add(new_, head, head->next);
}
 
static __inline__ void __list_del(struct list_head *prev, struct list_head *next)
{
    next->prev = prev;
    prev->next = next;
}
 
static __inline__ void list_del(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
}
 
#define list_entry(ptr,type,member) \
   ((type*)((char*)(ptr)-(unsigned long)(&((type*)0)->member)))
#define list_for_each(pos,head) \
for(pos=(head)->next;pos!=(head);pos=pos->next)
 
static __inline__ int list_count(struct list_head *head)
{
    struct list_head *pos;
    int count = 0;
    list_for_each(pos, head)
    {
        count++;
    }
    return count;
}

#endif  // __STUB_LIST_H__