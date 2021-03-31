
/****************************************************************************
* 単体で動作するトラッカーのドライバ
* 2つのトラッカーが回転・揺れながら左右のコントローラを追従する
*****************************************************************************/

#include <openvr_driver.h>

//openVRのサンプルドライバからdriverlog.cppも持ってくる
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

// トラッカーの動作定義クラス
class CTestTrackerExtProc : public ITrackedDeviceServerDriver {
private:
	TrackedDeviceIndex_t m_unDeviceIndex;
	PropertyContainerHandle_t m_ulPropertyContainer;
	//毎フレーム加算される。
	int m_nFrametime;
	static const int s_nLoopFrame = 160;//何フレームでループさせるか;
	bool m_bDispMatrix = false;//各デバイスの姿勢行列をログに表示か
	//デバイスの座標系を決めるコントローラー
	ETrackedControllerRole m_nRefControllerRole;
	uint32_t m_nRefControllerIndex;
public:
	CTestTrackerExtProc(ETrackedControllerRole refControllerRole) {
		m_unDeviceIndex = k_unTrackedDeviceIndexInvalid;
		m_ulPropertyContainer = k_ulInvalidPropertyContainer;
		m_nFrametime = 0;
		m_nRefControllerRole = refControllerRole;
		m_nRefControllerIndex = 0;
		//vrsettingsファイルの読み込み 存在しない場所を参照すると0や""となる?
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

		VRProperties()->SetUint64Property(m_ulPropertyContainer, Prop_CurrentUniverseId_Uint64, 2);// 0(無効),1(oculus用)以外の定数
		VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_ModelNumber_String, TRACKER_NAME);
		VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_ControllerType_String, TRACKER_NAME);
		VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_RenderModelName_String, DRIVER_FOLDER"tracker_b");// 3Dモデルの追加
		VRProperties()->SetBoolProperty(m_ulPropertyContainer, Prop_DeviceProvidesBatteryStatus_Bool, true);//バッテリー情報提供
		VRProperties()->SetFloatProperty(m_ulPropertyContainer, Prop_DeviceBatteryPercentage_Float, 0.5);//1.0で満タン

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
		pose.poseIsValid = true;
		pose.deviceIsConnected = true;


		TrackedDevicePose_t poses[k_unMaxTrackedDeviceCount];
		VRServerDriverHost()->GetRawTrackedDevicePoses(0, poses, k_unMaxTrackedDeviceCount);//すべてのデバイスのポーズの配列を得る
		if (m_bDispMatrix) {
			m_bDispMatrix = false;
			for (auto i = 0; i < k_unMaxTrackedDeviceCount; i++) {
				if (!poses[i].bDeviceIsConnected) {
					break;
				}
				PropertyContainerHandle_t handle = VRPropertiesRaw()->TrackedDeviceToPropertyContainer(i);//indexからプロパティハンドル
				std::string sn = VRProperties()->GetStringProperty(handle, Prop_SerialNumber_String);//プロパティハンドルからシリアルナンバー
				int role = VRProperties()->GetInt32Property(handle, Prop_ControllerRoleHint_Int32);//プロパティハンドルからコントローラーの役割
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

		//ワールド→デバイス変換定義
		//(回転行列からクォータニオンを得るのは複雑なため位置のみ追従)
		HmdMatrix34_t *mat = &(poses[m_nRefControllerIndex].mDeviceToAbsoluteTracking);
		SetVec3(pose.vecWorldFromDriverTranslation, mat->m[0][3], mat->m[1][3], mat->m[2][3]);
		//SetVec3(pose.vecWorldFromDriverTranslation, 0, 0, 0);//移動なし
		//SetQuaternion(&pose.qWorldFromDriverRotation, cos(theta / 2.0), 0, 0, sin(theta / 2.0));//z軸で定速回転
		SetQuaternion(&pose.qWorldFromDriverRotation, 1, 0, 0, 0);//回転なし
		//SetQuaternion(&pose.qWorldFromDriverRotation, 0.92388, 0, 0, 0.382683);//z軸45deg回転
		//デバイス→HMD変換定義
		SetVec3(pose.vecDriverFromHeadTranslation, 0, 0, 0);//移動なし
		//SetQuaternion(&pose.qDriverFromHeadRotation, cos(theta / 2.0), 0, 0, sin(theta / 2.0));//z軸で定速回転
		SetQuaternion(&pose.qDriverFromHeadRotation, 1, 0, 0, 0);//回転なし
		//SetQuaternion(&pose.qDriverFromHeadRotation, 0.92388, 0, 0, 0.382683);//z軸45deg回転
		//デバイス姿勢を設定
		//SetVec3(pose.vecPosition, 0, 0, 0);//移動
		SetVec3(pose.vecPosition, 0, 0.1*sin(theta), 0);//y軸をsinで揺らす
		SetQuaternion(&pose.qRotation, cos(theta / 2.0), 0, 0, sin(theta / 2.0));//z軸で定速回転
		//SetQuaternion(&pose.qRotation, 1, 0, 0, 0);//回転なし
		//SetQuaternion(&pose.qRotation, 0.92388, 0, 0, 0.382683);//z軸45deg回転
		return pose;
	}


	void RunFrame()
	{
		m_nFrametime++;
		if (m_nFrametime >= s_nLoopFrame) {
			m_nFrametime = 0;
			m_bDispMatrix = true;//1周期回った瞬間のフレームだけ行列表示
		}
		if (m_unDeviceIndex != k_unTrackedDeviceIndexInvalid)//デバイスがActivateされているなら
		{
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

public:
	EVRInitError Init(IVRDriverContext *pDriverContext) override//初期化
	{
		VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);
		InitDriverLog(VRDriverLog());

		char serialNumber[64];
		for (auto i = 0; i < s_TrackerNum; i++) {
			switch (i) {
			case 0:
				m_pTracker[i] = new CTestTrackerExtProc(ETrackedControllerRole::TrackedControllerRole_LeftHand);//左手に追従するトラッカー
				break;
			case 1:
				m_pTracker[i] = new CTestTrackerExtProc(ETrackedControllerRole::TrackedControllerRole_RightHand);//右手手に追従するトラッカー
				break;
			default:
				m_pTracker[i] = new CTestTrackerExtProc(ETrackedControllerRole::TrackedControllerRole_Invalid);
				break;
			}
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
