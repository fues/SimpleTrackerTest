#include <stdio.h>
#include <shared_device.h>

//トラッカー単体
//姿勢のポインタと画面表示文字列を持つ
class CTracker {
private:
	static const int s_TrackerTextLength = 64;
	static const int s_TrackerTextLines = 11;
	int m_nTrackerIndex;
	SharedDevice_t *m_pSharedTracker;
	char m_Texts[s_TrackerTextLines][s_TrackerTextLength];
public:
	CTracker(SharedDevice_t *device, int index) {
		m_pSharedTracker = device;
		m_nTrackerIndex = index;
		StateCrean();
		UpdateTexts();
	}
	void StateCrean() {
		m_pSharedTracker->vPosition.v[0] = 0.0;
		m_pSharedTracker->vPosition.v[1] = 1.0;
		m_pSharedTracker->vPosition.v[2] = -0.5;
		m_pSharedTracker->qRotation.w = 1.0;
		m_pSharedTracker->qRotation.x = 0.0;
		m_pSharedTracker->qRotation.y = 0.0;
		m_pSharedTracker->qRotation.z = 0.0;
		m_pSharedTracker->deviceIsConnected = true;
		m_pSharedTracker->poseIsValid = true;
		m_pSharedTracker->battery = 1.0;
	}
	void UpdateTexts() {
		sprintf_s(m_Texts[0], 64, "操作対象 = %d", m_nTrackerIndex);
		sprintf_s(m_Texts[1], 64, "x = %+10.6f", m_pSharedTracker->vPosition.v[0]);
		sprintf_s(m_Texts[2], 64, "y = %+10.6f", m_pSharedTracker->vPosition.v[1]);
		sprintf_s(m_Texts[3], 64, "z = %+10.6f", m_pSharedTracker->vPosition.v[2]);
		sprintf_s(m_Texts[4], 64, "qw = %+10.6f", m_pSharedTracker->qRotation.w);
		sprintf_s(m_Texts[5], 64, "qx = %+10.6f", m_pSharedTracker->qRotation.x);
		sprintf_s(m_Texts[6], 64, "qy = %+10.6f", m_pSharedTracker->qRotation.y);
		sprintf_s(m_Texts[7], 64, "qz = %+10.6f", m_pSharedTracker->qRotation.z);
		sprintf_s(m_Texts[8], 64, "deviceIsConnected = %d", m_pSharedTracker->deviceIsConnected);
		sprintf_s(m_Texts[9], 64, "poseIsValid = %d", m_pSharedTracker->poseIsValid);
		sprintf_s(m_Texts[10], 64, "battery = %f", m_pSharedTracker->battery);
	}
	SharedDevice_t *GetTrackerPtr() {
		return m_pSharedTracker;
	}
	char *GetTextPtr(int index) {
		return (char*)m_Texts[index];
	}
	int GetTextLines() {
		return s_TrackerTextLines;
	}
};
static const int s_TrackerNum = 2;//トラッカーの数
CTracker *g_Trackers[s_TrackerNum];
int g_nNowIndex;//今表示しているデバイス

//キーボード入力検出
int Inputkey(WPARAM wParam) {
	switch (wParam) {
	case 'W'://前
		g_Trackers[g_nNowIndex]->GetTrackerPtr()->vPosition.v[2] -= 0.01;
		break;
	case 'S'://後
		g_Trackers[g_nNowIndex]->GetTrackerPtr()->vPosition.v[2] += 0.01;
		break;
	case 'A'://左
		g_Trackers[g_nNowIndex]->GetTrackerPtr()->vPosition.v[0] -= 0.01;
		break;
	case 'D'://右
		g_Trackers[g_nNowIndex]->GetTrackerPtr()->vPosition.v[0] += 0.01;
		break;
	case 'Q'://上
		g_Trackers[g_nNowIndex]->GetTrackerPtr()->vPosition.v[1] += 0.01;
		break;
	case 'Z'://下
		g_Trackers[g_nNowIndex]->GetTrackerPtr()->vPosition.v[1] -= 0.01;
		break;
	case 'E'://バッテリー下げ
		g_Trackers[g_nNowIndex]->GetTrackerPtr()->battery -= 0.05;
		break;
	case 'R'://バッテリー上げ
		g_Trackers[g_nNowIndex]->GetTrackerPtr()->battery += 0.05;
		break;
	case 'C'://デバイス接続切り替え
		g_Trackers[g_nNowIndex]->GetTrackerPtr()->deviceIsConnected = !g_Trackers[g_nNowIndex]->GetTrackerPtr()->deviceIsConnected;
		break;
	case 'V'://姿勢有効切り替え
		g_Trackers[g_nNowIndex]->GetTrackerPtr()->poseIsValid = !g_Trackers[g_nNowIndex]->GetTrackerPtr()->poseIsValid;
		break;
	case VK_SPACE://表示・編集対象切り替え
		g_nNowIndex++;
		if (g_nNowIndex >= s_TrackerNum) {
			g_nNowIndex = 0;
		}
		break;
	default:
		return 1;
	}
	g_Trackers[g_nNowIndex]->UpdateTexts();
	return 0;
}
//イベントの処理
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	HDC hdc;
	PAINTSTRUCT ps;
	switch (msg)
	{
	case WM_KEYDOWN://キーボード入力
		if (!Inputkey(wParam)) {
			InvalidateRect(hWnd, NULL, TRUE);//再描画
		}
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_PAINT://画面描画
		hdc = BeginPaint(hWnd, &ps);//描画開始
		for (int i = 0; i < g_Trackers[g_nNowIndex]->GetTextLines(); i++) {
				TextOut(hdc, 10, 10 + (i * 25), (LPCSTR)g_Trackers[g_nNowIndex]->GetTextPtr(i), lstrlen((LPCSTR)g_Trackers[g_nNowIndex]->GetTextPtr(i)));//テキスト表示
		}
		EndPaint(hWnd, &ps);//描画終了
		return 0;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}
//windows のエントリーポイント
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdSow) {
	TCHAR szAppName[] = TEXT("Shared mem test");
	WNDCLASS wndClass;
	MSG msg;
	HWND hWnd;

	//共有メモリ作成
	auto *sharedDevicesMem = new CSharedDevicesMem(s_TrackerNum);
	sharedDevicesMem->Create();

	//デバイスの作成
	for (auto i = 0; i < s_TrackerNum; i++) {
		g_Trackers[i] = new CTracker(sharedDevicesMem->GetMemPtr() + i, i);
	}
	g_nNowIndex = 0;

	//動作やウィンドウの設定
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInstance;
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = szAppName;

	if (!RegisterClass(&wndClass)) return 0;
	hWnd = CreateWindow(szAppName, TEXT("Window Name"), WS_OVERLAPPEDWINDOW, 100, 300, 600, 400, NULL, NULL, hInstance, NULL);
	if (!hWnd) return 0;
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	//共有メモリ削除
	sharedDevicesMem->Delete();

	return (int)msg.wParam;

}