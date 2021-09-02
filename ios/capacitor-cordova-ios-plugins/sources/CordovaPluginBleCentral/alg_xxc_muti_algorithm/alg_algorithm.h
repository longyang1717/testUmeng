//
//  alg_algorithm.h
//  TestC
//
//  Created by long yang on 2019/9/23.
//  Copyright © 2019 龙洋. All rights reserved.
//

#ifndef alg_algorithm_h
#define alg_algorithm_h

#define  FS  250                //采样率
#define  FFT_WIN   512            //每次算法处理的数据长度
#define  FFT_STEP   FS/2        //计算步长（数据个数），对应于时间就是 半秒计算一次
#define  BLEn 29                //模拟接收蓝牙的数据长度
#define Bit7    0x80
#define Bit6    0x40
#define Bit5    0x20
#define Bit4    0x10
#define Bit3    0x08
#define Bit2    0x04
#define Bit1    0x02
#define Bit0    0x01

typedef struct
{
    unsigned int frame_dect_ok;    //检测帧的完整性，完整则为1，否则为0
    int frame_index;
    unsigned long  group_count;      //得到eeg的采样点组数

    double FP7_eeg_data[50];
    int FP7_leadoff_flag[50];

    double FP8_eeg_data[50];
    int FP8_leadoff_flag[50];

    int have_4sec_time_flag;    //得到4秒标志
    int time_flag_postion;        //时间标志在数据帧中的位置，如第一组时位置为0
    long time_counter;                //时间计数器
} get_eeg_info_t;

typedef struct {
    double tbr;
    double index_wa;
    double asy;
    double stress;
    double fatig;
    double energys[5];
    int isValid;
    int sqd[2];
    int sqdValid;
} algorithmTarget;

//标志位（输出变量）
typedef struct
{
    int valid_flag;                //算法运行标志，为1则表示运算进行了一次运算，其他情况为0
    int sqd_valid_flag;            //信号质量检测标志位，为1则表示运算进行了一次信号检测，其他情况为0
    int sqd_flag[2];            //信号质量检测结果，为0表示信号正常，其他情况为信号异常，结合sqd_valid_flag使用
    double power_relative[10][2]; // 能量值
    int poor_signal_flag;       //异常信号（接触正常，可能有波动）
    int poor_delay;
}struct_cfg;

//保存时间机制相关的参数,及其频谱变化的参数（中间变量）
typedef struct
{
    int fft_wind;                //FFT的时域数据长度
    int fft_step;                //FFT的步长
    int len2FFT;                //未进行处理的数据
    int nfft;                    //FFT变换长度，最好与数据长度一致
    int initial_flag;            //首次运行标志
}struct_cfg_window;

#define device_count 5
#define base_point 120  // 1s两个输出，1min为120个点，现在为一秒钟计算一次了，一分钟基线就是60个点
#define null  9.8    //给一个不能得到的值
#define INTERVAL_LEN    2    //必须大于1,用于算滴滴/鸟叫声时的间隔长度，先排序再算中间的值，若为偶数取中间值的均值
extern unsigned char chartohex(char ch);
extern unsigned char strToInt(unsigned char chH,unsigned char chL);

//保存计算结果（练习曲线的画图用）
typedef struct
{
    int index;
    int tbrhasnull;        //tbr是否有空值，1则有空值
    double *newtbrs;
    int *indexs;
    int nulllength;        //空值得数量
    int sections[4];    //分别存放走神、中立、专注、异常的数据个数
    double max;            //曲线最值
    double min;            //
    int *birds;            //鸟叫
    int bn;                //鸟叫的次数
    int *didis;            //水滴
    int dn;                //水滴的次数
    double baseHigh;
    double baseLow;
}struct_result;

typedef struct
{
    int error_flag;    //当分配内存等原因出问题时此处置1
    int have_null_flag;    //此数组里面是否有null；
    int null_num;        // 数组里面null的个数；
    int sections_num[3]; //此数组是统计三个区间(上中下)的个数
    float max_tbr; //最大值；
    float min_tbr;    //最小值；

    float * newtbrs_point; //数组是处理完成后的数组，将null值变为区间中值,即此处无null
    int newtbrs_len;

    int *didis_point; //此数组是计算滴滴叫的index；{2,100,102}
    //int didis_point[100];
    int didis_len;

    int *birds_point; //此数组是计算正念鸟叫的index{1,10,20}
    int birds_len;
} tbr_output_t;


void initAll(void);
int delete_device(char * serial);
int add_device(char *serial);
get_eeg_info_t Split_data_frame(char *str);
algorithmTarget get_algorithm_result(double data_raw[58],int index);
void curve_characters(double *indata, int indata_length, int *index_normal, struct_result *result);
double outlier_pro(double *indata, int len);
double median(double *indata, int len);
void result_smooth(int order, double *lp_a, double *lp_b, int delay, int length, double *indata, double *outdata);
void cal_report(double *indata, int indata_length, int *report);
void cur_smooth(double *indata, int indata_length);
void EEG_process(double indata[2][29], int len, double power_relative[10][2], double *corr);
void alg_preprocess(double indata[2][29], double outdata[2][29], int pre_flag);
void sosfilt(double sos[][6], double indata[29], double Y[][31], double zf[][2], int section);
void alg_fft(double data_fft[2][FFT_WIN], double feat_fft[2][FFT_WIN]);
void kfft(double pr[], double pi[], int n, int k, double fr[], double fi[]);
double cal_rms(double data_corr[2][FFT_WIN]);
double cal_corr(double data_corr[2][FFT_WIN]);
void alg_spectrum(double feat_fft[2][FFT_WIN], double power_absolute_log[10][2], double power_relative[10][2], double psd_absolute[10][2], double psd_relative[10][2]);
void sqd_fft(double data_sqd[FFT_WIN], int len, int *index_25hz, int *index_50hz, double fft_data[FFT_WIN]);
void smooth_array(double * array,int length,double * output);

typedef struct{
    double mean;
    double max;
    int rank;
} focusStruct;
void smooth_pro(double *indata, int len,double * output);
focusStruct focus_score_calculate(double *tbr, int len, double *score, double *mean, double *max, int *rank,int type);
void quantization_algorithm(double * array,int length,double baseHigh,double baseLow,double * output);
void rader_map(double *dat, int len, int *raders);

/*
 注意： 每次调用一次tbr_output_wave后，需要再调用一次freePoint
 */
extern tbr_output_t  tbr_output_wave(float *tbr_input_buf,int buf_length,float upBaseLine,float downBaseLine);
extern void freePoint(void);
void polyfit(int n, double x[], double y[], int poly_n, double a[]);

#endif /* alg_algorithm_h */

