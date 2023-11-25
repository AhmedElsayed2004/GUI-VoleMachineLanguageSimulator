#include "UI.h"

bool UI::Run(const Byte mainMemory[], const Byte CPURegister[], std::string IR, int programCounter, char screen)
{
	
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	MSG msg;
	while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
		if (msg.message == WM_QUIT)
			return false;


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
	DrawMemory(mainMemory, programCounter);

	if (m_event == Event::INVALID_INSTRUCTION)
		ShowMessage("Invalid Instruction.", "The current instruction is invalid");

	if (m_event == Event::PROGRAM_HALTED)
		ShowMessage("Program halted.", "The program has halted");

	if (m_event == Event::INVALID_START_ADDRESS)
		ShowMessage("Invalid start address", "Please enter a valid start address");

	if (m_openInstructionWindow)
		DrawNewProgramWindow();

	if (m_openHelpWindow)
		DrawHelpMenu();

	DrawRegisters(CPURegister);

	DrawOptions();

	DrawScreenWindow(screen);

	DrawPcIR(IR, programCounter);

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

	return true;
}

void UI::DrawMemory(const Byte mainMemory[], int programCounter)
{
	ImVec4 greenColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
	ImVec4 redColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);

	ImGui::SetNextWindowSize({ 800,400 });
	if (ImGui::Begin("Memory", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
	{
		if (ImGui::BeginTable("Memory", 17, ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersH | ImGuiWindowFlags_NoMove))
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
					if (row * 16 + column == programCounter + 1 || row * 16 + column == programCounter + 2)
					{
						if(m_event == Event::NONE)
							ImGui::TextColored(greenColor, "%c %c", mainMemory[row * 16 + column - 1].nibble[0], mainMemory[row * 16 + column - 1].nibble[1]);
						else
							ImGui::TextColored(redColor, "%c %c", mainMemory[row * 16 + column - 1].nibble[0], mainMemory[row * 16 + column - 1].nibble[1]);
					}
					else
					{
						ImGui::Text("%c %c", mainMemory[row * 16 + column - 1].nibble[0], mainMemory[row * 16 + column - 1].nibble[1]);
					}
				}
			}
			ImGui::EndTable();
		}
	}ImGui::End();
}

void UI::ShowMessage(std::string header, std::string message)
{
	const char* cStringHeader = header.c_str();
	const char* cStringMessage = message.c_str();

	ImGui::SetNextWindowSize({ 250,100 });
	if (ImGui::Begin(cStringHeader, NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove))
	{
		ImGui::Text(cStringMessage);

		ImGui::SetCursorPos(ImVec2(200, 60));
		if (ImGui::Button("Close"))
		{
			m_event = Event::NONE;
		}

	}ImGui::End();
}

void UI::DrawRegisters(const Byte CPURegister[])
{
	ImGui::SetNextWindowSize({ 100,350 });
	if (ImGui::Begin("Register", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
	{
		if (ImGui::BeginTable("Register", 2, ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersH | ImGuiWindowFlags_NoMove))
		{

			for (int row = 0; row < 16; row++)
			{
				char val;
				val = row < 10 ? '0' + row : 'A' + (row - 10);

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("R%c", val);
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%c %c", CPURegister[row].nibble[0], CPURegister[row].nibble[1]);
			}
			ImGui::EndTable();
		}
	}ImGui::End();
}

void UI::DrawNewProgramWindow()
{
	ImGui::SetNextWindowSize({ 400,400 });
	if (ImGui::Begin("Input instructions", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking))
	{
		ImGui::Text("Enter instructions");
		ImGui::InputTextMultiline("##instructions", instructions, IM_ARRAYSIZE(instructions), ImVec2(400, 200));

		ImGui::SetCursorPos(ImVec2(350, 350));
		if (ImGui::Button("OK"))
		{
			inputTaken = 1;
			m_openInstructionWindow = false;
		}

		ImGui::SetCursorPos(ImVec2(10, 350));
		ImGui::InputTextMultiline("Enter Start Address (in hexadecimal)", startAddress, IM_ARRAYSIZE(startAddress), ImVec2(50, 25));
	}ImGui::End();

}

void UI::DrawHelpMenu()
{
	bool isClosed = false;
	ImGui::SetNextWindowSize({ 1200,200 });
	if (ImGui::Begin("Help", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking) && !isClosed)
	{
		ImGui::Text("Instructions explanation");
		ImGui::Text("1 RXY  : LOAD the register R with the bit pattern found in the memory cell whose address is XY.");
		ImGui::Text("2 RXY  : LOAD the register R with the bit pattern XY.");
		ImGui::Text("3 RXY  : STORE the bit pattern found in register R in the memory cell whose address is XY.");
		ImGui::Text("4 0RS  : MOVE the bit pattern found in register R to register S. ");
		ImGui::Text("5 RST  : ADD the bit patterns in registers S and T as though they were two's complement representations and leave the result in register R");
		ImGui::Text("6 RST  : ADD the bit patterns in registers S and T as though they represented values in floating-point notation and leave the floating-point result in register R. ");
		ImGui::Text("7 RST  : OR the bit patterns in registers S and T and place the result in register R.");
		ImGui::Text("8 RST  : AND the bit patterns in register S and T and place the result in register R. ");
		ImGui::Text("A R0X  : ROTATE the bit pattern in register R one bit to the right X times. Each time place the bit that started at the low-order end at the high-order end. ");
		ImGui::Text("B RXY  : JUMP to the instruction located in the memory cell at address XY if the bit pattern in register R is equal to the bit pattern in register number 0");
		ImGui::Text("C 000  : HALT execution.");

		if (ImGui::Button("Close"))
		{
			isClosed = true;
		}ImGui::End();

	}ImGui::End();
}

void UI::DrawOptions()
{
	ImGui::SetNextWindowSize({ 400,400 });
	if (ImGui::Begin("Options", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove))
	{
		if (ImGui::Button("Excute instruction"))
		{
			inputTaken = 2;
		}
		if (ImGui::Button("Excute Entire Program"))
		{
			inputTaken = 3;
		}
		if (ImGui::Button("Clear"))
		{
			inputTaken = 4;
		}
		if (ImGui::Button("Load a new program"))
		{
			m_openInstructionWindow = true;
			for (int i = 0; i < 1000; ++i)
			{
				instructions[i] = 0;
			}
			for (int i = 0; i < 3; ++i)
			{
				startAddress[i] = 0;
			}
		}
		if (ImGui::Button("Help"))
		{
			m_openHelpWindow = true;
		}
	}ImGui::End();

}

void UI::DrawPcIR(std::string IR, int programCounter)
{
	if (ImGui::Begin("IR and PC", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove))
	{
		ImGui::Text(("IR : " + IR).c_str());
		if (!m_pcToDecimal)
			ImGui::Text("PC : %x", programCounter);
		else
			ImGui::Text("PC : %d", programCounter);

		if (m_pcToDecimal )
		{
			if(ImGui::Button("Convert PC to hex"))
				m_pcToDecimal = false;
		}
		else
		{
			if(ImGui::Button("Convert PC to decimal"))
				m_pcToDecimal = true;
		}

	}ImGui::End();
}

void UI::DrawScreenWindow(char screen)
{
	if (ImGui::Begin("Screen", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove))
	{
		char scr[] = { screen, '\0'};
		ImGui::Text(scr);
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

	//io.ConfigWindowsMoveFromTitleBarOnly;
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