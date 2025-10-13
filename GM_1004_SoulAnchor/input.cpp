
#include "main.h"
#include "input.h"


BYTE Input::m_OldKeyState[256];
BYTE Input::m_KeyState[256];

// コントローラー関連の変数
XINPUT_STATE Input::m_OldControllerState[XUSER_MAX_COUNT];
XINPUT_STATE Input::m_ControllerState[XUSER_MAX_COUNT];
bool Input::m_ControllerConnected[XUSER_MAX_COUNT];


void Input::Init()
{
    // キーボードの初期化
    memset(m_OldKeyState, 0, 256);
    memset(m_KeyState, 0, 256);

    // コントローラーの初期化
    memset(m_OldControllerState, 0, sizeof(m_OldControllerState));
    memset(m_ControllerState, 0, sizeof(m_ControllerState));
    memset(m_ControllerConnected, 0, sizeof(m_ControllerConnected));
}

void Input::Uninit()
{


}

void Input::Update()
{
    // キーボードの更新
    memcpy(m_OldKeyState, m_KeyState, 256);
    GetKeyboardState(m_KeyState);

    // コントローラーの更新（新規追加）
    for (int i = 0; i < XUSER_MAX_COUNT; i++)
    {
        // 前フレームの状態を保存
        m_OldControllerState[i] = m_ControllerState[i];

        // 現在の状態を取得
        DWORD result = XInputGetState(i, &m_ControllerState[i]);
        m_ControllerConnected[i] = (result == ERROR_SUCCESS);
    }
}

bool Input::GetKeyPress(BYTE KeyCode)
{
	return (m_KeyState[KeyCode] & 0x80);
}

bool Input::GetKeyTrigger(BYTE KeyCode)
{
	return ((m_KeyState[KeyCode] & 0x80) && !(m_OldKeyState[KeyCode] & 0x80));
}

bool Input::IsControllerConnected(int controllerIndex)
{
    if (controllerIndex < 0 || controllerIndex >= XUSER_MAX_COUNT)
        return false;

    return m_ControllerConnected[controllerIndex];
}

bool Input::GetControllerButtonPress(int controllerIndex, WORD button)
{
    if (controllerIndex < 0 || controllerIndex >= XUSER_MAX_COUNT)
        return false;

    if (!m_ControllerConnected[controllerIndex])
        return false;

    return (m_ControllerState[controllerIndex].Gamepad.wButtons & button) != 0;
}

bool Input::GetControllerButtonTrigger(int controllerIndex, WORD button)
{
    if (controllerIndex < 0 || controllerIndex >= XUSER_MAX_COUNT)
        return false;

    if (!m_ControllerConnected[controllerIndex])
        return false;

    bool currentPress = (m_ControllerState[controllerIndex].Gamepad.wButtons & button) != 0;
    bool oldPress = (m_OldControllerState[controllerIndex].Gamepad.wButtons & button) != 0;

    return currentPress && !oldPress;
}

float Input::GetControllerAxisValue(int controllerIndex, Controller_Axis axis)
{
    if (controllerIndex < 0 || controllerIndex >= XUSER_MAX_COUNT)
        return 0.0f;

    if (!m_ControllerConnected[controllerIndex])
        return 0.0f;

    const XINPUT_GAMEPAD& gamepad = m_ControllerState[controllerIndex].Gamepad;

    switch (axis)
    {
    case CONTROLLER_AXIS_LX:
        // 左スティック X軸（-1.0f ～ 1.0f）
        return gamepad.sThumbLX / 32767.0f;

    case CONTROLLER_AXIS_LY:
        // 左スティック Y軸（-1.0f ～ 1.0f）
        // Y軸を反転（上が正になるように）
        return gamepad.sThumbLY / 32767.0f;

    case CONTROLLER_AXIS_RX:
        // 右スティック X軸（-1.0f ～ 1.0f）
        return gamepad.sThumbRX / 32767.0f;

    case CONTROLLER_AXIS_RY:
        // 右スティック Y軸（-1.0f ～ 1.0f）
        // Y軸を反転（上が正になるように）
        return -(gamepad.sThumbRY / 32767.0f);

    case CONTROLLER_AXIS_LT:
        // 左トリガー（0.0f ～ 1.0f）
        return gamepad.bLeftTrigger / 255.0f;

    case CONTROLLER_AXIS_RT:
        // 右トリガー（0.0f ～ 1.0f）
        return gamepad.bRightTrigger / 255.0f;

    default:
        return 0.0f;
    }
}

float Input::GetControllerTriggerValue(int controllerIndex, Controller_Axis trigger)
{
    // GetControllerAxisValueと同じ実装
    return GetControllerAxisValue(controllerIndex, trigger);
}