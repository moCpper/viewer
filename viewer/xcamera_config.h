#pragma once
#include<vector>
#include<mutex>
#include<fstream>

struct XCameraData {
	char name[1024] = { 0 };
	char url[4096] = { 0 };				//摄像机主码流
	char sub_url[4096] = { 0 };			//摄像机辅码流
	char save_path[4096] = { 0 };		//视频录制存放目录
};

class XCameraConfig{
public:
	static XCameraConfig* Instance() {
		static XCameraConfig xc;
		return &xc;
	}
	void Push(const XCameraData& data);

	/// <summary>
	/// 获取摄像头
	/// </summary>
	/// <param name="index">摄像头索引，从0开始</param>
	/// <returns></returns>
	XCameraData GetCam(int index);

	/// <summary>
	/// 修改摄像机数据
	/// </summary>
	/// <param name="index"></param>
	/// <param name="data"></param>
	/// <returns></returns>
	bool SetCam(int index,XCameraData& data);

	bool DelCam(int index);

	/// <summary>
	/// 获取相机数量
	/// </summary>
	/// <returns>err return 0 </returns>
	int GetCamSize();

	bool Save(const char* path);

	bool Load(const char* path);

	~XCameraConfig();
private:
	XCameraConfig() = default;
	std::vector<XCameraData> cams_;
	std::mutex mux_;
};

