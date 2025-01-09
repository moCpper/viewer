#include "xdemux_task.h"
extern "C"
{
#include <libavformat/avformat.h>
}
using namespace std;

void XDemuxTask::Stop() {
    XThread::Stop();
    demux_.set_c(nullptr);
}

bool XDemuxTask::Open(std::string url, int timeout_ms)
{
    LOGDEBUG("XDemuxTask::Open begin!");
    demux_.set_c(nullptr);//断开之前的连接
    this->url_ = url;
    this->timeout_ms_ = timeout_ms;
    auto c = demux_.Open(url.c_str());
    if (!c)return false;
    demux_.set_c(c);
    demux_.set_time_out_ms(timeout_ms);
    LOGDEBUG("XDemuxTask::Open end!");
    return true;
}

bool XDemuxTask::Seek(long long ms){
    auto vp = demux_.CopyVideoPara();
    if (!vp) { return false; }
    auto pts = av_rescale_q(ms,{1,1000},*vp->time_base);
    return demux_.Seek(pts,video_index());
}

void XDemuxTask::Main()
{
    AVPacket pkt;
    while (!is_exit_)
    {
        if (is_pause()) {
            MSleep(1);
            continue;
        }
        if (!demux_.Read(&pkt))
        {
            //读取失败
            cout << "-" << flush;
            if (!demux_.is_connected())
            {
                Open(url_, timeout_ms_);
            }

            this_thread::sleep_for(1ms);
            continue;
        }
        //播放速度控制
        cout << "." << flush;
        if (syn_type_ == XSYN_VIDEO && pkt.stream_index == demux_.video_index()) {
            auto dur = demux_.RescaleToMs(pkt.duration, pkt.stream_index);
            if (dur <= 0) {
                dur = 40;
            }
            MSleep(dur);
        }
        Next(&pkt);
        av_packet_unref(&pkt);

        this_thread::sleep_for(1ms);
    }
}