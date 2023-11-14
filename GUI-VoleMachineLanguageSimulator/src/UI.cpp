#include "UI.h"

void UI::Run(const Byte mainMemory[], const Byte CPURegister[])
{
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	MSG msg;
	while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
		if (msg.message == WM_QUIT);

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



	ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
	DrawMemory(mainMemory);

	if (ImGui::Button("Load a new program"))
	{
		m_openInstructionWindow = true;
		for (int i = 0; i < 1000; ++i)
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

void UI::DrawMemory(const Byte mainMemory[])
{
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
}

void UI::SetEvent(Event event)
{
	m_event = event;
}

UI::UI(ID3D10Device* g_pd3dDevice, IDXGISwapChain* g_pSwapChain, UINT g_ResizeWidth, UINT g_ResizeHeight,
	ID3D10RenderTargetView* g_mainRenderTargetView, HWND& hWnd, WNDCLASSEXW& wc)
{
	m_g_pd3dDevice = g_pd3dDevice;
	m_g_pSwapChain = g_pSwapChain;
	m_g_ResizeWidth = g_ResizeWidth;
	m_g_ResizeHeight = g_ResizeHeight;
	m_g_mainRenderTargetView = g_mainRenderTargetView;
	m_hwnd = hWnd;

	// Initialize Direct3D
	if (!CreateDeviceD3D())
	{
		CleanupDeviceD3D();
		::UnregisterClassW(m_wc.lpszClassName, m_wc.hInstance);
	}

	// Show the window
	::ShowWindow(m_hwnd, SW_SHOWDEFAULT);
	::UpdateWindow(m_hwnd);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
	//io.ConfigViewportsNoAutoMerge = true;
	//io.ConfigViewportsNoTaskBarIcon = true;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(m_hwnd);
	ImGui_ImplDX10_Init(m_g_pd3dDevice);


}

UI::~UI()
{
	ImGui_ImplDX10_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanupDeviceD3D();
	::DestroyWindow(m_hwnd);
	::UnregisterClassW(m_wc.lpszClassName, m_wc.hInstance);
}

// Helper functions
bool UI::CreateDeviceD3D()
{
	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = m_hwnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	//createDeviceFlags |= D3D10_CREATE_DEVICE_DEBUG;
	HRESULT res = D3D10CreateDeviceAndSwapChain(nullptr, D3D10_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, D3D10_SDK_VERSION, &sd, &m_g_pSwapChain, &m_g_pd3dDevice);
	if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
		res = D3D10CreateDeviceAndSwapChain(nullptr, D3D10_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, D3D10_SDK_VERSION, &sd, &m_g_pSwapChain, &m_g_pd3dDevice);
	if (res != S_OK)
		return false;

	CreateRenderTarget();
	return true;
}

void UI::CleanupDeviceD3D()
{
	CleanupRenderTarget();
	if (m_g_pSwapChain) { m_g_pSwapChain->Release(); m_g_pSwapChain = nullptr; }
	if (m_g_pd3dDevice) { m_g_pd3dDevice->Release(); m_g_pd3dDevice = nullptr; }
}

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