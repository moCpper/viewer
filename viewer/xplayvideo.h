#pragma once

#include<QDialog>
#include"ui_xplayvideo.h"
#include"xplayer.h"

class XPlayVideo : public QDialog
{
	Q_OBJECT

public:
	XPlayVideo(QWidget *parent = nullptr);
	~XPlayVideo();

	bool Open(const char* url);
	void timerEvent(QTimerEvent* ev) override;
	void closeEvent(QCloseEvent* ev) override;
	void Close();
public slots:
	void SetSpeed();
	void PlayPos();		//控制播放进度（进度条松开）
	void Pause();		//播放和暂停
	void Move();		//进度条拖动
private:
	Ui::XPlayVideoClass ui;
	XPlayer player_;
};
