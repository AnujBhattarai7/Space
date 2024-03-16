#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include "Space.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define SP_DEAFULT_VULKAN_API_VERSION 1

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

            const std::vector<Vertex> vertices = {
                {{-0.5f, -0.5f, 0.0f}, {0.5f, 0.5f, 0.0f}},
                {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
                {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
                {{-0.5f, -0.5f, 0.0f}, {0.5f, 0.5f, 0.0f}},
                {{-0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
                {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
            };

            _VB.Init((Vertex *)vertices.data(), vertices.size());
        }

        void Update()
        {
            Window &_Window = WindowStack::GetActiveWindow();

            double previousTime = glfwGetTime();

            Vec2 Size = {1200, 900};

            while (_Run)
            {
                float _Dur;
                double currentTime = glfwGetTime();
                _FPS++;

                _Window.Update();

                if (_Render)
                {
                    Renderer::SetupRender();
                    Renderer::SetClearColor({0.5f, 1.0f, 1.0f, 1.0f});
                    Renderer::SetViewPort(Size);
                    
                    Renderer::Submit(_VB);

                    if (Input::IsKeyPressed(SP_KEY_R))
                        Size.x -= 1.0f;

                    Renderer::Render();

                    // Every one Second Print the FPS
                    if (currentTime - previousTime >= 1.0f)
                    {
                        SP_CORE_PRINT("FPS: " << _FPS)
                        previousTime = currentTime;
                        _FPS = 0;
                    }
                }
            }
            Renderer::Stop();
        }

        void ShutDown()
        {
            SP_CORE_PRINT("SHUTDOWN: \n")

            _VB.Destroy();
            // CleanUp();
            Renderer::ShutDown();
            WindowStack::ShutDown();

            std::cout << "\n";
        }

        void _OnEvent(const Event &_E)
        {
            if (_E.GetType() == EventType::WINDOW_CLOSE)
                _Run = false;

            if (_E.GetType() == EventType::WINDOW_RESIZE)
            {
                _Render = true;
                auto Size = Vec2(WindowStack::GetRenderWindow().GetWidth(), WindowStack::GetRenderWindow().GetHeight());
                Renderer::OnWindowResized();
                if (Size.x == 0 || Size.y == 0)
                    _Render = false;
            }
        }

    private:
        bool _Run = true;
        bool _Render = true;
        int _FPS = 0;

        VertexBuffer _VB;
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
