
#include "Strawberry/Core/UTF.hpp"
#include "Strawberry/Graphics/Instance.hpp"
#include "Strawberry/Graphics/Window.hpp"
#include "Strawberry/Graphics/Device.hpp"
#include "Strawberry/Graphics/Pipeline.hpp"
#include "Strawberry/Graphics/Swapchain.hpp"
#include "Strawberry/Graphics/Queue.hpp"


int main()
{
	using namespace Strawberry;

	Graphics::Window::Window window("StrawberryGraphics Test", Core::Math::Vec2i(1920, 1080));
	Graphics::Instance instance;
	Graphics::Surface surface = window.Create<Graphics::Surface, const Graphics::Instance&>(instance);
	Graphics::Device device = instance.Create<Graphics::Device>();
	Graphics::Pipeline pipeline = device.Create<Graphics::Pipeline::Builder>().Build();
	Graphics::Swapchain swapchain = device.Create<Graphics::Swapchain, const Graphics::Surface&>(surface, Core::Math::Vec2i{1920, 1080});
	Graphics::Queue queue = device.Create<Graphics::Queue>();

	while (!window.CloseRequested())
	{
		Graphics::Window::PollInput();

		while (auto event = window.NextEvent())
		{
			if (auto text = event->Value<Graphics::Window::Events::Text>())
			{
				std::u8string c = Core::ToUTF8(text->codepoint).Unwrap();
				std::cout << (const char*) c.data() << std::endl;
			}
		}
	}

	return 0;
}
