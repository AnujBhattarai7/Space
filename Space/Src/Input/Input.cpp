#include "PCH.h"
#include "Input.h"
#include "GLFW/glfw3.h"
#include "WindowStack.h"

namespace Space
{
	bool Input::IsKeyPressed(int _K)
	{
		GLFWwindow* _W = (GLFWwindow*)WindowStack::GetActiveWindow().GetWindow();
		if (glfwGetKey(_W, _K) == SP_PRESS)
			return true;

		return false;
	}

	bool Input::IsMouseButtonPressed(int _B)
	{
		GLFWwindow* _W = (GLFWwindow*)WindowStack::GetActiveWindow().GetWindow();
		if (glfwGetMouseButton(_W, _B) == SP_PRESS)
			return true;

		return false;
	}
}
