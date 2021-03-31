
/****************************************************************************
* �ʃv���O�������瑀�삳���g���b�J�[�̃h���C�o
* 2�̃g���b�J�[�̎p���͋��L����������Ďw�肳���
*****************************************************************************/

#include <openvr_driver.h>

//openVR�̃T���v���h���C�o����driverlog.cpp�������Ă���
#include <driverlog.h>


#include "shared_device.h"

using namespace vr;

#define DRIVER_NAME "SimpleTrackerDriverTest"
#define DRIVER_FOLDER "{" DRIVER_NAME "}"
#define TRACKER_NAME "SimpleTrackerTest"
#define TRACKER_ICON_FOLDER DRIVER_FOLDER"/icons/tracker"

void SetQuaternion(HmdQuaternion_t *q, double w, double x, double y, double z) {
	q->w = w;
	q->x = x;
	q->y = y;
	q->z = z;
}

void SetVec3(double vec3[3], double x, double y, double z) {
	vec3[0] = x;
	vec3[1] = y;
	vec3[2] = z;
}
// �g���b�J�[�̓����`�N���X
class CTestTrackerExtProc : public ITrackedDeviceServerDriver {
private:
	TrackedDeviceIndex_t m_unDeviceIndex;
	PropertyContainerHandle_t m_ulPropertyContainer;
	//���t���[�����Z�����B
	int m_nFrametime;
	static const int s_nLoopFrame = 160;//���t���[���Ń��[�v�����邩;
	//���L�������̃|�C���^
	SharedDevice_t *m_pSharedTrackers;
public:
	CTestTrackerExtProc(SharedDevice_t *sharedmem) {
		m_unDeviceIndex = k_unTrackedDeviceIndexInvalid;
		m_ulPropertyContainer = k_ulInvalidPropertyContainer;
		m_nFrametime = 0;
		m_pSharedTrackers = sharedmem;
	}

	~CTestTrackerExtProc() {}

	EVRInitError Activate(TrackedDeviceIndex_t unObjectId) override {
		m_unDeviceIndex = unObjectId;
		m_ulPropertyContainer = VRProperties()->TrackedDeviceToPropertyContainer(m_unDeviceIndex);

		VRProperties()->SetUint64Property(m_ulPropertyContainer, Prop_CurrentUniverseId_Uint64, 2);// 0(����),1(oculus�p)�ȊO�̒萔
		VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_ModelNumber_String, TRACKER_NAME);
		VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_ControllerType_String, TRACKER_NAME);
		VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_RenderModelName_String, DRIVER_FOLDER"tracker_b");// 3D���f���̒ǉ�
		VRProperties()->SetBoolProperty(m_ulPropertyContainer, Prop_DeviceProvidesBatteryStatus_Bool, true);//�o�b�e���[����
		VRProperties()->SetFloatProperty(m_ulPropertyContainer, Prop_DeviceBatteryPercentage_Float, 1.0);//1.0�Ŗ��^��

		//�A�C�R���̒ǉ�
		VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceOff_String, TRACKER_ICON_FOLDER"/off.png");
		VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceSearching_String, TRACKER_ICON_FOLDER"/searching.png");
		VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceSearchingAlert_String, TRACKER_ICON_FOLDER"/searching_alert.png");
		VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceReady_String, TRACKER_ICON_FOLDER"/ready.png");
		VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceReadyAlert_String, TRACKER_ICON_FOLDER"/ready_alert.png");
		VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceNotReady_String, TRACKER_ICON_FOLDER"/not_ready.png");
		VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceStandby_String, TRACKER_ICON_FOLDER"/standby.png");
		VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceAlertLow_String, TRACKER_ICON_FOLDER"/low.png");

		return VRInitError_None;
	}

	void Deactivate() override {
		m_unDeviceIndex = k_unTrackedDeviceIndexInvalid;
	}

	void EnterStandby() override {}
	void *GetComponent(const char *pchComponentNameAndVersion) override { return NULL; }

	void DebugRequest(const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize) override {
		//�N���C�A���g����̃f�o�b�O���N�G�X�g?
		//�Ƃ肠���� �󕶎���Ƃ��ĕԂ�
		DriverLog("%s", pchRequest);
		if (unResponseBufferSize >= 1) {
			pchResponseBuffer[0] = '\0';
		}
	}

	DriverPose_t GetPose() override {
		DriverPose_t pose = { 0 };
		pose.result = TrackingResult_Running_OK;
		pose.shouldApplyHeadModel = false;
		pose.poseTimeOffset = 0.0;
		pose.willDriftInYaw = false;
		//���[���h���f�o�C�X�ϊ���`
		SetVec3(pose.vecWorldFromDriverTranslation, 0, 0, 0);//�ړ��Ȃ�
		SetQuaternion(&pose.qWorldFromDriverRotation, 1, 0, 0, 0);//��]�Ȃ�
		//�f�o�C�X��HMD�ϊ���`
		SetVec3(pose.vecDriverFromHeadTranslation, 0, 0, 0);//�ړ��Ȃ�
		SetQuaternion(&pose.qDriverFromHeadRotation, 1, 0, 0, 0);//��]�Ȃ�
		//�f�o�C�X�p����ݒ�
		//���L����������p�����
		pose.deviceIsConnected = m_pSharedTrackers->deviceIsConnected;
		pose.poseIsValid = m_pSharedTrackers->poseIsValid;
		SetVec3(
			pose.vecPosition,
			m_pSharedTrackers->vPosition.v[0],
			m_pSharedTrackers->vPosition.v[1],
			m_pSharedTrackers->vPosition.v[2]
		);
		pose.qRotation = m_pSharedTrackers->qRotation;

		return pose;
	}


	void RunFrame()
	{
		if (m_unDeviceIndex != k_unTrackedDeviceIndexInvalid)//�f�o�C�X��Activate����Ă���Ȃ�
		{
			//�o�b�e���[���X�V
			VRProperties()->SetFloatProperty(m_ulPropertyContainer, Prop_DeviceBatteryPercentage_Float, m_pSharedTrackers->battery);//1.0�Ŗ��^��
			//�p���X�V
			VRServerDriverHost()->TrackedDevicePoseUpdated(m_unDeviceIndex, GetPose(), sizeof(DriverPose_t));
		}
	}

};

// �f�o�C�X�v���o�C�_�̃N���X
class CDeviceProvider : public IServerTrackedDeviceProvider
{
private:
	static const int s_TrackerNum = 2;
	CTestTrackerExtProc *m_pTracker[s_TrackerNum] = { nullptr,nullptr };
	CSharedDevicesMem *m_SharedTrackers;

public:
	EVRInitError Init(IVRDriverContext *pDriverContext) override//������
	{
		VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);
		InitDriverLog(VRDriverLog());

		char serialNumber[64];

		//���L�������̐ݒ�
		m_SharedTrackers = new CSharedDevicesMem(s_TrackerNum);
		if (!m_SharedTrackers->Create()) {
			DriverLog("!!!Failed to create shared memory!!!!");
			return VRInitError_Driver_Failed;
		}

		for (auto i = 0; i < s_TrackerNum; i++) {
			m_pTracker[i] = new CTestTrackerExtProc(m_SharedTrackers->GetMemPtr() + i);
			//�g���b�J�[��o�^
			sprintf_s(serialNumber, 64, "Test_%d", i);
			VRServerDriverHost()->TrackedDeviceAdded(serialNumber, TrackedDeviceClass_GenericTracker, m_pTracker[i]);//�������͎�����Prop_SerialNumber_String�ɐݒ肳���
		}

		return VRInitError_None;
	}
	void Cleanup() override//�J��
	{
		CleanupDriverLog();

		for (auto i = 0; i < s_TrackerNum; i++) {
			delete m_pTracker[i];
			m_pTracker[i] = nullptr;
		}

		//���L�������̊J��
		m_SharedTrackers->Delete();

		VR_CLEANUP_SERVER_DRIVER_CONTEXT()
	}
	void RunFrame() override//���t���[���Ăяo��
	{
		for (auto i = 0; i < s_TrackerNum; i++) {
			if (m_pTracker[i]) {
				m_pTracker[i]->RunFrame();
			}
		}
	}
	const char * const *GetInterfaceVersions() override { return k_InterfaceVersions; }
	bool ShouldBlockStandbyMode() override { return false; }
	void EnterStandby() override {}
	void LeaveStandby() override {}

};

CDeviceProvider g_serverDriverNull;
extern "C" __declspec(dllexport) void *HmdDriverFactory(const char *pInterfaceName, int *pReturnCode)
{
	if (0 == strcmp(IServerTrackedDeviceProvider_Version, pInterfaceName))
	{
		return &g_serverDriverNull;
	}
	if (pReturnCode)
		*pReturnCode = VRInitError_Init_InterfaceNotFound;

	return NULL;
}
