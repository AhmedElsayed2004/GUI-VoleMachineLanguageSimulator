#pragma once
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx10.h"
#include <d3d10_1.h>
#include <d3d10.h>
#include <tchar.h>

class UI {
public:
    UI(HWND hWnd);
    ~UI();

    void Initialize();
    void Render();
    void HandleMessages(UINT msg, WPARAM wParam, LPARAM lParam);

private:
    ID3D10Device* g_pd3dDevice = nullptr;
    IDXGISwapChain* g_pSwapChain = nullptr;
    UINT g_ResizeWidth = 0, g_ResizeHeight = 0;
    ID3D10RenderTargetView* g_mainRenderTargetView = nullptr;

    void CreateRenderTarget();
    void CleanupRenderTarget();
};