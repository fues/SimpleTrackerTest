
/****************************************************************************
* �P�̂œ��삷��g���b�J�[�̃h���C�o
* 2�̃g���b�J�[����]�E�h��Ȃ��獶�E�̃R���g���[����Ǐ]����
*****************************************************************************/

#include <openvr_driver.h>

//openVR�̃T���v���h���C�o����driverlog.cpp�������Ă���
#include <driverlog.h>

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
	bool m_bDispMatrix = false;//�e�f�o�C�X�̎p���s������O�ɕ\����
	//�f�o�C�X�̍��W�n�����߂�R���g���[���[
	ETrackedControllerRole m_nRefControllerRole;
	uint32_t m_nRefControllerIndex;
public:
	CTestTrackerExtProc(ETrackedControllerRole refControllerRole) {
		m_unDeviceIndex = k_unTrackedDeviceIndexInvalid;
		m_ulPropertyContainer = k_ulInvalidPropertyContainer;
		m_nFrametime = 0;
		m_nRefControllerRole = refControllerRole;
		m_nRefControllerIndex = 0;
		//vrsettings�t�@�C���̓ǂݍ��� ���݂��Ȃ��ꏊ���Q�Ƃ����0��""�ƂȂ�?
		static char const * settingKey = "driver_" DRIVER_NAME;
		char bufchar[64];
		bool bufbool = VRSettings()->GetBool(settingKey, "test_bool");
		uint32_t bufint = VRSettings()->GetInt32(settingKey, "test_int");
		float buffloat = VRSettings()->GetFloat(settingKey, "test_float");
		VRSettings()->GetString(settingKey, "test_string", bufchar, sizeof(bufchar));
		DriverLog("test_bool : %s\n", bufbool ? "true" : "false");
		DriverLog("test_int : %d\n", bufint);
		DriverLog("test_float : %f\n", buffloat);
		DriverLog("test_string : %s\n", bufchar);
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
		VRProperties()->SetFloatProperty(m_ulPropertyContainer, Prop_DeviceBatteryPercentage_Float, 0.5);//1.0�Ŗ��^��

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
		pose.poseIsValid = true;
		pose.deviceIsConnected = true;


		TrackedDevicePose_t poses[k_unMaxTrackedDeviceCount];
		VRServerDriverHost()->GetRawTrackedDevicePoses(0, poses, k_unMaxTrackedDeviceCount);//���ׂẴf�o�C�X�̃|�[�Y�̔z��𓾂�
		if (m_bDispMatrix) {
			m_bDispMatrix = false;
			for (auto i = 0; i < k_unMaxTrackedDeviceCount; i++) {
				if (!poses[i].bDeviceIsConnected) {
					break;
				}
				PropertyContainerHandle_t handle = VRPropertiesRaw()->TrackedDeviceToPropertyContainer(i);//index����v���p�e�B�n���h��
				std::string sn = VRProperties()->GetStringProperty(handle, Prop_SerialNumber_String);//�v���p�e�B�n���h������V���A���i���o�[
				int role = VRProperties()->GetInt32Property(handle, Prop_ControllerRoleHint_Int32);//�v���p�e�B�n���h������R���g���[���[�̖���
				if (role == m_nRefControllerRole) {
					m_nRefControllerIndex = i;
				}
				DriverLog("--%d---------------------------\n", i);
				DriverLog("seral = %s\n", sn.c_str());
				DriverLog("role = %d\n", role);
				HmdMatrix34_t const * const mat = &(poses[i].mDeviceToAbsoluteTracking);
				DriverLog("%+6.3f | %+6.3f | %+6.3f | %+6.3f |\n", mat->m[0][0], mat->m[0][1], mat->m[0][2], mat->m[0][3]);
				DriverLog("%+6.3f | %+6.3f | %+6.3f | %+6.3f |\n", mat->m[1][0], mat->m[1][1], mat->m[1][2], mat->m[1][3]);
				DriverLog("%+6.3f | %+6.3f | %+6.3f | %+6.3f |\n", mat->m[2][0], mat->m[2][1], mat->m[2][2], mat->m[2][3]);
			}
			DriverLog("-----------------------------\n");
		}

		double theta = 2.0 / s_nLoopFrame * 3.14159 *(double)m_nFrametime;

		//���[���h���f�o�C�X�ϊ���`
		//(��]�s�񂩂�N�H�[�^�j�I���𓾂�͕̂��G�Ȃ��߈ʒu�̂ݒǏ])
		HmdMatrix34_t *mat = &(poses[m_nRefControllerIndex].mDeviceToAbsoluteTracking);
		SetVec3(pose.vecWorldFromDriverTranslation, mat->m[0][3], mat->m[1][3], mat->m[2][3]);
		//SetVec3(pose.vecWorldFromDriverTranslation, 0, 0, 0);//�ړ��Ȃ�
		//SetQuaternion(&pose.qWorldFromDriverRotation, cos(theta / 2.0), 0, 0, sin(theta / 2.0));//z���Œ葬��]
		SetQuaternion(&pose.qWorldFromDriverRotation, 1, 0, 0, 0);//��]�Ȃ�
		//SetQuaternion(&pose.qWorldFromDriverRotation, 0.92388, 0, 0, 0.382683);//z��45deg��]
		//�f�o�C�X��HMD�ϊ���`
		SetVec3(pose.vecDriverFromHeadTranslation, 0, 0, 0);//�ړ��Ȃ�
		//SetQuaternion(&pose.qDriverFromHeadRotation, cos(theta / 2.0), 0, 0, sin(theta / 2.0));//z���Œ葬��]
		SetQuaternion(&pose.qDriverFromHeadRotation, 1, 0, 0, 0);//��]�Ȃ�
		//SetQuaternion(&pose.qDriverFromHeadRotation, 0.92388, 0, 0, 0.382683);//z��45deg��]
		//�f�o�C�X�p����ݒ�
		//SetVec3(pose.vecPosition, 0, 0, 0);//�ړ�
		SetVec3(pose.vecPosition, 0, 0.1*sin(theta), 0);//y����sin�ŗh�炷
		SetQuaternion(&pose.qRotation, cos(theta / 2.0), 0, 0, sin(theta / 2.0));//z���Œ葬��]
		//SetQuaternion(&pose.qRotation, 1, 0, 0, 0);//��]�Ȃ�
		//SetQuaternion(&pose.qRotation, 0.92388, 0, 0, 0.382683);//z��45deg��]
		return pose;
	}


	void RunFrame()
	{
		m_nFrametime++;
		if (m_nFrametime >= s_nLoopFrame) {
			m_nFrametime = 0;
			m_bDispMatrix = true;//1����������u�Ԃ̃t���[�������s��\��
		}
		if (m_unDeviceIndex != k_unTrackedDeviceIndexInvalid)//�f�o�C�X��Activate����Ă���Ȃ�
		{
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

public:
	EVRInitError Init(IVRDriverContext *pDriverContext) override//������
	{
		VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);
		InitDriverLog(VRDriverLog());

		char serialNumber[64];
		for (auto i = 0; i < s_TrackerNum; i++) {
			switch (i) {
			case 0:
				m_pTracker[i] = new CTestTrackerExtProc(ETrackedControllerRole::TrackedControllerRole_LeftHand);//����ɒǏ]����g���b�J�[
				break;
			case 1:
				m_pTracker[i] = new CTestTrackerExtProc(ETrackedControllerRole::TrackedControllerRole_RightHand);//�E���ɒǏ]����g���b�J�[
				break;
			default:
				m_pTracker[i] = new CTestTrackerExtProc(ETrackedControllerRole::TrackedControllerRole_Invalid);
				break;
			}
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
