#include "pch.hpp"
#include "VulkanSwapChain.hpp"
#include "VulkanDevice.hpp"
#include "VulkanContext.hpp"
#include "Core/Engine.hpp"

#include <GLFW/glfw3.h>

namespace Sphynx::Rendering {
	VulkanSwapChain::VulkanSwapChain() {
		Create();
	}

	VulkanSwapChain::~VulkanSwapChain() {
		for (vk::ImageView imageView : m_ImageViews)
			VulkanContext::LogicalDevice.destroyImageView(imageView, nullptr);
		for (vk::Framebuffer framebuffer : m_Framebuffers)
			VulkanContext::LogicalDevice.destroyFramebuffer(framebuffer, nullptr);
		VulkanContext::LogicalDevice.destroySwapchainKHR(m_SwapChain, nullptr);
	}

	void VulkanSwapChain::Recreate(vk::RenderPass renderpass) {
		VulkanContext::Window->SetResizeCallbackEnable(false);

		VulkanContext::LogicalDevice.waitIdle();

		Create();
		CreateFramebuffers(renderpass);

		VulkanContext::Window->SetResizeCallbackEnable(true);
	}

	void VulkanSwapChain::CreateFramebuffers(vk::RenderPass renderpass) {
		m_Framebuffers.resize(m_ImageViews.size());
		for (size_t i = 0; i < m_ImageViews.size(); i++) {
			std::array<vk::ImageView, 1> attachments = {
				m_ImageViews[i]
			};

			vk::FramebufferCreateInfo framebufferInfo;
			framebufferInfo.renderPass = renderpass;
			framebufferInfo.attachmentCount = (uint32)attachments.size();
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = m_Extent.width;
			framebufferInfo.height = m_Extent.height;
			framebufferInfo.layers = 1;

			vk::Result result = VulkanContext::LogicalDevice.createFramebuffer(&framebufferInfo, nullptr, &m_Framebuffers[i], vk::detail::getDispatchLoaderStatic());
			SE_ASSERT(result == vk::Result::eSuccess, Logging::Rendering, "Failed to create framebuffer");
		}
	}

	VulkanSwapChain::SupportDetails VulkanSwapChain::GetSupport(vk::PhysicalDevice device) {
		SupportDetails details;

		vk::Result result = device.getSurfaceCapabilitiesKHR(VulkanContext::Surface, &details.Capabilities);
		SE_ASSERT(result == vk::Result::eSuccess, "Failed to get the surface capabilities");

		uint32 formatCount = 0;
		result = device.getSurfaceFormatsKHR(VulkanContext::Surface, &formatCount, nullptr);
		SE_ASSERT(result == vk::Result::eSuccess, "Failed to get surface format count");

		if (formatCount != 0) {
			details.Formats.resize(formatCount);
			result = device.getSurfaceFormatsKHR(VulkanContext::Surface, &formatCount, details.Formats.data());
			SE_ASSERT(result == vk::Result::eSuccess, "Failed to get surface formats");
		}

		uint32 presentModeCount = 0;
		result = device.getSurfacePresentModesKHR(VulkanContext::Surface, &presentModeCount, nullptr);
		SE_ASSERT(result == vk::Result::eSuccess, "Failed to get surface present mode count");

		if (presentModeCount != 0) {
			details.PresentModes.resize(presentModeCount);
			result = device.getSurfacePresentModesKHR(VulkanContext::Surface, &presentModeCount, details.PresentModes.data());
			SE_ASSERT(result == vk::Result::eSuccess, "Failed to get surface present modes");
		}

		return details;
	}

	vk::Framebuffer VulkanSwapChain::GetFramebuffer(uint32 index) {
		if (index >= m_Framebuffers.size())
			return VK_NULL_HANDLE;
		return m_Framebuffers[index];
	}

	void VulkanSwapChain::Create() {
		SupportDetails swapChainSupport = GetSupport(VulkanContext::PhysicalDevice);
		std::optional<vk::SurfaceFormatKHR> surfaceFormat = ChooseFormat(swapChainSupport.Formats);
		SE_ASSERT(surfaceFormat.has_value(), Logging::Rendering, "Failed to get format for swapchain");
		m_Format = surfaceFormat->format;
		m_Extent = ChooseExtent(swapChainSupport.Capabilities, VulkanContext::Window->GetGLFWHandle());

		uint32 imageCount = swapChainSupport.Capabilities.minImageCount + 1;
		if (swapChainSupport.Capabilities.maxImageCount > 0 && imageCount > swapChainSupport.Capabilities.maxImageCount)
			imageCount = swapChainSupport.Capabilities.maxImageCount;

		vk::SwapchainCreateInfoKHR createInfo{};
		createInfo.surface = VulkanContext::Surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = m_Format;
		createInfo.imageColorSpace = surfaceFormat->colorSpace;
		createInfo.imageExtent = m_Extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

		VulkanQueueFamilyIndices indices(VulkanContext::PhysicalDevice);
		std::array<uint32, 2> queueFamilyIndices = { indices.GraphicsFamily.value(), indices.PresentFamily.value() };

		if (indices.GraphicsFamily != indices.PresentFamily) {
			createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			createInfo.queueFamilyIndexCount = (uint32)queueFamilyIndices.size();
			createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
		}
		else
			createInfo.imageSharingMode = vk::SharingMode::eExclusive;

		// non-rotated transform prefered
		if (swapChainSupport.Capabilities.currentTransform & vk::SurfaceTransformFlagBitsKHR::eIdentity)
			createInfo.preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
		else
			createInfo.preTransform = swapChainSupport.Capabilities.currentTransform;

		createInfo.compositeAlpha = ChooseCompositeAlpha(swapChainSupport.Capabilities.supportedCompositeAlpha);
		createInfo.presentMode = ChoosePresentMode(swapChainSupport.PresentModes);
		createInfo.clipped = true;
		vk::SwapchainKHR oldSwapchain = m_SwapChain;
		createInfo.oldSwapchain = oldSwapchain;

		vk::Result result = VulkanContext::LogicalDevice.createSwapchainKHR(&createInfo, nullptr, &m_SwapChain);
		SE_ASSERT(result == vk::Result::eSuccess, Logging::Rendering, "Failed to create swapchain");

		if (oldSwapchain) {
			for (auto oldImageView : m_ImageViews)
				VulkanContext::LogicalDevice.destroyImageView(oldImageView, nullptr);
			for (auto oldFramebuffer : m_Framebuffers)
				VulkanContext::LogicalDevice.destroyFramebuffer(oldFramebuffer, nullptr);
			m_ImageViews.clear();
			m_Framebuffers.clear();
			VulkanContext::LogicalDevice.destroySwapchainKHR(oldSwapchain, nullptr);
		}

		// get images
		result = VulkanContext::LogicalDevice.getSwapchainImagesKHR(m_SwapChain, &imageCount, nullptr);
		SE_ASSERT(result == vk::Result::eSuccess, "Failed to get swapchain imageCount");
		m_Images.resize(imageCount);
		result = VulkanContext::LogicalDevice.getSwapchainImagesKHR(m_SwapChain, &imageCount, m_Images.data());
		SE_ASSERT(result == vk::Result::eSuccess, "Failed to get swapchain images");

		// create image views
		vk::ImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
		imageViewCreateInfo.format = m_Format;
		imageViewCreateInfo.components = {
			vk::ComponentSwizzle::eR,
			vk::ComponentSwizzle::eG,
			vk::ComponentSwizzle::eB,
			vk::ComponentSwizzle::eA
		};
		imageViewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;
		m_ImageViews.resize(m_Images.size());
		for (size_t i = 0; i < m_ImageViews.size(); i++) {
			imageViewCreateInfo.image = m_Images[i];
			vk::Result imageViewResult = VulkanContext::LogicalDevice.createImageView(&imageViewCreateInfo, nullptr, &m_ImageViews[i]);
			SE_ASSERT(imageViewResult == vk::Result::eSuccess, Logging::Rendering, "Failed to create image view for swapchain");
		}
	}

	std::optional<vk::SurfaceFormatKHR> VulkanSwapChain::ChooseFormat(const std::vector<vk::SurfaceFormatKHR>& formats) {
		constexpr auto preferredImageFormats = std::array{
			vk::Format::eB8G8R8A8Unorm,
			vk::Format::eR8G8B8A8Unorm,
			vk::Format::eA8B8G8R8UnormPack32
		};
		for (const auto& format : formats) {
			if (std::find(preferredImageFormats.begin(), preferredImageFormats.end(), format.format) != preferredImageFormats.end())
				return format;
		}

		if (!formats.empty())
			return formats[0];

		return std::nullopt;
	}

	vk::PresentModeKHR VulkanSwapChain::ChoosePresentMode(const std::vector<vk::PresentModeKHR>& presentModes) {
		vk::PresentModeKHR bestMode = vk::PresentModeKHR::eFifo;

		// Search for presentMode that isn't vsync
		for (const auto& presentMode : presentModes) {
			if (presentMode == vk::PresentModeKHR::eMailbox) {
				bestMode = presentMode;
				break;
			}

			if (presentMode == vk::PresentModeKHR::eImmediate)
				bestMode = presentMode;
		}

		return bestMode;
	}

	vk::Extent2D VulkanSwapChain::ChooseExtent(const vk::SurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) {
		if (capabilities.currentExtent.width != std::numeric_limits<uint32>::max() &&
			capabilities.currentExtent.height != std::numeric_limits<uint32>::max()) {
			return capabilities.currentExtent;
		}
		else {
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);

			vk::Extent2D actualExtent = {
				(uint32)width,
				(uint32)height
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}

	vk::CompositeAlphaFlagBitsKHR VulkanSwapChain::ChooseCompositeAlpha(vk::CompositeAlphaFlagsKHR supportedFlags) {
		constexpr auto compositeAlphaFlags = std::array {
			vk::CompositeAlphaFlagBitsKHR::eOpaque,
			vk::CompositeAlphaFlagBitsKHR::ePreMultiplied,
			vk::CompositeAlphaFlagBitsKHR::ePostMultiplied,
			vk::CompositeAlphaFlagBitsKHR::eInherit
		};
		for (auto flag : compositeAlphaFlags) {
			if (supportedFlags & flag)
				return flag;
		}
		return vk::CompositeAlphaFlagBitsKHR::eOpaque;
	}
}