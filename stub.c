#define _CRT_SECURE_NO_WARNINGS
#include <pthread.h>
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include "stub_list.h"

LIST_HEAD(head);
struct stub
{
    struct list_head node;
    char             desc[256];
    void             *orig_f;
    void             *stub_f;
    unsigned int     stubpath;
    unsigned int     old_flg;
    unsigned char    assm[5];
};
 
pthread_mutex_t mutex;
int init_lock_flag = 0;

void initlock()
{
    pthread_mutex_init(&mutex, NULL);
}
void lock()
{
    (init_lock_flag == 0) && (initlock(), init_lock_flag = 1);
    pthread_mutex_lock(&mutex);
}
 
void unlock()
{
    pthread_mutex_unlock(&mutex);
}
 
#define PAGE_SIZE           4096
#define PAGE_HEAD(x)        ((unsigned long)(x) & ~(PAGE_SIZE - 1))
#define PAGE_NEXT_HEAD(x)   (PAGE_HEAD(x) + PAGE_SIZE)
 
static int set_mprotect(struct stub* pstub)
{
    /****************************************************************************
    * 函数mprotect用来设置内存区域的保护属性,可以将原始函数
    * 所在的内存设置为可读、可写、可执行，这样就可以改变原始函数所在内存的代码指令。
    * 由于 mprotect只能设置从内存页大小(4096)的整数倍开始的
    * 地址，因此利用PAGE_HEAD( pstub->orig_f )计算出一个比原始函数地址小
    * 且为内存页大小(4096)的整数倍的地址，把它当作mprotect的
    * 作用的起始地址，内存大小为两个内存页PAGE_SIZE << 1
    *****************************************************************************/
    return mprotect((void *)PAGE_HEAD(pstub->orig_f), PAGE_SIZE << 1, PROT_READ | PROT_WRITE | PROT_EXEC);  
}
 
static int set_asm_jmp(struct stub* pstub)
{
    unsigned int offset;
    /* 保存从原始函数地址开始的5个字节，因为之后我们会改写这块区域 */
    memcpy(pstub->assm, pstub->orig_f, sizeof(pstub->assm));
    *((char*)pstub->orig_f) = 0xE9;/* 这个是相对跳转指令jmp */
    /**************************************************************
     * 计算出桩函数与原始函数之间的相对地址，注意要减去jmp指令的
     * 5个字节(0xE9加上一个4字节的相对地址)，然后用这条jmp指令，改写
     * 原始函数地址开始的5个字节，这样调用原始函数，就会自动跳到桩函数
     **************************************************************/
    offset = (unsigned int)((long)pstub->stub_f - ((long)pstub->orig_f + 5));
    *((unsigned int*)((char*)pstub->orig_f + 1)) = offset;
    return 0;
}
 
static void restore_asm(struct stub* pstub)
{
    /* 恢复原始函数地址开始的5个字节 */
    memcpy(pstub->orig_f, pstub->assm, sizeof(pstub->assm));
}
 
 
int install_stub(void *orig_f, void *stub_f, char *desc)
{
    struct stub *pstub;
    pstub = (struct stub*)malloc(sizeof(struct stub));
    pstub->orig_f = orig_f;
    pstub->stub_f = stub_f;
    do
    {
        /* 设置该内存段属性 */
        if (set_mprotect(pstub))
        {
            break;
        }
        /* 用jmp指令去覆盖orig_f开始的5个字节 */
        if (set_asm_jmp(pstub))
        {
            break;
        }
 
        if (desc)
        {
            strncpy(pstub->desc, desc, sizeof(pstub->desc));
        }
        lock(); /* 如果有多个线程同时操作链表，要使用锁进行同步 */
        /* 如果对多个函数打桩，就需要保存多个函数的相关信息，这里使用链表储存 */
        list_add(&pstub->node, &head);
        unlock(); /* 操作完成，释放锁 */
        return 0;
    } while (0);
 
    free(pstub);
    return -1;
}
 
int uninstall_stub(void* stub_f)
{
    struct stub *pstub;
    struct list_head *pos;
    /* 移除桩函数就是将原始函数地址开始的5个字节恢复，然后将该
       函数的信息从链表中移除同时释放之前动态申请的内存 */
    list_for_each(pos, &head)
    {
        pstub = list_entry(pos, struct stub, node);
        if (pstub->stub_f == stub_f)
        {
            restore_asm(pstub);
            lock();
            list_del(&pstub->node);
            unlock();
            free(pstub);
            return 0;
        }
    }
    return -1;
}