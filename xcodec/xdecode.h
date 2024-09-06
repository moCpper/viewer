#pragma once
#include "xcodec.h"
struct AVBufferRef;
class XCODEC_API XDecode :public XCodec
{
public:
    bool Send(const AVPacket* pkt);  //���ͽ���
    bool Recv(AVFrame* frame);       //��ȡ����
    std::vector<AVFrame*> End();    //��ȡ����

    ////////////////////////////////////////////////////
    //// ��ʼ��Ӳ������  4 AV_HWDEVICE_TYPE_DXVA2
    bool InitHW(int type = 4);

};

