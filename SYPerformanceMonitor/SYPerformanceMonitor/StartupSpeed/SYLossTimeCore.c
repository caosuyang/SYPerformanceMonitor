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

#pragma mark - static
__unused static id (*orig_objc_msgSend)(id, SEL, ...);

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

#pragma mark - private methods
/// objc_msgSend方法执行前
void before_objc_msgSend(id self, SEL _cmd, uintptr_t lr) {
    
}

/// objc_msgSend方法执行后
uintptr_t after_objc_msgSend() {
    return 0;
}

#pragma mark - core methods
/// 用于记录 objc_msgSend 方法调用前的时间，表示开始时
static inline void push_call_record(id _self, Class _cls, SEL _cmd, uintptr_t lr) {
    
}

/// 用于记录 objc_msgSend 方法调用后的时间，表示结束时
/// pop_call_record - push_call_record 相减能够得到方法的执行耗时
static inline uintptr_t pop_call_record() {
    return 0;
}

static void release_thread_call_stack(void *ptr) {
//    thread_call_stack *cs = (thread_call_stack *)ptr;
//    if (!cs) return;
//    if (cs->stack) free(cs->stack);
//    free(cs);
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
void start_monitor() {
//    _call_record_enabled = true;
//    static dispatch_once_t onceToken;
//    dispatch_once(&onceToken, ^{
//        pthread_key_create(&_thread_key, &release_thread_call_stack);
//        rebind_symbols((struct rebinding[6]){
//            {"objc_msgSend", (void *)hook_Objc_msgSend, (void **)&orig_objc_msgSend},
//        }, 1);
//    });
}

void end_monitor() {
//    _call_record_enabled = false;
}
