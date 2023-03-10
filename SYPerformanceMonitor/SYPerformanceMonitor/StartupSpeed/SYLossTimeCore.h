//
//  SYLossTimeCore.h
//  SYPerformanceMonitor
//
//  Created by csy on 2023/3/10.
//

#ifndef SYLossTimeCore_h
#define SYLossTimeCore_h

#include <stdio.h>

typedef struct {
    __unsafe_unretained Class cls;
    SEL sel;
    uint64_t time; // us (1/1000 ms)
    int depth;
} call_record;

extern void start_monitor();
extern void end_monitor();
#endif /* SYLossTimeCore_h */
