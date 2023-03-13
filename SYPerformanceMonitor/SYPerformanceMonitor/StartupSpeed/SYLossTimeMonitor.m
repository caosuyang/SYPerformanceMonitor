//
//  SYLossTimeMonitor.m
//  SYPerformanceMonitor
//
//  Created by 曹素洋 on 2023/3/9.
//

#import "SYLossTimeMonitor.h"
#import "SYLossTimeCore.h"

@implementation SYLossTimeMonitor

#pragma mark - public methods
- (void)startMonitor {
    start_monitor();
}

- (void)endMonitor {
    end_monitor();
}
@end
