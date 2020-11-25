#include "task.h"
#include "stdint.h"
#include "assert.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static tid_t tid = 0;   //tid递增

struct task_struct* main_task;   //主任务tcb
struct task_struct* current_task;   //记录当前任务
struct list task_ready_list;
struct list task_all_list;

static struct list_elem* task_tag;   //保存队列中的任务节点

/**
 * init_task - 初始化任务基本信息
 * **/
void init_task(struct task_struct* ptask, char* name, int prio)
{
    memset(ptask, 0, sizeof(*ptask));
    ptask->tid = tid++;
    strcpy(ptask->name, name);

    if(ptask == main_task) {
        ptask->status = TASK_RUNNING;
        current_task = ptask;
    } else {
        ptask->status = TASK_READY;
    }

    ptask->priority = prio;
    ptask->ticks = prio;
    ptask->elapsed_ticks = 0;

    ptask->stack_magic = 0x19991120;
}

/**
 * task_create - 创建（初始化）一个任务
 * @ptask: 任务结构体指针
 * @function: 任务的功能函数
 * @func_arg: 任务功能函数的参数
 * **/
static void task_create(struct task_struct* ptask, task_func function, void* func_arg)
{
    ptask->function = function;
    ptask->func_args = func_arg;
}

/**
 * task_start - 创建一个优先级为prio，名字为name的任务
 * @name: 任务名
 * @prio: 任务优先级
 * @func: 任务处理函数
 * @func_arg: 任务参数
 * **/
struct task_struct* task_start(char* name, int prio, task_func function, void* func_arg)
{
    struct task_struct* task = (struct task_struct*)malloc(sizeof(struct task_struct));

    init_task(task, name, prio);
    task_create(task, function, func_arg);

    //之前不再队列中
    assert(!elem_find(&task_ready_list, &task->general_tag));
    //加入就绪任务队列
    list_append(&task_ready_list, &task->general_tag);

    //之前不再全部任务队列中
    assert(!elem_find(&task_all_list, &task->all_list_tag));
    //加入到全部任务队列
    list_append(&task_all_list, &task->all_list_tag);

    return task;
}

/**
 * make_main_task - 将main函数变为任务
 * **/
static void make_main_task(void)
{
    //main任务分配task_struct结构体
    main_task = (struct task_struct*)malloc(sizeof(struct task_struct));
    
    //初始化main任务的task_struct
    init_task(main_task, "main", 31);

    //main函数是当前任务，当前还不再task_ready_list中，只加入task_all_list
    assert(!elem_find(&task_all_list, &main_task->all_list_tag));
    list_append(&task_all_list, &main_task->all_list_tag);
}

/**
 * tid2task - 根据tid获得task_struct
 * @tid: 任务的tid
 * **/
struct task_struct* tid2task(tid_t tid)
{
    struct list_elem* pelem = task_all_list.head.next;
    struct task_struct* ptask = NULL;
    while(pelem != &task_all_list.tail) {
        ptask = elem2entry(struct task_struct, all_list_tag, pelem);
        if(ptask->tid == tid) {
            break;
        }
        ptask = NULL;
        pelem = pelem->next;
    }
    printf("ZZZZZ\n");
    return ptask;
}

/**
 * print_task_info - 打印task信息
 * **/
void print_task_info(struct task_struct* ptask)
{
    printf("tid = %d\n", ptask->tid);
    printf("name = %s\n", ptask->name);
    printf("priority = %d\n", ptask->priority);
    printf("stack_magic = %x\n", ptask->stack_magic);
}

/** 
 * task_init - 初始化任务 
 * **/
void task_init(void)
{
    printf("task_init start.\n");
    list_init(&task_ready_list);
    list_init(&task_all_list);
    
    //将当前main函数创建为任务
    make_main_task();

    printf("task_init done!\n");
}
