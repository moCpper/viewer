#pragma once
#include"xtools.h"
/**
 *  \name Audio format flags
 *
 *  Defaults to LSB byte order.
 */
 /* @{ */
#define AUDIO_U8        0x0008  /**< Unsigned 8-bit samples */
#define AUDIO_S8        0x8008  /**< Signed 8-bit samples */
#define AUDIO_U16LSB    0x0010  /**< Unsigned 16-bit samples */
#define AUDIO_S16LSB    0x8010  /**< Signed 16-bit samples */
#define AUDIO_U16MSB    0x1010  /**< As above, but big-endian byte order */
#define AUDIO_S16MSB    0x9010  /**< As above, but big-endian byte order */
#define AUDIO_U16       AUDIO_U16LSB
#define AUDIO_S16       AUDIO_S16LSB
/* @} */

/**
 *  \name int32 support
 */
 /* @{ */
#define AUDIO_S32LSB    0x8020  /**< 32-bit integer samples */
#define AUDIO_S32MSB    0x9020  /**< As above, but big-endian byte order */
#define AUDIO_S32       AUDIO_S32LSB
/* @} */

/**
 *  \name float32 support
 */
 /* @{ */
#define AUDIO_F32LSB    0x8120  /**< 32-bit floating point samples */
#define AUDIO_F32MSB    0x9120  /**< As above, but big-endian byte order */
#define AUDIO_F32       AUDIO_F32LSB
/* @} */

/**
 *  \name Native audio byte ordering
 */
 /* @{ */
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define AUDIO_U16SYS    AUDIO_U16LSB
#define AUDIO_S16SYS    AUDIO_S16LSB
#define AUDIO_S32SYS    AUDIO_S32LSB
#define AUDIO_F32SYS    AUDIO_F32LSB
#else
#define AUDIO_U16SYS    AUDIO_U16MSB
#define AUDIO_S16SYS    AUDIO_S16MSB
#define AUDIO_S32SYS    AUDIO_S32MSB
#define AUDIO_F32SYS    AUDIO_F32MSB
#endif

#include<vector>
#include<list>
#include<mutex>

struct XAudioSpec {
	int freq = 44100;			//��Ƶ������
	unsigned short format = AUDIO_S16SYS;
	unsigned char channels = 2;
	unsigned short samples = 1024;
};

struct XData {
	std::vector<unsigned char> data;
	int offset = 0;					//ƫ��λ��
	long long pts = 0;
};

/// <summary>
/// ��Ƶ���� ����
/// </summary>
class XCODEC_API XAudioPlay{
public:
	static XAudioPlay* Instance();

	//����Ƶ ��ʼ���� ���ûص�
	virtual bool Open(AVCodecParameters* para);
	virtual bool Open(XPara& para);
	virtual bool Open(XAudioSpec& spec) = 0;
	virtual void Close() = 0;
	virtual void Clear() {
		Close();
		SetSpeed(speed_);

	}
	//��ͣ
	virtual void Pause(bool is_pause) = 0;
	//��ȡ��ǰ�Ĳ���λ��
	virtual long long cur_pts() = 0;
	virtual void Push(AVFrame* frame);

	void Push(const unsigned char* data, int size,long long pts) {
		std::unique_lock<std::mutex> lock(mux_);
		audio_datas_.push_back(XData());
		audio_datas_.back().pts = pts;
		audio_datas_.back().data.assign(data, data + size);
	}

	//�����ٶ�
	virtual void SetSpeed(float s) {
		speed_ = s;
		auto spec = spec_;
		auto old_freq = spec.freq;
		spec.freq *= s;
		Open(spec);
		spec_.freq = old_freq;
	}

	//��������
	void set_volume(int v) { volume_ = v; }

	void set_time_base(double b) { time_base_ = b; }

	~XAudioPlay();
protected:
	XAudioPlay();
	virtual void Callback(unsigned char* stream, int len) = 0;
	static void AudioCallback(void* userdata, unsigned char* stream, int len) {
		auto ap = (XAudioPlay*)userdata;
		ap->Callback(stream,len);
	}
	double time_base_ = 0;
	std::list<XData> audio_datas_;		//��Ƶ�����б�
	std::mutex mux_;
	unsigned char volume_ = 128;		// 0~128����
	XAudioSpec spec_;
	long long cur_pts_ = 0;				//��ǰ����λ��
	float speed_ = 1;					//�ٶ�
};

