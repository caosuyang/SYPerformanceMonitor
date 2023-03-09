//
//  SYCPUMonitor.m
//  SYPerformanceMonitor
//
//  Created by csy on 2023/3/9.
//

#import "SYCPUMonitor.h"
#import "SYWeakProxy.h"
#import <mach/mach.h>

@interface SYCPUMonitor ()
/// cpu监听定时器
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
    // 1. 开启一个定时器每2秒去遍历每个线程
    self.cpuMonitorTimer = [NSTimer scheduledTimerWithTimeInterval:2.0 target:[SYWeakProxy proxyWithTarget:self] selector:@selector(getCpuInfo) userInfo:nil repeats:YES];
}

- (void)endMonitor {
    [self.cpuMonitorTimer invalidate];
}

#pragma mark - private methods
- (void)getCpuInfo {
    // 2. 累加cpuUsage字段获取到当前的整体CPU使用率
    integer_t cpuUsage = [SYCPUMonitor cpuUsage];
    NSLog(@"CpuUsage over currentThread: %d", cpuUsage);
}


/**
 在usr/include/mach/thread_info.h中，可以看到线程基本信息的结构体thread_basic_info_t
 struct thread_basic_info {
     time_value_t    user_time;         // user run time 用户运行时长
     time_value_t    system_time;    // system run time 系统运行时长
     integer_t       cpu_usage;          // scaled cpu usage percentage cpu使用率
     policy_t        policy;                   // scheduling policy in effect 调度策略
     integer_t       run_state;             // run state (see below) 运行状态
     integer_t       flags;                    // various flags (see below) 标记
     integer_t       suspend_count;   // suspend count for thread 暂停线程的数量
     integer_t       sleep_time;          // number of seconds that thread has been sleeping 线程休眠时间s
 };
 */
/// 获取当前 App 所在进程的 CPU 使用率
+ (integer_t)cpuUsage {
    // 1. 创建一个int类型的threads数组
    thread_act_array_t threads;
    // 2. 初始化线程数量，int类型，默认为0
    mach_msg_type_number_t threadCount = 0;
    // 3. 获取当前task
    const task_t thisTask = mach_task_self();
    // 4. 根据当前task获取所有线程
    // task_threads方法能够取到当前进程中的线程总数threadCount和所有线程的数组threads
    kern_return_t kr = task_threads(thisTask, &threads, &threadCount);
    
    if (kr != KERN_SUCCESS) return 0;

    // 5. 初始化cpu使用率
    integer_t cpuUsage = 0;
    // 6. 遍历所有线程来获取单个线程的基本信息
    for (int i = 0; i < threadCount; i++) {
        
        thread_info_data_t threadInfo;
        // 7. 初始化线程基本信息的结构体
        thread_basic_info_t threadBasicInfo;
        mach_msg_type_number_t threadInfoCount = THREAD_INFO_MAX;
        
        if (thread_info((thread_inspect_t) threads[i], THREAD_BASIC_INFO, (thread_info_t) threadInfo, &threadInfoCount) == KERN_SUCCESS) {
            
            // 8. 获取线程基本信息
            threadBasicInfo = (thread_basic_info_t) threadInfo;
            if (!(threadBasicInfo->flags & TH_FLAGS_IDLE)) {
                
                // 9. 获取cpu使用率
                // 累加每个线程cpu_usage字段值
                cpuUsage += threadBasicInfo->cpu_usage;
                // 10. 如果cpu消耗大于70 * 10
                if (cpuUsage > 70 * 10) {
                    // 11. 打印和记录堆栈
                    // todo: 打印和记录堆栈
                }
            }
        }
    }
    
    assert(vm_deallocate(mach_task_self(), (vm_address_t) threads, threadCount * sizeof(thread_t)) == KERN_SUCCESS);
    // 当前app所在进程cpu使用率
    return cpuUsage;
}
@end
