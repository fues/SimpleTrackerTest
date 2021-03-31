
/****************************************************************************
* 別プログラムから操作されるトラッカーのドライバ
* 2つのトラッカーの姿勢は共有メモリを介して指定される
*****************************************************************************/

#include <openvr_driver.h>

//openVRのサンプルドライバからdriverlog.cppも持ってくる
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
// トラッカーの動作定義クラス
class CTestTrackerExtProc : public ITrackedDeviceServerDriver {
private:
	TrackedDeviceIndex_t m_unDeviceIndex;
	PropertyContainerHandle_t m_ulPropertyContainer;
	//毎フレーム加算される。
	int m_nFrametime;
	static const int s_nLoopFrame = 160;//何フレームでループさせるか;
	//共有メモリのポインタ
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

		VRProperties()->SetUint64Property(m_ulPropertyContainer, Prop_CurrentUniverseId_Uint64, 2);// 0(無効),1(oculus用)以外の定数
		VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_ModelNumber_String, TRACKER_NAME);
		VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_ControllerType_String, TRACKER_NAME);
		VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_RenderModelName_String, DRIVER_FOLDER"tracker_b");// 3Dモデルの追加
		VRProperties()->SetBoolProperty(m_ulPropertyContainer, Prop_DeviceProvidesBatteryStatus_Bool, true);//バッテリー情報提供
		VRProperties()->SetFloatProperty(m_ulPropertyContainer, Prop_DeviceBatteryPercentage_Float, 1.0);//1.0で満タン

		//アイコンの追加
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
		//クライアントからのデバッグリクエスト?
		//とりあえず 空文字列として返す
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
		//ワールド→デバイス変換定義
		SetVec3(pose.vecWorldFromDriverTranslation, 0, 0, 0);//移動なし
		SetQuaternion(&pose.qWorldFromDriverRotation, 1, 0, 0, 0);//回転なし
		//デバイス→HMD変換定義
		SetVec3(pose.vecDriverFromHeadTranslation, 0, 0, 0);//移動なし
		SetQuaternion(&pose.qDriverFromHeadRotation, 1, 0, 0, 0);//回転なし
		//デバイス姿勢を設定
		//共有メモリから姿勢代入
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
		if (m_unDeviceIndex != k_unTrackedDeviceIndexInvalid)//デバイスがActivateされているなら
		{
			//バッテリー情報更新
			VRProperties()->SetFloatProperty(m_ulPropertyContainer, Prop_DeviceBatteryPercentage_Float, m_pSharedTrackers->battery);//1.0で満タン
			//姿勢更新
			VRServerDriverHost()->TrackedDevicePoseUpdated(m_unDeviceIndex, GetPose(), sizeof(DriverPose_t));
		}
	}

};

// デバイスプロバイダのクラス
class CDeviceProvider : public IServerTrackedDeviceProvider
{
private:
	static const int s_TrackerNum = 2;
	CTestTrackerExtProc *m_pTracker[s_TrackerNum] = { nullptr,nullptr };
	CSharedDevicesMem *m_SharedTrackers;

public:
	EVRInitError Init(IVRDriverContext *pDriverContext) override//初期化
	{
		VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);
		InitDriverLog(VRDriverLog());

		char serialNumber[64];

		//共有メモリの設定
		m_SharedTrackers = new CSharedDevicesMem(s_TrackerNum);
		if (!m_SharedTrackers->Create()) {
			DriverLog("!!!Failed to create shared memory!!!!");
			return VRInitError_Driver_Failed;
		}

		for (auto i = 0; i < s_TrackerNum; i++) {
			m_pTracker[i] = new CTestTrackerExtProc(m_SharedTrackers->GetMemPtr() + i);
			//トラッカーを登録
			sprintf_s(serialNumber, 64, "Test_%d", i);
			VRServerDriverHost()->TrackedDeviceAdded(serialNumber, TrackedDeviceClass_GenericTracker, m_pTracker[i]);//第一引数は自動でProp_SerialNumber_Stringに設定される
		}

		return VRInitError_None;
	}
	void Cleanup() override//開放
	{
		CleanupDriverLog();

		for (auto i = 0; i < s_TrackerNum; i++) {
			delete m_pTracker[i];
			m_pTracker[i] = nullptr;
		}

		//共有メモリの開放
		m_SharedTrackers->Delete();

		VR_CLEANUP_SERVER_DRIVER_CONTEXT()
	}
	void RunFrame() override//毎フレーム呼び出し
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
