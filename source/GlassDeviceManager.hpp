#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <mutex>

#include <hidsdi.h>
#include <setupapi.h>

namespace reshade
{
	class GlassDeviceManager
	{
	public:
		// 构造函数和析构函数
		GlassDeviceManager();
		~GlassDeviceManager();

		// 查找 3D 眼镜设备
		bool IsGlassDevicePresent(const std::wstring &targetVID, const std::wstring &targetPID, const std::wstring &targetMI);

		// 向设备发送数据
		void WriteToGlassDevice(const std::vector<unsigned char> &data);

		// 计算数据的校验和
		uint8_t GetCheckSum(unsigned char *buffer, int length);

		// 获取设备路径
		std::wstring GetDevicePath() const;

	private:
		std::wstring devpath;  // 存储设备路径
		std::mutex io_mutex;   // 保护 I/O 操作的互斥量

	};
}



