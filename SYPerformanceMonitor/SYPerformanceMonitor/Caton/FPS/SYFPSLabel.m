//
//  SYFPSLabel.m
//  SYPerformanceMonitor
//
//  Created by csy on 2023/3/8.
//  Copyright © 2023 caosy. All rights reserved.
//

#import "SYFPSLabel.h"
#import <QuartzCore/QuartzCore.h>
#import "SYWeakProxy.h"

@interface SYFPSLabel ()
/// CADisplayLink object.
@property (nonatomic, strong) CADisplayLink *link;
/// Last render time.
@property (nonatomic, assign) NSTimeInterval lastTimestamp;
/// Number of synchronous screen refreshes.
@property (nonatomic, assign) NSUInteger count;
@end

@implementation SYFPSLabel

#pragma mark - init
- (instancetype)initWithFrame:(CGRect)frame {
    if (frame.size.width == 0 && frame.size.height == 0) {
        frame.size = CGSizeMake(60.0, 20.0);
    }
    self = [super initWithFrame:frame];
    if (self) {
        [self setupProperties];
        [self initDisplayLink];
    }
    return self;
}

#pragma mark - life cycle
- (void)dealloc {
    [_link invalidate];
}

#pragma mark - event response
/// Method executes at the same frame rate as screen refresh rate.
- (void)fpsCount:(CADisplayLink *)link {
    if (_lastTimestamp == 0) {
        _lastTimestamp = link.timestamp;
    } else {
        _count++;
        NSTimeInterval curTimestamp = link.timestamp;
        // The difference between the start rendering time and the last rendering time.
        NSTimeInterval timeDiff = curTimestamp - _lastTimestamp;
        if (timeDiff < 1) return;
        _lastTimestamp = curTimestamp;
        // Calculate fps.
        float fps = _count / timeDiff;
        _count = 0;
        
        self.text = [NSString stringWithFormat:@"%.0f FPS", round(fps)];
    }
}

#pragma mark - public methods

#pragma mark - private methods
- (void)setupProperties {
    self.backgroundColor = [UIColor lightGrayColor];
    self.font = [UIFont systemFontOfSize:15];
    self.textColor = [UIColor redColor];
    self.textAlignment = NSTextAlignmentCenter;
    self.adjustsFontSizeToFitWidth = YES;
}

- (void)initDisplayLink {
    // There must be `self.link`, not `_link`.
    [self.link addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSRunLoopCommonModes];
}

#pragma mark - getters and setters
- (CADisplayLink *)link {
    if (_link == nil) {
        _link = [CADisplayLink displayLinkWithTarget:[SYWeakProxy proxyWithTarget:self] selector:@selector(fpsCount:)];
    }
    return _link;
}
@end
