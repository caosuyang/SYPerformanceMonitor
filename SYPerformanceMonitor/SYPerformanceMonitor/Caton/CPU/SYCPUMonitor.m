//
//  SYCPUMonitor.m
//  SYPerformanceMonitor
//
//  Created by csy on 2023/3/9.
//

#import "SYCPUMonitor.h"
#import "SYWeakProxy.h"

@interface SYCPUMonitor ()
@property (nonatomic, strong) NSTimer *cpuMonitorTimer;
@end

@implementation SYCPUMonitor

#pragma mark - init
+ (instancetype)shareInstance {
    static SYCPUMonitor *instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[self alloc] init];
    });
    return instance;
}

#pragma mark - public methods
- (void)startMonitor {
    // 1. 开启一个定时器，用于检测 CPU 消耗
    self.cpuMonitorTimer = [NSTimer scheduledTimerWithTimeInterval:2.0 target:[SYWeakProxy proxyWithTarget:self] selector:@selector(getCpuInfo) userInfo:nil repeats:YES];
}

- (void)endMonitor {
    [self.cpuMonitorTimer invalidate];
}

#pragma mark - private methods
- (void)getCpuInfo {
    
}
@end
