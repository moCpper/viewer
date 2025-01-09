#include "xviewer.h"
#include"xcamera_config.h"
#include"xcamera_widget.h"
#include"xcamera_record.h"
#include"xplayvideo.h"
#include<vector>
#include<iostream>
#include<sstream>
#include<map>
#include<QMouseEvent>
#include<QHBoxLayout>
#include<QVBoxLayout>
#include<QContextMenuEvent>
#include<QGridLayout>
#include<qDebug>
#include<QDialog>
#include<QDir>
#include<QFormLayout>
#include<QLineEdit>
#include<QMessageBox>

#define CAM_CONF_PATH "D:\\cppsoft\\ffmpeg\\code\\src\\xviewer\\test.db"
//解决中文乱码
#define C(s) QString::fromLocal8Bit(s)

//渲染窗口
static XCameraWidget* cam_wids[16]{ 0 };

//视频录制
static std::vector<XCameraRecord*> records;

//存储视频日期时间
struct XCamVideo {
    QString filepath;
    QDateTime datetime;
};
static std::map<QDate, std::vector<XCamVideo>> cam_videos;

XViewer::XViewer(QWidget *parent)
    : QWidget(parent){

    ui.setupUi(this);

    //去除原窗口边框
    setWindowFlags(Qt::FramelessWindowHint);

    //布局head和body 垂直布局器
    auto vlay = new QVBoxLayout();
    //边框间距
    vlay->setContentsMargins(0, 0, 0, 0);
    //元素间距
    vlay->setSpacing(0);
    vlay->addWidget(ui.head);
    vlay->addWidget(ui.body);
    this->setLayout(vlay);

    //相机列表 和相机预览
    //水平布局器
    auto hlay = new QHBoxLayout();
    ui.body->setLayout(hlay);
    //边框间距
    hlay->setContentsMargins(0, 0, 0, 0);
    hlay->addWidget(ui.left);
    hlay->addWidget(ui.cams);
    hlay->addWidget(ui.playback_wid);       //回放窗口

    auto m = left_menu_.addMenu(C("视图"));

    auto a = m->addAction(C("1窗口"));
    QObject::connect(a, &QAction::triggered, this, &XViewer::View1);
    a = m->addAction(C("4窗口"));
    QObject::connect(a, &QAction::triggered, this, &XViewer::View4);
    a = m->addAction(C("9窗口"));
    QObject::connect(a, &QAction::triggered, this, &XViewer::View9);
    a = m->addAction(C("16窗口"));
    QObject::connect(a, &QAction::triggered, this, &XViewer::View16);
    a = left_menu_.addAction(C("全部开始录制"));
    QObject::connect(a, &QAction::triggered, this, &XViewer::StartRecord);
    a = left_menu_.addAction(C("全部停止录制"));
    QObject::connect(a, &QAction::triggered, this, &XViewer::StopRecord);
    //默认9窗口
    View(9);
  /*  XCameraData data;
    ::strcpy_s(data.name, "cam");
    ::strcpy_s(data.url,"rtsp://127.0.0.1:5778/test");
    ::strcpy_s(data.sub_url, "rtsp://127.0.0.1:5778/test");
    ::strcpy_s(data.save_path, "D:\\cppsoft\\ffmpeg\\code\\src\\xviewer");
    XCameraConfig::Instance()->Push(data);
    XCameraConfig::Instance()->Save(CAM_CONF_PATH);*/
    XCameraConfig::Instance()->Load(CAM_CONF_PATH);

    ui.time_list->clear();
    RefreshCams();

    //启动定时器渲染视频
    startTimer(1);

    Preview();
}

XViewer::~XViewer(){}

void XViewer::Preview() {
    ui.cams->show();
    ui.playback_wid->hide();
}

void XViewer::Playback() {
    ui.cams->hide();
    ui.playback_wid->show();
}

void XViewer::RefreshCams(){
    auto c = XCameraConfig::Instance();
    ui.cam_list->clear();
    int count = c->GetCamSize();
    for (int i = 0; i < count; i++) {
        auto cam = c->GetCam(i);
        auto item = new QListWidgetItem(QIcon("D:/cppsoft/ffmpeg/code/src/xviewer/img/cam.png"),
            C(cam.name));
        ui.cam_list->addItem(item);
    }
}

void XViewer::AddCam() {
    SetCam(-1);
}

void XViewer::SetCam(int index) {
    auto c = XCameraConfig::Instance();
    QDialog dlg(this);
    dlg.resize(QSize(800, 200));
    QFormLayout lay;
    dlg.setLayout(&lay);
    //标题 输入框
    QLineEdit name_edit;
    lay.addRow(C("名称"), &name_edit);
    QLineEdit url_edit;
    lay.addRow(C("主码流"), &url_edit);
    QLineEdit sub_url_edit;
    lay.addRow(C("辅码流"), &sub_url_edit);
    QLineEdit save_path_edit;
    lay.addRow(C("保存目录"), &save_path_edit);

    QPushButton save;
    save.setText(C("保存"));

    QObject::connect(&save, &QPushButton::clicked, &dlg, &QDialog::accept);
    lay.addRow("", &save);

    if (index >= 0) {
        auto cam = c->GetCam(index);
        name_edit.setText(C(cam.name));
        url_edit.setText(C(cam.url));
        sub_url_edit.setText(C(cam.sub_url));
        save_path_edit.setText(C(cam.save_path));
    }

    for (;;) {
        if (dlg.exec() == QDialog::Accepted) {
            if (name_edit.text().isEmpty()) {
                QMessageBox::information(0, "error", C("请输入名称"));
                continue;
            }
            if (url_edit.text().isEmpty()) {
                QMessageBox::information(0, "error", C("请输入主码流"));
                continue;
            }
            if (sub_url_edit.text().isEmpty()) {
                QMessageBox::information(0, "error", C("请输入辅码流"));
                continue;
            }
            if (save_path_edit.text().isEmpty()) {
                QMessageBox::information(0, "error", C("请输入保存目录"));
                continue;
            }
            break;
        }
        return;
    }
    XCameraData data;
    ::strcpy_s(data.name, name_edit.text().toLocal8Bit());
    ::strcpy_s(data.url, url_edit.text().toLocal8Bit());
    ::strcpy_s(data.sub_url, sub_url_edit.text().toLocal8Bit());
    ::strcpy_s(data.save_path, save_path_edit.text().toLocal8Bit());
    if (index >= 0) {                        // 新增
        c->SetCam(index, data); 
    }else {                                  //修改                                  
        c->Push(data);
    }
    c->Save(CAM_CONF_PATH);
    RefreshCams();
}

void XViewer::SetCam() {
    int row = ui.cam_list->currentIndex().row();            //返回当前选中的项的索引
    if (row < 0) {
        QMessageBox::information(this, "error", C("请选择摄像机"));
        return;
    }
    SetCam(row);
}

void XViewer::DelCam() {
    int row = ui.cam_list->currentIndex().row();
    if (row < 0) {
        QMessageBox::information(this, "error", C("请选择摄像机"));
        return;
    }
    std::stringstream ss;
    ss << "您确认要删除该摄像机"
        << ui.cam_list->currentItem()->text().toLocal8Bit().constData();
    ss << "吗？";

    if (QMessageBox::information(this, "confirm", C(ss.str().c_str()),
        QMessageBox::Yes,QMessageBox::No) != QMessageBox::Yes){
        return;
    }
    XCameraConfig::Instance()->DelCam(row);
    RefreshCams();
}

void XViewer::View(int count) {
    int clos = ::sqrt(count);
    int wid_size = sizeof(cam_wids) / sizeof(QWidget*);

    QGridLayout* lay = (QGridLayout*)ui.cams->layout();
    if (!lay) {
        lay = new QGridLayout();
        lay->setContentsMargins(0, 0, 0, 0);
        lay->setSpacing(2);         //元素间距
        ui.cams->setLayout(lay);
    }

    for (int i = 0; i < count; i++) {
        if (!cam_wids[i]) {
            cam_wids[i] = new XCameraWidget();
            cam_wids[i]->setStyleSheet("background-color:rgb(51,51,51);");
        }
        lay->addWidget(cam_wids[i],i/clos,i%clos);
    }

    //清理多余窗体
    for (int i = count; i < wid_size; i++) {
        if (cam_wids[i]) {
            delete cam_wids[i];
            cam_wids[i] = nullptr;
        }
    }
}

void XViewer::timerEvent(QTimerEvent* ev) {
    int wid_size = sizeof(cam_wids) / sizeof(QWidget*);
    for (int i = 0; i < wid_size; i++) {
        if (cam_wids[i]) {
            //渲染多窗口视频
            cam_wids[i]->Draw();
        }
    }
}

//此slots处理左侧摄像机点击事件
void XViewer::SelectCamera(QModelIndex index) {
    qDebug() << "SelectCamera" << index.row();
    auto conf = XCameraConfig::Instance();
    auto cam = conf->GetCam(index.row());           //获取相机参数
    if (cam.name[0] == '\0') { return; }
    //相机视频存储路径
    std::stringstream ss;
    ss << cam.save_path << "/" << index.row() << "/";

    //遍历此目录
    QDir dir(C(ss.str().c_str()));
    if (!dir.exists()) { return; }
    //获取目录下文件列表
    QStringList filters;
    filters << "*.mp4" << "*.avi";
    dir.setNameFilters(filters);        //筛选
    ui.cal->ClearDate();
    cam_videos.clear();
    //所有文件列表
    auto files = dir.entryInfoList(); 
    for (auto& file : files) {
        //"cam_2024_08_02_17_54_58.mp4"
        QString filename = file.fileName();
        
        //去掉cam_ 和 .mp4
        auto tmp = filename.left(filename.size() - 4);
        tmp = tmp.right(tmp.length() - 4);
        //2024_08_02_17_54_58
        auto dt = QDateTime::fromString(tmp,"yyyy_MM_dd_hh_mm_ss");
        qDebug() << dt.date();
        ui.cal->AddDate(dt.date());
        //qDebug() << file.fileName();

        XCamVideo video;
        video.datetime = dt;
        video.filepath = file.absoluteFilePath();
        cam_videos[dt.date()].push_back(video);
    }

    //重新显示日期
    ui.cal->showNextMonth();
    ui.cal->showPreviousMonth();
}

//此slots处理日历中日期的点击事件
void XViewer::SelectDate(QDate date) {
    qDebug() << "SelectDate" << date.toString();
    auto dates = cam_videos[date];
    ui.time_list->clear();
    for (auto& d : dates) {
        auto item = new QListWidgetItem(d.datetime.time().toString());
        //item添加自定义数据(路径)
        item->setData(Qt::UserRole,d.filepath);
        ui.time_list->addItem(item);
    }
}

void XViewer::PlayVideo(QModelIndex index) {
    qDebug() << "PlayVideo" << index.row();
    auto item = ui.time_list->currentItem();
    if (!item) { return; }
    QString path = item->data(Qt::UserRole).toString();
    qDebug() << path;
    XPlayVideo play;
    play.Open(path.toLocal8Bit());
    play.exec();
}

void XViewer::View1() {
    qDebug() << "view 1";
    View(1);
}

void XViewer::View4() {
    qDebug() << "view 4";
    View(4);
}

void XViewer::View9() {
    qDebug() << "view 9";
    View(9);
}

void XViewer::View16() {
    qDebug() << "view 16";
    View(16);
}

void XViewer::StartRecord() {
    StopRecord();
    qDebug() << "开始全部摄像头录制";
    ui.status->setText(C("录制中....."));
    //获取配置列表
    auto conf = XCameraConfig::Instance();
    int count = conf->GetCamSize();
    for (int i = 0; i < count; i++) {
        auto cam = conf->GetCam(i);
        std::stringstream ss;
        ss << cam.save_path << "/" << i << "/";
        QDir dir;
        dir.mkpath(ss.str().c_str());
        XCameraRecord* rec = new XCameraRecord();
        rec->set_rtsp_url(cam.url);
        rec->set_save_path(ss.str());
        rec->set_file_sec(5);
        rec->Start();
        records.push_back(rec);
    }
}

void XViewer::StopRecord() {
    ui.status->setText(C("录制中....."));
    for (auto rec : records) {
        rec->Stop();
        delete rec;
    }
    records.clear();
}

void XViewer::contextMenuEvent(QContextMenuEvent* event) {
    //鼠标位置显示右键菜单
    left_menu_.exec(QCursor::pos());
    event->accept();
}

/// <summary>
/// 鼠标事件
/// </summary>
/// <param name="ev"></param>

static bool mouse_press = false;
static QPoint mouse_point;

void XViewer::mouseMoveEvent(QMouseEvent* ev){
    if (!mouse_press) {
        QWidget::mouseMoveEvent(ev);
        return;
    }
    this->move(ev->globalPos() - mouse_point);
}

void XViewer::mousePressEvent(QMouseEvent* ev){
    if (ev->button() == Qt::LeftButton) {
        mouse_press = true;
        mouse_point = ev->pos();
    }
}

void XViewer::mouseReleaseEvent(QMouseEvent* ev){
    mouse_press = false;
}

void XViewer::resizeEvent(QResizeEvent* ev) {
    int x = this->width() - ui.head_button->width();
    int y = ui.head_button->height();
    ui.head_button->move(x, y-20);
    auto cam_list_w = ui.cam_list->width();
    auto cam_list_h = ui.left->height();
    ui.cam_list->resize(cam_list_w, cam_list_h);
    auto time_list_w = ui.time_list->width();
    auto time_list_h = ui.playback_wid->height();
    ui.time_list->resize(time_list_w, time_list_h);
    ui.cal->resize(ui.playback_wid->width() - time_list_w, time_list_h);
}

void XViewer::MaxWindow(){
    ui.max->setVisible(false);
    ui.normal->setVisible(true);
    this->showMaximized();
}

void XViewer::NormalWindow() {
    ui.max->setVisible(true);
    ui.normal->setVisible(false);
    this->showNormal();
}