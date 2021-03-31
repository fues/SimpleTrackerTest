#include <stdio.h>
#include <shared_device.h>

//�g���b�J�[�P��
//�p���̃|�C���^�Ɖ�ʕ\�������������
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
		sprintf_s(m_Texts[0], 64, "����Ώ� = %d", m_nTrackerIndex);
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
static const int s_TrackerNum = 2;//�g���b�J�[�̐�
CTracker *g_Trackers[s_TrackerNum];
int g_nNowIndex;//���\�����Ă���f�o�C�X

//�L�[�{�[�h���͌��o
int Inputkey(WPARAM wParam) {
	switch (wParam) {
	case 'W'://�O
		g_Trackers[g_nNowIndex]->GetTrackerPtr()->vPosition.v[2] -= 0.01;
		break;
	case 'S'://��
		g_Trackers[g_nNowIndex]->GetTrackerPtr()->vPosition.v[2] += 0.01;
		break;
	case 'A'://��
		g_Trackers[g_nNowIndex]->GetTrackerPtr()->vPosition.v[0] -= 0.01;
		break;
	case 'D'://�E
		g_Trackers[g_nNowIndex]->GetTrackerPtr()->vPosition.v[0] += 0.01;
		break;
	case 'Q'://��
		g_Trackers[g_nNowIndex]->GetTrackerPtr()->vPosition.v[1] += 0.01;
		break;
	case 'Z'://��
		g_Trackers[g_nNowIndex]->GetTrackerPtr()->vPosition.v[1] -= 0.01;
		break;
	case 'E'://�o�b�e���[����
		g_Trackers[g_nNowIndex]->GetTrackerPtr()->battery -= 0.05;
		break;
	case 'R'://�o�b�e���[�グ
		g_Trackers[g_nNowIndex]->GetTrackerPtr()->battery += 0.05;
		break;
	case 'C'://�f�o�C�X�ڑ��؂�ւ�
		g_Trackers[g_nNowIndex]->GetTrackerPtr()->deviceIsConnected = !g_Trackers[g_nNowIndex]->GetTrackerPtr()->deviceIsConnected;
		break;
	case 'V'://�p���L���؂�ւ�
		g_Trackers[g_nNowIndex]->GetTrackerPtr()->poseIsValid = !g_Trackers[g_nNowIndex]->GetTrackerPtr()->poseIsValid;
		break;
	case VK_SPACE://�\���E�ҏW�Ώې؂�ւ�
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
//�C�x���g�̏���
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	HDC hdc;
	PAINTSTRUCT ps;
	switch (msg)
	{
	case WM_KEYDOWN://�L�[�{�[�h����
		if (!Inputkey(wParam)) {
			InvalidateRect(hWnd, NULL, TRUE);//�ĕ`��
		}
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_PAINT://��ʕ`��
		hdc = BeginPaint(hWnd, &ps);//�`��J�n
		for (int i = 0; i < g_Trackers[g_nNowIndex]->GetTextLines(); i++) {
				TextOut(hdc, 10, 10 + (i * 25), (LPCSTR)g_Trackers[g_nNowIndex]->GetTextPtr(i), lstrlen((LPCSTR)g_Trackers[g_nNowIndex]->GetTextPtr(i)));//�e�L�X�g�\��
		}
		EndPaint(hWnd, &ps);//�`��I��
		return 0;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}
//windows �̃G���g���[�|�C���g
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdSow) {
	TCHAR szAppName[] = TEXT("Shared mem test");
	WNDCLASS wndClass;
	MSG msg;
	HWND hWnd;

	//���L�������쐬
	auto *sharedDevicesMem = new CSharedDevicesMem(s_TrackerNum);
	sharedDevicesMem->Create();

	//�f�o�C�X�̍쐬
	for (auto i = 0; i < s_TrackerNum; i++) {
		g_Trackers[i] = new CTracker(sharedDevicesMem->GetMemPtr() + i, i);
	}
	g_nNowIndex = 0;

	//�����E�B���h�E�̐ݒ�
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

	//���L�������폜
	sharedDevicesMem->Delete();

	return (int)msg.wParam;

}