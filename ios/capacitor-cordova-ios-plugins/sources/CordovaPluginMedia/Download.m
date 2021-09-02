//
//  Download.m
//  又见正念
//
//  Created by long yang on 2019/10/23.
//

#import "Download.h"

@implementation DownloadResult

@end

@interface Download ()
{
    DownloadResultResponse _response;
    DownloadSpeed _speed;
    NSURL * _firmwareUrl;
}

@end

@implementation Download

+(Download *)sharedInstance
{
    static Download * _download=nil;
    static dispatch_once_t _once;
    dispatch_once(&_once, ^{
        // 2. 生成沙盒的路径
        NSString * path=[Download getMp3Path];
        NSString * dfuPath=[Download getDFUPath];

        NSFileManager * _manager=[NSFileManager defaultManager];
        NSError * _error;
        if(![_manager fileExistsAtPath:path]){
            [_manager createDirectoryAtPath:path withIntermediateDirectories:YES attributes:nil error:NULL];
        }
        if(![_manager fileExistsAtPath:dfuPath]){
            [_manager createDirectoryAtPath:dfuPath withIntermediateDirectories:YES attributes:nil error:NULL];
        }

        _download=[[Download alloc] init];
    });
    return _download;
}

+(NSString *)getMp3Path
{
    NSArray *docs = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *path = [docs[0] stringByAppendingPathComponent:@"mp3"];
    return path;
}

+(NSString *)getDFUPath
{
    NSArray *docs = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *path = [docs[0] stringByAppendingPathComponent:@"dfus"];
    return path;
}

-(BOOL)hasDownloadByUrl:(NSString *)downloadUrl
{
    NSString * _mp3Path=[Download getMp3Path];
    NSFileManager * _manager=[NSFileManager defaultManager];
    NSString * _needUrl=[NSString stringWithFormat:@"%@/%@",_mp3Path,[Download decoderUrlEncodeStr:[downloadUrl lastPathComponent]]];
    if([_manager fileExistsAtPath:_needUrl]){
        return YES;
    }
    return NO;
}

-(void)startDownloadByUrl:(NSString *)downloadUrl speed:(DownloadSpeed)speed response:(DownloadResultResponse)response
{
    NSURL * _url = [NSURL URLWithString:downloadUrl];
    NSURLSessionConfiguration * _configuration=[NSURLSessionConfiguration defaultSessionConfiguration];
    NSURLSession * _session = [NSURLSession sessionWithConfiguration:_configuration delegate:self delegateQueue:[NSOperationQueue mainQueue]];
    NSURLSessionDownloadTask * _downloadTask=[_session downloadTaskWithURL:_url];
    [_downloadTask resume];
    if(response){
        _response=[response copy];
    }
    if(speed){
        _speed=[speed copy];
    }
}

-(void)startDownloadByUrl:(NSString *)downloadUrl response:(DownloadResultResponse)response
{
    [self startDownloadByUrl:downloadUrl speed:^(int speed) {

    } response:response];
}

-(void)URLSession:(NSURLSession *)session downloadTask:(NSURLSessionDownloadTask *)downloadTask didFinishDownloadingToURL:(NSURL *)location
{
    NSString * _files=@"mp3";
    if([[[downloadTask.response.URL lastPathComponent] pathExtension] isEqualToString:@"zip"]){
        _files=@"dfus";
    }
    NSString *fileName = [NSString stringWithFormat:@"%@/%@",_files,[downloadTask.response.URL lastPathComponent]];
    // 2. 生成沙盒的路径
    NSArray *docs = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *path = [docs[0] stringByAppendingPathComponent:fileName];
    NSFileManager * _manager=[NSFileManager defaultManager];
    NSError * _error;
    if([_manager fileExistsAtPath:path]){
        if([_manager removeItemAtPath:path error:&_error]){
            NSLog(@"删除成功");
        } else {
            NSLog(@"删除失败:%@",[_error localizedDescription]);
        }
    }
    NSURL *toURL = [NSURL fileURLWithPath:path];
    [_manager moveItemAtURL:location toURL:toURL error:&_error];
    if(_response){
        if([_files isEqualToString:@"dfus"]){
            _firmwareUrl=toURL;
        }
        DownloadResult * _result=[DownloadResult new];
        _result.code=kDownloadResultCodeSuccess;
        _result.url=toURL;
        _response(_result);
    }
}

- (void)URLSession:(NSURLSession *)session downloadTask:(NSURLSessionDownloadTask *)downloadTask
                                           didWriteData:(int64_t)bytesWritten
                                      totalBytesWritten:(int64_t)totalBytesWritten
                              totalBytesExpectedToWrite:(int64_t)totalBytesExpectedToWrite
{
    int speed=100*(totalBytesWritten*1.0/totalBytesExpectedToWrite);
    if(_speed){
        _speed(speed);
    }
}

- (void)URLSession:(NSURLSession *)session task:(NSURLSessionTask *)task
                           didCompleteWithError:(nullable NSError *)error
{
    NSLog(@"下载文件失败:%@",[error localizedDescription]);
    if(error){
        if(_response){
            DownloadResult * _result=[DownloadResult new];
            _result.code=kDownloadResultCodeFailed;
            _result.error=error;
            _response(_result);
        }
    }
}

-(NSURL *)getFirmwareUrl
{
    return _firmwareUrl;
}

+(NSString *)urlEncodeStr:(NSString *)input{
    NSString *charactersToEscape = @"?!@#$^&%*+,:;='\"`<>()[]{}/\\| ";
    NSCharacterSet *allowedCharacters = [[NSCharacterSet characterSetWithCharactersInString:charactersToEscape] invertedSet];
    NSString *upSign = [input stringByAddingPercentEncodingWithAllowedCharacters:allowedCharacters];
    return upSign;
}

+ (NSString *)decoderUrlEncodeStr: (NSString *) input{
    NSMutableString *outputStr = [NSMutableString stringWithString:input];
    [outputStr replaceOccurrencesOfString:@"+" withString:@"" options:NSLiteralSearch range:NSMakeRange(0,[outputStr length])];
    return [outputStr stringByRemovingPercentEncoding];
}

@end
