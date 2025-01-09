#include "xviewer.h"
#include <QtWidgets/QApplication>
#include<qDebug>
#include<QDir>
#include"xcamera_config.h"
#include"xcamera_record.h"
#include"xplayvideo.h"

#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )


int main(int argc, char *argv[]){

  /*  const char* save_path = "./video/0/";
    QDir dir;
    dir.mkpath(save_path);

    XCameraRecord xr;
    xr.set_rtsp_url("rtsp://127.0.0.1:8554/test");
    xr.set_save_path(save_path);
    xr.Start();*/

    QApplication a(argc, argv);
    XViewer w;
    w.show();
    /*XPlayVideo xp;
    xp.Open("v1080.mp4");
    xp.exec();*/


   // auto re = a.exec();
    //xr.Stop();
    
    return a.exec();
}
