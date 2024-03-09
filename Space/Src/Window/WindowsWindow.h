#pragma once

#include "Window.h"

struct GLFWwindow;

namespace Space
{
	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowData &_WinData);
		WindowsWindow() : Window({"Untitled", 1200, 900}) {}
		~WindowsWindow();

		virtual int Init(const WindowData &_WinData) override;
		virtual int ShutDown() override;
		// Swaps the Buffers for clean visual
		virtual void Update() override;
		virtual void SetVSync(bool Value) override;
		// Returns if the window should close or not from WindowCloseCallback
		virtual bool ShouldClose() override;
		virtual void *GetWindow() override { return (void *)_Window; }
		const Vec2& GetFrameBufferSize() const override;

	private:
		GLFWwindow *_Window;
	};
}
