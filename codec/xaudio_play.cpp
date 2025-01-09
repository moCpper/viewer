#include "xaudio_play.h"
#include<iostream>
extern"C" {
#include<SDL.h>
#include<libavformat/avformat.h>
}
#pragma comment(lib,"SDL2.lib")

class CXAudioPlay : public XAudioPlay {
public:
    //暂停
    void Pause(bool is_pause) {
        if (is_pause) {
            SDL_PauseAudio(1);
            pause_begin = NowMs();
        }
        else {
            if (pause_begin > 0) {
                last_ms_ += (NowMs() - pause_begin);
            }
            SDL_PauseAudio(0);
        }
    }
    bool Open(XAudioSpec& spec) override {
        this->spec_ = spec;
        //退出上一次音频
        SDL_QuitSubSystem(SDL_INIT_AUDIO);

        SDL_AudioSpec sdl_spec;
        sdl_spec.freq = spec.freq;
        sdl_spec.format = spec.format;
        sdl_spec.channels = spec.channels;
        sdl_spec.samples = spec.samples;
        sdl_spec.silence = 0;
        sdl_spec.userdata = this;
        sdl_spec.callback = AudioCallback;
        if (SDL_OpenAudio(&sdl_spec, nullptr) < 0) {
            std::cout << SDL_GetError() << std::endl;
        }
        //开始播放
        SDL_PauseAudio(0);
        return true;
    }

    void Close() override{
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        std::unique_lock<std::mutex> lock(mux_);
        audio_datas_.clear();
        cur_pts_ = 0;         //当前播放位置
        last_ms_ = 0;         //上次的时间戳
        pause_begin = 0;      //暂停开始时间戳
    }

    void Callback(unsigned char* stream, int len) override {
        SDL_memset(stream, 0, len);
        std::unique_lock<std::mutex> lock(mux_);
        if (audio_datas_.empty()) { return; }
        auto buf = audio_datas_.front();
        int mixed_size = 0;                 //已经处理的字节数
        int need_size = len;                //需要处理的字节数
        cur_pts_ = buf.pts;                 //当前播放的pts
        last_ms_ = NowMs(); 

        while (mixed_size < len) {
            if (audio_datas_.empty()) { break; }
            buf = audio_datas_.front();
            int size = buf.data.size() - buf.offset;        //剩余未处理的数据
            if (size > need_size) {
                size = need_size;
            }
            SDL_MixAudio(stream + mixed_size,
                buf.data.data() + buf.offset,
                size,SDL_MIX_MAXVOLUME);
            need_size -= size;
            mixed_size += size;
            buf.offset += size;
            if (buf.offset >= buf.data.size()) {
                audio_datas_.pop_front();
            }
        }
    }

    //void SetSpeed(float s) override {
    //    //退出上一次音频
    //    SDL_QuitSubSystem(SDL_INIT_AUDIO);

    //}
    long long cur_pts() override {
        double ms = 0;
        if (last_ms_ > 0) {
            ms = NowMs() - last_ms_;            //距离上次写入缓冲的播放时间
        }
        //pts毫秒换算为pts的时间基数
        if (time_base_ > 0) {
            ms = ms / (double)1000 / (double)time_base_;
        }
        return cur_pts_ + speed_ * ms;
    }

private:
    long long cur_pts_ = 0;         //当前播放位置
    long long last_ms_ = 0;         //上次的时间戳
    long long pause_begin = 0;      //暂停开始时间戳
};


XAudioPlay* XAudioPlay::Instance(){
    static CXAudioPlay cx;
    return &cx;
}

bool XAudioPlay::Open(XPara& para) {
    if (para.time_base->num > 0) {
        time_base_ = (double)para.time_base->den / (double)para.time_base->num;
    }
    return Open(para.para);
}

bool XAudioPlay::Open(AVCodecParameters* para){
    XAudioSpec spec;
    spec.channels = para->channels;
    spec.freq = para->sample_rate;
    switch (para->format) {
    case AV_SAMPLE_FMT_S16:                             ///<signed 16 bits
    case AV_SAMPLE_FMT_S16P:
        spec.format = AUDIO_S16;
        break;
    case AV_SAMPLE_FMT_S32:                             ///< signed 32 bits
    case AV_SAMPLE_FMT_S32P:
        spec.format = AUDIO_S32;
        break;
    case AV_SAMPLE_FMT_FLT:                             ///< float
    case AV_SAMPLE_FMT_FLTP:
        spec.format = AUDIO_F32;
        break;
    default:
        break;
    }

    return Open(spec);
}

void XAudioPlay::Push(AVFrame* frame){
    if (!frame || !frame->data[0]) { return; }
    std::vector<unsigned char> buf;
    int sample_size = av_get_bytes_per_sample((AVSampleFormat)frame->format);
    int channels = frame->channels;
    unsigned char* L = frame->data[0];
    unsigned char* R = frame->data[1];
    unsigned char* data = nullptr;
    if (channels == 1) {
        Push(frame->data[0], frame->nb_samples * sample_size, frame->pts);
        return;
    }
    //暂时支持双通道
    switch (frame->format) {                        
    //case AV_SAMPLE_FMT_S16P:                    
    case AV_SAMPLE_FMT_S32P:                        
    case AV_SAMPLE_FMT_FLTP:
        buf.resize(frame->linesize[0]);
        data = buf.data();
        //LLLL RRRR   32
        //LR LR LR LR 4
        for (int i = 0; i < frame->nb_samples; i++) {
            ::memcpy(data + i * sample_size * channels,
                L + i*sample_size,sample_size);
            ::memcpy(data + i * sample_size * channels + sample_size,
                R + i*sample_size,sample_size);
        }
        Push(data, frame->linesize[0],frame->pts);
        return;
        break;
    default:
        break;
    }
    this->Push(frame->data[0],frame->linesize[0],frame->pts);
}

XAudioPlay::XAudioPlay(){
    SDL_Init(SDL_INIT_AUDIO);
}

XAudioPlay::~XAudioPlay(){

}

