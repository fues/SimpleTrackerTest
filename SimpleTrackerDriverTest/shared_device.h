#pragma once

#include <openvr_driver.h>
#include <windows.h>

using namespace vr;

//共有メモリの名前
static const char *SharedMemName = "Sharedmem_OpenVRDriver_SimpleTest";

//共有メモリに割り当てるデバイス構造体
typedef struct {
	HmdVector3_t vPosition;
	HmdQuaternion_t qRotation;
	bool deviceIsConnected;
	bool poseIsValid;
	float battery;
}SharedDevice_t;

class CSharedDevicesMem {
private:
	HANDLE m_hSharedMem;
	int m_nShareDeviceNum;
	SharedDevice_t *m_pSharedDevice;
public:
	CSharedDevicesMem(const int shareDeviceNum);
	bool Create();
	void Delete();
	SharedDevice_t *GetMemPtr();
};

//以下実体///////////////////////////


CSharedDevicesMem::CSharedDevicesMem(const int shareDeviceNum) {
	//デバイスをいくつ共有するか
	m_nShareDeviceNum = shareDeviceNum;
}
bool CSharedDevicesMem::Create() {
	DWORD memSize = sizeof(SharedDevice_t)*m_nShareDeviceNum;//構造体*デバイス数の領域を確保
	m_hSharedMem = CreateFileMapping(NULL, NULL, PAGE_READWRITE, NULL, memSize, SharedMemName);
	if (!m_hSharedMem) {
		return false;
	}
	m_pSharedDevice = (SharedDevice_t*)MapViewOfFile(m_hSharedMem, FILE_MAP_ALL_ACCESS, NULL, NULL, memSize);//共有メモリのポインタを取得
	return true;
}
void CSharedDevicesMem::Delete() {
	UnmapViewOfFile(m_pSharedDevice);
	CloseHandle(m_hSharedMem);
}

SharedDevice_t * CSharedDevicesMem::GetMemPtr()
{
	return m_pSharedDevice;
}