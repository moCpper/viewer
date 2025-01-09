#include "xcamera_config.h"

#define CAM_CONF_PATH "D:\\cppsoft\\ffmpeg\\code\\src\\xviewer\\test.db"

void XCameraConfig::Push(const XCameraData& data) {
	std::unique_lock<std::mutex> lock(mux_);
	cams_.push_back(data);
}

XCameraData XCameraConfig::GetCam(int index) {
	std::unique_lock<std::mutex> lock(mux_);
	if (index < 0 || index > cams_.size()) {
		return XCameraData();
	}
	return cams_[index];
}

int XCameraConfig::GetCamSize() {
	std::unique_lock<std::mutex> lock(mux_);
	return cams_.size();
}

bool XCameraConfig::Save(const char* path){
	if (!path) { return false; }
	std::ofstream ofs(path,std::ios::binary);
	if (!ofs) { ofs.close(); return false; }
	std::unique_lock<std::mutex> lock(mux_);
	for (auto& cam : cams_) {
		ofs.write((char*)&cam, sizeof(cam));
	}
	ofs.close();
	return true;
}

bool XCameraConfig::Load(const char* path){
	if (!path) { return false; }
	std::ifstream ifs(path, std::ios::binary);
	if (!ifs) { ifs.close(); return false; }
	XCameraData data;
	std::unique_lock<std::mutex> lock(mux_);
	cams_.clear();
	for (;;) {
		ifs.read((char*)&data, sizeof(data));
		if (ifs.gcount() != sizeof(data)) {
			ifs.close();
			return true;
		}
		cams_.push_back(data);
	}
	ifs.close();
	return true;
}

XCameraConfig::~XCameraConfig(){}

bool XCameraConfig::SetCam(int index, XCameraData& data) {
	std::unique_lock<std::mutex> lock(mux_);
	if (index < 0 || index > cams_.size()) { return false; }
	cams_[index] = data;
	return true;
}

bool XCameraConfig::DelCam(int index) {
	std::unique_lock<std::mutex> lock(mux_);
	if (index < 0 || index > cams_.size()) { return false; }
	cams_.erase(cams_.begin() + index);
	std::ofstream ofs(CAM_CONF_PATH, std::ios::binary | std::ios::out | std::ios::trunc);
	ofs.seekp(std::ios_base::beg);
	if (!ofs) { ofs.close(); return false; }
	for (auto& cam : cams_) {
		ofs.write((char*)&cam, sizeof(cam));
	}
	ofs.close();
	return true;
}