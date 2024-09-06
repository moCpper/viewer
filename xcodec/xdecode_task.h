#pragma once
#include "xtools.h"
#include "xdecode.h"
class XCODEC_API XDecodeTask :public XThread
{
public:
    /// <summary>
    /// �򿪽�����
    /// </summary>
    bool Open(AVCodecParameters* para);

    //������������
    void Do(AVPacket* pkt) override; 

    //�߳�������
    void Main() override;

    // �̰߳�ȫ�����ص�ǰ��Ҫ��Ⱦ��AVFrame�����û�з���nullptr
    // need_view_������Ⱦ
    // ���ؽ����Ҫ�� XFreeFrame �ͷ�
    AVFrame* GetFrame();

    void set_stream_index(int i) { stream_index_ = i; }

    void set_frame_cache(bool is) { frame_cache_ = is; }
    bool is_open() { return is_open_; }

    //����ͬ��ʱ��
    void set_syn_pts(long long p) { syn_pts_ = p; }
    void set_block_size(int s) { block_size_ = s; }

    //����ʱ�����
    void set_time_base(AVRational* time_base);

    long long cur_ms() { return cur_ms_; }


    /// <summary>
    /// ������
    /// </summary>
    void Clear();

    void Stop();
private:
    long long cur_pts_ = -1;     //��ǰ���뵽��pts ���Խ�������Ϊ׼��
    AVRational* time_base_ = nullptr; 
    long long cur_ms_ = 0;
    int block_size_ = 0;                  //������С
    long long syn_pts_ = -1;                //ͬ��ʱ�� -1��ͬ��
    bool is_open_ = false;
    int stream_index_ = 0;
    std::mutex mux_;
    XDecode decode_;
    XAVPacketList pkt_list_;    
    AVFrame* frame_ = nullptr;              //�����洢
    bool need_view_ = false;                //�Ƿ���Ҫ��Ⱦ��ÿֻ֡��Ⱦһ�Σ�ͨ��GetFrame
    std::list<AVFrame*> frames_;            //�洢��Ƶ����
    bool frame_cache_ = false;              //�Ƿ񻺳�frame����
};

