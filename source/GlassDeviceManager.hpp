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
		// ���캯������������
		GlassDeviceManager();
		~GlassDeviceManager();

		// ���� 3D �۾��豸
		bool IsGlassDevicePresent(const std::wstring &targetVID, const std::wstring &targetPID, const std::wstring &targetMI);

		// ���豸��������
		void WriteToGlassDevice(const std::vector<unsigned char> &data);

		// �������ݵ�У���
		uint8_t GetCheckSum(unsigned char *buffer, int length);

		// ��ȡ�豸·��
		std::wstring GetDevicePath() const;

	private:
		std::wstring devpath;  // �洢�豸·��
		std::mutex io_mutex;   // ���� I/O �����Ļ�����

	};
}



