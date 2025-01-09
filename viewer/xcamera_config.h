#pragma once
#include<vector>
#include<mutex>
#include<fstream>

struct XCameraData {
	char name[1024] = { 0 };
	char url[4096] = { 0 };				//�����������
	char sub_url[4096] = { 0 };			//�����������
	char save_path[4096] = { 0 };		//��Ƶ¼�ƴ��Ŀ¼
};

class XCameraConfig{
public:
	static XCameraConfig* Instance() {
		static XCameraConfig xc;
		return &xc;
	}
	void Push(const XCameraData& data);

	/// <summary>
	/// ��ȡ����ͷ
	/// </summary>
	/// <param name="index">����ͷ��������0��ʼ</param>
	/// <returns></returns>
	XCameraData GetCam(int index);

	/// <summary>
	/// �޸����������
	/// </summary>
	/// <param name="index"></param>
	/// <param name="data"></param>
	/// <returns></returns>
	bool SetCam(int index,XCameraData& data);

	bool DelCam(int index);

	/// <summary>
	/// ��ȡ�������
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

