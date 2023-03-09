//
//  SYMemoryMonitor.m
//  SYPerformanceMonitor
//
//  Created by csy on 2023/3/9.
//

#import "SYMemoryMonitor.h"
#import <mach/mach.h>
#import "SYWeakProxy.h"

@interface SYMemoryMonitor ()
/// 内存监听定时器
@property (nonatomic, strong) NSTimer *vmMonitorTimer;
@end

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
    // 1. 开启一个定时器每2秒去遍历每个线程
    self.vmMonitorTimer = [NSTimer scheduledTimerWithTimeInterval:2.0 target:[SYWeakProxy proxyWithTarget:self] selector:@selector(getMemoryInfo) userInfo:nil repeats:YES];
}

- (void)endMonitor {
    [self.vmMonitorTimer invalidate];
}

#pragma mark - private methods
- (void)getMemoryInfo {
    // 2. 通过phys_footprint字段获取到当前实际物理内存使用情况
    integer_t memoryUsage = [SYMemoryMonitor memoryUsage];
    NSLog(@"MemoryUsage over currentThread: %d", memoryUsage);
}

/**
 struct task_vm_info {
     mach_vm_size_t  virtual_size;                  // virtual memory size (bytes) 虚拟内存大小
     integer_t       region_count;                       // number of memory regions 内存区域数量
     integer_t       page_size;
     mach_vm_size_t  resident_size;              // resident memory size (bytes) 常驻内存大小
     mach_vm_size_t  resident_size_peak;    // peak resident size (bytes) 常驻内存峰值
     mach_vm_size_t  phys_footprint;            // added for rev1 物理内存大小
 };
 */
/// 获取实际物理内存的使用情况
+ (integer_t)memoryUsage {
    // 1. 创建一个task_vm_info结构体
    task_vm_info_data_t vmInfo;
    // 2. 初始化线程数量
    mach_msg_type_number_t count = TASK_VM_INFO_COUNT;
    // 3. 根据当前task获取所有线程
    kern_return_t result = task_info(mach_task_self(), TASK_VM_INFO, (task_info_t) &vmInfo, &count);
    if (result != KERN_SUCCESS) return 0;
    // 4. 初始化物理内存使用
    integer_t physFootprint = 0;
    physFootprint = (integer_t) vmInfo.phys_footprint;
    return physFootprint;
}
@end
