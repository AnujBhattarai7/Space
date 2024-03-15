#include "Space.h"

#include <vulkan/vulkan_core.h>

#include <cstring>

#include <vector>
#include <optional>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define SP_DEAFULT_VULKAN_API_VERSION 0

void OnEvent(const Space::Event &_E);

const uint32_t WIDTH = 1200;
const uint32_t HEIGHT = 900;

namespace Space
{
    WindowStack *WindowStack::_Stack = new WindowStack();
    Input *Input::_I = new Input();
    Renderer *Renderer::_Renderer = new Renderer();

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
            Renderer::Init(VulkanApiVersion::VULKAN_1_0);
        }

        void Update()
        {
            Window &_Window = WindowStack::GetActiveWindow();

            Vec2 Size = {1200, 900};

            while (_Run)
            {
                _Window.Update();

                if (_Render)
                {
                    Renderer::SetupRender();
                    Renderer::SetClearColor({0.5f, 1.0f, 1.0f, 1.0f});
                    Renderer::SetViewPort(Size);

                    if (Input::IsKeyPressed(SP_KEY_R))
                        Size.x -= 1.0f;

                    Renderer::Render();
                }
            }
        }

        void ShutDown()
        {
            SP_CORE_PRINT("SHUTDOWN: \n")

            // CleanUp();
            Renderer::ShutDown();
            WindowStack::ShutDown();

            std::cout << "\n";
        }

        void _OnEvent(const Event &_E)
        {
            if (_E.GetType() == EventType::WINDOW_CLOSE)
                _Run = false;

            if(_E.GetType() == EventType::WINDOW_RESIZE)
            {
                _Render = true;
                auto Size = Vec2(WindowStack::GetRenderWindow().GetWidth(), WindowStack::GetRenderWindow().GetHeight());
                if(Size.x == 0 || Size.y == 0)
                    _Render = false;
            }
        }

    private:
        bool _Run = true;
        bool _Render = true;
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
