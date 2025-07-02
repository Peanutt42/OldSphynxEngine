#pragma once

#include "pch.hpp"

#include <vulkan/vulkan.hpp>

namespace Sphynx::Rendering {
	inline static vk::detail::DispatchLoaderDynamic s_DispatchLoader;

	class VulkanInstance {
	public:
		vk::Instance Instance;

		bool Validation = false;
		std::vector<const char*> ValidationLayers;

		VulkanInstance(bool validation);
		~VulkanInstance();

	private:
		VulkanInstance(const VulkanInstance&) = delete;
		VulkanInstance(VulkanInstance&&) = delete;
		VulkanInstance& operator=(const VulkanInstance&) = delete;
		VulkanInstance& operator=(VulkanInstance&&) = delete;

		void _ConfigureExtensions(vk::InstanceCreateInfo& createInfo);

		// Returns if validation layers are usable (local VulkanSDK installation is required for validation layers!)
		bool _ConfigureValidationLayers(vk::InstanceCreateInfo& createInfo);

		void _ConfigureDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo);
		void _CreateDebugMessenger(const vk::DebugUtilsMessengerCreateInfoEXT& createInfo), _DestroyDebugMessenger();

	private:
		vk::DebugUtilsMessengerEXT m_DebugMessenger;
		std::vector<vk::ExtensionProperties> m_SupportedExtensionProperties;
		std::vector<const char*> m_RequiredExtensions;
	};
}