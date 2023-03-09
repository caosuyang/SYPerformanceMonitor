//
//  SYWeakProxy.h
//  SYPerformanceMonitor
//
//  Created by csy on 2023/3/8.
//  Copyright © 2023 caosy. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/// Use Proxy to solve the problem of circular references between CADisplayLink/NSTimer and Target.
@interface SYWeakProxy : NSProxy
/// Generate a weak reference proxy object based on target object.
/// - Parameter target: Target object.
+ (instancetype)proxyWithTarget:(id)target;
@end

NS_ASSUME_NONNULL_END
