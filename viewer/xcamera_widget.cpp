#include "xcamera_widget.h"
#include"xdemux_task.h"
#include"xdecode_task.h"
#include"xvideo_view.h"
#include"xcamera_config.h"
#include<QStyleOption>
#include<QPainter>
#include<QDragEnterEvent>
#include<QDropEvent>
#include<QDebug>
#include<QListWidget>

XCameraWidget::XCameraWidget(QWidget* p):QWidget(p){
	//������ק
	this->setAcceptDrops(true);
}

XCameraWidget::~XCameraWidget() {
	if (demux_) {
		demux_->Stop();
		delete demux_;
		demux_ = nullptr;
	}
	if (decode_) {
		decode_->Stop();
		delete decode_;
		decode_ = nullptr;
	}
	if (view_) {
		view_->Close();
		delete view_;
		view_ = nullptr;
	}
}

//��Ⱦ
void XCameraWidget::paintEvent(QPaintEvent* p) {
	//��Ⱦ��ʽ��
	QStyleOption opt;
	opt.init(this);
	QPainter painter(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}

bool XCameraWidget::Open(const char* url){
	if (demux_) {
		demux_->Stop();
	}
	if (decode_) {
		decode_->Stop();
	}
	//�򿪽��װ
	demux_ = new XDemuxTask();
	if (!demux_->Open(url)) { return false; }
	//����Ƶ������
	decode_ = new XDecodeTask();
	auto para = demux_->CopyVideoPara();
	if (!decode_->Open(para->para)) {
		return false;
	}
	demux_->set_next(decode_);

	//��ʼ����Ⱦ����
	view_ = XVideoView::Create();
	view_->set_win_id((void*)this->winId());
	view_->Init(para->para);

	//�������װ�ͽ���thread
	demux_->Start();
	decode_->Start();

	return true;
}

void XCameraWidget::Draw() {
	if (!decode_ || !demux_ || !view_) { return; }
	auto f = decode_->GetFrame();
	if (!f) { return; }
	view_->DrawFrame(f);
	XFreeFrame(&f);
}

//��ק����
void XCameraWidget::dragEnterEvent(QDragEnterEvent* e) {
	//������ק����
	e->acceptProposedAction();
}

//��ק�ɿ�
void XCameraWidget::dropEvent(QDropEvent* e) {
	//��ȡurl
	auto wid = (QListWidget*)e->source();
	qDebug() << wid->currentRow();
	auto cam = XCameraConfig::Instance()->GetCam(wid->currentRow());
	Open(cam.sub_url);
}
