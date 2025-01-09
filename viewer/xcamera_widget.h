#pragma once
#include <QWidget>

class XDecodeTask;
class XDemuxTask;
class XVideoView;
class XCameraWidget :public QWidget{
	Q_OBJECT
public:
	XCameraWidget(QWidget* p = nullptr);

	//拖拽进入
	void dragEnterEvent(QDragEnterEvent* e) override;
	//拖拽松开
	void dropEvent(QDropEvent* e) override;

	//渲染
	void paintEvent(QPaintEvent* p) override;

	//打开rtsp 开始解封装 解码
	bool Open(const char* url);

	//渲染视频
	void Draw();

	~XCameraWidget();
private:
	XDecodeTask* decode_ = nullptr;
	XDemuxTask* demux_ = nullptr;
	XVideoView* view_ = nullptr;
};

