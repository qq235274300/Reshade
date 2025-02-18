#include "GlassDeviceManager.hpp"
#include <stdexcept>
#include "dll_log.hpp"

#pragma comment(lib, "hid.lib")
#pragma comment(lib, "setupapi.lib")

reshade::GlassDeviceManager::GlassDeviceManager()
	:devpath(L"")
{
}

reshade::GlassDeviceManager::~GlassDeviceManager()
{
}

bool reshade::GlassDeviceManager::IsGlassDevicePresent(const std::wstring &targetVID, const std::wstring &targetPID, const std::wstring &targetMI)
{
	GUID hidGuid;
	HidD_GetHidGuid(&hidGuid);

	HDEVINFO deviceInfoSet = SetupDiGetClassDevs(&hidGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if (deviceInfoSet == INVALID_HANDLE_VALUE) {
		throw std::runtime_error("Failed to get device info set.");
	}

	SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
	deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

	DWORD deviceIndex = 0;
	while (SetupDiEnumDeviceInterfaces(deviceInfoSet, NULL, &hidGuid, deviceIndex, &deviceInterfaceData)) {
		DWORD requiredSize = 0;
		SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData, NULL, 0, &requiredSize, NULL);

		std::vector<char> detailDataBuffer(requiredSize);
		PSP_DEVICE_INTERFACE_DETAIL_DATA deviceInterfaceDetailData = reinterpret_cast<PSP_DEVICE_INTERFACE_DETAIL_DATA>(&detailDataBuffer[0]);
		deviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		SP_DEVINFO_DATA devInfoData;
		devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

		if (SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData, deviceInterfaceDetailData, requiredSize, NULL, &devInfoData)) {
			DWORD dataType, dataSize = 0;
			SetupDiGetDeviceRegistryProperty(deviceInfoSet, &devInfoData, SPDRP_HARDWAREID, &dataType, NULL, 0, &dataSize);
			std::vector<BYTE> propertyBuffer(dataSize);

			if (SetupDiGetDeviceRegistryProperty(deviceInfoSet, &devInfoData, SPDRP_HARDWAREID, &dataType, propertyBuffer.data(), dataSize, NULL)) {
				std::wstring hardwareID(reinterpret_cast<wchar_t *>(propertyBuffer.data()));

				//std::wcout << hardwareID << std::endl;

				if (hardwareID.find(targetVID) != std::string::npos &&
					hardwareID.find(targetPID) != std::string::npos &&
					hardwareID.find(targetMI) != std::string::npos &&
					//Special hack forÊÓÑÄ, 2 interface with only col different, and Col03 is the right one. Why? Nobody knows 
					hardwareID.find(L"Col03") != std::string::npos) {

					SetupDiDestroyDeviceInfoList(deviceInfoSet);
					devpath = deviceInterfaceDetailData->DevicePath;
					reshade::log::message(reshade::log::level::info, "3D Glass Device Path: {}", devpath);
					return true;
				}
			}
		}
		++deviceIndex;
	}

	SetupDiDestroyDeviceInfoList(deviceInfoSet);
	return false;
}

void reshade::GlassDeviceManager::WriteToGlassDevice(const std::vector<unsigned char> &data)
{

	HANDLE deviceHandle = CreateFile(devpath.c_str(),
		GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if (deviceHandle == INVALID_HANDLE_VALUE) {
		reshade::log::message(reshade::log::level::error, "Failed to open device for writing!");
		return;
	}

	try {
		DWORD bytesWritten = data.size();
		if (!WriteFile(deviceHandle, data.data(), data.size(), &bytesWritten, NULL)) {
			reshade::log::message(reshade::log::level::error, "Failed to write data!");
		}
		else {
			std::lock_guard<std::mutex> lock(io_mutex);
			//std::cout << "Data Written: ";
			reshade::log::message(reshade::log::level::error, "Data Written :");
			for (size_t i = 0; i < bytesWritten; ++i) {
				//std::cout << std::hex << static_cast<int>(data[i]) << " ";
				reshade::log::message(reshade::log::level::error, "{:02x} ", data[i]);
			}
			//std::cout << std::endl;
		}
	}
	catch (const std::exception &e) {
		//std::cerr << "Exception occurred while writing: " << e.what() << std::endl;
		reshade::log::message(reshade::log::level::error, "Exception occurred while writing:!");
	}

	CloseHandle(deviceHandle);
}

uint8_t reshade::GlassDeviceManager::GetCheckSum(unsigned char *buffer, int length)
{
	uint8_t checksum = 0;
	for (int i = 0; i < length; checksum += buffer[i++]);
	return checksum;
}



std::wstring reshade::GlassDeviceManager::GetDevicePath() const
{
	return devpath;
}
