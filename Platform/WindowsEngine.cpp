#include "../Utils/vendor/stdafx.h"
#include "../resource.h"
#include "IO/GameInputs.h"
#include "WindowsEngine.h"

#include "imgui.h"
#include <memory>
#include <iostream>



std::unique_ptr<Keyboard> wKbd = std::make_unique<Keyboard>();
std::unique_ptr<Mouse> wMouse = std::make_unique<Mouse>();


bool WindowsEngine::imguiInit = false;
HINSTANCE WindowsEngine::hAppInstance;	// handle Windows de l'instance actuelle de l'application


void WindowsEngine::SetWindowsAppInstance(HINSTANCE hInstance)
{
	hAppInstance = hInstance; // Stocke le handle d'instance de l'application, plusieurs fonctions spécifiques en auront besoin
}


bool WindowsEngine::InitAppInstance()
{
	TCHAR szTitle[MAX_LOADSTRING];					// Le texte de la barre de titre

	// Initialise les chaînes globales
	LoadString(hAppInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hAppInstance, IDC_PETITMOTEUR3D, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hAppInstance);

	hMainWnd = CreateWindow(
		szWindowClass, 
		szTitle, 
		WS_OVERLAPPEDWINDOW & WS_OVERLAPPEDWINDOW &  ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hAppInstance, NULL);

	if (!hMainWnd)
	{
		return false;
	}
	hAccelTable = LoadAccelerators(hAppInstance, MAKEINTRESOURCE(IDC_PETITMOTEUR3D));
	SetWindowPos(hMainWnd, 0, 0, 0, 1280, 720, SWP_SHOWWINDOW | SWP_NOMOVE);
	RAWINPUTDEVICE rid{};
	rid.usUsagePage = 0x01; //mouse page
	rid.usUsage = 0x02; //mouse usage
	rid.hwndTarget = nullptr; //mouse usage
	rid.dwFlags = 0; //mouse usage

	RegisterRawInputDevices(&rid, 1, sizeof(rid));

	return true;
}

d3d11_graphics* WindowsEngine::createRenderingDevicePlatform(GRAPHICS_MODE cdsMode) {
	m_graphics = std::make_unique<d3d11_graphics>(hMainWnd, cdsMode);
	return m_graphics.get();
}

Timer::value_type WindowsEngine::getTimePlatform() const
{
	return static_cast<Timer::value_type>(m_clock.getTimeCount());
}
Timer::value_type WindowsEngine::getTimeIntervalInSeconds(
	Timer::count_type start,
	Timer::count_type stop) const
{
	return m_clock.getTimeBetweenCounts(start, stop);
}


ATOM WindowsEngine::MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex{};

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW ;
	wcex.lpfnWndProc = &WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszClassName = szWindowClass;

	return RegisterClassEx(&wcex);
}

int WindowsEngine::Show()
{
	ShowWindow(hMainWnd, SW_SHOWNORMAL );
	UpdateWindow(hMainWnd);

	return 0;
}


bool WindowsEngine::runPlatform()
{
	MSG msg;
	bool bBoucle = true;

	// Y-a-t'il un message Windows à traiter?
	while (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		// Est-ce un message de fermeture ?
		if (msg.message == WM_QUIT || msg.message == WM_DESTROY)
		{
			bBoucle = false;
		}

		// distribuer le message
		if (!::TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}
	
	
	
	return bBoucle;
}

int WindowsEngine::initPlatform()
{

	InitAppInstance();
	Show();
	if (GetKeyboardLayout(0) == (HKL)0x40C040C)
		GameInputs::Init("res/json/TouchesAZERTY.json");
	else
		GameInputs::Init("res/json/TouchesQWERTY.json");

	return 0;
}


LRESULT CALLBACK WindowsEngine::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
		return true;

	if (imguiInit && (ImGui::GetIO().WantCaptureKeyboard))
		return true;


	int wmId = 0, wmEvent = 0;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	
	/**********************************************************************/
	case WM_KEYUP:
		wKbd->onKeyReleased(static_cast<unsigned char>(wParam));
		break;
	case WM_CHAR:
		wKbd->onChar(static_cast<char>(wParam));
		break;
	case WM_KEYDOWN:
		if (!(lParam & 0x40000000) || wKbd->autorepeatIsEnable()) {
 			wKbd->onKeyPressed(static_cast<unsigned char>(wParam));
		}
		break;
	case WM_KILLFOCUS:
		wKbd->clearStates();
		wMouse->flush();
		wMouse->freeCursor();
		wMouse->enableCursor();
		break;

	/**********************************************************************/
	// Raw mouse input
	case WM_INPUT:
	{
		static std::vector<char> rawbuffer;

		UINT size;
		// fill size
		GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER));
		// get data
		rawbuffer.resize(size);
		GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, rawbuffer.data(), &size, sizeof(RAWINPUTHEADER));

		auto& ri = reinterpret_cast<const RAWINPUT&>(*rawbuffer.data());
		if ((ri.header.dwType == RIM_TYPEMOUSE) &&
			(ri.data.mouse.lLastX != 0 || ri.data.mouse.lLastY != 0)
			)
			wMouse->onRawDelta(ri.data.mouse.lLastX, ri.data.mouse.lLastY);

		break;
	}



	/**********************************************************************/
	case WM_MOUSEMOVE:
	{
		const POINTS pt = MAKEPOINTS(lParam);
		wMouse->onMouseMoved(pt.x, pt.y);
		break; 
	}

	case WM_LBUTTONDOWN:
	{
		const POINTS pt = MAKEPOINTS(lParam);
		wMouse->onLeftPress();
		break;
	}
	case WM_LBUTTONUP:
		{
		const POINTS pt = MAKEPOINTS(lParam);
		wMouse->onLeftRelease(pt.x, pt.y);
		break;
	}
	case WM_RBUTTONDOWN:
	{
		const POINTS pt = MAKEPOINTS(lParam);
		wMouse->onRightPress();
		break;
	}
	case WM_RBUTTONUP:
		{
		const POINTS pt = MAKEPOINTS(lParam);
		wMouse->onRightRelease(pt.x, pt.y);
		break; 
	}
	case WM_MOUSEWHEEL:
		{
		const POINTS pt = MAKEPOINTS(lParam);
		if (GET_WHEEL_DELTA_WPARAM(wParam) > 0) wMouse->onWheelUp(pt.x, pt.y);
		else									wMouse->onWheelDown(pt.x, pt.y);
		break; 
	}

	/**********************************************************************/

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// Évitez d'ajouter du code ici...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;


	case WM_SIZE:
		{
			UINT width = LOWORD(lParam);
			UINT height = HIWORD(lParam);

			std::cout << "Resize to : " << width << "," << height  ;
			static bool firstCall = true;

			if (firstCall) {
				firstCall = false;
				break;
			}
			auto& gfx = WindowsEngine::getInstance().getGraphics();
			gfx.setWinSize(width, height);
		break;
		}

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

INT_PTR CALLBACK WindowsEngine::About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
