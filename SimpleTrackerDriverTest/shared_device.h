#pragma once

#include <openvr_driver.h>
#include <windows.h>

using namespace vr;

//���L�������̖��O
static const char *SharedMemName = "Sharedmem_OpenVRDriver_SimpleTest";

//���L�������Ɋ��蓖�Ă�f�o�C�X�\����
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

//�ȉ�����///////////////////////////


CSharedDevicesMem::CSharedDevicesMem(const int shareDeviceNum) {
	//�f�o�C�X���������L���邩
	m_nShareDeviceNum = shareDeviceNum;
}
bool CSharedDevicesMem::Create() {
	DWORD memSize = sizeof(SharedDevice_t)*m_nShareDeviceNum;//�\����*�f�o�C�X���̗̈���m��
	m_hSharedMem = CreateFileMapping(NULL, NULL, PAGE_READWRITE, NULL, memSize, SharedMemName);
	if (!m_hSharedMem) {
		return false;
	}
	m_pSharedDevice = (SharedDevice_t*)MapViewOfFile(m_hSharedMem, FILE_MAP_ALL_ACCESS, NULL, NULL, memSize);//���L�������̃|�C���^���擾
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