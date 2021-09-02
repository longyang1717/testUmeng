//
//  alg_algorithm.c
//  TestC
//
//  Created by long yang on 2019/9/23.
//  Copyright © 2019 龙洋. All rights reserved.
//

#include "alg_algorithm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define filt1_2nd_n  8                            // 第一次预处理的二阶滤波器个数
const double filt1_sos[filt1_2nd_n][6] = {
    { 0.311538774228124, 0.623077548456248, 0.311538774228124, 1, 0.0736238463849783, 0.172531250527518 },
    { 0.00335294643902567, -0.00670589287805134, 0.00335294643902567, 1, -1.90886497891192, 0.911279007236065 },
    { 279.284972009193, -558.569944018386, 279.284972009193, 1, -1.95979168899663, 0.962270121310565 },
    { 0.0594633014335182, -0.0367619511873464, 0.0594633014335182, 1, -0.603069256508999, 0.950956781501069 },
    { 1.60037263931895, -0.989397147986892, 1.60037263931895, 1, -0.578174027308896, 0.959957449423027 },
    { 0.329557556928536, -0.203742115374692, 0.329557556928536, 1, -0.633385592112332, 0.960332677301608 },
    { 1.79017169208869, -1.10673647064671, 1.79017169208869, 1, -0.567968771835226, 0.984473568901825 },
    { 16.4201677630197, -10.1514277081815, 16.4201677630197, 1, -0.658268694604014, 0.984711116556859 },
}; //第一次预处理的滤波系数
#define filt2_2nd_n  1                        // 第二次预处理的二阶滤波器个数
const double filt2_sos[6] = { 0.674878167862040, -1.34975633572408, 0.674878167862040, 1, -1.24110061349823, 0.458412057949929 }; //第二次预处理的滤波系数

//频谱变换之前加的窗，与处理的数据长度有关（当前为2秒）
double window[FFT_WIN] ={
    0.080000, 0.080035, 0.080139, 0.080313, 0.080556, 0.080869, 0.081251, 0.081703, 0.082224, 0.082814, 0.083473, 0.084201, 0.084998, 0.085864, 0.086799, 0.087802,
    0.088873, 0.090013, 0.091221, 0.092496, 0.093839, 0.095250, 0.096728, 0.098273, 0.099884, 0.101563, 0.103307, 0.105118, 0.106994, 0.108936, 0.110943, 0.113014,
    0.115151, 0.117351, 0.119616, 0.121944, 0.124335, 0.126789, 0.129306, 0.131884, 0.134525, 0.137226, 0.139989, 0.142812, 0.145695, 0.148638, 0.151639, 0.154700,
    0.157819, 0.160995, 0.164229, 0.167520, 0.170867, 0.174270, 0.177728, 0.181241, 0.184808, 0.188429, 0.192103, 0.195829, 0.199608, 0.203438, 0.207319, 0.211250,
    0.215231, 0.219261, 0.223340, 0.227466, 0.231640, 0.235860, 0.240126, 0.244438, 0.248794, 0.253195, 0.257638, 0.262125, 0.266653, 0.271223, 0.275833, 0.280483,
    0.285173, 0.289901, 0.294667, 0.299470, 0.304309, 0.309184, 0.314094, 0.319038, 0.324015, 0.329025, 0.334067, 0.339140, 0.344244, 0.349377, 0.354539, 0.359729,
    0.364946, 0.370190, 0.375459, 0.380753, 0.386071, 0.391413, 0.396777, 0.402162, 0.407569, 0.412995, 0.418441, 0.423905, 0.429387, 0.434885, 0.440399, 0.445929,
    0.451472, 0.457029, 0.462599, 0.468180, 0.473772, 0.479374, 0.484985, 0.490605, 0.496232, 0.501865, 0.507505, 0.513149, 0.518797, 0.524449, 0.530103, 0.535758,
    0.541414, 0.547070, 0.552725, 0.558377, 0.564027, 0.569674, 0.575316, 0.580952, 0.586583, 0.592206, 0.597822, 0.603428, 0.609025, 0.614612, 0.620188, 0.625751,
    0.631301, 0.636838, 0.642360, 0.647866, 0.653356, 0.658829, 0.664284, 0.669720, 0.675137, 0.680533, 0.685908, 0.691261, 0.696591, 0.701897, 0.707179, 0.712436,
    0.717666, 0.722870, 0.728046, 0.733193, 0.738312, 0.743400, 0.748458, 0.753484, 0.758477, 0.763438, 0.768365, 0.773258, 0.778115, 0.782936, 0.787721, 0.792468,
    0.797177, 0.801847, 0.806477, 0.811067, 0.815616, 0.820124, 0.824589, 0.829011, 0.833389, 0.837723, 0.842012, 0.846256, 0.850453, 0.854603, 0.858705, 0.862760,
    0.866765, 0.870722, 0.874628, 0.878483, 0.882288, 0.886040, 0.889741, 0.893388, 0.896982, 0.900522, 0.904008, 0.907439, 0.910813, 0.914132, 0.917395, 0.920600,
    0.923748, 0.926838, 0.929869, 0.932841, 0.935754, 0.938607, 0.941400, 0.944132, 0.946803, 0.949413, 0.951960, 0.954446, 0.956868, 0.959228, 0.961524, 0.963757,
    0.965925, 0.968029, 0.970069, 0.972043, 0.973952, 0.975796, 0.977573, 0.979285, 0.980930, 0.982508, 0.984019, 0.985464, 0.986841, 0.988150, 0.989392, 0.990565,
    0.991671, 0.992708, 0.993677, 0.994577, 0.995409, 0.996172, 0.996865, 0.997490, 0.998045, 0.998532, 0.998949, 0.999296, 0.999574, 0.999783, 0.999922, 0.999991,
    0.999991, 0.999922, 0.999783, 0.999574, 0.999296, 0.998949, 0.998532, 0.998045, 0.997490, 0.996865, 0.996172, 0.995409, 0.994577, 0.993677, 0.992708, 0.991671,
    0.990565, 0.989392, 0.988150, 0.986841, 0.985464, 0.984019, 0.982508, 0.980930, 0.979285, 0.977573, 0.975796, 0.973952, 0.972043, 0.970069, 0.968029, 0.965925,
    0.963757, 0.961524, 0.959228, 0.956868, 0.954446, 0.951960, 0.949413, 0.946803, 0.944132, 0.941400, 0.938607, 0.935754, 0.932841, 0.929869, 0.926838, 0.923748,
    0.920600, 0.917395, 0.914132, 0.910813, 0.907439, 0.904008, 0.900522, 0.896982, 0.893388, 0.889741, 0.886040, 0.882288, 0.878483, 0.874628, 0.870722, 0.866765,
    0.862760, 0.858705, 0.854603, 0.850453, 0.846256, 0.842012, 0.837723, 0.833389, 0.829011, 0.824589, 0.820124, 0.815616, 0.811067, 0.806477, 0.801847, 0.797177,
    0.792468, 0.787721, 0.782936, 0.778115, 0.773258, 0.768365, 0.763438, 0.758477, 0.753484, 0.748458, 0.743400, 0.738312, 0.733193, 0.728046, 0.722870, 0.717666,
    0.712436, 0.707179, 0.701897, 0.696591, 0.691261, 0.685908, 0.680533, 0.675137, 0.669720, 0.664284, 0.658829, 0.653356, 0.647866, 0.642360, 0.636838, 0.631301,
    0.625751, 0.620188, 0.614612, 0.609025, 0.603428, 0.597822, 0.592206, 0.586583, 0.580952, 0.575316, 0.569674, 0.564027, 0.558377, 0.552725, 0.547070, 0.541414,
    0.535758, 0.530103, 0.524449, 0.518797, 0.513149, 0.507505, 0.501865, 0.496232, 0.490605, 0.484985, 0.479374, 0.473772, 0.468180, 0.462599, 0.457029, 0.451472,
    0.445929, 0.440399, 0.434885, 0.429387, 0.423905, 0.418441, 0.412995, 0.407569, 0.402162, 0.396777, 0.391413, 0.386071, 0.380753, 0.375459, 0.370190, 0.364946,
    0.359729, 0.354539, 0.349377, 0.344244, 0.339140, 0.334067, 0.329025, 0.324015, 0.319038, 0.314094, 0.309184, 0.304309, 0.299470, 0.294667, 0.289901, 0.285173,
    0.280483, 0.275833, 0.271223, 0.266653, 0.262125, 0.257638, 0.253195, 0.248794, 0.244438, 0.240126, 0.235860, 0.231640, 0.227466, 0.223340, 0.219261, 0.215231,
    0.211250, 0.207319, 0.203438, 0.199608, 0.195829, 0.192103, 0.188429, 0.184808, 0.181241, 0.177728, 0.174270, 0.170867, 0.167520, 0.164229, 0.160995, 0.157819,
    0.154700, 0.151639, 0.148638, 0.145695, 0.142812, 0.139989, 0.137226, 0.134525, 0.131884, 0.129306, 0.126789, 0.124335, 0.121944, 0.119616, 0.117351, 0.115151,
    0.113014, 0.110943, 0.108936, 0.106994, 0.105118, 0.103307, 0.101563, 0.099884, 0.098273, 0.096728, 0.095250, 0.093839, 0.092496, 0.091221, 0.090013, 0.088873,
    0.087802, 0.086799, 0.085864, 0.084998, 0.084201, 0.083473, 0.082814, 0.082224, 0.081703, 0.081251, 0.080869, 0.080556, 0.080313, 0.080139, 0.080035, 0.080000
};

typedef struct
{
    double filt1_zf[filt1_2nd_n + 1][4] ;    //保存每次滤波器的初始和结束状态，第一次预处理
    double filt2_zf[filt2_2nd_n + 1][4] ;  //保存每次滤波器的初始和结束状态，第二次预处理
    double data_raw[2][FFT_WIN + BLEn] ;        //原始数据缓存区
    double data_filt1[2][FFT_WIN + BLEn] ;        //第一次预处理数据缓存区
    double data_filt2[2][FFT_WIN + BLEn] ;        //第二次预处理数据缓存区
}struct_data_buffer;

//当前算法使用的变量
double filt1_zf[filt1_2nd_n + 1][4] = { 0 };    //保存每次滤波器的初始和结束状态，第一次预处理
double filt2_zf[filt2_2nd_n + 1][4] = { 0 };  //保存每次滤波器的初始和结束状态，第二次预处理
double g_data_raw[2][FFT_WIN + BLEn] = { 0 };        //原始数据缓存区
double g_data_filt1[2][FFT_WIN + BLEn] = { 0 };        //第一次预处理数据缓存区
double g_data_filt2[2][FFT_WIN + BLEn] = { 0 };        //第二次预处理数据缓存区
struct_cfg cfg;
struct_cfg_window cfg_time_step;
struct_result result;

/********************************聚会模式相关的参数/********************************/
int device_meet_mode[] = { 0, 0, 0, 0, 0 }; // 五个设备，0为空置，1为占用
int device_meet_busy = 0;  //多少个设备被占用
char *device_name[] = { "dev1", "dev2", "dev3", "dev4","dev5" };

//设备0
struct_cfg dev0_cfg;
struct_cfg_window dev0_cfg_time_step;
struct_data_buffer data_device0;

//设备1
struct_cfg dev1_cfg;
struct_cfg_window dev1_cfg_time_step;
struct_data_buffer data_device1;

//设备2
struct_cfg dev2_cfg;
struct_cfg_window dev2_cfg_time_step;
struct_data_buffer data_device2;

//设备3
struct_cfg dev3_cfg;
struct_cfg_window dev3_cfg_time_step;
struct_data_buffer data_device3;

//设备4
struct_cfg dev4_cfg;
struct_cfg_window dev4_cfg_time_step;
struct_data_buffer data_device4;

void arg_initial(struct_cfg *cfg, struct_cfg_window *cfg_time_step)
{

    cfg->valid_flag = 0;
    cfg->sqd_valid_flag = 0;
    cfg->sqd_flag[0] = 1;
    cfg->sqd_flag[1] = 1;

    //struct_cfg_window
    cfg_time_step->fft_wind = FFT_WIN;
    cfg_time_step->fft_step = FFT_STEP;
    cfg_time_step->len2FFT = 0;
    cfg_time_step->nfft = FFT_WIN;
    cfg_time_step->initial_flag = 1;
}

void initAll(void)
{
    arg_initial(&cfg, &cfg_time_step);
    arg_initial(&dev0_cfg, &dev0_cfg_time_step);
    arg_initial(&dev1_cfg, &dev1_cfg_time_step);
    arg_initial(&dev2_cfg, &dev2_cfg_time_step);
    arg_initial(&dev3_cfg, &dev3_cfg_time_step);
    arg_initial(&dev4_cfg, &dev4_cfg_time_step);
}

//****************************************** 其他调用函数的定义************************

int add_device(char *serial)
{
    if (device_meet_busy == device_count)  //设备已满
        return -1;

    int i = 0, j;
    for (i = 0; i < device_count; i++)
    {
        if (0 == device_meet_mode[i]||strcmp(serial, device_name[i])==0)
        {
            device_name[i] = serial;
            device_meet_busy = device_meet_busy + 1;
            device_meet_mode[i] = 1;
            return i;
        }
    }
    return 0;
}

int delete_device(char * serial)
{
    int i=0;
    for (i = 0; i < device_count; i++)
    {
        if (strcmp(serial, device_name[i])==0)
        {
            device_name[i] = "temp";
            device_meet_busy = device_meet_busy - 1;
            device_meet_mode[i] = 0;
            return i;
        }
    }
    return -1;
}

int max3(double vec3[3]) {
    int index = 0;
    index = (vec3[1] > vec3[0]) ? 1 : 0;
    index = (vec3[2] > vec3[index]) ? 2 : index;
    return index;
}

void sqd_fft(double data_sqd[FFT_WIN], int len, int *index_25hz, int *index_50hz, double fft_data[FFT_WIN])
{
    int i = 0;
    double average = 0, pr[FFT_WIN], pi[FFT_WIN], fr[FFT_WIN] = { 0 }, fi[FFT_WIN] = { 0 };
    for (i = 0; i < len; i++)
    {
        average = average + data_sqd[i];
    }
    average = average / len;
    for (i = 0; i < len; i++)
    {
        pr[i] = data_sqd[i] - average;
        pi[i] = 0.0;
    }
    //fft处理
    kfft(pr, pi, 512, 9, fr, fi);  //调用FFT函数   nfft = 2^k ，即 k = log2(nfft)
    for (i = 0; i < len; i++)  //加窗处理
    {
        fft_data[i] = 2.0*pr[i]/len;
        fft_data[i] = fft_data[i] * fft_data[i];
    }

    double vec3[3] = { fft_data[*index_25hz-1], fft_data[*index_25hz], fft_data[*index_25hz+1] };
    i = max3(vec3);
    *index_25hz = *index_25hz + (i - 1);

    vec3[0] = fft_data[*index_50hz - 1];
    vec3[1] = fft_data[*index_50hz];
    vec3[2] = fft_data[*index_50hz + 1];
    i = max3(vec3);
    *index_50hz = *index_50hz + (i - 1);
}

int eeg_detect(double data_sqd[512], int len)
{
    //功能：
    //    对输入的数据进行信号质量的判断
    //输入：
    //    data_sqd输入数据及其长度len
    //输出：
    //    sqd_flag，表示如下：
    //            0：信号正常
    //            1：电极脱落或者未接触（信号溢出。大于200mV）
    //            2：25Hz干扰
    //            3：50Hz干扰
    //            4：信号出现极大跳变
    //            5：信号为直线（未溢出）

    int sqd_flag = 0;

    //类型1：一半的数据幅值大于200mv(主要表现为一条直线,电极脱落)
    int thr = 201600;
    int i = len - 1, count = 0;
    int win = 256;    //只判断一秒钟的数据
    while (i>len - 256)
    {
        if (data_sqd[i] > thr)
            count++;
        if (count > win / 2)
        {
            sqd_flag = 1;
            return sqd_flag;
        }
        i--;
    }

    // type4：信号出现跳变（两个点之间幅度变化大于1mv）, 输出标签：4
    int    thr_diff = 1000;
    i = len - 1;
    while (i > len - 256 + 1)
    {
        if (fabs(data_sqd[i] - data_sqd[i - 1]) > thr_diff)
        {
            sqd_flag = 4;
            return sqd_flag;
        }
        i--;
    }

    //type 5：有一部分信号呈一条直线（幅值可能不定）, 输出标签： 5
    win = 20;
    i = len - 1;
    int j = 0;
    double diff = 0.0, sum=0;
    while (i > len - 256)  //具体实现为：一段数据的波动极小，比如20个数据之间的波动之和小于4uv
    {
        sum = 0; j = i;
        while (j>i - win)
        {
            diff = fabs(data_sqd[j] - data_sqd[j - 1]);
            sum = sum + diff;
            j--;
        }
        if (sum < win / 4)
        {
            sqd_flag = 5;
            return sqd_flag;
        }
        i--;
    }

    // FFT之前增加判断，以减少FFT的误判，没有直线，没有跳变,为了增加实时效果，判断的数据长度只有最后一秒，但是对FFT来说不够
    win = 20;
    i = len - 256; //已经判断过了
    while (i >= win)
    {
        sum = 0; j = i; diff = 0;
        while (j>i - win)
        {
            diff = fabs(data_sqd[j] - data_sqd[j - 1]);
            sum = sum + diff;
            j--;

            if (diff > thr_diff)
                return sqd_flag;
        }
        if (sum < win / 4)
            return sqd_flag;
        i--;
    }

    //类型2、3：工频干扰
    int index_25hz = 51, index_50hz = 102, index_20hz = 41, index_24hz = 48, index_26hz = 54, index_30hz = 61, index_45hz = 92, index_49hz = 99;
    int thr_25 = 1200, thr_50 = 2000;
    int freqvalue_25, freqvalue_50;
    double fft_data[FFT_WIN] = { 0 };
    sqd_fft( data_sqd, len, &index_25hz, &index_50hz, fft_data);

    double freq20_24 = 0, freq26_30 = 0, freq45_49 = 0, n = index_24hz - index_20hz + 1;
    for (i = index_20hz; i <= index_24hz; i++)
        freq20_24 = freq20_24 + fft_data[i];
    freq20_24 = 1.0*freq20_24 /n ;

    for (i = index_26hz; i <= index_30hz; i++)
        freq26_30 = freq26_30 + fft_data[i];
    freq26_30 = freq26_30 / n;

    for (i = index_45hz; i <= index_49hz; i++)
        freq45_49 = freq45_49 + fft_data[i];
    freq45_49 = freq45_49 / n;

    double value25 = fft_data[index_25hz], value50 = fft_data[index_50hz];

    if ((value25>5 * freq20_24) && (value25>5 * freq26_30) && (value25>thr_25))
        sqd_flag = 2;
    else if ((value50>20 * freq45_49) && (value50>thr_50) && (freq45_49>0))
        sqd_flag = 3;
    else if ((value25>thr_25) && (value25>fft_data[index_25hz-1]) && (value25>fft_data[index_25hz+1]) && (value25>4*freq20_24) && (value25>4*freq26_30)  )
        sqd_flag = 2;
    else if ( (value50>thr_50) && (value50>fft_data[index_50hz-1]) && (value50>fft_data[index_50hz+1]) && (value50>4*freq45_49) )
        sqd_flag = 3;
    return sqd_flag;
}

void poor_signal_detect()
{
    int i = 0, j = 0;
    double filt1[2][BLEn] = { 0 };
    for (i = 0; i < BLEn; i++)
    {
        filt1[0][i] = g_data_filt1[0][512 + i];
        filt1[1][i] = g_data_filt1[1][512 + i];
    }
    if (cfg.poor_signal_flag == 1)
    {
        if (cfg.poor_delay <= 512)
            cfg.poor_delay = cfg.poor_delay + BLEn;
        else
            cfg.poor_signal_flag = 0;
    }
    for (i = 0; i < 2; i++)
    {
        for (j = 0; j < BLEn; j++)
        {
            if (fabs(filt1[i][j])>100)
            {
                cfg.poor_signal_flag = 1;
                cfg.poor_delay = 0;
                break;
            }
        }
        if (cfg.poor_signal_flag == 1)
            break;
    }
}

void EEG_process(double indata[2][29], int len, double power_relative_out[10][2], double *feat_corr)
{
    /*
     功能：
     对从蓝牙接收到的29个数据进行两次预处理，并进行判断，如果累计接收到的数据大于0.5秒，则进行一次算法运算（计算能量及其指标）
     input:
     每次用蓝牙接收到的数据indata及其长度len
     output:
     cfg.valid_flag: 1 算法完成了一次计算，可以调用各个频段的能量特征，0表示没有进行算法计算，不调用输出参数
     power_relative[10][2] ;    // 相对能量 （全局变量）
     feat_corr:两个信号的相关性
     */

    //初始化
    cfg.valid_flag = 0;
    cfg.sqd_valid_flag = 0;
    int i = 0, j = 0;

    //保存原始数据，用于信号质量检测
    for (i = 0; i < FFT_WIN ; i++)
    {
        g_data_raw[0][i] = g_data_raw[0][i + BLEn];
        g_data_raw[1][i] = g_data_raw[1][i + BLEn];
    }
    for (i = 0; i < len; i++)
    {
        g_data_raw[0][FFT_WIN  + i] = indata[0][i];
        g_data_raw[1][FFT_WIN + i] = indata[1][i];
    }

    //第一次预处理：2Hz高通、65Hz低通、49 - 51Hz带阻，用于计算FFT和脑波能量
    double filt1_out[2][BLEn] = { 0 };
    alg_preprocess(indata, filt1_out, 1); // 第一次预处理
    for (i = 0; i < FFT_WIN ; i++)
    {
        g_data_filt1[0][i] = g_data_filt1[0][i + BLEn];
        g_data_filt1[1][i] = g_data_filt1[1][i + BLEn];
    }
    for (i = 0; i < len; i++)
    {
        g_data_filt1[0][FFT_WIN + i] = filt1_out[0][i];
        g_data_filt1[1][FFT_WIN + i] = filt1_out[1][i];
    }

    //第二次预处理：高通滤波22Hz
    double filt2_out[2][BLEn] = { 0 };
    alg_preprocess(filt1_out, filt2_out, 2);
    for (i = 0; i < FFT_WIN ; i++)
    {
        g_data_filt2[0][i] = g_data_filt2[0][i + BLEn];
        g_data_filt2[1][i] = g_data_filt2[1][i + BLEn];
    }
    for (i = 0; i < len; i++)
    {
        g_data_filt2[0][FFT_WIN + i] = filt2_out[0][i];
        g_data_filt2[1][FFT_WIN + i] = filt2_out[1][i];
    }

    //干扰检测
    poor_signal_detect();

    //时间判断机制，每0.5秒调用一次算法运行
    cfg_time_step.len2FFT = cfg_time_step.len2FFT + len;
    if (1 == cfg_time_step.initial_flag)
    {
        if (cfg_time_step.len2FFT < cfg_time_step.fft_wind)
            return;
        cfg_time_step.len2FFT = cfg_time_step.len2FFT - cfg_time_step.fft_wind;
        cfg_time_step.initial_flag = 0;
    }
    else if (cfg_time_step.len2FFT < cfg_time_step.fft_step){
        return;
    }
    else{
        cfg_time_step.len2FFT = cfg_time_step.len2FFT - cfg_time_step.fft_step;
    }

    //信号质量检测
    double data_sqd[2][FFT_WIN] = { 0 };
    j = 0;
    for (i = BLEn - cfg_time_step.len2FFT; i < FFT_WIN + BLEn - cfg_time_step.len2FFT; i++, j++)
    {
        data_sqd[0][j] = g_data_raw[0][i];
        data_sqd[1][j] = g_data_raw[1][i];
    }
    cfg.sqd_flag[0] = eeg_detect(data_sqd[0], FFT_WIN);
    cfg.sqd_flag[1] = eeg_detect(data_sqd[1], FFT_WIN);
    cfg.sqd_valid_flag = 1;

    // 特征计算
    if ((0 == cfg.sqd_flag[0]) && ((0 == cfg.sqd_flag[1])))  //信号质量正常，则进行算法计算
    {
        //获取要进行计算的数据
        double data_fft[2][FFT_WIN] = { 0 };
        double data_corr[2][FFT_WIN] = { 0 };
        for (i = BLEn - cfg_time_step.len2FFT, j = 0; i < FFT_WIN + BLEn - cfg_time_step.len2FFT; i++, j++)
        {
            data_fft[0][j] = g_data_filt1[0][i];
            data_fft[1][j] = g_data_filt1[1][i];
            data_corr[0][j] = g_data_filt2[0][i];
            data_corr[1][j] = g_data_filt2[1][i];
        }

        //计算FFT和脑波能量
        double feat_fft[2][FFT_WIN] = { 0 },psd[2][FFT_WIN] = { 0 };
        alg_fft(data_fft, feat_fft);        // 快速傅里叶变换
        for (i = 0; i < FFT_WIN; i++)
        {
            //psd[0][i] = feat_fft[0][i] * feat_fft[0][i] * 2 / FFT_WIN / FS;   //功率谱的实际计算方法，但是结果与1.0的有差异
            //psd[1][i] = feat_fft[1][i] * feat_fft[1][i] * 2 / FFT_WIN / FS;
            psd[0][i] = feat_fft[0][i] * feat_fft[0][i] * 2 / FFT_WIN / FS;    //dbay1.0用的方法，大家习惯了这种方式，所以与其他产品无法对比
            psd[1][i] = feat_fft[1][i] * feat_fft[1][i] * 2 / FFT_WIN / FS;
        }
        double power_absolute[10][2], power_relative[10][2], psd_absolute[10][2], psd_relative[10][2];
        alg_spectrum(psd, power_absolute, power_relative, psd_absolute, psd_relative);

        //计算相关性和rms
        //double feat_rms = 0;
        //feat_rms = cal_rms(data_fft);
        //double feat_corr = 0;
        *feat_corr = cal_corr(data_corr);

        //保存各个频段的能量计算结果
        //cfg.rms = feat_rms;
        //cfg.corr = feat_corr;
        for (i = 0; i < 10; i++)
        {
            //g_power_absolute[i][0] = power_absolute[i][0];
            //g_power_absolute[i][1] = power_absolute[i][1];
            power_relative_out[i][0] = power_relative[i][0];
            power_relative_out[i][1] = power_relative[i][1];
            //g_psd_absolute[i][0] = psd_absolute[i][0];
            //g_psd_absolute[i][1] = psd_absolute[i][1];
            //g_psd_relative[i][0] = psd_relative[i][0];
            //g_psd_relative[i][1] = psd_relative[i][1];
        }
        cfg.valid_flag = 1;        //算法完成一次计算
    }


}

void cfg_pxx_range(int band_index[10][2])
{
    int band_range[10][2] = { {1,4}, {4,8}, {8,13}, {8,10}, {10,13}, {13,15}, {13,37}, {20,37}, {37,45}, {13,22} };
    double freq_res = 0, idx_per_hz = 0;
    int i = 0;

    //    freq_res = FS / cfg_time_step.nfft;            //频率分辨率 frequency resolution
    idx_per_hz = 1.0*cfg_time_step.nfft / FS;       // NFFT / fs为每Hz需要的点数
    for (i = 0; i < 10; i++)
    {
        band_index[i][0] = round(band_range[i][0] * idx_per_hz + 1);
        band_index[i][1] = round(band_range[i][1] * idx_per_hz + 1);
    }
}

void alg_spectrum(double feat_fft[2][FFT_WIN], double power_absolute_log[10][2], double power_relative[10][2], double psd_absolute[10][2], double psd_relative[10][2])
{
    //功能：
    //    基于FFT的计算结果，分别获取各个频段的能量
    //输入：
    //    feat_fft：频谱图，基于改图获取各个频段的能量
    //输出：
    //    power_absolute： 绝对能量
    //    power_relative : 相对能量
    //    psd_absolute:    绝对功率谱
    //    psd_relative :    相对功率谱

    int i = 0,j=0;
    int band_index[10][2] = {0};
    cfg_pxx_range(band_index);
    int ind_sta = 0, ind_end = 0;
    double sum0, sum1, freq_res;
    freq_res = 1.0* FS / cfg_time_step.nfft;   //频率分辨率
    double power_absolute[10][2] = {0};

    //计算绝对能量及其功率谱
    for (i = 0; i < 10; i++)
    {
        ind_sta = band_index[i][0]-1;
        ind_end = band_index[i][1]-1;
        sum0 = 0;
        sum1 = 0;
        for (j = ind_sta; j <= ind_end; j++)
        {
            sum0 = sum0 + feat_fft[0][j];
            sum1 = sum1 + feat_fft[1][j];
        }
        power_absolute_log[i][0] = log10(sum0*freq_res);                //积分运算，计算各个频段的积分（矩形面积之和），//为了和1.0对齐，不再进行积分计算了
        power_absolute_log[i][1] = log10(sum1*freq_res);
        power_absolute[i][0] = sum0*freq_res;
        power_absolute[i][1] = sum1*freq_res;
        psd_absolute[i][0] = log10(sum0) / (ind_end - ind_sta + 1);
        psd_absolute[i][1] = log10(sum1) / (ind_end - ind_sta + 1);
    }

    //相对值
    double power_all[2] = { 0 };
    power_all[0] = power_absolute[1 - 1][ 0] + power_absolute[2 - 1][ 0] + power_absolute[3 - 1][ 0] + power_absolute[7 - 1][ 0] + power_absolute[9 - 1][ 0] ;
    power_all[1] = power_absolute[1 - 1][ 1] + power_absolute[2 - 1][ 1] + power_absolute[3 - 1][ 1] + power_absolute[7 - 1][ 1] + power_absolute[9 - 1][ 1] ;
    double psd_all[2] = { 0 };
    psd_all[0] = psd_absolute[1 - 1][ 0] + psd_absolute[2 - 1][ 0] + psd_absolute[3 - 1][ 0] + psd_absolute[7 - 1][ 0] + psd_absolute[9 - 1][ 0] ;
    psd_all[1] = psd_absolute[1 - 1][ 1] + psd_absolute[2 - 1][ 1] + psd_absolute[3 - 1][ 1] + psd_absolute[7 - 1][ 1] + psd_absolute[9 - 1][ 1] ;
    for (i = 0; i < 10; i++)
    {
        power_relative[i][0] = 100 * power_absolute[i][0] / power_all[0];
        power_relative[i][1] = 100 * power_absolute[i][1] / power_all[1];

        psd_relative[i][0] = 100 * psd_absolute[i][0] / psd_all[0];
        psd_relative[i][1] = 100 * psd_absolute[i][1] / psd_all[1];
    }
}

void cal_std(double * data, int len, double *result)
{
    double sum = 0, var = 0, average = 0, standard = 0;
    for (int i = 0; i <len; i++)
    {
        sum += data[i];    //求和
    }
    average = sum / len;
    for (int j = 0; j <len; j++)
    {
        var += pow(data[j] - average, 2);//求方差
    }
    var = var / (len - 1);
    standard = pow(var, 0.5);    //求标准差

    result[0] = average;
    result[1] = standard;
}

double cal_corr(double data_corr[2][FFT_WIN])
{
    int i = 0, j = 0, N = FFT_WIN;
    double corr = 0.0, sum = 0;
    double num = 0, den = 0; //分子numerator和分母denominator
    double ave1 = 0, ave2 = 0, std1 = 0, std2 = 0;

    double ave_std[2] = { 0 };
    cal_std(data_corr[0], FFT_WIN, ave_std);
    ave1 = ave_std[0];
    std1 = ave_std[1];
    cal_std(data_corr[1], FFT_WIN, ave_std);
    ave2 = ave_std[0];
    std2 = ave_std[1];
    for (j = 0; j < N; j++)
    {
        sum = sum + ((data_corr[0][j] - ave1) / std1)*((data_corr[1][j] - ave2) / std2);
    }
    corr = sum / (N - 1);
    return corr;
}

double cal_rms(double data_corr[2][FFT_WIN])
{
    int i = 0, j = 0, N = FFT_WIN;
    double rms = 0.0, sum;
    for (i = 0; i < 2; i++)   //two channel
    {
        sum = 0;
        for (j = 0; j < N; j++)
            sum = sum + data_corr[i][j] * data_corr[i][j];
        rms = rms + sqrt(sum / N);
    }
    rms = rms / 2;
    return rms;
}

void alg_fft(double data_fft[2][FFT_WIN], double feat_fft[2][FFT_WIN])
{
    //功能：对输入数据data_fft进行FFT计算，并保存在feat_fft中

    int i = 0, j = 0;
    double data[FFT_WIN] = { 0 };
    double pr[FFT_WIN]={0}, pi[FFT_WIN]={0}, fr[FFT_WIN]={0}, fi[FFT_WIN]={0};
    //    double *window = cfg_time_step.window;
    for (i = 0; i < 2; i++)   //two channel
    {
        for (j = 0; j < FFT_WIN; j++)  //加窗处理
        {
            pr[j] = data_fft[i][j] * window[j] * 1.586; //海明窗恢复系数
            pi[i] = 0.0;
        }
        //fft处理
        kfft(pr, pi, 512, 9, fr, fi);  //调用FFT函数   nfft = 2^k ，即 k = log2(nfft)
        for (j = 0; j < FFT_WIN; j++)  //加窗处理
        {
            feat_fft[i][j] = pr[j];
        }
    }
}

void alg_preprocess(double indata[2][29], double outdata[2][29], int pre_flag)
{
    //功能：
    //    以二阶级联的形式的实现滤波器的处理，两次预处理共同使用同一个函数，以标志位pre_flag进行区分，1为第一次预处理，2为第二次预处理
    //输入：
    //    indata：要进行滤波的数据
    //    pre_flag：1为第一次预处理，2为第二次预处理
    //输出：
    //    outdata：滤波之后的数据

    int ii = 0, j = 0, i = 0;
    int section = filt1_2nd_n;            //二阶滤波器的个数
    double zf[filt1_2nd_n + 1][2] = { 0 };
    double *sos = filt1_sos;
    double Y[filt1_2nd_n + 1][31] = { 0 };
    if (2 == pre_flag)
    {
        section = filt2_2nd_n;
        sos = filt2_sos;
    }

    for (ii = 0; ii < 2; ii++)  //two channel EEG
    {
        for (i = 0; i <= section; i++)
        {
            if (1 == pre_flag)
            {
                zf[i][0] = filt1_zf[i][ii * 2 + 0];
                zf[i][1] = filt1_zf[i][ii * 2 + 1];
            }
            else{
                zf[i][0] = filt2_zf[i][ii * 2 + 0];
                zf[i][1] = filt2_zf[i][ii * 2 + 1];
            }
        }
        sosfilt(sos, indata[ii], Y, zf, section);  //多个2阶滤波器的具体实现

        for (j = 0; j < 29; j++)        //获取滤波后的值
        {
            outdata[ii][j] = Y[section][2 + j];    //获取最后一行
        }
        for (i = 0; i <= section; i++)    //最后两列作为下一次滤波的初始条件
        {
            if (1 == pre_flag)
            {
                filt1_zf[i][ii * 2 + 0] = Y[i][29];
                filt1_zf[i][ii * 2 + 1] = Y[i][30];
            }
            else{
                filt2_zf[i][ii * 2 + 0] = Y[i][29];
                filt2_zf[i][ii * 2 + 1] = Y[i][30];
            }
        }
    }

    //保存滤波后的数据(测试专用)
    //FILE *stream;
    //if (1 == pre_flag){
    //    stream = fopen("data_result\\filt1_data_c.txt", "a");    //保存滤波后的数据
    //}
    //else{
    //    stream = fopen("data_result\\filt2_data_c.txt", "a");
    //}
    //for (i = 0; i < 29; i = i + 1)
    //{
    //    fprintf(stream, "%f    %f\n", outdata[0][i], outdata[1][i]);
    //}
    //fclose(stream);
}

void sosfilt(double sos[][6], double indata[29], double Y[][31], double zf[][2], int section)
{
    //功能：
    //    以二阶级联的形式滤波器的具体实现，以矩阵的形式实现过个滤波器的处理（该函数参数的Matlab的代码）
    //参数说明：
    //    sos：各个滤波器的滤波系数，一行为一个滤波器
    //    indata[29]：输入数据
    //    Y[][31]：所有2阶滤波器的输出
    //    zf[][2]：各个滤波器的终止状态
    //    section：滤波器的个数

    //double Y[8 + 1][29 + 2] = {0};
    int i = 0, j = 0;
    //第一行的3-end列幅值当前数据，1-2列为为上一次的最后两个数据
    for (i = 0 + 2; i < 29 + 2; i++)
    {
        Y[0][i] = indata[i - 2];
    }
    for (i = 0; i <= section; i++)
    {
        Y[i][0] = zf[i][0];
        Y[i][1] = zf[i][1];
    }

    //filter
    int ii = 0;
    double a[3] = { 0 }, b[3] = { 0 }, sum_a = 0, sum_b = 0;
    for (i = 0; i < 29; i++)
    {
        ii = i + 2; sum_a = 0; sum_b = 0;
        for (j = 0; j < section; j++) // 滤波次数，几个2阶滤波器
        {
            b[0] = sos[j][0];
            b[1] = sos[j][1];
            b[2] = sos[j][2];
            a[0] = sos[j][3];
            a[1] = sos[j][4];
            a[2] = sos[j][5];
            sum_b = b[0] * Y[j][ii] + b[1] * Y[j][ii - 1] + b[2] * Y[j][ii - 2];
            sum_a = a[1] * Y[j + 1][ii - 1] + a[2] * Y[j + 1][ii - 2];
            Y[j + 1][ii] = sum_b - sum_a;
        }
    }
}

void kfft(double pr[], double pi[], int n, int k, double fr[], double fi[])
{
    //double pr[n]    存放n个采样输入的实部，返回离散傅里叶变换的摸
    //double pi[n]    存放n个采样输入的虚部
    //double fr[n]    返回离散傅里叶变换的n个实部
    //double fi[n]    返回离散傅里叶变换的n个虚部
    //int n    采样点数
    //int k    满足n = 2^k

    int it, m, is, i, j, nv, l0;
    double p, q, s, vr, vi, poddr, poddi;
    for (it = 0; it <= n - 1; it++)  //将pr的实部和虚部循环赋值给fr[]和fi[]
    {
        m = it;
        is = 0;
        for (i = 0; i <= k - 1; i++)
        {
            j = m / 2;
            is = 2 * is + (m - 2 * j);
            m = j;
        }
        fr[it] = pr[is];
        fi[it] = pi[is];
    }
    pr[0] = 1.0;
    pi[0] = 0.0;
    p = 6.283185306 / (1.0*n);
    pr[1] = cos(p); //将w=e^-j2pi/n用欧拉公式表示
    pi[1] = -sin(p);

    for (i = 2; i <= n - 1; i++)  //计算pr[]
    {
        p = pr[i - 1] * pr[1];
        q = pi[i - 1] * pi[1];
        s = (pr[i - 1] + pi[i - 1])*(pr[1] + pi[1]);
        pr[i] = p - q; pi[i] = s - p - q;
    }
    for (it = 0; it <= n - 2; it = it + 2)
    {
        vr = fr[it];
        vi = fi[it];
        fr[it] = vr + fr[it + 1];
        fi[it] = vi + fi[it + 1];
        fr[it + 1] = vr - fr[it + 1];
        fi[it + 1] = vi - fi[it + 1];
    }
    m = n / 2;
    nv = 2;
    for (l0 = k - 2; l0 >= 0; l0--) //蝴蝶操作
    {
        m = m / 2;
        nv = 2 * nv;
        for (it = 0; it <= (m - 1)*nv; it = it + nv)
            for (j = 0; j <= (nv / 2) - 1; j++)
            {
                p = pr[m*j] * fr[it + j + nv / 2];
                q = pi[m*j] * fi[it + j + nv / 2];
                s = pr[m*j] + pi[m*j];
                s = s*(fr[it + j + nv / 2] + fi[it + j + nv / 2]);
                poddr = p - q;
                poddi = s - p - q;
                fr[it + j + nv / 2] = fr[it + j] - poddr;
                fi[it + j + nv / 2] = fi[it + j] - poddi;
                fr[it + j] = fr[it + j] + poddr;
                fi[it + j] = fi[it + j] + poddi;
            }
    }
    for (i = 0; i<n; i++)
    {
        pr[i] = sqrt(fr[i] * fr[i] + fi[i] * fi[i]);  //幅度的计算
    }
    return;
}

void global_get(struct_cfg *cfg, struct_cfg_window *cfg_time_step, struct_cfg *dev_cfg, struct_cfg_window *dev_cfg_time_step, struct_data_buffer *data_device)
{
    int i = 0, j = 0;
    cfg_time_step->len2FFT = dev_cfg_time_step->len2FFT;
    cfg_time_step->initial_flag = dev_cfg_time_step->initial_flag;

    //double filt1_zf[filt1_2nd_n + 1][4] = { 0 };    //保存每次滤波器的初始和结束状态，第一次预处理
    for (i = 0; i < filt1_2nd_n + 1; i++)
    {
        for (j = 0; j < 4; j++)
        {
            filt1_zf[i][j] = data_device->filt1_zf[i][j];
        }
    }

    //double filt2_zf[filt2_2nd_n + 1][4] = { 0 };  //保存每次滤波器的初始和结束状态，第二次预处理
    for (i = 0; i < filt2_2nd_n + 1; i++)
    {
        for (j = 0; j < 4; j++)
        {
            filt2_zf[i][j] = data_device->filt2_zf[i][j];
        }
    }

    //double g_data_raw[2][FFT_WIN + BLEn] = { 0 };        //原始数据缓存区
    for (i = 0; i < 2; i++)
    {
        for (j = 0; j < FFT_WIN + BLEn; j++)
        {
            g_data_raw[i][j] = data_device->data_raw[i][j];
        }
    }

    //double g_data_filt1[2][FFT_WIN + BLEn] = { 0 };        //第一次预处理数据缓存区
    for (i = 0; i < 2; i++)
    {
        for (j = 0; j < FFT_WIN + BLEn; j++)
        {
            g_data_filt1[i][j] = data_device->data_filt1[i][j];
        }
    }
    //double g_data_filt2[2][FFT_WIN + BLEn] = { 0 };        //第二次预处理数据缓存区
    for (i = 0; i < 2; i++)
    {
        for (j = 0; j < FFT_WIN + BLEn; j++)
        {
            g_data_filt2[i][j] = data_device->data_filt2[i][j];
        }
    }


}

void global_save(struct_cfg *cfg, struct_cfg_window *cfg_time_step, struct_cfg *dev_cfg, struct_cfg_window *dev_cfg_time_step, struct_data_buffer *data_device)
{
    int i = 0, j = 0;
    dev_cfg_time_step->len2FFT = cfg_time_step->len2FFT;
    dev_cfg_time_step->initial_flag = cfg_time_step->initial_flag;

    //double filt1_zf[filt1_2nd_n + 1][4] = { 0 };    //保存每次滤波器的初始和结束状态，第一次预处理
    for (i = 0; i < filt1_2nd_n + 1; i++)
    {
        for (j = 0; j < 4; j++)
        {
            data_device->filt1_zf[i][j] = filt1_zf[i][j];
        }
    }

    //double filt2_zf[filt2_2nd_n + 1][4] = { 0 };  //保存每次滤波器的初始和结束状态，第二次预处理
    for (i = 0; i < filt2_2nd_n + 1; i++)
    {
        for (j = 0; j < 4; j++)
        {
            data_device->filt2_zf[i][j] = filt2_zf[i][j];
        }
    }

    //double g_data_raw[2][FFT_WIN + BLEn] = { 0 };        //原始数据缓存区
    for (i = 0; i < 2; i++)
    {
        for (j = 0; j < FFT_WIN + BLEn; j++)
        {
            data_device->data_raw[i][j] = g_data_raw[i][j];
        }
    }

    //double g_data_filt1[2][FFT_WIN + BLEn] = { 0 };        //第一次预处理数据缓存区
    for (i = 0; i < 2; i++)
    {
        for (j = 0; j < FFT_WIN + BLEn; j++)
        {
            data_device->data_filt1[i][j] = g_data_filt1[i][j];
        }
    }
    //double g_data_filt2[2][FFT_WIN + BLEn] = { 0 };        //第二次预处理数据缓存区
    for (i = 0; i < 2; i++)
    {
        for (j = 0; j < FFT_WIN + BLEn; j++)
        {
            data_device->data_filt2[i][j] = g_data_filt2[i][j];
        }
    }

}

algorithmTarget get_algorithm_result(double data_raw[58],int index)
{
    switch (index) {
        case 0:
            global_get(&cfg, &cfg_time_step, &dev0_cfg, &dev0_cfg_time_step, &data_device0);
            break;
        case 1:
            global_get(&cfg, &cfg_time_step, &dev1_cfg, &dev1_cfg_time_step, &data_device1);    //获取相关设备的参数
            break;
        case 2:
            global_get(&cfg, &cfg_time_step, &dev2_cfg, &dev2_cfg_time_step, &data_device2);    //获取相关设备的参数
            break;
        case 3:
            global_get(&cfg, &cfg_time_step, &dev3_cfg, &dev3_cfg_time_step, &data_device3);    //获取相关设备的参数
            break;
        case 4:
            global_get(&cfg, &cfg_time_step, &dev4_cfg, &dev4_cfg_time_step, &data_device4);    //获取相关设备的参数
            break;
    }

    algorithmTarget target;
    double raw[2][29] = {0};
    for (int i = 0; i < 29; i++) {
        raw[0][i] = data_raw[i];
        raw[1][i] = data_raw[i + 29];
    }
    double power_relative[10][2] = {0};
    memcpy(power_relative , cfg.power_relative, 10*2*sizeof(double));
    double corr=0,index_wa=0,index_focus=0,index_asy=0,index_fatig=0,index_stress=0;                // 相对能量
    EEG_process(raw, BLEn, power_relative, &corr);
    target.isValid = cfg.valid_flag;
    target.sqdValid=cfg.sqd_valid_flag;
    memcpy(target.sqd, cfg.sqd_flag, 2*sizeof(int));
    if (cfg.sqd_valid_flag == 1) {
        //******************组合成任意想要的特征********************//
        double TBR, delta, theta, beta, alpha, Hibeta, ACbeta, gamma ;
        //各频段为：{'delta','theta','alpha','lower alpha','upper alpha','smr','beta','high beta','gamma', 'AC beta' };
        //**** 1.delta theta alpha beta gamma ACbeta对应的下标为 0 1 2 6 8 9 ，
        delta = (power_relative[0][0] + power_relative[0][1]) / 2.0;
        theta = (power_relative[1][0] + power_relative[1][1]) / 2.0;
        alpha = (power_relative[2][0] + power_relative[2][1]) / 2.0;
        beta = (power_relative[6][0] + power_relative[6][1]) / 2.0;
        gamma = (power_relative[8][0] + power_relative[8][1]) / 2.0;
        ACbeta = (power_relative[9][0] + power_relative[9][1]) / 2.0;
        Hibeta = (power_relative[7][0] + power_relative[7][1]) / 2.0;

        //**** 2.沉浸曲线 index_wa :用到的输出有theta、ACbeta、相关性corr 这三个特征进行计算
        if (0 == ACbeta)  //beta不能为零
            TBR = 0;
        else{
            TBR = log10((power_relative[1][0]/power_relative[9][0]+power_relative[1][1]/power_relative[9][1])/2.0);//log10(theta / ACbeta);
            if (TBR < -1)
                TBR = -1;
            else if (TBR>1)
                TBR = 1;
        }
        index_wa = -2.0*corr + TBR;
        //异常情况的标志保存，便于后续问题的分析，同时不影响当次练习
        if (isnan(index_wa) == 1)   //int isnan(x)，当x时nan返回1，其它返回0；
        {
            if (isnan(corr) == 1)
                cfg.sqd_flag[0] = 11;
            if (isnan(TBR) == 1)
                cfg.sqd_flag[1] = 11;
            index_wa = 0;
        }
        if (isinf(index_wa) != 0)   //int isinf(x) ,当x是正无穷是返回1，当x是负无穷时返回-1，其它返回0。
        {
            if (isinf(corr) != 0)
                cfg.sqd_flag[0] = 12;
            if (isinf(TBR) != 0)
                cfg.sqd_flag[1] = 12;
            index_wa = 0;
        }


        //**** 3.专注度 focus：
        index_focus = TBR ;

        //**** 4.情绪: 用到的指标为 alpha 能量
        index_asy = log10(power_relative[2][1]) - log10(power_relative[2][0]);  //右脑-左脑的alpha能量

        //**** 5.疲劳度 : index_fatig = (delta + theta + alpha)/beta
        index_fatig = log10(((power_relative[0][0]+power_relative[1][0]+power_relative[2][0])/power_relative[6][0]+(power_relative[0][1]+power_relative[1][1]+power_relative[2][1])/power_relative[6][1])/2.0);

        //**** 6.压力： index_stress = Hibeta/beta
        index_stress = log10((power_relative[7][0]/power_relative[6][0] + power_relative[7][1]/power_relative[6][1])/2.0);

        target.tbr = index_wa;
        target.index_wa = index_focus;
        target.asy=index_asy;
        target.stress=index_stress;
        target.fatig=index_fatig;
        target.energys[0] = alpha;// alpha
        target.energys[1] = beta; // beta
        target.energys[2] = theta; // theta
        target.energys[3] = delta; //  delta
        target.energys[4] = gamma;// gamma
    }
    memcpy( cfg.power_relative , power_relative, 10*2*sizeof(double));
    switch (index) {
        case 0:
            global_save(&cfg, &cfg_time_step, &dev0_cfg, &dev0_cfg_time_step, &data_device0);
            break;
        case 1:
            global_save(&cfg, &cfg_time_step, &dev1_cfg, &dev1_cfg_time_step, &data_device1);    //获取相关设备的参数
            break;
        case 2:
            global_save(&cfg, &cfg_time_step, &dev2_cfg, &dev2_cfg_time_step, &data_device2);    //获取相关设备的参数
            break;
        case 3:
            global_save(&cfg, &cfg_time_step, &dev3_cfg, &dev3_cfg_time_step, &data_device3);    //获取相关设备的参数
            break;
        case 4:
            global_save(&cfg, &cfg_time_step, &dev4_cfg, &dev4_cfg_time_step, &data_device4);    //获取相关设备的参数
            break;
    }

    return target;
}

// IIR滤波器离线处理
void filter(int ord, double *a, double *b, int np, double *x, double *y)
{
    int i, j;
    y[0] = b[0] * x[0];
    for (i = 1; i<ord + 1; i++)
    {
        y[i] = 0.0;
        for (j = 0; j<i + 1; j++)
            y[i] = y[i] + b[j] * x[i - j];
        for (j = 0; j<i; j++)
            y[i] = y[i] - a[j + 1] * y[i - j - 1];
    }
    /* end of initial part */
    for (i = ord + 1; i<np + 1; i++)
    {
        y[i] = 0.0;
        for (j = 0; j<ord + 1; j++)
            y[i] = y[i] + b[j] * x[i - j];
        for (j = 0; j<ord; j++)
            y[i] = y[i] - a[j + 1] * y[i - j - 1];
    }
    return;
} /* end of filter */

/*
 函数名：Split_data_frame
 输入参数：
 str为需要输入的蓝牙的整个数据帧，为字符串形式
 get_data为一个解析算出来的EEG数据信息

 */
get_eeg_info_t Split_data_frame(char *str) {
    get_eeg_info_t get_data;
    unsigned long len, i, j;
    get_data.frame_index = strToInt(str[0], str[1]);
    len = (strlen(str) - 6) / 16;
    if (len == ((strToInt(str[2], str[3]) - 3) / 8)) {
        get_data.group_count = len;
        get_data.frame_dect_ok = 1;
        for (i = 0, j = 0; i < len; i++) {
            unsigned char eeg_header = strToInt(str[4 + i * 16], str[4 + i * 16 + 1]);
            if (eeg_header == 0x80) {    //timercounter
                long cnt;
                unsigned char m;
                get_data.have_4sec_time_flag = 1;
                get_data.time_flag_postion = (int) i;
                cnt = 0;
                for (m = 0; m < 6; m++) {
                    cnt <<= 8;
                    cnt += strToInt(str[4 + i * 16 + 14 - m], str[4 + i * 16 + 15 - m]);
                }
                get_data.time_counter = cnt;
                get_data.group_count--;
            } else {
                unsigned char m;
                long dat_tmp;

                //lead off dect
                if (eeg_header & Bit6) {
                    get_data.FP7_leadoff_flag[j] = 1;
                } else {
                    get_data.FP7_leadoff_flag[j] = 0;
                }
                if (eeg_header & Bit5) {
                    get_data.FP8_leadoff_flag[j] = 1;
                } else {
                    get_data.FP8_leadoff_flag[j] = 0;
                }

                dat_tmp = 0;
                for (m = 0; m < 3; m++)    // FP7
                {
                    dat_tmp <<= 8;
                    dat_tmp += strToInt(str[4 + i * 16 + 4 + 2 * m],
                                        str[4 + i * 16 + 5 + 2 * m]);//4-9
                }

                if (dat_tmp & 0x800000)    //负数
                {
                    dat_tmp = dat_tmp - 0xffffff + 1;
                }
                get_data.FP7_eeg_data[j] = (double) dat_tmp *
                0.02404054005940755;    //(2420000 * dat_tmp) / 8388608 / 12;    //uv

                dat_tmp = 0;
                for (m = 0; m < 3; m++)    // FP8
                {
                    dat_tmp <<= 8;
                    dat_tmp += strToInt(str[4 + i * 16 + 10 + 2 * m],
                                        str[4 + i * 16 + 11 + 2 * m]);//10,11;12,13;14,15
                }

                if (dat_tmp & 0x800000)    //负数
                {
                    dat_tmp = dat_tmp - 0xffffff + 1;
                }
                get_data.FP8_eeg_data[j] = (double) dat_tmp *
                0.02404054005940755;    //(2420000 * dat_tmp) / 8388608 / 12; //uv
                j++;
            }
        }
    } else {
        //printf("test dect ng,len=%d,%d",len,((strToInt(str[2],str[3])-3)/8));
        get_data.frame_dect_ok = 0;
    }
    return get_data;
}


#pragma mark -- 计算tbr报告

//****************************************** 2.基线练习报告（四个状态） **************
//**** 2.1 计算基线的均值及其标准差
void cal_report(double *indata, int indata_length, int *report)
{
    double base_data[base_point] = { 0 };
    int base_len = base_point,i=0,j=0;
    for (i = 0; i < base_len; i++)
    {
        base_data[i] = indata[i];   //基线数据
    }
    double result_base[2] = { 0 };
    cal_std(base_data, base_len, result_base);

    //**** 2.2 练习数据的均值及其标准差
    double *test_data;
    int test_len = 0;
    test_len = indata_length - base_point;
    test_data = (double*)malloc(sizeof(double)*test_len);
    for (i = base_point; i < indata_length; i++)
    {
        test_data[j++] = indata[i];   //测试数据
    }
    double result_test[2] = { 0 };
    cal_std(test_data, test_len, result_test);
    free(test_data);

    //**** 指标1：沉浸度/沉浸指数
    double up_th = result_base[0] + result_base[1];
    double down_th = result_base[0] - result_base[1];
    int count = 0, wa_score = 0;
    for (i = 0; i < indata_length; i++)
    {
        if (indata[i] <= down_th)
            count++;
    }
    wa_score = round(1.0*count / indata_length * 100);
    report[0] = wa_score;

    //**** 指标2：专注度（利用基线对数据进行量化，用于计算专注度）
    double x1 = up_th, x2 = down_th, y1 = 60, y2 = 40;   //y=a*x+b, x-映射前，y-映射后
    double a = 0, b = 0;
    double *focus_data, sum = 0;
    int focus_score;
    a = (y1 - y2) / (x1 - x2);
    b = y1 - a*x1;
    focus_data = (double*)malloc(sizeof(double)*indata_length);
    for (i = 0; i < indata_length; i++)
    {
        focus_data[i] = indata[i] * a + b;
        if (focus_data[i]>100)
            focus_data[i] = 100;
        if (focus_data[i]<0)
            focus_data[i] = 0;
        sum += focus_data[i];
    }
    //focus_score = 100 - round(sum / indata_length);  //只有计算专注度需要
    focus_score = round(sum / indata_length);
    report[1] = focus_score;
    free(focus_data);

    //**** 指标3：趋势性（不用斜率算了，计算稍微有些复杂，直接对比均值简单些）
    double ratio = 0;
    int trend_score = 0;
    ratio = (result_base[0] - result_test[0]) / result_test[1];        //值越大，向下的趋势性越好
    ratio = ratio*0.5;
    if (ratio>1)
        ratio = 1;
    if (ratio < -1)
        ratio = -1;
    trend_score = round(60 + 40 * ratio);  //给定一个基础分数（60或80），练习基线低则加分，高则减分
    report[2] = trend_score;

    //**** 指标4：稳定性,区间[50-60-100],中间值为60，值越大越稳定
    int stability = 0;
    ratio = (result_base[1] - result_test[1]) / result_base[1] * 100; //由于该改值波动范围相对较小，所以不适合采用趋势性的计算方法
    ratio = round(ratio * 1);
    if (ratio>40)
        ratio = 40;
    if (ratio < -40)
        ratio = -40;
    stability = round(60 + ratio);   //练习平稳（相对基线）则加分，否则减分
    report[3] = stability;

    //**** 指标5：基线稳定性（该指标只有基线异常导致数据结果异常时进行输出，可供参考）
    int    base_abnormal = 0;
    if ((stability >= 90) && (wa_score <= 10))
        base_abnormal = 1;
    report[4] = base_abnormal;

}

double median(double *indata, int len)   //计算中值
{
    int i, j, n = len;
    double buf,med;
    double *sortdata = (double*)malloc(len * sizeof(double));
    for (i = 0; i<=n - 1; i++)  //比较n-1轮
    {
        sortdata[i] = indata[i];
    }

    //冒泡排序(从大到小)
    for (i = 0; i<n - 1; ++i)  //比较n-1轮
    {
        for (j = 0; j<n - 1 - i; ++j)  //每轮比较n-1-i次,
        {
            if (sortdata[j] < sortdata[j + 1])
            {
                buf = sortdata[j];
                sortdata[j] = sortdata[j + 1];
                sortdata[j + 1] = buf;
            }
        }
    }
    int index = 0;
    if (len % 2 == 0) // 偶数
    {
        index = len / 2 - 1;
        med = (sortdata[index] + sortdata[index+1]) / 2.0;  //参考的Maltab的计算方式
    }
    else
    {
        index = (len - 1) / 2;
        med = sortdata[index];
    }
    free(sortdata);

    return med;
}

double outlier_pro(double *indata, int len)
{
    // 对标Matlab的filloutliersh函数，outdata = filloutliers(indata, 'center', 'median');
    // c = -1 / (sqrt(2)*erfcinv(3 / 2));  % 等于 1.482602218505602

    int i;
    double med = 0.0, MAD, c = 1.482602;
    med = median(indata, len);

    double *meddata = (double*)malloc(len * sizeof(double));
    for (i = 0; i < len; i++)
    {
        meddata[i] = fabs(indata[i] - med);
    }

    double med2 = 0;
    med2 = median(meddata, len);

    MAD = c * median(meddata, len);

    for (i = 0; i < len; i++)
    {
        if (fabs(indata[i] - med)>3 * MAD)
            indata[i] = med;
    }

    free(meddata);
    return med;
}

void map100_attention(double *indata, int len, double *outdata)
{
    // 50~90分的映射模型
    double x[2] = { 0.2182, 0.9510 };
    double y[2] = { 90, 30 };
    double a = -81.8777;
    double b = 107.8657;

    // 大于90分的，用另一个射影，为了能包含更多的数据
    double x1[2] = { -0.5, 0.2182 };
    double y1[2] = { 99, 90 };
    double a1 = -12.5313;
    double b1 = 92.7343;

    // <20分，用另一个射影，为了能包含更多的数据
    double x3[2] = { 0.9510, 2.0 };
    double y3[2] = { 30, 0 };
    double a3 = -28.5987;
    double b3 = 57.1973;

    for (int i = 0; i < len; i++)
    {
        //默认为20 - 90分
        outdata[i] = (a*indata[i] + b);

        //>90分的数据
        if (outdata[i]>y[0])
        {
            outdata[i] = (a1*indata[i] + b1);
            if (outdata[i]>y1[0])
                outdata[i] = y1[0];
            continue;
        }
        //较低的分数
        if (outdata[i]<y3[0])
        {
            outdata[i] = (a3*indata[i] + b3);
            if (outdata[i]<0)
                outdata[i] = 0;
            //continue;
        }
    }
}

void map100_fatig(double *indata, int len, double *outdata)
{
    // 50~90分的映射模型
    double x[2] = { 0.479253, 0 };
    double y[2] = { 50, 10 };
    double a = 83.4632;
    double b = 10;

    // 大于90分的，用另一个射影，为了能包含更多的数据
    double x1[2] = { 1.5000, 0.9585 };
    double y1[2] = { 99, 90 };
    double a1 = 16.6207;
    double b1 = 74.0690;

    // <20分，用另一个射影，为了能包含更多的数据
    double x3[2] = { 0, -1 };
    double y3[2] = { 10, 0 };
    double a3 = 10;
    double b3 = 10;

    for (int i = 0; i < len; i++)
    {
        //默认为20 - 90分
        outdata[i] = (a*indata[i] + b);

        //>90分的数据
        if (outdata[i]>y1[1])
        {
            outdata[i] = (a1*indata[i] + b1);
            if (outdata[i]>y1[0])
                outdata[i] = y1[0];
            continue;
        }
        //较低的分数
        if (outdata[i]<y3[0])
        {
            outdata[i] = (a3*indata[i] + b3);
            if (outdata[i]<0)
                outdata[i] = 0;
            //continue;
        }
    }

}

void map100_stress(double *indata, int len, double *outdata)
{
    // 50~90分的映射模型
    double x[2] = { -0.171671, -0.3 };
    double y[2] = { 50, 20 };
    double a = 233.7741;
    double b = 90.1322;

    // 大于90分的，用另一个射影，为了能包含更多的数据
    double x1[2] = { 0.1, -0.0433 };
    double y1[2] = { 99, 80 };
    double a1 = 132.5501;
    double b1 = 85.7450;

    // <20分，用另一个射影，为了能包含更多的数据
    double x3[2] = { -0.3000, -0.5000 };
    double y3[2] = { 20, 0 };
    double a3 = 100;
    double b3 = 50;

    for (int i = 0; i < len; i++)
    {
        //默认为20 - 90分
        outdata[i] = (a*indata[i] + b);

        //>90分的数据
        if (outdata[i]>y1[1])
        {
            outdata[i] = (a1*indata[i] + b1);
            if (outdata[i]>y1[0])
                outdata[i] = y1[0];
            continue;
        }
        //较低的分数
        if (outdata[i]<y3[0])
        {
            outdata[i] = (a3*indata[i] + b3);
            if (outdata[i]<0)
                outdata[i] = 0;
            //continue;
        }
    }

}


void cur_smooth(double *indata, int indata_length)
{
    //计算基线均值及其标准差
    double base_data[base_point] = { 0 };
    int base_len = base_point, i = 0, j = 0;
    for (i = 0; i < base_len; i++)
    {
        base_data[i] = indata[i];   //基线数据
    }
    double result_base[2] = { 0 };
    cal_std(base_data, base_len, result_base);

    //检测异常值（基线数据段）
    for (i = 0; i < base_len; i++)
    {
        if (fabs(base_data[i] - result_base[0]) >2 * result_base[1])
            indata[i] = result_base[0] + (base_data[i] - result_base[0])*0.5;
    }
    //数据平滑
    double *tmp = (double*)malloc(indata_length * sizeof(double));
    for (i = 0 ; i < indata_length ; i++)
    {
        tmp[i] = indata[i];
    }
    for (i = 0+2; i < indata_length-2; i++)
    {
        indata[i] = (tmp[i - 2] + tmp[i - 1] + tmp[i] + tmp[i + 1] + tmp[i + 2]) / 5.0;
    }
    free(tmp);
}

void index_smooth(double *indata, int len)
{
    if (len < 5)
        return;

    int i = 0;
    double *tmp = (double*)malloc(len * sizeof(double));
    for (i = 0; i < len; i++)
    {
        tmp[i] = indata[i];
    }
    for (i = 0 + 2; i < len - 2; i++)
    {
        indata[i] = (tmp[i - 2] + tmp[i - 1] + tmp[i] + tmp[i + 1] + tmp[i + 2]) / 5.0;
    }
    free(tmp);
}

void index_report(double *score, int len, double *mean, double *max)
{
    int i = 0;
    *max = 0;
    double sum = 0;
    for (i = 0; i < len; i++)
    {
        sum = sum + score[i];
        if (*max < score[i])
        {
            *max = score[i];
        }
    }
    *mean = sum / len;
}


void smooth_pro(double *indata, int len,double * output)
{
    int i = 0 ;
    double *tmp = (double*)malloc(len * sizeof(double));
    for (i = 0; i < len; i++)
    {
        tmp[i] = indata[i];
    }
    for (i = 0+2; i < len-2; i++)
    {
        indata[i] = (tmp[i - 2] + tmp[i - 1] + tmp[i] + tmp[i + 1] + tmp[i + 2]) / 5.0;
    }

    for (i = 0; i <= len; i++)
    {
        output[i] = indata[i];
    }

    free(tmp);
}

int user_rank(double data)
{
    double user_model[100] = {
        -0.296394, -0.060643, 0.018215, 0.050501, 0.104130, 0.150935, 0.179272, 0.193675,
        0.210175, 0.218551, 0.226344, 0.236536, 0.244415, 0.251926, 0.260509, 0.267749,
        0.275162, 0.279568, 0.285926, 0.291533, 0.296576, 0.302068, 0.309881, 0.315034,
        0.320189, 0.326317, 0.330916, 0.337012, 0.344075, 0.349062, 0.354818, 0.359426,
        0.363448, 0.370706, 0.375865, 0.381488, 0.386401, 0.392804, 0.398180, 0.401857,
        0.406054, 0.411560, 0.416905, 0.421997, 0.426208, 0.431985, 0.438335, 0.446298,
        0.452212, 0.457924, 0.464043, 0.469677, 0.474437, 0.479618, 0.485892, 0.489655,
        0.494948, 0.500873, 0.506558, 0.511665, 0.517096, 0.522581, 0.526608, 0.532067,
        0.538573, 0.544272, 0.550642, 0.555081, 0.560729, 0.564562, 0.568901, 0.574791,
        0.578926, 0.583295, 0.589161, 0.595508, 0.601645, 0.608636, 0.614069, 0.619524,
        0.626874, 0.635815, 0.648467, 0.656724, 0.665211, 0.671929, 0.679570, 0.688206,
        0.698467, 0.708263, 0.718332, 0.737510, 0.748345, 0.759441, 0.770533, 0.797393,
        0.815504, 0.834919, 0.871769, 0.949829
    };
    int focus_rank = 0, i = 0;
    for (i = 99; i >= 0; i--)
    {
        if (data<user_model[i])
            focus_rank = focus_rank + 1;
        else
            break;
    }
    if (focus_rank>99)
        focus_rank = 99;
    return focus_rank;
}


// 0 是专注度  2是压力值 3是疲劳度 4是情绪值
focusStruct focus_score_calculate(double *feat, int len, double *score, double *mean, double *max, int *rank, int type)
{
    //    功能：计算得分、异常处理、平滑处理，及其用户排名
    //    参数说明：
    //        tbr：脑电算法计算的tbt值（注意，不是相关性和TBR结合的数据，是纯粹的TBR特征），double型
    //        len：tbr对应的长度
    //        score：输出对应的分数曲线，数据范围为0~100
    //    函数返回：
    //        rank：用户排名，即该次练习的专注力超越了多少用户

    focusStruct focus;
    double index_mean = 0, index_max = 0;
    if (0 == type){
        map100_attention(feat, len, score);    //计算得分
    }
    else if (3 == type){
        map100_fatig(feat, len, score);
    }
    else if (2 == type){
        map100_stress(feat, len, score);
    }
    else if (4 == type){
        for (int i = 0; i < len; i++){
            score[i] = feat[i];
        }
    }

    outlier_pro(score, len);    // 异常处理
    index_smooth(score, len);        //平滑处理
    index_report(score, len, &index_mean, &index_max);

    if (0 == type){
        //计算用户排名：超越了多少用户
        double tbr_med = 0;
        tbr_med = median(feat, len);     //每次练习的TBR均值（实际用中值代替，减少异常值的影响），便于后续个人成长评估模型的分析
        *rank = user_rank(tbr_med);
        focus.rank=*rank;
    }
    if (4 == type){//情绪值统计正向情绪作为最终得分
        //        index_mean = 0;
        //        for (int i = 0; i < len; i++)
        //        {
        //            if (score[i] >0){
        //                index_mean++;
        //            }
        //        }
        //        index_mean = index_mean/len*100;
    }

    *max = index_max;
    *mean = index_mean;
    focus.max = index_max;
    focus.mean = index_mean;

    return focus;
}

void quantization_algorithm(double * array,int length,double baseHigh,double baseLow,double * output)
{
    //专注度（利用基线对数据进行量化，用于计算专注度）
    //    double up_th = average + std;  //其中average 保存的是基线均值，std保存的是标准差
    //    double down_th = average - std;

    double x1 = baseHigh, x2 = baseLow, y1 = 60, y2 = 40;   //y=a*x+b, x-映射前，y-映射后
    double a, b;
    double * focus_data;

    a = (y1 - y2) / (x1 - x2);
    b = y1 - a*x1;

    focus_data = (double*)malloc(sizeof(double)*length);   //indata为输入数组，长度为 indata_length，focus_data用于保存量化后的曲线

    for (int i = 0; i < length; i++)
    {
        focus_data[i] = array[i] * a + b;
        if (focus_data[i]>100)
            focus_data[i] = 100;
        if (focus_data[i]<0)
            focus_data[i] = 0;
    }
    memcpy(output, focus_data, length*sizeof(double));
    free(focus_data);
}


struct_result calculate_tbr(double *indata, int len) {
    int i = 0, j = 0;
    int *index = (int *) malloc(len * sizeof(int));
    //特征曲线预处理，去除其中为1000的点
    double *fea_normal = (double *) malloc(len * sizeof(double));
    memset(fea_normal, 0, sizeof(double) * len);
    int *index_normal = (int *) malloc(len * sizeof(int));
    memset(index_normal, 0, sizeof(int) * len);

    double *fea_null = (double *) malloc(len * sizeof(double));
    memset(fea_null, 0, sizeof(double) * len);
    int *index_null = (int *) malloc(len * sizeof(int));
    result.indexs = (int *) malloc(len * sizeof(int));
    result.newtbrs = (double *) malloc(len * sizeof(double));
    memset(result.indexs, 0, sizeof(int) * len);
    memset(index_null, 0, sizeof(int) * len);
    int len_normal = 0, len_null = 0, k = 0;
    j = 0;
    for (i = 0; i < len; i++) {
        if (fabs(indata[i] - 1000) > 0.001) {// 正常
            fea_normal[j] = indata[i];
            index_normal[j++] = i;
            result.indexs[i]=1;
        } else {
            fea_null[k] = 0;
            index_null[k++] = i;
            result.indexs[i]=0;
        }
    }
    len_null = k;
    len_normal = j;

    //增加基线离群值的检测，及其特征的平滑过程（用于指标曲线的处理）
    cur_smooth(fea_normal, len_normal);

    //计算三个区间的数目及其鸟叫和水滴位置、统计三个状态的时间长度
    int *birds = (int *) malloc(len * sizeof(int));
    memset(birds, 0, sizeof(int) * len);
    int *didis = (int *) malloc(len * sizeof(int));
    memset(didis, 0, sizeof(int) * len);
    result.birds = birds;
    result.didis = didis;

    curve_characters(fea_normal, len_normal, index_normal,&result);

    for(int i=0;i<j;i++){
        result.newtbrs[index_normal[i]]=fea_normal[i];
    }
    for(int i=0;i<k;i++){
        result.newtbrs[index_null[i]]=(result.baseHigh+result.baseLow)/2;
    }

    if (len_null == 0)
        result.tbrhasnull = 0;
    else {
        result.tbrhasnull = 1;
        result.nulllength = len_null;
        result.sections[3] = len_null; //异常的数据个数，又以前的三个状态改成了四个状态的及其数量，沉浸指数等指标计算不再包括异常的数据长度
    }


    //计算特征的四个维度（另加一个基线异常的指标）
    int report[5] = {0};            //保存：沉浸度[0-100]、专注度[0-100]、趋势性[0-100]、稳定性[0-100]、基线异常标志[0/1]
    cal_report(fea_normal, len_normal,
               report);  //**************************计算练习报告*******************************//
    report[1] = 100 - report[1];  //若为计算专注度，则需要反向,其他不需要改步
    result.index=report[0];
    return result;
}

unsigned char chartohex(char ch)
{
    unsigned char dat;
    if((ch <= '9')&&(ch >= '0'))
    {
        dat = ch-'0';
    }
    else if((ch <= 'f')&&(ch >= 'a'))
    {
        dat = ch - 'a'+10;
    }
    else    //大写
    {
        dat = ch - 'A'+10;
    }
    //    printf("dat=%x",dat);
    return dat;
}


unsigned char strToInt(unsigned char chH,unsigned char chL)
{
    unsigned char dat = (chartohex(chH))<<4;
    //    printf("hex = %d",dat);
    dat += chartohex(chL);
    //    printf("hex = %d",dat);
    return (dat);
}


tbr_output_t tbr_output;

/*
 从小到大排序
 */
void lowestToSort(float *a, int l)//a为数组地址，l为数组长度。
{
    int i, j;
    float v;
    //排序主体
    for(i = 0; i < l - 1; i ++){
        for(j = i+1; j < l; j ++)
        {
            if(a[i] > a[j])//如前面的比后面的大，则交换。
            {
                v = a[i];
                a[i] = a[j];
                a[j] = v;
            }
        }
    }
}

/*
 函数 ：tbr_output_wave
 输入： tbr_input_buf     为TBR的数组
 buf_length       为TBR数组的长度
 upBaseLine       为上基线值
 downBaseLine     为下基线值

 输出  tbr_output_t 结构体
 */
tbr_output_t  tbr_output_wave(float *tbr_input_buf,int buf_length,float upBaseLine,float downBaseLine)
{
    int i;
    float BaseLine_mean;
    int pre_none_flag;
    float CriticalData;
    int haveCriticalFlag = 0;
    int lastState;
    tbr_output.newtbrs_point = (float *)malloc((sizeof(float))*buf_length);
    if(tbr_output.newtbrs_point==NULL){
        tbr_output.birds_len = 1; //printf("malloc newtbrs_point NULL\n");
        return tbr_output;
    }
    tbr_output.birds_point = (int *)malloc((sizeof(int))*(buf_length/4));
    if(tbr_output.birds_point==NULL){
        tbr_output.birds_len = 1;    //printf("malloc birds_point NULL\n");
        return tbr_output;
    }
    tbr_output.didis_point = (int *)malloc((sizeof(int))*(buf_length/4));
    if(tbr_output.didis_point==NULL){
        tbr_output.birds_len = 1;     //printf("malloc didis_point NULL\n");
        return tbr_output;
    }

    pre_none_flag = 0;

    //init tbr_output
    tbr_output.have_null_flag = 0;
    tbr_output.null_num = 0;
    tbr_output.sections_num[0] = 0;
    tbr_output.sections_num[1] = 0;
    tbr_output.sections_num[2] = 0;
    tbr_output.max_tbr = -2.0;
    tbr_output.min_tbr = 2.0;
    tbr_output.didis_len = 0;
    tbr_output.birds_len = 0;
    tbr_output.newtbrs_len = buf_length;

    //get mean
    BaseLine_mean = downBaseLine + ((upBaseLine - downBaseLine)/2);

    for(i = 0;i < buf_length;i++){

        if(tbr_input_buf[i] == (float)null){    //当为null时
            tbr_output.have_null_flag = 1;
            tbr_output.null_num++;
            if((pre_none_flag == 1)||(i == 0))
            {
                if((tbr_input_buf[i+1] != (float)null)&& (i < (buf_length -1)))
                    tbr_output.newtbrs_point[i] = tbr_input_buf[i+1];
                else
                    tbr_output.newtbrs_point[i] = BaseLine_mean;
            }
            else
            {
                tbr_output.newtbrs_point[i] = tbr_output.newtbrs_point[i-1];
            }

            pre_none_flag = 1;
        }
        else
        {
            pre_none_flag = 0;
            tbr_output.newtbrs_point[i] = tbr_input_buf[i];
        }
        if(tbr_output.max_tbr < tbr_output.newtbrs_point[i]&&!isinf(tbr_output.newtbrs_point[i]))
            tbr_output.max_tbr = tbr_output.newtbrs_point[i];
        if(tbr_output.min_tbr > tbr_output.newtbrs_point[i]&&!isinf(tbr_output.newtbrs_point[i]))
            tbr_output.min_tbr = tbr_output.newtbrs_point[i];

        if(tbr_output.newtbrs_point[i] > upBaseLine)
        {
            tbr_output.sections_num[0]++;    //上区域
        }
        else if(tbr_output.newtbrs_point[i] > downBaseLine)
        {
            tbr_output.sections_num[1]++;    //中区域
        }
        else
        {
            tbr_output.sections_num[2]++;    //下区域  专注
        }
    }


    if(tbr_output.newtbrs_point[0] > upBaseLine)
        lastState = 0;
    else if(tbr_output.newtbrs_point[0] > downBaseLine)
        lastState = 1;
    else
        lastState = 2;
    for (i = 0; i < buf_length - INTERVAL_LEN - 1; i += INTERVAL_LEN) { // 1个数据代表30/250 秒
        int k,m;

        float mean;
        float buf[INTERVAL_LEN];

        for(k = 0;k < INTERVAL_LEN;k++)
        {
            buf[k] = tbr_output.newtbrs_point[i + k];
            //printf("newtbrs_point2ge%f\n",tbr_output.newtbrs_point[i-INTERVAL_LEN+1+k]);
        }
        lowestToSort(buf,INTERVAL_LEN);    //小大排序数组buf
        //printf("buf 2ge%f,%f\n",buf[0],buf[1]);
        if((INTERVAL_LEN % 2) == 0)//偶数时
        {
            m = INTERVAL_LEN/2;
            mean = (buf[m-1] + buf[m])/2;
        }
        else
        {
            m = INTERVAL_LEN / 2;
            mean = buf[m];
        }
        //printf("mean = %f\n",mean);


        if (mean > upBaseLine && lastState != 0 /*this.move*/) { // 高区间
            tbr_output.didis_point[tbr_output.didis_len] = i + INTERVAL_LEN;
            tbr_output.didis_len++;
            lastState = 0;
        }
        else if (mean > downBaseLine && mean <= upBaseLine && lastState != 1/*this.move*/) {// 中区间
            lastState = 1;
        }
        else if (mean < downBaseLine && lastState != 2) { // 低区间
            tbr_output.birds_point[tbr_output.birds_len] = i + INTERVAL_LEN;
            tbr_output.birds_len++;
            lastState = 2;
        }
    }

    return tbr_output;
}

/*
 freePoint函数为释放    tbr_output 中指向的内存块
 */
void freePoint(void){
    free(tbr_output.newtbrs_point);  // 释放内存空间
    free(tbr_output.didis_point);
    free(tbr_output.birds_point);
}



void smooth_array(double *array, int length, double *output) {
    outlier_pro(array, length);  //替换cur_smooth的离群值检测和替换
    double a[3] = { 1, -1.91119706742607, 0.914975834801433 };
    double b[3] = { 0.000944691843840162, 0.00188938368768032, 0.000944691843840162 };
    int order = 2;    /* ORDER is the number of element a[] or b[] minus 1;*/
    int delay = 25;
    int len = length - 1;    //数据长度-1
    filter(order, a, b, len, array, output);  //indata和outdata为数据数据和输出数据，double类型的数组，len为数据的长度-1
    //2.2延时复位，后面补零
    int i = 0;
    for (i = delay; i <= len; i++)
    {
        output[i - delay] = output[i];
    }
    for (i = length - delay; i <= len; i++)
    {
        output[i] = output[len - delay - 1];
    }
}

void curve_characters(double *indata, int indata_length, int *index_normal, struct_result *result)
{
    //获取基线和练习数据
    double base_data[base_point] = { 0 };
    int base_len = base_point, i = 0, j = 0;
    for (i = 0; i < base_len; i++)
    {
        base_data[i] = indata[i];   //基线数据
    }
    double result_base[2] = { 0 };
    cal_std(base_data, base_len, result_base);

    //**** 2.2 练习数据的均值及其标准差
    double *test_data;
    int test_len = 0;
    test_len = indata_length - base_point;
    test_data = (double*)malloc(sizeof(double)*test_len);
    for (i = base_point; i < indata_length; i++)
    {
        test_data[j++] = indata[i];   //测试数据
    }
    double result_test[2] = { 0 };
    cal_std(test_data, test_len, result_test);
    free(test_data);
    double up_th = result_base[0] + result_base[1];
    double down_th = result_base[0] - result_base[1];
    result->baseHigh=up_th;
    result->baseLow=down_th;

    //计算曲线特征（鸟叫、水滴、三个区间的数量）

    int count_wander = 0, count_atten = 0, count_neut = 0;
    int bn = 0, dn = 0;
    double min = 1000.0, max = -1000.0, mean;
    for (i = 0; i < indata_length; i++)
    {
        if (indata[i] < down_th)    //attention
            count_atten++;
        else if (indata[i] > up_th)   //wander
            count_wander++;
        else
            count_neut++;
        if ((i>1) && (i <= indata_length - 3))
        {
            mean = (indata[i] + indata[i + 1] + indata[i + 2]) / 3.0;
            if ((indata[i] > up_th) && (indata[i - 1] < up_th) && (mean>up_th))   //从中立到走神，水滴声
            {
                result->didis[dn] = i;
                dn++;
            }
            if ((indata[i] < down_th) && (indata[i - 1] > down_th) && (mean<down_th))   //从中立专注，鸟叫
            {
                result->birds[bn] = i;
                bn++;
            }
        }
        if (indata[i] > max)
            max = indata[i];
        if (indata[i] < min)
            min = indata[i];
    }
    result->sections[0] = count_wander;
    result->sections[1] = count_neut;
    result->sections[2] = count_atten;
    result->max = max;
    result->min = min;
    result->bn = bn;
    result->dn = dn;

}

void rader_sort(double *indata, int len)//a为数组地址，l为数组长度。
{
    int i, j;
    double  buf = 0;;

    //冒泡排序(从大到小)
    for (i = 0; i<len - 1; ++i)  //比较n-1轮
    {
        for (j = 0; j<len - 1 - i; ++j)  //每轮比较n-1-i次,
        {
            if (indata[j] < indata[j + 1])
            {
                buf = indata[j];
                indata[j] = indata[j + 1];
                indata[j + 1] = buf;
            }
        }
    }

}

double cal_mean(double *indata, int len)   //计算中值
{
    int i;
    double mean=0,sum=0;
    for (i = 0; i < len; i++)  //比较n-1轮
    {
        sum += indata[i];
    }
    mean = sum / len;

    return mean;

}

void gauss_solve(int n, double A[], double x[], double b[])
{
    int i, j, k, r;
    double max;
    for (k = 0; k<n - 1; k++)
    {
        max = fabs(A[k*n + k]); /*find maxmum*/
        r = k;
        for (i = k + 1; i<n - 1; i++)
            if (max<fabs(A[i*n + i]))
            {
                max = fabs(A[i*n + i]);
                r = i;
            }
        if (r != k)
            for (i = 0; i<n; i++) /*change array:A[k]&A[r] */
            {
                max = A[k*n + i];
                A[k*n + i] = A[r*n + i];
                A[r*n + i] = max;
            }
        max = b[k]; /*change array:b[k]&b[r] */
        b[k] = b[r];
        b[r] = max;
        for (i = k + 1; i<n; i++)
        {
            for (j = k + 1; j<n; j++)
                A[i*n + j] -= A[i*n + k] * A[k*n + j] / A[k*n + k];
            b[i] -= A[i*n + k] * b[k] / A[k*n + k];
        }
    }

    for (i = n - 1; i >= 0; x[i] /= A[i*n + i], i--)
        for (j = i + 1, x[i] = b[i]; j<n; j++)
            x[i] -= A[i*n + j] * x[j];
}


//*==================polyfit(n,x,y,poly_n,a)===================*/
//*=======拟合y=a0+a1*x+a2*x^2+……+apoly_n*x^poly_n========*/
//*=====n是数据个数 xy是数据值 poly_n是多项式的项数======*/
//*===返回a0,a1,a2,……a[poly_n]，系数比项数多一（常数项）=====*/

void polyfit(int n, double x[], double y[], int poly_n, double a[])
{
    int i, j;
    double *tempx = (double*)malloc(n * sizeof(double));
    double *sumxx = (double*)malloc((poly_n * 2 + 1) * sizeof(double));
    double *tempy = (double*)malloc(n * sizeof(double));
    double *sumxy = (double*)malloc((poly_n + 1) * sizeof(double));
    double *ata = (double*)malloc(((poly_n + 1)*(poly_n + 1)) * sizeof(double));

    for (i = 0; i<n; i++)
    {
        tempx[i] = 1;
        tempy[i] = y[i];
    }
    for (i = 0; i<2 * poly_n + 1; i++)
        for (sumxx[i] = 0, j = 0; j<n; j++)
        {
            sumxx[i] += tempx[j];
            tempx[j] *= x[j];
        }
    for (i = 0; i<poly_n + 1; i++)
        for (sumxy[i] = 0, j = 0; j<n; j++)
        {
            sumxy[i] += tempy[j];
            tempy[j] *= x[j];
        }
    for (i = 0; i<poly_n + 1; i++)
        for (j = 0; j<poly_n + 1; j++)
            ata[i*(poly_n + 1) + j] = sumxx[i + j];
    gauss_solve(poly_n + 1, ata, a, sumxy);

    free(tempx);
    free(sumxx);
    free(tempy);
    free(sumxy);
    free(ata);
}

double rader_cal_std(double *data, int len)
{
    double sum = 0, var = 0, average = 0, standard = 0;
    for (int i = 0; i <len; i++)
    {
        sum += data[i];    //求和
    }
    average = sum / len;
    for (int j = 0; j <len; j++)
    {
        var += pow(data[j] - average, 2);//求方差
    }
    var = var / (len - 1);
    standard = pow(var, 0.5);    //求标准差

    return standard;
}


/// 获取雷达图指标
/// @param dat 指标数组
/// @param len 数组长度
/// @param radar_stability 正念稳定度
/// @param radar_duration 正念持续性
/// @param radar_trend 正念趋势性
/// @param radar_best 正念最佳值
/// @param radar_mean 正念平均值
void rader_map(double *dat, int len, int * raders)
{
    int radar_stability, radar_duration,  radar_trend,  radar_best,  radar_mean=0;
    int i = 0;
    //  去除异常点，否则影响最值和标准差的计算
    outlier_pro(dat, len);    // 异常处理

    //  1.正念稳定度
    double *dat_b = (double*)malloc(120 * sizeof(double));
    for (i = 0; i < 120; i++){
        dat_b[i] = dat[i+59];
    }
    double *dat_e = (double*)malloc((len - 59) * sizeof(double));
    for (i = 59; i < len; i++){
        dat_e[i - 59] = dat[i];
    }
    double rate = 0,std_b,std_e;
    std_b = rader_cal_std(dat_b, 120);
    std_e = rader_cal_std(dat_e, len - 59);
    rate = std_e / std_b;
    double stability = 0;
    stability = -80*rate + 156;
    if (stability>100)
        stability = 100;
    else if(stability < 0)
        stability = 0;
    radar_stability = round(stability);
    free(dat_b);
    free(dat_e);

    // 2.正念持续性
    double base = 0, down_val = 0, duration=0;
    double *dat_b2 = (double*)malloc(57 * sizeof(double));
    for (i = 0; i < 57; i++){
        dat_b2[i] = dat[i + 59];
    }
    rader_sort(dat_b2, 57);
    double *dat_sort = (double*)malloc(47 * sizeof(double));
    for (i = 0; i < 47; i++){
        dat_sort[i] = dat_b2[i+5];
    }
    base = (median(dat_sort, 47) + cal_mean(dat_sort, 47)) / 2.0;
    down_val = base - 0.5* rader_cal_std(dat_sort, 47);
    int count = 0;
    for (i = 10; i < len; i++) //前10各点不用
    {
        if (dat[i] <= down_val)
            count++;
    }
    duration = 1.0*count / (len - 10) * 100 * 2;
    if (duration>100)
        duration = 100;
    radar_duration = round(duration);
    free(dat_b2);
    free(dat_sort);

    // 3.正念趋势性
    double *dat_e2 = (double*)malloc((len - 59) * sizeof(double));
    double *index = (double*)malloc((len - 59) * sizeof(double));
    for (i = 59; i < len; i++){
        dat_e2[i - 59] = dat[i];
        index[i - 59] = i - 58;
    }
    double p[2] = { 0 }, trend=0;
    //长度 x轴 数据y轴 1 p是输出的，第一个是斜率
    polyfit(len - 59, index, dat_e2, 1, p);
    trend = -25925.925925*p[1] + 61.111111;
    if (trend>100)
        trend = 100;
    else if (trend < 30)
        trend = 30;
    radar_trend = round(trend);

    free(dat_e2);
    free(index);

    //4.正念最佳值、正念平均值
    double *dat_b3 = (double*)malloc(120 * sizeof(double));
    for (i = 0; i < 120; i++){
        dat_b3[i] = dat[i + 59];
    }
    double x1 = 0, x2 = 0, std_val = 0, mean_valu = 0;;
    std_val = rader_cal_std(dat_b3, 120);
    mean_valu = cal_mean(dat_b3, 120);
    x1 = mean_valu - std_val;
    x2 = mean_valu + std_val;
    double y1 = 70, y2 = 50,a,b;
    a = (y1 - y2) / (x1 - x2);
    b = y1 - a*x1;

    double *dat_qe = (double*)malloc(len * sizeof(double)),mind_best=0,mind_mean=0;
    for (i = 0; i < len; i++)
    {
        dat_qe[i] = dat[i]* a+ b;
        if (dat_qe[i]>100)
            dat_qe[i] = 100;
        else if (dat_qe[i] < 0)
            dat_qe[i] = 0;

        if (dat_qe[i]>mind_best)
            mind_best = dat_qe[i];

        mind_mean = dat_qe[i] + mind_mean;
    }
    mind_mean = mind_mean / len;
    radar_best = round(mind_best);
    radar_mean = round(mind_mean);

    free(dat_b3);
    free(dat_qe);

    raders[0]=radar_stability;
    raders[1]=radar_duration;
    raders[2]=radar_trend;
    raders[3]=radar_best;
    raders[4]=radar_mean;
}

