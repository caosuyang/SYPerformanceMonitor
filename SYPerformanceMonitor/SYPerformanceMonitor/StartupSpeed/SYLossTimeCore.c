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
static bool _call_record_enabled = true;
static pthread_key_t _thread_key;
__unused static id (*orig_objc_msgSend)(id, SEL, ...);
static uint64_t _min_time_cost = 1000; //us
static int _max_call_depth = 3;

static call_record *_callRecords;
//static int otp_record_num;
//static int otp_record_alloc;
static int _recordNum;
static int _recordAlloc;

#pragma mark - struct
// 1. 设计结构体 CallRecord 记录调用方法详细信息，包括 obj 和 SEL 等
typedef struct {
    id self;   // 通过 object_getClass 能够得到 Class 再通过 NSStringFromClass 能够得到类名
    Class cls;
    SEL cmd; // 通过 NSStringFromSelector 方法能够得到方法名
    uintptr_t lr;
    //  int prevHitIndex;
    //  char isWatchHit;
    uint64_t time; //us
} thread_call_record;

// 2. 设计结构体 ThreadCallStack，需要用 index 记录当前调用方法树的深度
typedef struct {
//    FILE *file;
//    char *spacesStr;
    thread_call_record *stack;
    int allocated_length;
    int index; //index 记录当前调用方法树的深度
//    int numWatchHits;
//    int lastPrintedIndex;
//    int lastHitIndex;
//    char isLoggingEnabled;
//    char isCompleteLoggingEnabled;
    bool is_main_thread;
} thread_call_stack;


//// 3. pthread_setspecific() 可以将私有数据设置在指定线程上，pthread_getspecific() 用来读取这个私有数据。利用这个特性，我们就可以将 ThreadCallStack 的数据和该线程绑定在一起，随时进行数据存取
//static inline ThreadCallStack * getThreadCallStack() {
//    ThreadCallStack *cs = (ThreadCallStack *)pthread_getspecific(threadKey); // 读取
//    if (cs == NULL) {
//        cs = (ThreadCallStack *)malloc(sizeof(ThreadCallStack));
//#ifdef MAIN_THREAD_ONLY
//        cs->file = (pthread_main_np()) ? newFileForThread() : NULL;
//#else
//        //    cs->file = newFileForThread();
//#endif
//        cs->isLoggingEnabled = (cs->file != NULL);
//        cs->isCompleteLoggingEnabled = 0;
//        //    cs->spacesStr = (char *)malloc(DEFAULT_CALLSTACK_DEPTH + 1);
//        //    memset(cs->spacesStr, ' ', DEFAULT_CALLSTACK_DEPTH);
//        //    cs->spacesStr[DEFAULT_CALLSTACK_DEPTH] = '\0';
//        //    cs->stack = (CallRecord *)calloc(DEFAULT_CALLSTACK_DEPTH, sizeof(CallRecord)); // 分配 CallRecord 默认空间
//        //    cs->allocatedLength = DEFAULT_CALLSTACK_DEPTH;
//        cs->index = cs->lastPrintedIndex = cs->lastHitIndex = -1;
//        cs->numWatchHits = 0;
//        pthread_setspecific(threadKey, cs); // 保存数据
//    }
//    return cs;
//}
//
//// 4. 因为要记录深度，而一个方法的调用里会有更多的方法调用，所以我们可以在方法的调用里增加两个方法 pushCallRecord 和 popCallRecord，分别记录方法调用的开始时间和结束时间，这样才能够在开始时对深度加一、在结束时减一
//// 开始时
//static inline void pushCallRecord(id obj, uintptr_t lr, SEL _cmd, ThreadCallStack *cs) {
//    int nextIndex = (++cs->index); // 增加深度
//    if (nextIndex >= cs->allocatedLength) {
//        //    cs->allocatedLength += CALLSTACK_DEPTH_INCREMENT;
//        cs->stack = (CallRecord *)realloc(cs->stack, cs->allocatedLength * sizeof(CallRecord));
//        cs->spacesStr = (char *)realloc(cs->spacesStr, cs->allocatedLength + 1);
//        memset(cs->spacesStr, ' ', cs->allocatedLength);
//        cs->spacesStr[cs->allocatedLength] = '\0';
//    }
//    CallRecord *newRecord = &cs->stack[nextIndex];
//    newRecord->obj = obj;
//    newRecord->_cmd = _cmd;
//    newRecord->lr = lr;
//    newRecord->isWatchHit = 0;
//}
//// 结束时
//static inline CallRecord * popCallRecord(ThreadCallStack *cs) {
//    return &cs->stack[cs->index--]; // 减少深度
//}
//
//#pragma mark - public methods
//void start_monitor() {
//    callRecordEnabled = true;
//    //    static dispatch_once_t onceToken;
//    //    dispatch_once(&onceToken, ^{
//    //        (&threadKey, &releaseThreadCallStack);
//    //        rebind_symbols((struct rebinding[6]){
//    //            {"objc_msgSend", (void *)hookObjcMsgSend, (void **)&origObjcMsgSend},
//    //        }, 1);
//    //    });
//}
//
//void end_monitor() {
//    callRecordEnabled = false;
//}
//
////#pragma mark - private methods
//static void releaseThreadCallStack(void *ptr) {
//    ThreadCallStack *cs = (ThreadCallStack *)ptr;
//    if (!cs) return;
//    if (cs->stack) free(cs->stack);
//    free(cs);
//}
//
//void beforeObjcMsgSend(id self, SEL _cmd, uintptr_t lr) {
//    pushCallRecord(self, object_getClass(self), _cmd, lr);
//}
//
//uintptr_t afterObjcMsgSend() {
//    //    return popCallRecord();
//    return 1;
//}
//
////static inline uintptr_t popCallRecord() {
////    ThreadCallStack *cs = getThreadCallStack();
////    int curIndex = cs->index;
////    int nextIndex = cs->index--;
////    CallRecord *pRecord = &cs->stack[nextIndex];
////
////    if (cs->is_main_thread && _call_record_enabled) {
////        struct timeval now;
////        gettimeofday(&now, NULL);
////        uint64_t time = (now.tv_sec % 100) * 1000000 + now.tv_usec;
////        if (time < pRecord->time) {
////            time += 100 * 1000000;
////        }
////        uint64_t cost = time - pRecord->time;
////        if (cost > _min_time_cost && cs->index < _max_call_depth) {
////            if (!_smCallRecords) {
////                _smRecordAlloc = 1024;
////                _smCallRecords = malloc(sizeof(smCallRecord) * _smRecordAlloc);
////            }
////            _smRecordNum++;
////            if (_smRecordNum >= _smRecordAlloc) {
////                _smRecordAlloc += 1024;
////                _smCallRecords = realloc(_smCallRecords, sizeof(smCallRecord) * _smRecordAlloc);
////            }
////            smCallRecord *log = &_smCallRecords[_smRecordNum - 1];
////            log->cls = pRecord->cls;
////            log->depth = curIndex;
////            log->sel = pRecord->cmd;
////            log->time = cost;
////        }
////    }
////    return pRecord->lr;
////}
//
////replacement objc_msgSend (arm64)
//// https://blog.nelhage.com/2010/10/amd64-and-va_arg/
//// http://infocenter.arm.com/help/topic/com.arm.doc.ihi0055b/IHI0055B_aapcs64.pdf
//// https://developer.apple.com/library/ios/documentation/Xcode/Conceptual/iPhoneOSABIReference/Articles/ARM64FunctionCallingConventions.html
//#define call(b, value) \
//__asm volatile ("stp x8, x9, [sp, #-16]!\n"); \
//__asm volatile ("mov x12, %0\n" :: "r"(value)); \
//__asm volatile ("ldp x8, x9, [sp], #16\n"); \
//__asm volatile (#b " x12\n");
//
//#define save() \
//__asm volatile ( \
//"stp x8, x9, [sp, #-16]!\n" \
//"stp x6, x7, [sp, #-16]!\n" \
//"stp x4, x5, [sp, #-16]!\n" \
//"stp x2, x3, [sp, #-16]!\n" \
//"stp x0, x1, [sp, #-16]!\n");
//
//#define load() \
//__asm volatile ( \
//"ldp x0, x1, [sp], #16\n" \
//"ldp x2, x3, [sp], #16\n" \
//"ldp x4, x5, [sp], #16\n" \
//"ldp x6, x7, [sp], #16\n" \
//"ldp x8, x9, [sp], #16\n" );
//
//#define link(b, value) \
//__asm volatile ("stp x8, lr, [sp, #-16]!\n"); \
//__asm volatile ("sub sp, sp, #16\n"); \
//call(b, value); \
//__asm volatile ("add sp, sp, #16\n"); \
//__asm volatile ("ldp x8, lr, [sp], #16\n");
//
//#define ret() __asm volatile ("ret\n");
//
//__attribute__((__naked__))
//static void hookObjcMsgSend() {
//    // Save parameters.
//    save()
//
//    __asm volatile ("mov x2, lr\n");
//    __asm volatile ("mov x3, x4\n");
//
//    // Call our before_objc_msgSend.
//    call(blr, &beforeObjcMsgSend)
//
//    // Load parameters.
//    load()
//
//    // Call through to the original objc_msgSend.
//    call(blr, origObjcMsgSend)
//
//    // Save original objc_msgSend return value.
//    save()
//
//    // Call our after_objc_msgSend.
//    call(blr, &afterObjcMsgSend)
//
//    // restore lr
//    __asm volatile ("mov lr, x0\n");
//
//    // Load original objc_msgSend return value.
//    load()
//
//    // return
//    ret()
//}
