#include "xcamera_record.h"
#include"xdemux_task.h"
#include"xmux_task.h"
#include<chrono>
#include<iomanip>
#include<sstream>

//���ɴ洢����Ƶ�ļ���
static std::string GetFileName(std::string path) {
	std::stringstream ss;
	//��ȡ��ǰʱ��
	auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	auto time_str = std::put_time(localtime(&t), "%Y_%m_%d_%H_%M_%S");
	ss << path << "/" << "cam_" << time_str << ".mp4";
	return ss.str();
}

void XCameraRecord::Main() {
	XDemuxTask demux;
	XMuxTask xmux_;
	if (rtsp_url_.empty()) {
		LOGERROR("open rtsp url failed !");
		return;
	}
	while (!is_exit_) {
		if (demux.Open(rtsp_url_)) {			//��������1��
			break;
		}
		MSleep(3000);
		continue;
	}
	auto vpara = demux.CopyVideoPara();
	if (!vpara) {
		demux.Stop();
		return;
	}
	//�������װ�̣߳���ǰ������ֹ��ʱ
	demux.Start();
	auto apara = demux.CopyAudioPara();

	AVCodecParameters* para = nullptr;			//��Ƶ����
	AVRational* time_base = nullptr;			//��Ƶʱ�����
	if (apara) {
		para = apara->para;
		time_base = apara->time_base;
	}

	if (!xmux_.Open(GetFileName(save_path_).c_str(), vpara->para, vpara->time_base,		//��Ƶ����
		para, time_base)) {												//��Ƶ����
		LOGERROR("mux open rtsp_url_ failed!");
		demux.Stop();
		xmux_.Stop();
		return;
	}
	demux.set_next(&xmux_);
	xmux_.Start();

	auto cur = NowMs();
	while (!is_exit_) {
		//��ʱ�����µ��ļ�
		if (NowMs() - cur > file_sec_ * 1000) {
			cur = NowMs();
			xmux_.Stop();								//ֹͣ�洢��д������
			if (!xmux_.Open(GetFileName(save_path_).c_str(), vpara->para, vpara->time_base,		//��Ƶ����
				para, time_base)) {										//��Ƶ����
				LOGERROR("mux open rtsp_url_ failed!");
				demux.Stop();
				xmux_.Stop();
				return;
			}
			xmux_.Start();
		}
		MSleep(10);
	}
	xmux_.Stop();
	demux.Stop();
}