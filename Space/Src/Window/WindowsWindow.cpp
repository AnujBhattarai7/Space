#include "PCH.h"
#include "WindowsWindow.h"
#include "GLFW/glfw3.h"

#include "KeyCodes.h"
#include "MouseCodes.h"
#include "Input.h"
#include "Window.h"

namespace Space
{
	WindowsWindow::WindowsWindow(const WindowData &_WinData)
		: Window(_WinData)
	{
		Init(_WinData);
	}

	WindowsWindow::~WindowsWindow()
	{
		ShutDown();
	}

	int WindowsWindow::Init(const WindowData &_WinData)
	{
		SP_CORE_PRINT("Window: " << _WinData._Name << " Creation!!")

		if (_Data._Name == "" || _Data._H == 0 || _Data._W == 0)
			_Data = _WinData;

		// Initializes glfw
		if (!_GLFW_INIT)
		{
			if (glfwInit() != int(true))
			{
				SP_CORE_ERROR("Fail to load GLFW!!!...");
				return false;
			}
			_GLFW_INIT = true;
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		// Set Opengl Version
		_Window = glfwCreateWindow(_Data._W, _Data._H, _WinData._Name.c_str(), NULL, NULL);

		if (!_Window)
		{
			SP_CORE_ERROR("Window Failed to Create!!...");
			return false;
		}

		// Preparing _Window for rendering
		// glfwMakeContextCurrent(_Window);
		SetVSync(_Data._VSync);

		glfwSetWindowUserPointer(_Window, &_Data);
		glfwSetCursorPos(_Window, _Data._W / 2, _Data._H / 2);
		glfwSetInputMode(_Window, GLFW_LOCK_KEY_MODS, GLFW_TRUE);

		// Callbacks && Events
		// Callback called when the Window should close
		glfwSetWindowCloseCallback(_Window, [](GLFWwindow *window)
								   {
			WindowData* Data = (WindowData*)glfwGetWindowUserPointer(window);
			Data->_Close = true;

			SP_CORE_PRINT("[EVENT]:[WINDOW_CLOSE]");
			Data->_EventFunction(WindowClose()); });

		glfwSetKeyCallback(_Window, [](GLFWwindow *window, int key, int scancode, int action, int mods)
						   {
			WindowData* Data = (WindowData*)glfwGetWindowUserPointer(window);
			
			// Mods
			static int _PreviousMod = 0;
			int Mod = mods - _PreviousMod;
			_PreviousMod = mods;
			bool _FinalState = true;

			// If the previous Mod is higher then a mod was released thus
			if(Mod < 0){
				_FinalState = false;
				// Make Mod Positive
				Mod = -Mod;
			}

			switch (Mod)
			{
			case SP_MOD_ALT:
				Input::SetKeyMod(int(KeyMods::ALT), _FinalState);
				break;
			case SP_MOD_CAPS_LOCK:
				Input::SetKeyMod(int(KeyMods::CAPS_LOCK), _FinalState);
				break;
			case SP_MOD_CONTROL:
				Input::SetKeyMod(int(KeyMods::CONTROL), _FinalState);
				break;
			case SP_MOD_NUM_LOCK:
				Input::SetKeyMod(int(KeyMods::NUM_LOCK), _FinalState);
				break;
			case SP_MOD_SHIFT:
				Input::SetKeyMod(int(KeyMods::SHIFT), _FinalState);
				break;
			case SP_MOD_SUPER:
				Input::SetKeyMod(int(KeyMods::SUPER), _FinalState);
				break;
			default:
				break;
			}
			
			if (action == SP_PRESS) {
				Data->_EventFunction(KeyPressed(key));
				SP_CORE_PRINT("[KEY_PRESSED]: " << GetModName(Input::GetLastPressedMod()) << GetKeyName(key));
				Input::SetKeyPressed(key);
			}
			else if (action == SP_RELEASE)
			{
				Data->_EventFunction(KeyReleased(key));
				SP_CORE_PRINT("[KEY_RELEASED]: " << GetKeyName(key));
				Input::SetKeyPressed(-1);
			} });

		glfwSetMouseButtonCallback(_Window, [](GLFWwindow *window, int button, int action, int mods)
								   {
			WindowData* Data = (WindowData*)glfwGetWindowUserPointer(window);
			
			if (action == SP_PRESS) {
				Data->_EventFunction(MouseButtonPressed(button));
				SP_CORE_PRINT("[EVENT]:[MOUSE_BUTTON_PRESSED]: " << GetButtonName(button));
				Input::SetMouseButtonPressed(button);
			}
			else if (action == SP_RELEASE) {
				Data->_EventFunction(MouseButtonReleased(button));
				SP_CORE_PRINT("[EVENT]:[MOUSE_BUTTON_RELEASED]: " << GetButtonName(button));
				Input::SetMouseButtonPressed(-1);
			} });

		glfwSetWindowSizeCallback(_Window, [](GLFWwindow *window, int width, int height)
								  {
			WindowData* Data = (WindowData*)glfwGetWindowUserPointer(window);
			Data->_H = height;
			Data->_W = width;

			SP_CORE_PRINT("[EVENT]:[WINDOW_RESIZE]: " << width << " " << height);
			Data->_EventFunction(WindowResize(width, height)); });

		glfwSetScrollCallback(_Window, [](GLFWwindow *window, double xoffset, double yoffset)
							  {
			WindowData* Data = (WindowData*)glfwGetWindowUserPointer(window);
			Input::UpdateMouseScroll(Vec2(xoffset, yoffset));
			SP_CORE_PRINT("[EVENT]:[MOUSE_SCROLL]: " << xoffset << " " << yoffset);

			Data->_EventFunction(MouseScrolled(xoffset, yoffset)); });

		glfwSetFramebufferSizeCallback(_Window, [](GLFWwindow *window, int width, int height)
									   { glViewport(0, 0, width, height); });

		glfwSetCursorPosCallback(_Window, [](GLFWwindow *window, double xpos, double ypos)
								 {
			WindowData* Data = (WindowData*)glfwGetWindowUserPointer(window);
			Input::UpdateCursorPos(Vec2(xpos, ypos));

			Data->_EventFunction(MouseMovement(xpos, ypos)); });

		return true;
	}

	int WindowsWindow::ShutDown()
	{
		// Deletes the Window
		if (_Window == nullptr)
			return true;

		glfwDestroyWindow(_Window);
		_Window = nullptr;

		SP_CORE_PRINT("Window: " << _Data._Name << " ShutDown!!")

		return true;
	}

	void WindowsWindow::Update()
	{
		glfwPollEvents();
		glfwSwapBuffers(_Window);
	}

	void WindowsWindow::SetVSync(bool Value)
	{
		if (_Data._VSync != Value)
		{
			glfwSwapInterval(int(Value));
			_Data._VSync = Value;
		}
	}

	bool WindowsWindow::ShouldClose()
	{
		return _Data._Close;
	}

	inline const Vec2 &WindowsWindow::GetFrameBufferSize() const
	{
		int _W, _H;
		glfwGetFramebufferSize(_Window, &_W, &_H);
		return Vec2(_W, _H);
	}
}
