//
//  SYLossTimeCore.c
//  SYPerformanceMonitor
//
//  Created by csy on 2023/3/10.
//

#include "SYLossTimeCore.h"
#include <objc/runtime.h>
#include <pthread/pthread.h>
#include <dispatch/dispatch.h>
#include "fishhook.h"
#include <sys/time.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <stddef.h>
//#include <stdint.h>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <sys/time.h>
//#include <objc/message.h>
//#include <objc/runtime.h>
//#include <dispatch/dispatch.h>
//#include <pthread.h>

#pragma mark - static
__unused static id (*orig_objc_msgSend)(id, SEL, ...);
static bool _call_record_enabled = true;
static uint64_t _min_time_cost = 1000; //us
static int _max_call_depth = 3;
static pthread_key_t _thread_key;

static call_record *_syCallRecords;
//static int otp_record_num;
//static int otp_record_alloc;
static int _syRecordNum;
static int _syRecordAlloc;

#pragma mark - struct
// thread_call_record 记录调用方法详细信息，包括 obj 和 SEL 等
typedef struct {
    id self; //通过 object_getClass 能够得到 Class 再通过 NSStringFromClass 能够得到类名
    Class cls;
    SEL cmd; //通过 NSStringFromSelector 方法能够得到方法名
    uint64_t time; //us
    uintptr_t lr; // link register
} thread_call_record;

// thread_call_stack，需要用 index 记录当前调用方法树的深度
typedef struct {
    thread_call_record *stack;
    int allocated_length;
    int index; //index 记录当前调用方法树的深度
    bool is_main_thread;
} thread_call_stack;

#pragma mark - core methods
// fix: Static declaration of 'push_call_record' follows non-static declaration
static inline thread_call_stack * get_thread_call_stack() {
    thread_call_stack *cs = (thread_call_stack *)pthread_getspecific(_thread_key);
    if (cs == NULL) {
        cs = (thread_call_stack *)malloc(sizeof(thread_call_stack));
        cs->stack = (thread_call_record *)calloc(128, sizeof(thread_call_record));
        cs->allocated_length = 64;
        cs->index = -1;
        cs->is_main_thread = pthread_main_np();
        pthread_setspecific(_thread_key, cs);
    }
    return cs;
}

/// 用于记录 objc_msgSend 方法调用前的时间，表示开始时
static inline void push_call_record(id _self, Class _cls, SEL _cmd, uintptr_t lr) {
    thread_call_stack *cs = get_thread_call_stack();
    if (cs) {
        int nextIndex = (++cs->index);
        if (nextIndex >= cs->allocated_length) {
            cs->allocated_length += 64;
            cs->stack = (thread_call_record *)realloc(cs->stack, cs->allocated_length * sizeof(thread_call_record));
        }
        thread_call_record *newRecord = &cs->stack[nextIndex];
        newRecord->self = _self;
        newRecord->cls = _cls;
        newRecord->cmd = _cmd;
        newRecord->lr = lr;
        if (cs->is_main_thread && _call_record_enabled) {
            struct timeval now;
            gettimeofday(&now, NULL);
            newRecord->time = (now.tv_sec % 100) * 1000000 + now.tv_usec;
        }
    }
}

/// 用于记录 objc_msgSend 方法调用后的时间，表示结束时
/// pop_call_record - push_call_record 相减能够得到方法的执行耗时
static inline uintptr_t pop_call_record() {
    thread_call_stack *cs = get_thread_call_stack();
    int curIndex = cs->index;
    int nextIndex = cs->index--;
    thread_call_record *pRecord = &cs->stack[nextIndex];
    
    if (cs->is_main_thread && _call_record_enabled) {
        struct timeval now;
        gettimeofday(&now, NULL);
        uint64_t time = (now.tv_sec % 100) * 1000000 + now.tv_usec;
        if (time < pRecord->time) {
            time += 100 * 1000000;
        }
        uint64_t cost = time - pRecord->time;
        if (cost > _min_time_cost && cs->index < _max_call_depth) {
            if (!_syCallRecords) {
                _syRecordAlloc = 1024;
                _syCallRecords = malloc(sizeof(call_record) * _syRecordAlloc);
            }
            _syRecordNum++;
            if (_syRecordNum >= _syRecordAlloc) {
                _syRecordAlloc += 1024;
                _syCallRecords = realloc(_syCallRecords, sizeof(call_record) * _syRecordAlloc);
            }
            call_record *log = &_syCallRecords[_syRecordNum - 1];
            log->cls = pRecord->cls;
            log->depth = curIndex;
            log->sel = pRecord->cmd;
            log->time = cost;
        }
    }
    return pRecord->lr;
}

static void release_thread_call_stack(void *ptr) {
    thread_call_stack *cs = (thread_call_stack *)ptr;
    if (!cs) return;
    if (cs->stack) free(cs->stack);
    free(cs);
}

#pragma mark - private methods
/// objc_msgSend方法执行前
void before_objc_msgSend(id self, SEL _cmd, uintptr_t lr) {
    push_call_record(self, objc_getClass((const char * _Nonnull) self), _cmd, lr);
}

/// objc_msgSend方法执行后
uintptr_t after_objc_msgSend() {
    return pop_call_record();
}

#pragma mark - replacement objc_msgSend (arm64)
/// 针对 arm64 架构，编写一个可保留未知参数并跳转到 c 中任意函数指针的汇编代码，实现对 objc_msgSend 的 Hook
/// 下面是具体的汇编代码

// 调用 preObjc_msgSend，使用 bl label 语法。bl 执行一个分支链接操作，label 是无条件分支的，是和本指令的地址偏移，范围是 -128MB 到 +128MB
// 调用原始 objc_msgSend。使用 blr xn 语法。blr 除了从指定寄存器读取新的 PC 值外效果和 bl 一样。xn 是通用寄存器的 64 位名称分支地址，范围是 0 到 31
#define call(b, value) \
__asm volatile ("stp x8, x9, [sp, #-16]!\n"); \
__asm volatile ("mov x12, %0\n" :: "r"(value)); \
__asm volatile ("ldp x8, x9, [sp], #16\n"); \
__asm volatile (#b " x12\n");

// sp 是堆栈寄存器，存放栈的偏移地址，每次都指向栈顶。
// 保存 {x0-x9} 偏移地址到 sp 寄存器
#define save() \
__asm volatile ( \
"stp x8, x9, [sp, #-16]!\n" \
"stp x6, x7, [sp, #-16]!\n" \
"stp x4, x5, [sp, #-16]!\n" \
"stp x2, x3, [sp, #-16]!\n" \
"stp x0, x1, [sp, #-16]!\n");

// 读取 {x0-x9} 从保存到 sp 栈顶的偏移地址读起
#define load() \
__asm volatile ( \
"ldp x0, x1, [sp], #16\n" \
"ldp x2, x3, [sp], #16\n" \
"ldp x4, x5, [sp], #16\n" \
"ldp x6, x7, [sp], #16\n" \
"ldp x8, x9, [sp], #16\n" );

//// link unused
//#define link(b, value) \
//__asm volatile ("stp x8, lr, [sp, #-16]!\n"); \
//__asm volatile ("sub sp, sp, #16\n"); \
//call(b, value); \
//__asm volatile ("add sp, sp, #16\n"); \
//__asm volatile ("ldp x8, lr, [sp], #16\n");

// 跳出条件
#define ret() __asm volatile ("ret\n");

__attribute__((__naked__))
static void hook_Objc_msgSend() {
    // 保存参数
    save()
    
    // 交换参数
    __asm volatile ("mov x2, lr\n");
    __asm volatile ("mov x3, x4\n");
    
    // 调用 before_objc_msgSend
    call(blr, &before_objc_msgSend)
    
    // 读取参数
    load()
    
    // 调用原始 objc_msgSend
    call(blr, orig_objc_msgSend)
    
    // 保存原始 objc_msgSend 返回值
    save()
    
    // 调用 after_objc_msgSend
    call(blr, &after_objc_msgSend)
    
    // 重新存贮 lr
    // br 无条件分支到寄存器中的地址
    __asm volatile ("mov lr, x0\n");
    
    // 读取原始 objc_msgSend 返回值
    load()
    
    // 跳出条件
    ret()
}

#pragma mark - public methods
void start_monitor(void) {
    _call_record_enabled = true;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        pthread_key_create(&_thread_key, &release_thread_call_stack);
        rebind_symbols((struct rebinding[6]){
            {"objc_msgSend", (void *)hook_Objc_msgSend, (void **)&orig_objc_msgSend},
        }, 1);
    });
}

void end_monitor(void) {
    _call_record_enabled = false;
}

call_record *get_call_records(int *num) {
    if (num) {
        *num = _syRecordNum;
    }
    return _syCallRecords;
}
