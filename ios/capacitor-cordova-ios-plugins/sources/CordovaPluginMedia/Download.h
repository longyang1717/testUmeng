//
//  Download.h
//  又见正念
//
//  Created by long yang on 2019/10/23.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

enum DownloadResultCode{
    kDownloadResultCodeSuccess=0,
    kDownloadResultCodeFailed,
};
typedef NSInteger DownloadResultCode;

@interface DownloadResult : NSObject

@property (nonatomic) DownloadResultCode code;
@property (nonatomic,copy) NSURL * url;
@property (nonatomic,copy) NSError * error;

@end

typedef void(^DownloadSpeed)(int);
typedef void(^DownloadResultResponse)(DownloadResult *);

@interface Download : NSObject<NSURLSessionDelegate,NSURLSessionTaskDelegate>

+(Download *)sharedInstance;

+(NSString *)getMp3Path;

+(NSString *)getDFUPath;

-(BOOL)hasDownloadByUrl:(NSString *)downloadUrl;

-(void)startDownloadByUrl:(NSString *)downloadUrl response:(DownloadResultResponse)response;

-(void)startDownloadByUrl:(NSString *)downloadUrl speed:(DownloadSpeed)speed response:(DownloadResultResponse)response;

-(NSURL *)getFirmwareUrl;

@end

NS_ASSUME_NONNULL_END
