#pragma once

#include "Core.h"

namespace Space
{
    // All of the Events
    enum class EventType
    {
        // Window Events
        WINDOW_CLOSE = 1,
        WINDOW_RESIZE,
        WINDOW_MOVE, // TO IMPLEMENT
        // Mouse Events
        MOUSEBUTTON_PRESSED,
        MOUSEBUTTON_RELEASED,
        MOUSE_MOVEMENT,
        MOUSE_SCROLL,
        // KeyBoard Events
        KEY_PRESSED,
        KEY_RELEASED
    };

    // Returns the name of the Event For Visual Debuggin
    static const char *GetEventName(EventType _ET)
    {
        switch (_ET)
        {
        case Space::EventType::WINDOW_CLOSE:
            return "WINDOW_CLOSE";
        case Space::EventType::WINDOW_RESIZE:
            return "WINDOW_RESIZE";
        case Space::EventType::WINDOW_MOVE:
            return "WINDOW_MOVE";
        case Space::EventType::MOUSEBUTTON_PRESSED:
            return "MOUSEBUTTON_PRESSED";
        case Space::EventType::MOUSEBUTTON_RELEASED:
            return "MOUSEBUTTON_RELEASED";
        case Space::EventType::MOUSE_MOVEMENT:
            return "MOUSE_MOVEMENT";
        case Space::EventType::KEY_PRESSED:
            return "KEY_PRESSED";
        case Space::EventType::KEY_RELEASED:
            return "KEY_RELEASED";
        case Space::EventType::MOUSE_SCROLL:
            return "MOUSE_SCROLL";
        default:
            return nullptr;
        }
    }

    // The base class of all the events
    // Just Stores the EventType
    class Event
    {
    public:
        Event(EventType _T) { _Type = _T; }

        const EventType GetType() const { return _Type; }

    protected:
        EventType _Type;
    };

    // Mouse Events class Implementations

    class MouseButtonPressed : public Event
    {
    public:
        MouseButtonPressed(int B)
            : Event(EventType::MOUSEBUTTON_PRESSED)
        {
            _B = B;
        }

        const int GetButton() const { return _B; }
        static EventType GetStaticType() { return EventType::MOUSEBUTTON_PRESSED; }

    private:
        int _B = 0;
    };

    class MouseButtonReleased : public Event
    {
    public:
        MouseButtonReleased(int B)
            : Event(EventType::MOUSEBUTTON_RELEASED)
        {
            _B = B;
        }

        static EventType GetStaticType() { return EventType::MOUSEBUTTON_RELEASED; }

        const int GetButton() const { return _B; }

    private:
        int _B = 0;
    };

    class MouseScrolled : public Event
    {
    public:
        MouseScrolled(int XOffset, int YOffset)
            : Event(EventType::MOUSE_SCROLL)
        {
            _XOffset = XOffset;
            _YOffset = YOffset;
        }

        static EventType GetStaticType() { return EventType::MOUSE_SCROLL; }

        const int GetXScrollOffset() const { return _XOffset; }
        const int GetYScrollOffset() const { return _YOffset; }

    private:
        int _XOffset, _YOffset = 0;
    };

    class MouseMovement : public Event
    {
    public:
        MouseMovement(int X, int Y)
            : Event(EventType::MOUSE_MOVEMENT)
        {
            _X = X;
            _Y = Y;
        }

        static EventType GetStaticType() { return EventType::MOUSE_MOVEMENT; }

        const int GetCursorPosX() const { return _X; }
        const int GetCursorPosY() const { return _Y; }

    private:
        int _X, _Y = 0;
    };

    // Key Events class Implementations

    class KeyPressed : public Event
    {
    public:
        KeyPressed(int K)
            : Event(EventType::KEY_PRESSED)
        {
            _K = K;
        }

        const int GetButton() const { return _K; }
        static EventType GetStaticType() { return EventType::KEY_PRESSED; }

    private:
        int _K = 0;
    };

    class KeyReleased : public Event
    {
    public:
        KeyReleased(int K)
            : Event(EventType::KEY_RELEASED)
        {
            _K = K;
        }

        const int GetButton() const { return _K; }
        static EventType GetStaticType() { return EventType::KEY_RELEASED; }

    private:
        int _K = 0;
    };

    // Windows Events class Implementations

    class WindowClose : public Event
    {
    public:
        WindowClose()
            : Event(EventType::WINDOW_CLOSE)
        {
        }

        static EventType GetStaticType() { return EventType::WINDOW_CLOSE; }
    };

    class WindowResize : public Event
    {
    public:
        WindowResize(int W, int H)
            : Event(EventType::WINDOW_RESIZE)
        {
            _W = W;
            _H = H;
        }

        const int GetHeight() const { return _H; }
        const int GetWidth() const { return _W; }
        static EventType GetStaticType() { return EventType::WINDOW_RESIZE; }

    private:
        int _W, _H;
    };
}
