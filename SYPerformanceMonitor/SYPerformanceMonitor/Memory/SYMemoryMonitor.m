//
//  SYMemoryMonitor.m
//  SYPerformanceMonitor
//
//  Created by csy on 2023/3/9.
//

#import "SYMemoryMonitor.h"

@implementation SYMemoryMonitor
#pragma mark - init
+ (instancetype)shareInstance {
    static SYMemoryMonitor *instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[self alloc] init];
    });
    return instance;
}

#pragma mark - public methods
- (void)startMonitor {

}

- (void)endMonitor {

}
@end
