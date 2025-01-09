#pragma once
#include"xtools.h"
#include"xdemux_task.h"
#include"xdecode_task.h"
#include"xvideo_view.h"

class XCODEC_API XPlayer : public XThread{
public:
	//回调接收音视频包
	void Do(AVPacket* pkt) override;
	bool Open(const char* url,void* winId);
	void Stop();
	//处理同步
	void Main();
	void Start();
	//渲染视频，播放音频
	void Update();
	void SetSpeed(float s);

	//总时长 毫秒
	long long total_ms() { return total_ms_; }

	//当前播放的位置 毫秒
	long long pos_ms() {return pos_ms_;}

	//设置视频播放位置，ms
	bool Seek(long long ms);

	//暂停或播放
	void Pause(bool is_pause) override;
private:
	long long total_ms_ = 0;
	long long pos_ms_ = 0;
	bool audio_ = true;
	XDemuxTask demux_;						//解封装
	XDecodeTask audio_decode_;				//音频解码
	XDecodeTask video_decode_;				//视频解码
	XVideoView* view_ = nullptr;			//视频渲染
};

