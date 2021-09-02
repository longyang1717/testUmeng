//
//  BLECentralPlugin.m
//  BLE Central Cordova Plugin
//
//  (c) 2104-2018 Don Coleman
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#define LEN_FILTER 29

#import "BLECentralPlugin.h"
#import <Cordova/CDV.h>
#include "alg_algorithm.h"
#import <CommonCrypto/CommonDigest.h>

@interface BLECentralPlugin() {
    NSDictionary *bluetoothStates;

    NSFileHandle * _handle;
    NSMutableDictionary * _allDict;
    NSMutableDictionary * _indexArray;
    BOOL _isWriting;
}
- (CBPeripheral *)findPeripheralByUUID:(NSString *)uuid;
- (void)stopScanTimer:(NSTimer *)timer;
@end

@implementation BLECentralPlugin

@synthesize manager;
@synthesize peripherals;

- (void)pluginInitialize {
    NSLog(@"Cordova BLE Central Plugin");
    NSLog(@"(c)2014-2016 Don Coleman");

    [super pluginInitialize];

    peripherals = [NSMutableSet new];
    _allDict=[NSMutableDictionary new];
    _indexArray=[NSMutableDictionary new];
    manager = [[CBCentralManager alloc] initWithDelegate:self queue:nil options:@{CBCentralManagerOptionShowPowerAlertKey: @NO}];

    connectCallbacks = [NSMutableDictionary new];
    connectCallbackLatches = [NSMutableDictionary new];
    readCallbacks = [NSMutableDictionary new];
    writeCallbacks = [NSMutableDictionary new];
    notificationCallbacks = [NSMutableDictionary new];
    startNotificationCallbacks = [NSMutableDictionary new];
    stopNotificationCallbacks = [NSMutableDictionary new];
    bluetoothStates = [NSDictionary dictionaryWithObjectsAndKeys:
                       @"unknown", @(CBCentralManagerStateUnknown),
                       @"resetting", @(CBCentralManagerStateResetting),
                       @"unsupported", @(CBCentralManagerStateUnsupported),
                       @"unauthorized", @(CBCentralManagerStateUnauthorized),
                       @"off", @(CBCentralManagerStatePoweredOff),
                       @"on", @(CBCentralManagerStatePoweredOn),
                       nil];
    readRSSICallbacks = [NSMutableDictionary new];

    NSString * _path=[NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) lastObject];
    NSString * _fileName=[_path stringByAppendingPathComponent:@"datas"];
    NSFileManager * _manager=[NSFileManager defaultManager];
    if(![_manager fileExistsAtPath:_fileName]){
        [_manager createDirectoryAtPath:_fileName withIntermediateDirectories:YES attributes:nil error:nil];
    }
    
    _isWriting=NO;
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(startWriteData) name:@"startWriteData" object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(pauseWriteData) name:@"pauseWriteData" object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(stopWriteData) name:@"stopWriteData" object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(pauseWriteData) name:@"resumeWriteData" object:nil];

    initAll();
}

-(void)startWriteData
{
    [self createFileHandler];
}

-(void)resumeWriteData
{
    _isWriting=YES;
}

-(void)pauseWriteData
{
    _isWriting=!_isWriting;
}

-(void)stopWriteData
{
    if(_handle!=nil){
        [_handle closeFile];
    }
    _handle=nil;
}

-(void)createFileHandler
{
    NSUserDefaults * _userDefaults=[NSUserDefaults standardUserDefaults];
    NSString * _path=[NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) lastObject];
    NSString * _trainingId=[_userDefaults objectForKey:@"CurrentTrainingId"];
    NSString * _username=[_userDefaults objectForKey:@"CurrentUserId"];
    
    NSString * _version=[_userDefaults objectForKey:@"version"];
    if(_version==NULL){
        _version=@"0.2.1";
    }

    NSString * _fileName=[_path stringByAppendingPathComponent:@"datas"];
    NSFileManager * _manager=[NSFileManager defaultManager];
    if(![_manager fileExistsAtPath:_fileName]){
        [_manager createDirectoryAtPath:_fileName withIntermediateDirectories:YES attributes:nil error:nil];
    }
    
    _fileName=[NSString stringWithFormat:@"%@/%@.csv",_fileName,_trainingId];

    if(![_manager fileExistsAtPath:_fileName]){
        [_manager createFileAtPath:_fileName contents:nil attributes:nil];
    } else {
        _handle=[NSFileHandle fileHandleForUpdatingAtPath:_fileName];
        [_handle seekToEndOfFile];
    }
    if(!_handle){
        _handle=[NSFileHandle fileHandleForWritingAtPath:_fileName];
        NSString * _string=[NSString stringWithFormat:@"%%username=%@\n%%version=%@\n%%fs=250\ntimestamp,AF7,AF8,leadoff,leadL,leadR,ticker,index,sn\n",[self md5:_username],_version];
        @try {
            [_handle writeData:[_string dataUsingEncoding:NSUTF8StringEncoding]];
        } @catch (NSException *exception) {
            UIAlertController * _controller=[UIAlertController alertControllerWithTitle:@"提示" message:@"写入数据出错!!!" preferredStyle:UIAlertControllerStyleAlert];
            [_controller addAction:[UIAlertAction actionWithTitle:@"确定" style:UIAlertActionStyleDefault handler:^(UIAlertAction * action){

            }]];
            [self.viewController presentViewController:_controller animated:YES completion:NULL];
        } @finally {

        }
        [_handle seekToEndOfFile];
    }
}

- (NSString *)md5:(NSString *)string {
    const char *cStr = [string UTF8String];
    unsigned char digest[CC_MD5_DIGEST_LENGTH];
    CC_MD5(cStr, (CC_LONG)strlen(cStr), digest);
    NSMutableString *result = [NSMutableString stringWithCapacity:CC_MD5_DIGEST_LENGTH * 2];
    for (int i = 0; i < CC_MD5_DIGEST_LENGTH; i++) {
        [result appendFormat:@"%02X", digest[i]];
    }
    return result;
}

#pragma mark - Cordova Plugin Methods

// TODO add timeout
- (void)connect:(CDVInvokedUrlCommand *)command {
    NSLog(@"connect");
    NSString *uuid = [command argumentAtIndex:0];

    CBPeripheral *peripheral = [self findPeripheralByUUID:uuid];

    if (peripheral) {
        NSLog(@"Connecting to peripheral with UUID : %@", uuid);

        [connectCallbacks setObject:[command.callbackId copy] forKey:[peripheral uuidAsString]];
        [manager connectPeripheral:peripheral options:nil];
    } else {
        NSString *error = [NSString stringWithFormat:@"Could not find peripheral %@.", uuid];
        NSLog(@"%@", error);
        CDVPluginResult *pluginResult = nil;
        pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_ERROR messageAsString:error];
        [self.commandDelegate sendPluginResult:pluginResult callbackId:command.callbackId];
    }

}

// This works different than Android. iOS needs to know about the peripheral UUID
// If not scanning, try connectedPeripheralsWIthServices or peripheralsWithIdentifiers
- (void)autoConnect:(CDVInvokedUrlCommand *)command {
    NSLog(@"autoConnect");
    NSString *uuid = [command argumentAtIndex:0];

    CBPeripheral *peripheral = [self findPeripheralByUUID:uuid];

    if (peripheral) {
        NSLog(@"Autoconnecting to peripheral with UUID : %@", uuid);

        [connectCallbacks setObject:[command.callbackId copy] forKey:[peripheral uuidAsString]];
        [manager connectPeripheral:peripheral options:nil];
    } else {
        NSString *error = [NSString stringWithFormat:@"Could not find peripheral %@.", uuid];
        NSLog(@"%@", error);
        CDVPluginResult *pluginResult = nil;
        pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_ERROR messageAsString:error];
        [self.commandDelegate sendPluginResult:pluginResult callbackId:command.callbackId];
    }

}

// disconnect: function (device_id, success, failure) {
- (void)disconnect:(CDVInvokedUrlCommand*)command {
    NSLog(@"disconnect");

    NSString *uuid = [command argumentAtIndex:0];
    CBPeripheral *peripheral = [self findPeripheralByUUID:uuid];

    if (!peripheral) {
        NSString *message = [NSString stringWithFormat:@"Peripheral %@ not found", uuid];
        NSLog(@"%@", message);
        CDVPluginResult *pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_ERROR messageAsString:message];
        [self.commandDelegate sendPluginResult:pluginResult callbackId:command.callbackId];

    } else {

        [connectCallbacks removeObjectForKey:uuid];
        [self cleanupOperationCallbacks:peripheral withResult:[CDVPluginResult resultWithStatus:CDVCommandStatus_ERROR messageAsString:@"Peripheral disconnected"]];

        if (peripheral && peripheral.state != CBPeripheralStateDisconnected) {
            [manager cancelPeripheralConnection:peripheral];
        }

        CDVPluginResult *pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_OK];
        [self.commandDelegate sendPluginResult:pluginResult callbackId:command.callbackId];
    }
}

// read: function (device_id, service_uuid, characteristic_uuid, success, failure) {
- (void)read:(CDVInvokedUrlCommand*)command {
    NSLog(@"read");

    BLECommandContext *context = [self getData:command prop:CBCharacteristicPropertyRead];
    if (context) {
        CBPeripheral *peripheral = [context peripheral];
        if ([peripheral state] != CBPeripheralStateConnected) {
            CDVPluginResult *pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_ERROR messageAsString:@"Peripheral is not connected"];
            [self.commandDelegate sendPluginResult:pluginResult callbackId:command.callbackId];
            return;
        }
        CBCharacteristic *characteristic = [context characteristic];

        NSString *key = [self keyForPeripheral: peripheral andCharacteristic:characteristic];
        [readCallbacks setObject:[command.callbackId copy] forKey:key];

        [peripheral readValueForCharacteristic:characteristic];  // callback sends value
    }
}

-(void)handleString:(NSString *)string withPeripheral:(CBPeripheral *)peripheral
{
    double _date=[[NSDate date] timeIntervalSince1970]*1000;
    NSString * _laterString=[string substringFromIndex:4];
    NSInteger _count=_laterString.length/16;
    NSString * _zhenIndex=[string substringToIndex:2];
    _zhenIndex = [NSString stringWithFormat:@"%lu",strtoul([_zhenIndex UTF8String],0,16)];

    NSMutableDictionary * _tempDict=[_allDict objectForKey:peripheral.name];
    NSMutableArray * _datas=[_tempDict objectForKey:@"data"];
    NSMutableArray * _logs=[_tempDict objectForKey:@"log"];
    int _tickerLength=[[_tempDict objectForKey:@"tickerLength"] intValue];
    int _totalIndex=[[_tempDict objectForKey:@"totalIndex"] intValue];
    _tickerLength+=_count;

    for(int i=0;i<_count;i++){
        NSString * _leadString=[_laterString substringWithRange:NSMakeRange(16*i+0, 2)];
        NSString * _tempString=[_laterString substringWithRange:NSMakeRange(16*i, 16)];
        NSString * _leftString=[_tempString substringWithRange:NSMakeRange(4, 6)];
        NSString * _rightString=[_tempString substringWithRange:NSMakeRange(10, 6)];
        long _leftValue=[[self binarySixtyToTenty:_leftString] longLongValue];
        float _leftFloat=1000 *((2420*_leftValue*1.0) / 8388608 / 12);
        long _rightValue=[[self binarySixtyToTenty:_rightString] longLongValue];
        float _rightFloat=1000 * ((2420 * _rightValue*1.0) / 8388608 / 12);
        NSString * _leftLead=[_leadString substringToIndex:1];
        NSString * _rightLead=[_leadString substringFromIndex:1];
        _leadString=[NSString stringWithFormat:@"%@%@",[self toBinarySystemWithDecimalSystem:_leftLead],[self toBinarySystemWithDecimalSystem:_rightLead]];
        if(![[_laterString substringWithRange:NSMakeRange(16*i+0, 2)] isEqualToString:@"80"]){
            [_datas addObject:[NSNumber numberWithFloat:_leftFloat]];
            [_datas addObject:[NSNumber numberWithFloat:_rightFloat]];

            int _leftSignal=[[_tempDict objectForKey:@"leftSignal"] intValue];
            int _rightSignal=[[_tempDict objectForKey:@"rightSignal"] intValue];
            NSString * _writeString=[NSString stringWithFormat:@"%.2f,%.3f,%.3f,%@,%d,%d,%d,%@,%@\n",_date,_leftFloat,_rightFloat,_leftLead,_leftSignal,_rightSignal,_totalIndex,_zhenIndex,peripheral.name];

            NSData * _data=[_writeString dataUsingEncoding:NSUTF8StringEncoding];
            NSError * _error=nil;
            if(_isWriting){
                if (@available(iOS 13.0, *)) {
                    @try {
                        [_handle writeData:_data error:&_error];
                        if(_error){
                            NSDictionary * _dict=[NSDictionary dictionaryWithObjectsAndKeys:@"write error below ios"@"desc",@"write csv",@"module",[NSString stringWithFormat:@"error:%@",_error.localizedFailureReason],@"level", nil];
                            [[NSNotificationCenter defaultCenter] postNotificationName:@"writeNativeLog" object:_dict];
                        }
                    } @catch (NSException *exception) {
                        NSDictionary * _dict=[NSDictionary dictionaryWithObjectsAndKeys:@"write error upper ios 13"@"desc",@"write csv",@"module",[NSString stringWithFormat:@"error:%@",exception.reason],@"level", nil];
                        [[NSNotificationCenter defaultCenter] postNotificationName:@"writeNativeLog" object:_dict];
                        @throw exception;
                    } @finally {

                    }
                } else {
                    @try {
                        [_handle writeData:_data];
                    } @catch (NSException *exception) {
                        NSDictionary * _dict=[NSDictionary dictionaryWithObjectsAndKeys:@"below ios 13"@"desc",@"write csv",@"module",[NSString stringWithFormat:@"error:%@",exception.reason],@"level", nil];
                        [[NSNotificationCenter defaultCenter] postNotificationName:@"writeNativeLog" object:_dict];
                    } @finally {

                    }
                }
            }
        } else {
            
        }
    }
}

// 将有符号的16进制数转换成10进制数
-(NSString *)binarySixtyToTenty:(NSString *)string
{
    @try {
            NSString * _erString=nil;
        for(int i=0;i<string.length;i++){//  1：先将16进制数转换成二进制数
            if(_erString==nil){
                _erString=[self toBinarySystemWithDecimalSystem:[string substringWithRange:NSMakeRange(i, 1)]];
            } else {
                _erString=[NSString stringWithFormat:@"%@%@",_erString,[self toBinarySystemWithDecimalSystem:[string substringWithRange:NSMakeRange(i, 1)]]];
            }
        }
        NSString * _firstString=[_erString substringToIndex:1];
        NSString * _handleString=nil;
        BOOL _isFu=NO;
        if([_firstString isEqualToString:@"1"]){// 判断第一位是否是1，如果是就代表是负数
            _isFu=YES;
            NSString * _laterString=[_erString substringFromIndex:1];
            for(int i=0;i<_laterString.length;i++){// 负数需要每一位取反，然后再加一
                int _aa=[[_laterString substringWithRange:NSMakeRange(i, 1)] intValue];
                if(_aa==0){
                    _aa=1;
                } else {
                    _aa=0;
                }
                if(_handleString==nil){
                    _handleString=[NSString stringWithFormat:@"%d",_aa];
                } else {
                    _handleString=[NSString stringWithFormat:@"%@%d",_handleString,_aa];
                }
            }
        } else {
            _handleString=_erString;
        }
        int _totalValue=0;
        for(NSInteger i=_handleString.length-1;i>0;i--){
            NSString * _value=[_handleString substringWithRange:NSMakeRange(i, 1)];
            _totalValue+=[_value intValue]*pow(2, _handleString.length-i-1);
        }
        if(_isFu){
            _totalValue=(_totalValue+1)*-1;
        }
        return [NSString stringWithFormat:@"%d",_totalValue];
    } @catch (NSException *exception) {
        NSDictionary * _dict=[NSDictionary dictionaryWithObjectsAndKeys:@"sixty to tenty"@"desc",@"binary transfer",@"module",[NSString stringWithFormat:@"error:%@",exception.reason],@"level", nil];
        [[NSNotificationCenter defaultCenter] postNotificationName:@"writeNativeLog" object:_dict];
    } @finally {

    }
}

//  十进制转换成二进制
- (NSString *)toBinarySystemWithDecimalSystem:(NSString *)string
{
    NSInteger _length=[string length]*4;
    NSInteger num = [string integerValue];//[decimal intValue];
    NSInteger remainder = 0; //余数
    NSInteger divisor = 0; //除数
    NSString * prepare = @"";
    if([[string lowercaseString] isEqualToString:@"a"]){
        num=10;
    } else if([[string lowercaseString] isEqualToString:@"b"]){
        num=11;
    } else if([[string lowercaseString] isEqualToString:@"c"]){
        num=12;
    } else if([[string lowercaseString] isEqualToString:@"d"]){
        num=13;
    } else if([[string lowercaseString] isEqualToString:@"e"]){
        num=14;
    } else if([[string lowercaseString] isEqualToString:@"f"]){
        num=15;
    }
    while (true)
    {
        remainder = num%2;
        divisor = num/2;
        num = divisor;
        prepare = [prepare stringByAppendingFormat:@"%ld",remainder];

        if (divisor == 0)
        {
            break;
        }
    }
    NSString * result = @"";
    for (NSInteger i = 0;i<_length;i++)
    {
        result = [result stringByAppendingFormat:@"%@", (_length-i>prepare.length?@"0":[prepare substringWithRange:NSMakeRange(_length-i-1 , 1)])];
    }
    return result;
}


- (NSString *)hexStringFromData:(NSData *)string{
    Byte *bytes = (Byte *)[string bytes]; //下面是Byte 转换为16进制。
    NSString *hexStr=@""; for(int i=0;i<[string length];i++) {
        NSString *newHexStr = [NSString stringWithFormat:@"%X",bytes[i]&0xFF];///16进制数
        if([newHexStr length]==1)
            hexStr = [NSString stringWithFormat:@"%@0%@",hexStr,newHexStr];
        else
            hexStr = [NSString stringWithFormat:@"%@%@",hexStr,newHexStr];
    }
    return hexStr;
}


// write: function (device_id, service_uuid, characteristic_uuid, value, success, failure) {
- (void)write:(CDVInvokedUrlCommand*)command {
    BLECommandContext *context = [self getData:command prop:CBCharacteristicPropertyWrite];
    NSData *message = [command argumentAtIndex:3]; // This is binary
    if (context) {
        if (message != nil) {
            CBPeripheral *peripheral = [context peripheral];
            if ([peripheral state] != CBPeripheralStateConnected) {
                CDVPluginResult *pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_ERROR messageAsString:@"Peripheral is not connected"];
                [self.commandDelegate sendPluginResult:pluginResult callbackId:command.callbackId];
                return;
            }
            CBCharacteristic *characteristic = [context characteristic];

            NSString *key = [self keyForPeripheral: peripheral andCharacteristic:characteristic];
            [writeCallbacks setObject:[command.callbackId copy] forKey:key];

            // TODO need to check the max length
            [peripheral writeValue:message forCharacteristic:characteristic type:CBCharacteristicWriteWithResponse];

            // response is sent from didWriteValueForCharacteristic
        } else {
            CDVPluginResult *pluginResult = nil;
            pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_ERROR messageAsString:@"message was null"];
            [self.commandDelegate sendPluginResult:pluginResult callbackId:command.callbackId];
        }
    }
}

// writeWithoutResponse: function (device_id, service_uuid, characteristic_uuid, value, success, failure) {
- (void)writeWithoutResponse:(CDVInvokedUrlCommand*)command {
    NSLog(@"writeWithoutResponse");

    BLECommandContext *context = [self getData:command prop:CBCharacteristicPropertyWriteWithoutResponse];
    NSData *message = [command argumentAtIndex:3]; // This is binary

    if (context) {
        CDVPluginResult *pluginResult = nil;
        if (message != nil) {
            CBPeripheral *peripheral = [context peripheral];
            if ([peripheral state] != CBPeripheralStateConnected) {
                CDVPluginResult *pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_ERROR messageAsString:@"Peripheral is not connected"];
                [self.commandDelegate sendPluginResult:pluginResult callbackId:command.callbackId];
                return;
            }
            CBCharacteristic *characteristic = [context characteristic];

            // TODO need to check the max length
            [peripheral writeValue:message forCharacteristic:characteristic type:CBCharacteristicWriteWithoutResponse];

            pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_OK];
        } else {
            pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_ERROR messageAsString:@"message was null"];
        }
        [self.commandDelegate sendPluginResult:pluginResult callbackId:command.callbackId];
    }
}

// success callback is called on notification
// notify: function (device_id, service_uuid, characteristic_uuid, success, failure) {
- (void)startNotification:(CDVInvokedUrlCommand*)command {
    NSLog(@"registering for notification");

    BLECommandContext *context = [self getData:command prop:CBCharacteristicPropertyNotify]; // TODO name this better

    if (context) {
        CBPeripheral *peripheral = [context peripheral];
        if ([peripheral state] != CBPeripheralStateConnected) {
            CDVPluginResult *pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_ERROR messageAsString:@"Peripheral is not connected"];
            [self.commandDelegate sendPluginResult:pluginResult callbackId:command.callbackId];
            return;
        }
        CBCharacteristic *characteristic = [context characteristic];

        NSString *key = [self keyForPeripheral: peripheral andCharacteristic:characteristic];
        NSString *callback = [command.callbackId copy];
        [startNotificationCallbacks setObject: callback forKey: key];
        [stopNotificationCallbacks removeObjectForKey:key];

        [peripheral setNotifyValue:YES forCharacteristic:characteristic];

    }

}

// stopNotification: function (device_id, service_uuid, characteristic_uuid, success, failure) {
- (void)stopNotification:(CDVInvokedUrlCommand*)command {
    NSLog(@"stop notification");

    BLECommandContext *context = [self getData:command prop:CBCharacteristicPropertyNotify];

    if (context) {
        CBPeripheral *peripheral = [context peripheral];    // FIXME is setNotifyValue:NO legal to call on a peripheral not connected?
        CBCharacteristic *characteristic = [context characteristic];

        NSString *key = [self keyForPeripheral: peripheral andCharacteristic:characteristic];
        NSString *callback = [command.callbackId copy];
        [stopNotificationCallbacks setObject: callback forKey: key];

        [peripheral setNotifyValue:NO forCharacteristic:characteristic];
        // callback sent from peripheral:didUpdateNotificationStateForCharacteristic:error:

    }
}

- (void)isEnabled:(CDVInvokedUrlCommand*)command {
    CDVPluginResult *pluginResult = nil;
    int bluetoothState = [manager state];

    BOOL enabled = bluetoothState == CBCentralManagerStatePoweredOn;

    if (enabled) {
        pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_OK];
    } else {
        pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_ERROR messageAsInt:bluetoothState];
    }
    [self.commandDelegate sendPluginResult:pluginResult callbackId:command.callbackId];
}

- (void)scan:(CDVInvokedUrlCommand*)command {
    NSLog(@"scan");
    discoverPeripheralCallbackId = [command.callbackId copy];

    NSArray<NSString *> *serviceUUIDStrings = [command argumentAtIndex:0];
    NSNumber *timeoutSeconds = [command argumentAtIndex:1];
    NSArray<CBUUID *> *serviceUUIDs = [self uuidStringsToCBUUIDs:serviceUUIDStrings];

    [manager scanForPeripheralsWithServices:serviceUUIDs options:nil];

    [NSTimer scheduledTimerWithTimeInterval:[timeoutSeconds floatValue]
                                     target:self
                                   selector:@selector(stopScanTimer:)
                                   userInfo:[command.callbackId copy]
                                    repeats:NO];
}

- (void)startScan:(CDVInvokedUrlCommand*)command {
    NSLog(@"startScan");
    discoverPeripheralCallbackId = [command.callbackId copy];
    NSArray<NSString *> *serviceUUIDStrings = [command argumentAtIndex:0];
    NSArray<CBUUID *> *serviceUUIDs = [self uuidStringsToCBUUIDs:serviceUUIDStrings];

    [manager scanForPeripheralsWithServices:serviceUUIDs options:nil];
}

- (void)startScanWithOptions:(CDVInvokedUrlCommand*)command {
    NSLog(@"startScanWithOptions");
    discoverPeripheralCallbackId = [command.callbackId copy];
    NSArray<NSString *> *serviceUUIDStrings = [command argumentAtIndex:0];
    NSArray<CBUUID *> *serviceUUIDs = [self uuidStringsToCBUUIDs:serviceUUIDStrings];
    NSDictionary *options = command.arguments[1];

    NSMutableDictionary *scanOptions = [NSMutableDictionary new];
    NSNumber *reportDuplicates = [options valueForKey: @"reportDuplicates"];
    if (reportDuplicates) {
        [scanOptions setValue:reportDuplicates
                       forKey:CBCentralManagerScanOptionAllowDuplicatesKey];
    }

    [manager scanForPeripheralsWithServices:serviceUUIDs options:scanOptions];
}

- (void)stopScan:(CDVInvokedUrlCommand*)command {
    NSLog(@"stopScan");

    [manager stopScan];

    if (discoverPeripheralCallbackId) {
        discoverPeripheralCallbackId = nil;
    }

    CDVPluginResult *pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_OK];
    [self.commandDelegate sendPluginResult:pluginResult callbackId:command.callbackId];
}


- (void)isConnected:(CDVInvokedUrlCommand*)command {
    CDVPluginResult *pluginResult = nil;
    CBPeripheral *peripheral = [self findPeripheralByUUID:[command argumentAtIndex:0]];

    if (peripheral && peripheral.state == CBPeripheralStateConnected) {
        pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_OK];
    } else {
        pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_ERROR messageAsString:@"Not connected"];
    }
    [self.commandDelegate sendPluginResult:pluginResult callbackId:command.callbackId];
}

- (void)startStateNotifications:(CDVInvokedUrlCommand *)command {
    CDVPluginResult *pluginResult = nil;

    if (stateCallbackId == nil) {
        stateCallbackId = [command.callbackId copy];
        int bluetoothState = [manager state];
        NSString *state = [bluetoothStates objectForKey:[NSNumber numberWithInt:bluetoothState]];
        pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_OK messageAsString:state];
        [pluginResult setKeepCallbackAsBool:TRUE];
        NSLog(@"Start state notifications on callback %@", stateCallbackId);
    } else {
        pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_ERROR messageAsString:@"State callback already registered"];
    }

    [self.commandDelegate sendPluginResult:pluginResult callbackId:command.callbackId];
}

- (void)stopStateNotifications:(CDVInvokedUrlCommand *)command {
    CDVPluginResult *pluginResult = nil;

    if (stateCallbackId != nil) {
        // Call with NO_RESULT so Cordova.js will delete the callback without actually calling it
        pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_NO_RESULT];
        [self.commandDelegate sendPluginResult:pluginResult callbackId:stateCallbackId];
        stateCallbackId = nil;
    }

    pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_OK];
    [self.commandDelegate sendPluginResult:pluginResult callbackId:command.callbackId];
}

- (void)onReset {
    stateCallbackId = nil;
}

- (void)readRSSI:(CDVInvokedUrlCommand*)command {
    NSLog(@"readRSSI");
    NSString *uuid = [command argumentAtIndex:0];

    CBPeripheral *peripheral = [self findPeripheralByUUID:uuid];

    if (peripheral && peripheral.state == CBPeripheralStateConnected) {
        [readRSSICallbacks setObject:[command.callbackId copy] forKey:[peripheral uuidAsString]];
        [peripheral readRSSI];
    } else {
        NSString *error = [NSString stringWithFormat:@"Need to be connected to peripheral %@ to read RSSI.", uuid];
        NSLog(@"%@", error);
        CDVPluginResult *pluginResult = nil;
        pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_ERROR messageAsString:error];
        [self.commandDelegate sendPluginResult:pluginResult callbackId:command.callbackId];
    }
}

// Returns a list of the peripherals (containing any of the specified services) currently connected to the system.
// https://developer.apple.com/documentation/corebluetooth/cbcentralmanager/1518924-retrieveconnectedperipheralswith?language=objc
- (void)connectedPeripheralsWithServices:(CDVInvokedUrlCommand*)command {
    NSLog(@"connectedPeripheralsWithServices");
    NSArray *serviceUUIDStrings = [command argumentAtIndex:0];
    NSArray<CBUUID *> *serviceUUIDs = [self uuidStringsToCBUUIDs:serviceUUIDStrings];

    NSArray<CBPeripheral *> *connectedPeripherals = [manager retrieveConnectedPeripheralsWithServices:serviceUUIDs];
    NSMutableArray<NSDictionary *> *connected = [NSMutableArray new];

    for (CBPeripheral *peripheral in connectedPeripherals) {
        [peripherals addObject:peripheral];
        [connected addObject:[peripheral asDictionary]];
    }

    CDVPluginResult *pluginResult = nil;
    pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_OK messageAsArray:connected];
    NSLog(@"Connected peripherals with services %@ %@", serviceUUIDStrings, connected);
    [self.commandDelegate sendPluginResult:pluginResult callbackId:command.callbackId];
}

// Returns a list of known peripherals by their identifiers.
// https://developer.apple.com/documentation/corebluetooth/cbcentralmanager/1519127-retrieveperipheralswithidentifie?language=objc
- (void)peripheralsWithIdentifiers:(CDVInvokedUrlCommand*)command {
    NSLog(@"peripheralsWithIdentifiers");
    NSArray *identifierUUIDStrings = [command argumentAtIndex:0];
    NSArray<NSUUID *> *identifiers = [self uuidStringsToNSUUIDs:identifierUUIDStrings];

    NSArray<CBPeripheral *> *foundPeripherals = [manager retrievePeripheralsWithIdentifiers:identifiers];
    // TODO are any of these connected?
    NSMutableArray<NSDictionary *> *found = [NSMutableArray new];

    for (CBPeripheral *peripheral in foundPeripherals) {
        [peripherals addObject:peripheral];   // TODO do we save these?
        [found addObject:[peripheral asDictionary]];
    }

    CDVPluginResult *pluginResult = nil;
    pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_OK messageAsArray:found];
    NSLog(@"Peripherals with identifiers %@ %@", identifierUUIDStrings, found);
    [self.commandDelegate sendPluginResult:pluginResult callbackId:command.callbackId];
}


#pragma mark - timers

-(void)stopScanTimer:(NSTimer *)timer {
    NSLog(@"stopScanTimer");

    [manager stopScan];

    if (discoverPeripheralCallbackId) {
        discoverPeripheralCallbackId = nil;
    }
}

-(void)hasConnectDevice:(CBPeripheral *)peripheral
{
    if([_allDict objectForKey:peripheral.name]){
        [_allDict removeObjectForKey:peripheral.name];
    }
    NSMutableDictionary * _peripheralDict=[NSMutableDictionary new];
    NSMutableArray * _datas=[NSMutableArray new];
    NSMutableArray * _signals=[NSMutableArray new];
    NSMutableArray * _tbrs=[NSMutableArray new];
    NSMutableArray * _energys=[NSMutableArray new];
    NSMutableArray * _index_was=[NSMutableArray new];
    NSMutableArray * _asys=[NSMutableArray new];
    NSMutableArray * _stresses=[NSMutableArray new];
    NSMutableArray * _relaxs=[NSMutableArray new];
    NSMutableArray * _logs=[NSMutableArray new];
    [_peripheralDict setObject:_datas forKey:@"data"];
    [_peripheralDict setObject:_signals forKey:@"signal"];
    [_peripheralDict setObject:_tbrs forKey:@"tbr"];
    [_peripheralDict setObject:_energys forKey:@"energy"];
    [_peripheralDict setObject:_index_was forKey:@"index_wa"];
    [_peripheralDict setObject:_asys forKey:@"asy"];
    [_peripheralDict setObject:_stresses forKey:@"stress"];
    [_peripheralDict setObject:_relaxs forKey:@"relax"];
    [_peripheralDict setObject:_logs forKey:@"log"];
    [_peripheralDict setObject:[NSNumber numberWithInt:0] forKey:@"tickerLength"];
    [_peripheralDict setObject:[NSNumber numberWithInt:0] forKey:@"totalIndex"];
    [_peripheralDict setObject:[NSNumber numberWithInt:0] forKey:@"leftSignal"];
    [_peripheralDict setObject:[NSNumber numberWithInt:0] forKey:@"rightSignal"];
    [_allDict setObject:_peripheralDict forKey:peripheral.name];

    int index=add_device((char *)[[peripheral name] UTF8String]);
    [_indexArray setObject:[NSNumber numberWithInt:index] forKey:peripheral.name];
}

-(void)hasEndDevice:(CBPeripheral *)perpheral
{
    int index=delete_device((char *)[[perpheral name] UTF8String]);
    [_allDict removeObjectForKey:perpheral.name];
}

#pragma mark - CBCentralManagerDelegate

- (void)centralManager:(CBCentralManager *)central didDiscoverPeripheral:(CBPeripheral *)peripheral advertisementData:(NSDictionary *)advertisementData RSSI:(NSNumber *)RSSI {

    [peripherals addObject:peripheral];
    [peripheral setAdvertisementData:advertisementData RSSI:RSSI];

    if (discoverPeripheralCallbackId) {
        CDVPluginResult *pluginResult = nil;
        pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_OK messageAsDictionary:[peripheral asDictionary]];
        NSLog(@"Discovered %@", [peripheral asDictionary]);
        [pluginResult setKeepCallbackAsBool:TRUE];
        [self.commandDelegate sendPluginResult:pluginResult callbackId:discoverPeripheralCallbackId];
    }
}

- (void)centralManagerDidUpdateState:(CBCentralManager *)central
{
    NSLog(@"Status of CoreBluetooth central manager changed %ld %@", (long)central.state, [self centralManagerStateToString: central.state]);

    if (central.state == CBCentralManagerStateUnsupported)
    {
        NSLog(@"=============================================================");
        NSLog(@"WARNING: This hardware does not support Bluetooth Low Energy.");
        NSLog(@"=============================================================");
    }

    if (stateCallbackId != nil) {
        CDVPluginResult *pluginResult = nil;
        NSString *state = [bluetoothStates objectForKey:@(central.state)];
        pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_OK messageAsString:state];
        [pluginResult setKeepCallbackAsBool:TRUE];
        NSLog(@"Report Bluetooth state \"%@\" on callback %@", state, stateCallbackId);
        [self.commandDelegate sendPluginResult:pluginResult callbackId:stateCallbackId];
    }

    // check and handle disconnected peripherals
    for (CBPeripheral *peripheral in peripherals) {
        if (peripheral.state == CBPeripheralStateDisconnected) {
            [self centralManager:central didDisconnectPeripheral:peripheral error:nil];
        }
    }
}

- (void)centralManager:(CBCentralManager *)central didConnectPeripheral:(CBPeripheral *)peripheral {
    NSLog(@"didConnectPeripheral");

    peripheral.delegate = self;

    // NOTE: it's inefficient to discover all services
    [peripheral discoverServices:nil];

    [self hasConnectDevice:peripheral];

    // NOTE: not calling connect success until characteristics are discovered
}

- (void)centralManager:(CBCentralManager *)central didDisconnectPeripheral:(CBPeripheral *)peripheral error:(NSError *)error {
    NSLog(@"didDisconnectPeripheral");

    NSString *connectCallbackId = [connectCallbacks valueForKey:[peripheral uuidAsString]];
    [connectCallbacks removeObjectForKey:[peripheral uuidAsString]];
    [self cleanupOperationCallbacks:peripheral withResult:[CDVPluginResult resultWithStatus:CDVCommandStatus_ERROR messageAsString:@"Peripheral disconnected"]];

    if (connectCallbackId) {

        NSMutableDictionary *dict = [NSMutableDictionary dictionaryWithDictionary:[peripheral asDictionary]];

        // add error info
        [dict setObject:@"Peripheral Disconnected" forKey:@"errorMessage"];
        if (error) {
            [dict setObject:[error localizedDescription] forKey:@"errorDescription"];
        }
        // remove extra junk
        [dict removeObjectForKey:@"rssi"];
        [dict removeObjectForKey:@"advertising"];
        [dict removeObjectForKey:@"services"];

        CDVPluginResult *pluginResult = nil;
        pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_ERROR messageAsDictionary:dict];
        [self.commandDelegate sendPluginResult:pluginResult callbackId:connectCallbackId];
    }
}

- (void)centralManager:(CBCentralManager *)central didFailToConnectPeripheral:(CBPeripheral *)peripheral error:(NSError *)error {
    NSLog(@"didFailToConnectPeripheral");

    NSString *connectCallbackId = [connectCallbacks valueForKey:[peripheral uuidAsString]];
    [connectCallbacks removeObjectForKey:[peripheral uuidAsString]];
    [self cleanupOperationCallbacks:peripheral withResult:[CDVPluginResult resultWithStatus:CDVCommandStatus_ERROR messageAsString:@"Peripheral disconnected"]];

    CDVPluginResult *pluginResult = nil;
    pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_ERROR messageAsDictionary:[peripheral asDictionary]];
    [self.commandDelegate sendPluginResult:pluginResult callbackId:connectCallbackId];
}

#pragma mark CBPeripheralDelegate

- (void)peripheral:(CBPeripheral *)peripheral didDiscoverServices:(NSError *)error {
    NSLog(@"didDiscoverServices");

    // save the services to tell when all characteristics have been discovered
    NSMutableSet *servicesForPeriperal = [NSMutableSet new];
    [servicesForPeriperal addObjectsFromArray:peripheral.services];
    [connectCallbackLatches setObject:servicesForPeriperal forKey:[peripheral uuidAsString]];

    for (CBService *service in peripheral.services) {
        [peripheral discoverCharacteristics:nil forService:service]; // discover all is slow
    }
}

- (void)peripheral:(CBPeripheral *)peripheral didDiscoverCharacteristicsForService:(CBService *)service error:(NSError *)error {
    NSLog(@"didDiscoverCharacteristicsForService");

    NSString *peripheralUUIDString = [peripheral uuidAsString];
    NSString *connectCallbackId = [connectCallbacks valueForKey:peripheralUUIDString];
    NSMutableSet *latch = [connectCallbackLatches valueForKey:peripheralUUIDString];

    [latch removeObject:service];

    if ([latch count] == 0) {
        // Call success callback for connect
        if (connectCallbackId) {
            CDVPluginResult *pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_OK messageAsDictionary:[peripheral asDictionary]];
            [pluginResult setKeepCallbackAsBool:TRUE];
            [self.commandDelegate sendPluginResult:pluginResult callbackId:connectCallbackId];
        }
        [connectCallbackLatches removeObjectForKey:peripheralUUIDString];
    }

    NSLog(@"Found characteristics for service %@", service);
    for (CBCharacteristic *characteristic in service.characteristics) {
        NSLog(@"Characteristic %@", characteristic);
    }
}

-(int)getLeadStatus:(int)leadLeft leadRight:(int)leadRight
{
    if(leadLeft==0&&leadRight==0){
        return 0;// 两个都戴好
    } else if (leadLeft==1||leadRight==1){
        return -1;// 两个都不戴好（脱落）
    } else {
        return 1; // 接触不良
    }
}

-(double)getNotNan:(double)value
{
    if(isnan(value)){
        return 0;
    } else {
        return value;
    }
}

- (void)peripheral:(CBPeripheral *)peripheral didUpdateValueForCharacteristic:(CBCharacteristic *)characteristic error:(NSError *)error {

    NSString *key = [self keyForPeripheral: peripheral andCharacteristic:characteristic];
    NSString *notifyCallbackId = [notificationCallbacks objectForKey:key];

    if (notifyCallbackId) {
        NSData *data = characteristic.value; // send RAW data to Javascript

        CDVPluginResult *pluginResult = nil;
        if (error) {
            NSLog(@"%@", error);
            pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_ERROR messageAsString:[error localizedDescription]];
        } else {
            NSData *data = characteristic.value; // send RAW data to Javascript
            NSString * _string=[self hexStringFromData:data];
            if([characteristic.UUID.UUIDString isEqualToString:@"6E400003-B5A3-F393-E0A9-E50E24DCCA9E"]){
                [self handleString:_string withPeripheral:peripheral];

                NSMutableDictionary * _tempDict=[_allDict objectForKey:peripheral.name];
                NSMutableArray * _datas=[_tempDict objectForKey:@"data"];
                if(_datas.count>=2*LEN_FILTER){
                    int arrayLength=LEN_FILTER*2;
                    double _doubles[arrayLength];
                    for(int i=0;i<LEN_FILTER+0;i++){
                        float _left=[[_datas objectAtIndex: 2*i] floatValue];
                        float _right=[[_datas objectAtIndex:2*i+1] floatValue];
                        int index=i;
                        _doubles[index]=_left;
                        _doubles[index+LEN_FILTER]=_right; // PS:需要将一维数组转换成matlab需要的一维数组形式 前1024个数据是left通道 后1024个数据是右通道
                    }
                    @try {
                        [_datas removeObjectsInRange:NSMakeRange(0, LEN_FILTER*2)];
                        int index=[[_indexArray objectForKey:peripheral.name] intValue];
                        algorithmTarget target=get_algorithm_result(_doubles, index);
                        if(target.sqdValid==1){
                            int _LeadLeft=[self getNotNan:target.sqd[0]];
                            int _LeadRight=[self getNotNan:target.sqd[1]];
                            [_tempDict setObject:[NSNumber numberWithInt:_LeadLeft] forKey:@"leftSignal"];
                            [_tempDict setObject:[NSNumber numberWithInt:_LeadRight] forKey:@"rightSignal"];

                            NSMutableArray * _signals=[_tempDict objectForKey:@"signal"];
                            NSMutableArray * _tbrs=[_tempDict objectForKey:@"tbr"];
                            NSMutableArray * _energys=[_tempDict objectForKey:@"energy"];
                            NSMutableArray * _index_was=[_tempDict objectForKey:@"index_wa"];
                            NSMutableArray * _asys=[_tempDict objectForKey:@"asy"];
                            NSMutableArray * _stresses=[_tempDict objectForKey:@"stress"];
                            NSMutableArray * _relaxs=[_tempDict objectForKey:@"relax"];
                            NSMutableArray * _logs=[_tempDict objectForKey:@"log"];
                            [_signals addObject:@[[NSNumber numberWithInt:_LeadLeft],[NSNumber numberWithInt:_LeadRight]]];
                            [_tbrs addObject:[NSNumber numberWithDouble:[self getNotNan:target.tbr]]];
                            [_index_was addObject:[NSNumber numberWithDouble:[self getNotNan:target.index_wa]]];
                            [_asys addObject:[NSNumber numberWithDouble:[self getNotNan:target.asy]]];
                            [_stresses addObject:[NSNumber numberWithDouble:[self getNotNan:target.stress]]];
                            [_relaxs addObject:[NSNumber numberWithDouble:[self getNotNan:target.fatig]]];
                            NSMutableArray * _tempEnergy=[NSMutableArray new];
                            for(int j=0;j<5;j++){
                                [_tempEnergy addObject:[NSNumber numberWithDouble:[self getNotNan:target.energys[j]]]];
                            }
                            [_energys addObject:_tempEnergy];

//                            if(_signals.count==1){
                                NSArray * _returnArray=[NSArray arrayWithObjects:/*@"target"*/peripheral.name,[NSNumber numberWithInt:[self getLeadStatus:_LeadLeft leadRight:_LeadRight]],[NSNumber numberWithDouble:target.stress], nil];
                                pluginResult=[CDVPluginResult resultWithStatus:CDVCommandStatus_OK messageAsArray:_returnArray];
                                [pluginResult setKeepCallbackAsBool:TRUE]; // keep for notification
                                [self.commandDelegate sendPluginResult:pluginResult callbackId:notifyCallbackId];

                                [_signals removeAllObjects];
                                [_tbrs removeAllObjects];
                                [_energys removeAllObjects];
                                [_index_was removeAllObjects];
                                [_asys removeAllObjects];
                                [_stresses removeAllObjects];
                                [_relaxs removeAllObjects];
//                            }
                        }
                    } @catch (NSException *exception) {
                        NSDictionary * _dict=[NSDictionary dictionaryWithObjectsAndKeys:[NSString stringWithFormat:@"get algorithm:%@",exception.reason],@"desc",@"call algorithm",@"module",@"error",@"level", nil];
                        [[NSNotificationCenter defaultCenter] postNotificationName:@"writeNativeLog" object:_dict];
                    } @finally {

                    }
                }
            } else if([characteristic.UUID.UUIDString isEqualToString:@"6E400004-B5A3-F393-E0A9-E50E24DCCA9E"]){
                if(_string.length==8*2){
                    int _index=([[_string substringWithRange:NSMakeRange(0, 1)] intValue]*16+[[_string substringWithRange:NSMakeRange(1, 1)] intValue]);
                    if(_index==129){
                        NSString * _versionStr=@"";
                        for(int i=5;i<8;i++){
                            int _index1=[[_string substringWithRange:NSMakeRange(2*i, 1)] intValue];
                            int _index2=[[_string substringWithRange:NSMakeRange(2*i+1, 1)] intValue];
                            if([_versionStr isEqualToString:@""]){
                                _versionStr=[NSString stringWithFormat:@"%d",_index1*16+_index2];
                            } else {
                                _versionStr=[NSString stringWithFormat:@"%@.%d",_versionStr,_index1*16+_index2];
                            }
                        }
                        [[NSUserDefaults standardUserDefaults] setObject:_versionStr forKey:@"version"];
                        [[NSUserDefaults standardUserDefaults] synchronize];
                    }
                }
                pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_OK messageAsArrayBuffer:data];
                [pluginResult setKeepCallbackAsBool:TRUE]; // keep for notification
                [self.commandDelegate sendPluginResult:pluginResult callbackId:notifyCallbackId];
            } else {
                pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_OK messageAsArrayBuffer:data];
                [pluginResult setKeepCallbackAsBool:TRUE]; // keep for notification
                [self.commandDelegate sendPluginResult:pluginResult callbackId:notifyCallbackId];
            }
        }
    }

    NSString *readCallbackId = [readCallbacks objectForKey:key];

    if(readCallbackId) {
        NSData *data = characteristic.value; // send RAW data to Javascript
        CDVPluginResult *pluginResult = nil;

        if (error) {
            NSLog(@"%@", error);
            pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_ERROR messageAsString:[error localizedDescription]];
        } else {
            pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_OK messageAsArrayBuffer:data];
        }

        [self.commandDelegate sendPluginResult:pluginResult callbackId:readCallbackId];

        [readCallbacks removeObjectForKey:key];
    }
}

- (void)peripheral:(CBPeripheral *)peripheral didUpdateNotificationStateForCharacteristic:(CBCharacteristic *)characteristic error:(NSError *)error {
    NSString *key = [self keyForPeripheral: peripheral andCharacteristic:characteristic];
    NSString *startNotificationCallbackId = [startNotificationCallbacks objectForKey:key];
    NSString *stopNotificationCallbackId = [stopNotificationCallbacks objectForKey:key];

    CDVPluginResult *pluginResult = nil;

    if (!characteristic.isNotifying && stopNotificationCallbackId) {
        if (error) {
            NSLog(@"%@", error);
            pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_ERROR messageAsString:[error localizedDescription]];
        } else {
            pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_OK];
        }
        [self.commandDelegate sendPluginResult:pluginResult callbackId:stopNotificationCallbackId];
        [stopNotificationCallbacks removeObjectForKey:key];
        [notificationCallbacks removeObjectForKey:key];
        NSAssert(![startNotificationCallbacks objectForKey:key], @"%@ existed in both start and stop notification callback dicts!", key);
    }

    if (characteristic.isNotifying && startNotificationCallbackId) {
        if (error) {
            NSLog(@"%@", error);
            pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_ERROR messageAsString:[error localizedDescription]];
            [self.commandDelegate sendPluginResult:pluginResult callbackId:startNotificationCallbackId];
            [startNotificationCallbacks removeObjectForKey:key];
        } else {
            // notification start succeeded, move the callback to the value notifications dict
            [notificationCallbacks setObject:startNotificationCallbackId forKey:key];
            [startNotificationCallbacks removeObjectForKey:key];
        }
    }
}


- (void)peripheral:(CBPeripheral *)peripheral didWriteValueForCharacteristic:(CBCharacteristic *)characteristic error:(NSError *)error {
    // This is the callback for write

    NSString *key = [self keyForPeripheral: peripheral andCharacteristic:characteristic];
    NSString *writeCallbackId = [writeCallbacks objectForKey:key];

    if (writeCallbackId) {
        CDVPluginResult *pluginResult = nil;
        if (error) {
            NSLog(@"%@", error);
            pluginResult = [CDVPluginResult
                resultWithStatus:CDVCommandStatus_ERROR
                messageAsString:[error localizedDescription]
            ];
        } else {
            pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_OK];
        }
        [self.commandDelegate sendPluginResult:pluginResult callbackId:writeCallbackId];
        [writeCallbacks removeObjectForKey:key];
    }

}

- (void)peripheral:(CBPeripheral*)peripheral didReadRSSI:(NSNumber*)rssi error:(NSError*)error {
    NSLog(@"didReadRSSI %@", rssi);
    NSString *key = [peripheral uuidAsString];
    NSString *readRSSICallbackId = [readRSSICallbacks objectForKey: key];
    [peripheral setSavedRSSI:rssi];
    if (readRSSICallbackId) {
        CDVPluginResult* pluginResult = nil;
        if (error) {
            NSLog(@"%@", error);
            pluginResult = [CDVPluginResult
                resultWithStatus:CDVCommandStatus_ERROR
                messageAsString:[error localizedDescription]];
        } else {
            pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_OK
                messageAsInt: (int) [rssi integerValue]];
        }
        [self.commandDelegate sendPluginResult:pluginResult callbackId: readRSSICallbackId];
        [readRSSICallbacks removeObjectForKey:readRSSICallbackId];
    }
}

#pragma mark - internal implemetation

- (CBPeripheral*)findPeripheralByUUID:(NSString*)uuid {
    CBPeripheral *peripheral = nil;

    for (CBPeripheral *p in peripherals) {

        NSString* other = p.identifier.UUIDString;

        if ([uuid isEqualToString:other]) {
            peripheral = p;
            break;
        }
    }
    return peripheral;
}

// RedBearLab
-(CBService *) findServiceFromUUID:(CBUUID *)UUID p:(CBPeripheral *)p {
    for(int i = 0; i < p.services.count; i++) {
        CBService *s = [p.services objectAtIndex:i];
        if ([self compareCBUUID:s.UUID UUID2:UUID])
            return s;
    }

    return nil; //Service not found on this peripheral
}

// Find a characteristic in service with a specific property
-(CBCharacteristic *) findCharacteristicFromUUID:(CBUUID *)UUID service:(CBService*)service prop:(CBCharacteristicProperties)prop {
    NSLog(@"Looking for %@ with properties %lu", UUID, (unsigned long)prop);
    for(int i=0; i < service.characteristics.count; i++)
    {
        CBCharacteristic *c = [service.characteristics objectAtIndex:i];
        if ((c.properties & prop) != 0x0 && [c.UUID.UUIDString isEqualToString: UUID.UUIDString]) {
            return c;
        }
    }
   return nil; //Characteristic with prop not found on this service
}

// Find a characteristic in service by UUID
-(CBCharacteristic *) findCharacteristicFromUUID:(CBUUID *)UUID service:(CBService*)service {
    NSLog(@"Looking for %@", UUID);
    for(int i=0; i < service.characteristics.count; i++)
    {
        CBCharacteristic *c = [service.characteristics objectAtIndex:i];
        if ([c.UUID.UUIDString isEqualToString: UUID.UUIDString]) {
            return c;
        }
    }
   return nil; //Characteristic not found on this service
}

// RedBearLab
-(int) compareCBUUID:(CBUUID *) UUID1 UUID2:(CBUUID *)UUID2 {
    char b1[16];
    char b2[16];
    [UUID1.data getBytes:b1 length:16];
    [UUID2.data getBytes:b2 length:16];

    if (memcmp(b1, b2, UUID1.data.length) == 0)
        return 1;
    else
        return 0;
}

// expecting deviceUUID, serviceUUID, characteristicUUID in command.arguments
-(BLECommandContext*) getData:(CDVInvokedUrlCommand*)command prop:(CBCharacteristicProperties)prop {
    NSLog(@"getData");

    CDVPluginResult *pluginResult = nil;

    NSString *deviceUUIDString = [command argumentAtIndex:0];
    NSString *serviceUUIDString = [command argumentAtIndex:1];
    NSString *characteristicUUIDString = [command argumentAtIndex:2];

    CBUUID *serviceUUID = [CBUUID UUIDWithString:serviceUUIDString];
    CBUUID *characteristicUUID = [CBUUID UUIDWithString:characteristicUUIDString];

    CBPeripheral *peripheral = [self findPeripheralByUUID:deviceUUIDString];

    if (!peripheral) {

        NSLog(@"Could not find peripheral with UUID %@", deviceUUIDString);

        NSString *errorMessage = [NSString stringWithFormat:@"Could not find peripheral with UUID %@", deviceUUIDString];
        pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_ERROR messageAsString:errorMessage];
        [self.commandDelegate sendPluginResult:pluginResult callbackId:command.callbackId];

        return nil;
    }

    CBService *service = [self findServiceFromUUID:serviceUUID p:peripheral];

    if (!service)
    {
        NSLog(@"Could not find service with UUID %@ on peripheral with UUID %@",
              serviceUUIDString,
              peripheral.identifier.UUIDString);


        NSString *errorMessage = [NSString stringWithFormat:@"Could not find service with UUID %@ on peripheral with UUID %@",
                                  serviceUUIDString,
                                  peripheral.identifier.UUIDString];
        pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_ERROR messageAsString:errorMessage];
        [self.commandDelegate sendPluginResult:pluginResult callbackId:command.callbackId];

        return nil;
    }

    CBCharacteristic *characteristic = [self findCharacteristicFromUUID:characteristicUUID service:service prop:prop];

    // Special handling for INDICATE. If charateristic with notify is not found, check for indicate.
    if (prop == CBCharacteristicPropertyNotify && !characteristic) {
        characteristic = [self findCharacteristicFromUUID:characteristicUUID service:service prop:CBCharacteristicPropertyIndicate];
    }

    // As a last resort, try and find ANY characteristic with this UUID, even if it doesn't have the correct properties
    if (!characteristic) {
        characteristic = [self findCharacteristicFromUUID:characteristicUUID service:service];
    }

    if (!characteristic)
    {
        NSLog(@"Could not find characteristic with UUID %@ on service with UUID %@ on peripheral with UUID %@",
              characteristicUUIDString,
              serviceUUIDString,
              peripheral.identifier.UUIDString);

        NSString *errorMessage = [NSString stringWithFormat:
                                  @"Could not find characteristic with UUID %@ on service with UUID %@ on peripheral with UUID %@",
                                  characteristicUUIDString,
                                  serviceUUIDString,
                                  peripheral.identifier.UUIDString];
        pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_ERROR messageAsString:errorMessage];
        [self.commandDelegate sendPluginResult:pluginResult callbackId:command.callbackId];

        return nil;
    }

    BLECommandContext *context = [[BLECommandContext alloc] init];
    [context setPeripheral:peripheral];
    [context setService:service];
    [context setCharacteristic:characteristic];
    return context;
}

-(NSString *) keyForPeripheral: (CBPeripheral *)peripheral andCharacteristic:(CBCharacteristic *)characteristic {
    return [NSString stringWithFormat:@"%@|%@|%@", [peripheral uuidAsString], [characteristic.service UUID], [characteristic UUID]];
}

+(BOOL) isKey: (NSString *)key forPeripheral:(CBPeripheral *)peripheral {
    NSArray *keyArray = [key componentsSeparatedByString: @"|"];
    return [[peripheral uuidAsString] compare:keyArray[0]] == NSOrderedSame;
}

-(void) cleanupOperationCallbacks: (CBPeripheral *)peripheral withResult:(CDVPluginResult *) result {
    for(id key in readCallbacks.allKeys) {
        if([BLECentralPlugin isKey:key forPeripheral:peripheral]) {
            NSString *callbackId = [readCallbacks valueForKey:key];
            [self.commandDelegate sendPluginResult:result callbackId:callbackId];
            [readCallbacks removeObjectForKey:key];
            NSLog(@"Cleared read callback %@ for key %@", callbackId, key);
        }
    }
    for(id key in writeCallbacks.allKeys) {
        if([BLECentralPlugin isKey:key forPeripheral:peripheral]) {
            NSString *callbackId = [writeCallbacks valueForKey:key];
            [self.commandDelegate sendPluginResult:result callbackId:callbackId];
            [writeCallbacks removeObjectForKey:key];
            NSLog(@"Cleared write callback %@ for key %@", callbackId, key);
        }
    }
    for(id key in startNotificationCallbacks.allKeys) {
        if([BLECentralPlugin isKey:key forPeripheral:peripheral]) {
            NSString *callbackId = [startNotificationCallbacks valueForKey:key];
            [self.commandDelegate sendPluginResult:result callbackId:callbackId];
            [startNotificationCallbacks removeObjectForKey:key];
            NSLog(@"Cleared start notification callback %@ for key %@", callbackId, key);
        }
    }
    for(id key in stopNotificationCallbacks.allKeys) {
        if([BLECentralPlugin isKey:key forPeripheral:peripheral]) {
            NSString *callbackId = [stopNotificationCallbacks valueForKey:key];
            [self.commandDelegate sendPluginResult:result callbackId:callbackId];
            [stopNotificationCallbacks removeObjectForKey:key];
            NSLog(@"Cleared stop notification callback %@ for key %@", callbackId, key);
        }
    }
    for(id key in notificationCallbacks.allKeys) {
        if([BLECentralPlugin isKey:key forPeripheral:peripheral]) {
            NSString *callbackId = [notificationCallbacks valueForKey:key];
            [self.commandDelegate sendPluginResult:result callbackId:callbackId];
            [notificationCallbacks removeObjectForKey:key];
            NSLog(@"Cleared notification callback %@ for key %@", callbackId, key);
        }
    }
}

#pragma mark - util

- (NSString*) centralManagerStateToString: (int)state {
    switch(state)
    {
        case CBCentralManagerStateUnknown:
            return @"State unknown (CBCentralManagerStateUnknown)";
        case CBCentralManagerStateResetting:
            return @"State resetting (CBCentralManagerStateUnknown)";
        case CBCentralManagerStateUnsupported:
            return @"State BLE unsupported (CBCentralManagerStateResetting)";
        case CBCentralManagerStateUnauthorized:
            return @"State unauthorized (CBCentralManagerStateUnauthorized)";
        case CBCentralManagerStatePoweredOff:
            return @"State BLE powered off (CBCentralManagerStatePoweredOff)";
        case CBCentralManagerStatePoweredOn:
            return @"State powered up and ready (CBCentralManagerStatePoweredOn)";
        default:
            return @"State unknown";
    }

    return @"Unknown state";
}

- (NSArray<CBUUID *> *) uuidStringsToCBUUIDs: (NSArray<NSString *> *)uuidStrings {
    NSMutableArray *uuids = [NSMutableArray new];
    for (int i = 0; i < [uuidStrings count]; i++) {
        CBUUID *uuid = [CBUUID UUIDWithString:[uuidStrings objectAtIndex: i]];
        [uuids addObject:uuid];
    }
    return uuids;
}

- (NSArray<NSUUID *> *) uuidStringsToNSUUIDs: (NSArray<NSString *> *)uuidStrings {
    NSMutableArray *uuids = [NSMutableArray new];
    for (int i = 0; i < [uuidStrings count]; i++) {
        NSUUID *uuid = [[NSUUID alloc]initWithUUIDString:[uuidStrings objectAtIndex: i]];
        [uuids addObject:uuid];
    }
    return uuids;
}

@end
