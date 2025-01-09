#include "xplayer.h"
#include"xaudio_play.h"

bool XPlayer::Open(const char* url, void* winId){
    if (!demux_.Open(url)) {
        return false;
    }
    auto vp = demux_.CopyVideoPara();
    if (vp) {
        //��Ƶ��ʱ��
        this->total_ms_ = vp->total_ms;

        if (!video_decode_.Open(vp->para)) {
            return false;
        }
        //������Ƶ֡
        video_decode_.set_stream_index(demux_.video_index());
        //����
        video_decode_.set_block_size(100);
        //��Ƶ��Ⱦ
        if(!view_){
            view_ = XVideoView::Create();
        }
        view_->set_win_id(winId);
        if (!view_->Init(vp->para)) {
            return false;
        }
    }

    auto ap = demux_.CopyAudioPara();
    if (ap) {
        if (!audio_decode_.Open(ap->para)) {
            return false;
        }
        //����
        audio_decode_.set_block_size(100);

        //������Ƶ֡
        audio_decode_.set_stream_index(demux_.audio_index());

        //frame ����
        audio_decode_.set_frame_cache(true);

        //init��Ƶ����
        XAudioPlay::Instance()->Open(*ap);

        //����ʱ�����
        double time_base = 0;
    }
    else {
        demux_.set_syn_type(XSYN_VIDEO);            //������Ƶͬ��
    }

    //���װ���ݴ�����ǰthis
    demux_.set_next(this);

    return true;
}

void XPlayer::Stop(){
    XThread::Exit();
    demux_.Exit();
    audio_decode_.Exit();
    video_decode_.Exit();
    Wait();
    demux_.Wait();
    audio_decode_.Wait();
    video_decode_.Wait();
    if (view_) {
        view_->Close();
        delete view_;
        view_ = nullptr;
    }
    XAudioPlay::Instance()->Close();
}

void XPlayer::Do(AVPacket* pkt) {
    if (audio_decode_.is_open()) {
        audio_decode_.Do(pkt);
    }
    if (video_decode_.is_open()) {
        video_decode_.Do(pkt);
    }
}

//��������Ƶ��ͬ��
void XPlayer::Main() {
    long long syn = 0;
    auto au = XAudioPlay::Instance();
    auto ap = demux_.CopyAudioPara();
    auto vp = demux_.CopyVideoPara();
    video_decode_.set_time_base(vp->time_base);
    while (!is_exit_) {
        if (is_pause()) {
            MSleep(1);
            continue;
        }
        this->pos_ms_ = video_decode_.cur_ms();
        if (ap) {
            syn = XRescale(au->cur_pts(), ap->time_base, vp->time_base);
            audio_decode_.set_syn_pts(au->cur_pts() + 10000);
            video_decode_.set_syn_pts(syn);
        }
        MSleep(1);
    }
}

void XPlayer::Start(){
    demux_.Start();
    if (video_decode_.is_open()) {
        video_decode_.Start();
    }
    if (audio_decode_.is_open()) {
        audio_decode_.Start();
    }
    XThread::Start();
}

void XPlayer::Update(){
    //��Ⱦ��Ƶ
    if (view_) {
        auto f = video_decode_.GetFrame();
        if (f) {
            view_->DrawFrame(f);
            XFreeFrame(&f);
        }
    }
        //��Ƶ����
        auto au = XAudioPlay::Instance();
        auto f = audio_decode_.GetFrame();
        if (!f) { return; }
        au->Push(f);
        XFreeFrame(&f);
}

void XPlayer::SetSpeed(float s) {
    XAudioPlay::Instance()->SetSpeed(s);
}

bool XPlayer::Seek(long long ms){
    demux_.Seek(ms);
    audio_decode_.Clear();
    video_decode_.Clear();
    XAudioPlay::Instance()->Clear();
    return true;
}

void XPlayer::Pause(bool is_pause) {
    XThread::Pause(is_pause);
    demux_.Pause(is_pause);
    audio_decode_.Pause(is_pause);
    video_decode_.Pause(is_pause);
    XAudioPlay::Instance()->Pause(is_pause);
}
