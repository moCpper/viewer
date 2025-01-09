#include "xmux_task.h"
extern "C"
{
#include <libavformat/avformat.h>
}
using namespace std;
void XMuxTask::Do(AVPacket* pkt)
{
    pkts_.Push(pkt);
    Next(pkt);
}
void XMuxTask::Main()
{
    xmux_.WriteHead();
    //找到关键帧
    while (!is_exit_) {
        unique_lock<mutex> lock(mux_);
        auto pkt = pkts_.Pop();
        if (!pkt){
            MSleep(1);
            continue;
        }
        if (pkt->stream_index == xmux_.video_index() && pkt->flags & AV_PKT_FLAG_KEY) {     //关键帧
            xmux_.Write(pkt);
            av_packet_free(&pkt);
            break;
        }
        //丢掉非视频关键帧
        av_packet_free(&pkt);
    }

    while (!is_exit_)
    {
        unique_lock<mutex> lock(mux_);
        auto pkt = pkts_.Pop();
        if (!pkt)
        {
            MSleep(1);
            continue;
        }

        xmux_.Write(pkt);
        cout << "W"<< flush;

        av_packet_free(&pkt);
    }
    xmux_.WriteEnd();
    xmux_.set_c(nullptr);
}
bool XMuxTask::Open(const char* url,
    AVCodecParameters* video_para,
    AVRational* video_time_base,
    AVCodecParameters* audio_para ,
    AVRational* audio_time_base 
)
{
    auto c = xmux_.Open(url,video_para,audio_para);
    if (!c)return false;
    xmux_.set_c(c);
    xmux_.set_src_video_time_base(video_time_base);
    xmux_.set_src_audio_time_base(audio_time_base);
    return true;
}