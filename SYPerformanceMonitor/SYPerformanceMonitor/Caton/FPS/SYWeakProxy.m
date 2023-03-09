//
//  SYWeakProxy.m
//  SYPerformanceMonitor
//
//  Created by csy on 2023/3/8.
//  Copyright © 2023 caosy. All rights reserved.
//

#import "SYWeakProxy.h"

@interface SYWeakProxy ()
/// Target object.
@property (nonatomic, weak) id target;
@end

@implementation SYWeakProxy

#pragma mark - init
- (instancetype)initWithTarget:(id)target {
    _target = target;
    return self;
}

#pragma mark - public methods
+ (instancetype)proxyWithTarget:(id)target {
    SYWeakProxy *proxy = [[SYWeakProxy alloc] initWithTarget:target];
    return proxy;
}

#pragma mark - override
- (NSMethodSignature *)methodSignatureForSelector:(SEL)sel {
    return [_target methodSignatureForSelector:sel];
}

- (void)forwardInvocation:(NSInvocation *)invocation {
    [invocation invokeWithTarget:_target];
}
@end
