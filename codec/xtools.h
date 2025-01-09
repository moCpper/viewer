#pragma once

//兼容Linux  _WIN32 windows 32为和64位
#ifdef _WIN32
#ifdef XCODEC_EXPORTS
#define XCODEC_API __declspec(dllexport)
#else
#define XCODEC_API __declspec(dllimport)
#endif
#else
#define XCODEC_API
#endif

#include <thread>
#include <iostream>
#include <mutex>
#include <list>
struct AVPacket;
struct AVCodecParameters;
struct AVRational;
struct AVFrame;
struct AVCodecContext;




//日志级别 DEBUG INFO ERROR FATAL
enum XLogLevel
{
    XLOG_TYPE_DEBUG,
    XLOG_TYPE_INFO,
    XLOG_TPYE_ERROR,
    XLOG_TYPE_FATAL
};
#define LOG_MIN_LEVEL XLOG_TYPE_DEBUG
#define XLOG(s,level) \
    if(level>=LOG_MIN_LEVEL) \
    std::cout<<level<<":"<<__FILE__<<":"<<__LINE__<<":\n"\
    <<s<<std::endl;
#define LOGDEBUG(s) XLOG(s,XLOG_TYPE_DEBUG)
#define LOGINFO(s) XLOG(s,XLOG_TYPE_INFO)
#define LOGERROR(s) XLOG(s,XLOG_TPYE_ERROR)
#define LOGFATAL(s) XLOG(s,XLOG_TYPE_FATAL)


XCODEC_API void MSleep(unsigned int ms);

//获取当前时间戳 毫秒
XCODEC_API long long NowMs();

XCODEC_API void XFreeFrame(AVFrame** frame);

XCODEC_API void PrintErr(int err);

XCODEC_API long long XRescale(long long pts, AVRational* src_time_base,
    AVRational* des_time_base);

class XCODEC_API XThread
{
public:
    //启动线程
    virtual void Start();

    //设置线程退处标志
    virtual void Exit();

    //停止线程（设置退出标志）
    virtual void Stop();

    //等待线程退出
    virtual void Wait();

    //执行任务 需要重载
    virtual void Do(AVPacket* pkt) {}

    //传递到下一个责任链函数
    virtual void Next(AVPacket* pkt)
    {
        std::unique_lock<std::mutex> lock(m_);
        if (next_)
            next_->Do(pkt);

    }

    //设置责任链下一个节点（线程安全）
    void set_next(XThread* xt)
    {
        std::unique_lock<std::mutex> lock(m_);
        next_ = xt;
    }

    //暂停或是播放
    virtual void Pause(bool is_pause) { is_pause_ = is_pause; }

    bool is_pause() { return is_pause_; }
protected:
    
    //线程入口函数
    virtual void Main() = 0;

    //标志线程退出
    bool is_exit_ = false;
    
    //线程索引号
    int index_ = 0;

    bool is_pause_ = false;
private:
    std::thread th_;
    std::mutex m_;
    XThread *next_ = nullptr;//责任链下一个节点

};
class XTools
{
};



//音视频参数
class XCODEC_API XPara
{
public:
    AVCodecParameters* para = nullptr;  //音视频参数
    AVRational* time_base = nullptr;    //时间基数
    long long total_ms = 0;             //总时长 毫秒

    //创建对象
    static XPara* Create();
    ~XPara();
private:
    //私有是禁止创建栈中对象
    XPara();
};


/// <summary>
/// 线程安全avpacket list
/// </summary>
class XCODEC_API XAVPacketList
{
public:
    AVPacket* Pop();
    void Push(AVPacket* pkt);
    int Size();
    void Clear();
private:
    std::list<AVPacket*> pkts_;
    int max_packets_ = 1000;//最大列表数量，超出清理
    std::mutex mux_;
};