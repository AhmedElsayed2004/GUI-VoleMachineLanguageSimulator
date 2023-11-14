#include "UI.h"
void UI::CreateRenderTarget()
{
    ID3D10Texture2D* pBackBuffer;
    m_g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    m_g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_g_mainRenderTargetView);
    pBackBuffer->Release();
}

void UI::CleanupRenderTarget()
{
    if (m_g_mainRenderTargetView) { m_g_mainRenderTargetView->Release(); m_g_mainRenderTargetView = nullptr; }
}

UI::UI(ID3D10Device* g_pd3dDevice, IDXGISwapChain* g_pSwapChain, UINT g_ResizeWidth, UINT g_ResizeHeight, ID3D10RenderTargetView* g_mainRenderTargetView)
{
    m_g_pd3dDevice = g_pd3dDevice;
    m_g_pSwapChain = g_pSwapChain;
    m_g_ResizeWidth = g_ResizeWidth;
    m_g_ResizeHeight = g_ResizeHeight;
    m_g_mainRenderTargetView = g_mainRenderTargetView;
    
}
void UI::Run(Byte mainMemory[], Byte CPURegister[])
{
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    bool done = false;
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }

        // Handle window resize (we don't resize directly in the WM_SIZE handler)
        if (m_g_ResizeWidth != 0 && m_g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            m_g_pSwapChain->ResizeBuffers(0, m_g_ResizeWidth, m_g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            m_g_ResizeWidth = m_g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX10_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

        ImGui::SetNextWindowSize({ 800,400 });
        if (ImGui::Begin("Table", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
        {
            if (ImGui::BeginTable("Memory", 17, ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersH))
            {
                ImGui::TableSetupColumn("");
                for (int i = 0; i < 16; ++i)
                {
                    char val[2];
                    val[0] = i < 10 ? '0' + i : 'A' + (i - 10);
                    val[1] = '\0';
                    ImGui::TableSetupColumn(val);
                }
                ImGui::TableHeadersRow();

                for (int row = 0; row < 16; row++)
                {
                    char val[2];
                    val[0] = row < 10 ? '0' + row : 'A' + (row - 10);
                    val[1] = '\0';

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text(val);
                    for (int column = 1; column < 17; column++)
                    {
                        ImGui::TableSetColumnIndex(column);
                        ImGui::Text("%c %c", mainMemory[row * 16 + column - 1].nibble[0], mainMemory[row * 16 + column - 1].nibble[1]);
                    }
                }
                ImGui::EndTable();
            }
        }ImGui::End();


        if(ImGui::Button("Load a new program"))
        {
            m_openInstructionWindow = true;
            for (int i = 0;i < 1000;++i)
            {
                instructions[i] = 0;
            }
        }
        if (m_openInstructionWindow)
        {
            ImGui::SetNextWindowSize({ 400,400 });
            if (ImGui::Begin("Input instructions"))
            {
                ImGui::Text("Enter instructions");
                ImGui::InputTextMultiline("##instructions", instructions, IM_ARRAYSIZE(instructions), ImVec2(400, 200));
                if (ImGui::Button("OK"))
                {
                    inputTaken = 1;
                    m_openInstructionWindow = false;
                }
            }ImGui::End();
        }

        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        m_g_pd3dDevice->OMSetRenderTargets(1, &m_g_mainRenderTargetView, nullptr);
        m_g_pd3dDevice->ClearRenderTargetView(m_g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX10_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        m_g_pSwapChain->Present(1, 0); // Present with vsync
}