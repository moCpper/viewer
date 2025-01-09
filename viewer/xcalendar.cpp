#include "xcalendar.h"
#include<QPainter>

XCalendar::XCalendar(QWidget* p):QCalendarWidget(p){

}

void XCalendar::paintCell(QPainter* painter, 
	const QRect& rec, const QDate& date) const{

	//有视频的日期特性显示
	//测试日期 4号
	if (mdate_.find(date) == mdate_.end()) {	//没有视频
		QCalendarWidget::paintCell(painter, rec, date);
		return;
	}

	auto font = painter->font();
	//设置字体
	font.setPixelSize(40);

	//选中状态刷背景色
	if (date == selectedDate()) {
		painter->setBrush(QColor(118, 178, 224));		//刷子颜色
		painter->drawRect(rec);							//绘制背景
	}
	painter->setFont(font);							//设置字体和颜色
	painter->setPen(QColor(255, 0, 0));				//字颜色
	painter->drawText(rec, Qt::AlignCenter, QString::number(date.day()));

}
