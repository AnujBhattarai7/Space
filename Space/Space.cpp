#include "Space.h"

#include <vulkan/vulkan_core.h>

#include <cstring>

#include <vector>
#include <optional>

#define SP_DEAFULT_VULKAN_API_VERSION 0

void OnEvent(const Space::Event &_E);

namespace Space
{
    WindowStack *WindowStack::_Stack = new WindowStack();
    Input *Input::_I = new Input();
    Renderer* Renderer::_Renderer = new Renderer();

    class App
    {
    public:
        App() { Init(); }
        ~App() { ShutDown(); }

        void Init()
        {
            WindowStack::AddWindows(new WindowsWindow({"Space", 1200, 900}));
            WindowStack::GetActiveWindow().SetEventFunction(OnEvent);
            // Set Which RenderApi to use
            Renderer::Init(RenderApiType::VULKAN_1);
        }

        void Update()
        {
            Window &_Window = WindowStack::GetActiveWindow();

            while (_Run)
            {
                _Window.Update();
            }
        }

        void ShutDown()
        {
            SP_CORE_PRINT("SHUTDOWN: \n")

            Renderer::ShutDown();
            WindowStack::ShutDown();
            
            std::cout << "\n";
        }

        void _OnEvent(const Event &_E)
        {
            if (_E.GetType() == EventType::WINDOW_CLOSE)
                _Run = false;
        }

    private:
        bool _Run = true;
    };
}

Space::App *_App;

void OnEvent(const Space::Event &_E)
{
    _App->_OnEvent(_E);
}

int main(int argc, char const *argv[])
{
    _App = new Space::App();
    _App->Update();
    delete _App;

    return 0;
}
