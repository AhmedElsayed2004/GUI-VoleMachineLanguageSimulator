#include "Machine.h"
#include "UI.h"

static ID3D10Device* g_pd3dDevice = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D10RenderTargetView* g_mainRenderTargetView = nullptr;

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


int main()
{
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"Vole Machine Simulator", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Vole Machine Simulator", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

    UI ui(g_pd3dDevice, g_pSwapChain, g_ResizeWidth, g_ResizeHeight, g_mainRenderTargetView, hwnd, wc);

    Machine machine;
    bool isRunning = true;
    // UI RUNNER
    while (isRunning)
    {
        if (ui.inputTaken)
        {
            machine.Run(ui.inputTaken,ui.instructions,ui.startAddress);
            ui.inputTaken = 0;
            ui.SetEvent(machine.GetEvent());
        }   
        isRunning = ui.Run(machine.m_memory, machine.cpu.cpuRegister, machine.cpu.IR, machine.cpu.programCounter, machine.screen);

    }

}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
