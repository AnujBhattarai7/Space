#pragma once

#include <string>

#include "UUID.h"
#include "Events.h"
#include "Maths.h"

namespace Space
{
	static bool _GLFW_INIT = false;
	int _SHUT_GLFW();
	void SetRequiredExtensions(std::vector<const char *> &_Extensions);

	// The Data Struct which stores the Basic Data about the Windows
	struct WindowData
	{
		std::string _Name;
		int _W, _H;
		bool _VSync = true, _Close = false;

		void (*_EventFunction)(const Event &E);

		WindowData(const std::string &Name, int W, int H)
			: _Name(Name), _W(W), _H(H) {}
		WindowData(const WindowData &_Other)
			: _Name(_Other._Name), _W(_Other._W), _H(_Other._H) {}
		WindowData() {}
	};

	class Window
	{
	public:
		virtual ~Window() {}
		Window(const WindowData &_WinData) : _Data(_WinData) {}

		Window(const Window& _Window) = delete;
		Window &operator=(const Window&) = delete;

		// Getters
		inline const std::string &GetName() const { return _Data._Name; }
		inline int GetWidth() const { return _Data._W; }
		inline int GetHeight() const { return _Data._H; }
		inline bool GetVSync() const { return _Data._VSync; }
		inline const UUID &GetID() const { return _ID; }
		virtual const Vec2& GetFrameBufferSize() const = 0;

		// Calls the func when an event is done
		inline void SetEventFunction(void (*func)(const Event &E)) { _Data._EventFunction = func; }
		// If true then sets VSync which is a display feature to lock it to 60 HZ
		virtual void SetVSync(bool Value) = 0;
		// Returns if the _Window should close or not
		virtual bool ShouldClose() = 0;

		virtual int Init(const WindowData &_WinData) = 0;
		virtual int ShutDown() = 0;
		// Updates the _Window
		virtual void Update() = 0;
		// Returns the raw _Window Pointer for Input, ...etc
		virtual void *GetWindow() = 0;

		// Operator ==
		bool operator==(const Window &_Window)
		{
			if (_ID == _Window.GetID())
				return true;
			return false;
		}

	protected:
		WindowData _Data;
		UUID _ID;
	};
}

namespace std
{
	template <typename T>
	struct hash;

	template <>
	struct hash<Space::Window>
	{
		_size_t_ operator()(const Space::Window &Win) const
		{
			return (uint64_t)Win.GetID();
		}
	};

}