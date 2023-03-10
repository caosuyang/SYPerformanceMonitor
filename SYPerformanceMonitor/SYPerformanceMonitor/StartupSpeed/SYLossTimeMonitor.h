//
//  SYLossTimeMonitor.h
//  SYPerformanceMonitor
//
//  Created by 曹素洋 on 2023/3/9.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/// 耗时检查工具
@interface SYLossTimeMonitor : NSObject
/// 初始化一个监听器
+ (instancetype)shareInstance;

/// 开始监听
- (void)startMonitor;
/// 结束监听
- (void)endMonitor;
@end

NS_ASSUME_NONNULL_END
