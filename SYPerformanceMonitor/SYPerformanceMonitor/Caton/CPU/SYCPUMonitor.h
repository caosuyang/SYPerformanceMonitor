//
//  SYCPUMonitor.h
//  SYPerformanceMonitor
//
//  Created by csy on 2023/3/9.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/// CPU monitor.
@interface SYCPUMonitor : NSObject
+ (instancetype)shareInstance;

- (void)startMonitor;
- (void)endMonitor;
@end

NS_ASSUME_NONNULL_END
