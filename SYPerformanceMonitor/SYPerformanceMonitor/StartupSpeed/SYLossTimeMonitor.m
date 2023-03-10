//
//  SYLossTimeMonitor.m
//  SYPerformanceMonitor
//
//  Created by 曹素洋 on 2023/3/9.
//

#import "SYLossTimeMonitor.h"

@implementation SYLossTimeMonitor
#pragma mark - init
+ (instancetype)shareInstance {
    static SYLossTimeMonitor *instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[self alloc] init];
    });
    return instance;
}

#pragma mark - public methods
- (void)startMonitor {

}

- (void)endMonitor {

}
@end
