#pragma once

#include <vector>
#include "Window.h"

namespace Space
{
	class WindowStack
	{
	public:
		static int AddWindows(Window *_W);
		static void PopWindows(int i);

		static Window &GetWindows(int i);

		static int GetWindowIndex(Window *_W);

		// Returns the _Stack Pointer
		static WindowStack *Get();
		// Flushes the Window*'s from _Windows vector
		static void Flush();
		// Deletes The _Stack and Flush
		static void ShutDown();

		static void SetRenderWindowIndex(int i) { _Stack->_RenderIndex = i; }

		// Getters
		static int Size() { return _Stack->_Windows.size(); }
		static Window &GetActiveWindow() { return GetWindows(_Stack->_Active_Window); }
		static Window &GetRenderWindow() { return GetWindows(_Stack->_RenderIndex); }

	private:
		WindowStack() {}

		static WindowStack *_Stack;
		std::vector<Window *> _Windows{};

		// Specifies which index to replace
		int _Replace_Index = -1;
		int _Active_Window = 0;
		int _RenderIndex = 0;
	};
}
