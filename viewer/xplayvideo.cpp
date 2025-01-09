#include "xplayvideo.h"
#include<qDebug>

XPlayVideo::XPlayVideo(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	/*startTimer(10);*/
}

XPlayVideo::~XPlayVideo() { Close(); }

void XPlayVideo::timerEvent(QTimerEvent* ev) {
	if (player_.is_pause()) {
		return;
	}
	player_.Update();
	auto pos = player_.pos_ms();
	auto total_ms = player_.total_ms();
	ui.pos->setMaximum(total_ms);
	ui.pos->setValue(pos);
	/*if (!view_) { return; }
	auto f = decode_.GetFrame();
	if (!f) { return; }
	view_->DrawFrame(f);
	XFreeFrame(&f);*/
}

bool XPlayVideo::Open(const char* url){
	if (!player_.Open(url, (void*)this->ui.video->winId())) {
		return false;
	}
	player_.Start();
	player_.Pause(false);			//播放状态
	startTimer(10);
	//关闭上次数据
	//if (!demux_.Open(url)) {return false;}				//解封装
	//auto vp = demux_.CopyVideoPara();
	//if (!vp) {
	//	return false;
	//}
	//if (!decode_.Open(vp->para)) { return false; }		//解码
	//demux_.set_next(&decode_);
	//if (!view_) {
	//	view_ = XVideoView::Create();
	//}
	//view_->set_win_id((void*)this->winId());
	//if (!view_->Init(vp->para)) {						//SDL2渲染
	//	return false;
	//}
	//demux_.set_syn_type(XSYN_VIDEO);
	//demux_.Start();
	//decode_.Start();
	return true;
}

void XPlayVideo::closeEvent(QCloseEvent* ev) {
	Close();
}

void XPlayVideo::Close() {
	player_.Stop();
	//demux_.Stop();
	//decode_.Stop();
	//if (view_) {
	//	view_->Close();
	//	delete view_;
	//	view_ = nullptr;
	//}
}

void XPlayVideo::SetSpeed() {
	float speed = 1;
	int s = ui.speed->value();
	if (s <= 10) {
		speed = (float)s / (float)10;
	}else {
		speed = s - 9;
	}
	ui.speedtxt->setText(QString::number(speed));
	player_.SetSpeed(speed);
}

void XPlayVideo::PlayPos() {
	player_.Seek(ui.pos->value());
	player_.Pause(false);
}

void XPlayVideo::Pause(){
	player_.Pause(!player_.is_pause());
}

void XPlayVideo::Move() {
	player_.Pause(true);
}