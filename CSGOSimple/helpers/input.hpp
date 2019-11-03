#pragma once

#define NOMINMAX
#include <Windows.h>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <stdexcept>
#include <d3d9.h>

#define VK_CHAR_0 0x30
#define VK_CHAR_1 0x31
#define VK_CHAR_2 0x32
#define VK_CHAR_3 0x33
#define VK_CHAR_4 0x34
#define VK_CHAR_5 0x35
#define VK_CHAR_6 0x36
#define VK_CHAR_7 0x37
#define VK_CHAR_8 0x38
#define VK_CHAR_9 0x39
#define VK_CHAR_A 0x41
#define VK_CHAR_B 0x42
#define VK_CHAR_C 0x43
#define VK_CHAR_D 0x44
#define VK_CHAR_E 0x45
#define VK_CHAR_F 0x46
#define VK_CHAR_G 0x47
#define VK_CHAR_H 0x48
#define VK_CHAR_I 0x49
#define VK_CHAR_G 0x4A
#define VK_CHAR_K 0x4B
#define VK_CHAR_L 0x4C
#define VK_CHAR_M 0x4D
#define VK_CHAR_N 0x4E
#define VK_CHAR_O 0x4F
#define VK_CHAR_P 0x50
#define VK_CHAR_Q 0x51
#define VK_CHAR_R 0x52
#define VK_CHAR_S 0x53
#define VK_CHAR_T 0x54
#define VK_CHAR_U 0x55
#define VK_CHAR_V 0x56
#define VK_CHAR_W 0x57
#define VK_CHAR_X 0x58
#define VK_CHAR_Y 0x59
#define VK_CHAR_Z 0x5A

#include "../singleton.hpp"

enum class KeyState
{
    None = 1,
    Down,
    Up,
    Pressed /*Down and then up*/
};

DEFINE_ENUM_FLAG_OPERATORS(KeyState);

class InputSys 
    : public Singleton<InputSys>
{
    friend class Singleton<InputSys>;

    InputSys();
    ~InputSys();

public:
    void Initialize();

    HWND GetMainWindow() const { return m_hTargetWindow; }

    KeyState      GetKeyState(uint32_t vk);
    bool          IsKeyDown(uint32_t vk);
    bool          WasKeyPressed(uint32_t vk);

    void RegisterHotkey(uint32_t vk, std::function<void(void)> f);
    void RemoveHotkey(uint32_t vk);

private:
    static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    bool ProcessMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    bool ProcessMouseMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    bool ProcessKeybdMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);


    HWND            m_hTargetWindow;
    LONG_PTR        m_ulOldWndProc;
    KeyState       m_iKeyMap[256];

    std::function<void(void)> m_Hotkeys[256];
};