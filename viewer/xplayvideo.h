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
	void PlayPos();		//���Ʋ��Ž��ȣ��������ɿ���
	void Pause();		//���ź���ͣ
	void Move();		//�������϶�
private:
	Ui::XPlayVideoClass ui;
	XPlayer player_;
};
