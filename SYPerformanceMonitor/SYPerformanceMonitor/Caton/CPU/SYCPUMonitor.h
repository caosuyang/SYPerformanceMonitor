//
//  SYCPUMonitor.h
//  SYPerformanceMonitor
//
//  Created by csy on 2023/3/9.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/// CPU监听器
@interface SYCPUMonitor : NSObject
/// 初始化一个CPU监听器
+ (instancetype)shareInstance;

/// 开始监听
- (void)startMonitor;
/// 结束监听
- (void)endMonitor;
@end

NS_ASSUME_NONNULL_END
