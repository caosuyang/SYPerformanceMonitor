//
//  SYRunLoopMonitor.m
//  SYPerformanceMonitor
//
//  Created by csy on 2023/3/9.
//

#import "SYRunLoopMonitor.h"
#import "SYWeakProxy.h"

@interface SYRunLoopMonitor () {
    /// dispatch_semaphore，用于线程同步
    dispatch_semaphore_t semaphore;
    /// 观察者runLoopObs
    CFRunLoopObserverRef runLoopObs;
    /// 主线程RunLoop活动状态obsActivity
    CFRunLoopActivity obsActivity;
}
@end

@implementation SYRunLoopMonitor

#pragma mark - init
+ (instancetype)shareInstance {
    static SYRunLoopMonitor *instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[self alloc] init];
    });
    return instance;
}

#pragma mark - public methods
- (void)startMonitor {
    if (runLoopObs) return;
    // 1. 创建一个dispatch_semaphore，用于线程同步
    semaphore = dispatch_semaphore_create(0);
    // 2. 创建一个CFRunLoopObserverContext观察者
    CFRunLoopObserverContext cxt = {0, (__bridge void *)(self), NULL, NULL};
    runLoopObs = CFRunLoopObserverCreate(kCFAllocatorDefault, kCFRunLoopAllActivities, YES, 0, &runLoopObserverCallBack, &cxt);
    // 3. 将创建好的观察者runLoopObs添加到主线程kCFRunLoopCommonModes模式下观察
    CFRunLoopAddObserver(CFRunLoopGetMain(), runLoopObs, kCFRunLoopCommonModes);
    // 4. 创建一个子线程用来监控主线程RunLoop活动状态obsActivity
    dispatch_async(dispatch_get_global_queue(0, 0), ^{
        // 5. 子线程开启一个持续的loop用来进行监控
        while (YES) {
            // 6. 设置触发卡顿的时间阈值NSEC_PER_SEC * 3，即3秒
            long wait = dispatch_semaphore_wait(self->semaphore, dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_SEC * 3));
            // 7. 判断是否发生超时
            if (wait != 0) {
                if (!self->runLoopObs) {
                    self->semaphore = 0;
                    self->obsActivity = 0;
                    return;
                }
                // 8. 判断是否处于进入睡眠前kCFRunLoopBeforeSources状态，或者唤醒后kCFRunLoopAfterWaiting状态
                if (self->obsActivity == kCFRunLoopBeforeSources || self->obsActivity == kCFRunLoopAfterWaiting) {
                    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^{
                        // 9. dump 出堆栈的信息
                        // todo: dump 出堆栈的信息
                    });
                }
            }
        }
    });
}

- (void)endMonitor {
    if (!runLoopObs) return;
    // 1. 将创建好的观察者runLoopObs从主线程kCFRunLoopCommonModes模式下移出观察
    CFRunLoopRemoveObserver(CFRunLoopGetMain(), runLoopObs, kCFRunLoopCommonModes);
    // 2. 将创建好的观察者runLoopObs释放
    CFRelease(runLoopObs);
    // 3. 将创建好的观察者runLoopObs置为空值
    runLoopObs = NULL;
}

#pragma mark - private methods
/// RunLoop监听回调
/// - Parameters:
///   - observer: 观察者runLoopObs
///   - activity: 主线程RunLoop活动状态obsActivity
///   - info: RunLoop监听器对象
static void runLoopObserverCallBack(CFRunLoopObserverRef observer, CFRunLoopActivity activity, void *info) {
    SYRunLoopMonitor *monitor = (__bridge SYRunLoopMonitor *)(info);
    monitor->obsActivity = activity;
    
    dispatch_semaphore_t semaphore = monitor->semaphore;
    dispatch_semaphore_signal(semaphore);
}
@end
