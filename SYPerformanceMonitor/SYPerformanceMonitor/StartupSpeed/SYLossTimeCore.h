//
//  SYLossTimeCore.h
//  SYPerformanceMonitor
//
//  Created by csy on 2023/3/10.
//

#ifndef SYLossTimeCore_h
#define SYLossTimeCore_h

#include <stdio.h>
#include <objc/objc.h>

typedef struct {
    __unsafe_unretained Class cls;
    SEL sel;
    uint64_t time; // us (1/1000 ms)
    int depth;
} call_record;

/// 开始监听
extern void start_monitor();
/// 结束监听
extern void end_monitor();
#endif /* SYLossTimeCore_h */
