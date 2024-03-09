#pragma once

#include "RepeatCodes.h"
#include "Maths.h"

namespace Space
{
    class Input
    {
    public:
        // Funcs which check if given conditions are true
        static bool IsKeyPressed(int _K);
        static bool IsMouseButtonPressed(int _B);
        inline static bool GetKeyModState(int ModIndex) { return _I->_KeyMods[ModIndex]; }

        // Setters which update the Key and Mouse Pressed through Event Calls
        inline static void SetKeyPressed(int K) { _I->_Key = K; }
        inline static void SetMouseButtonPressed(int B) { _I->_MouseButton = B; }
        inline static void SetKeyMod(int ModIndex, bool _Cond)
        {
            _I->_KeyMods[ModIndex] = _Cond;
            
            if (_Cond == true)
                _I->_LatestKeyMod = (KeyMods)ModIndex;
            else
                _I->_LatestKeyMod = KeyMods::None;
        }

        inline static KeyMods GetLastPressedMod() { return _I->_LatestKeyMod; }

        // Updating Mouse Data
        inline static void UpdateMouseScroll(const Vec2 &Offset) { _I->_ScrollOffset = Offset; }
        inline static void UpdateCursorPos(const Vec2 &Pos) { _I->_CursorPos = Pos; }

        // Getters
        static Input *Get() { return _I; }

        inline static const Vec2 &GetMouseScroll() { return _I->_ScrollOffset; }
        inline static const Vec2 &GetCursorPos() { return _I->_CursorPos; }

        // ShutDown the Input Module
        static void ShutDown()
        {
            SP_CORE_PRINT("[INPUT]: ShutDown!!")
            delete _I;
        }

    private:
        static Input *_I;
        // Currently Pressed Keys & Buttons
        int _Key = -1, _MouseButton = -1;
        // Contains Which keymods are pressed
        bool _KeyMods[6];
        KeyMods _LatestKeyMod;
        // The MouseScrollOffset and current CursorPos
        Vec2 _ScrollOffset, _CursorPos;
    };
}