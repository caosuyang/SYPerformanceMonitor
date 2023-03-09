//
//  SYMemoryMonitor.h
//  SYPerformanceMonitor
//
//  Created by csy on 2023/3/9.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/// 内存使用监控器
@interface SYMemoryMonitor : NSObject
/// 初始化一个CPU监听器
+ (instancetype)shareInstance;

/// 开始监听
- (void)startMonitor;
/// 结束监听
- (void)endMonitor;
@end

NS_ASSUME_NONNULL_END
