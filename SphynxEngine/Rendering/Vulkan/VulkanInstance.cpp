#include "pch.hpp"
#include "VulkanInstance.hpp"
#include "VulkanContext.hpp"
#include "Core/Engine.hpp"

#include <GLFW/glfw3.h>

namespace Sphynx::Rendering {
	VulkanInstance::VulkanInstance(bool validation)
		: Validation(validation)
	{
		vk::ApplicationInfo appInfo{};
		appInfo.pApplicationName = "Sphynx Engine";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Sphynx Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(Engine::EngineVersion.Major, Engine::EngineVersion.Minor, Engine::EngineVersion.Patch);
		// NOTE: On a old laptop of mine, in order to use renderdoc, you need at least VK_API_VERSION_1_1 or it crashes when enumerating the gpu devices
		appInfo.apiVersion = VK_API_VERSION_1_0;

		vk::InstanceCreateInfo createInfo{};
		createInfo.pApplicationInfo = &appInfo;

		m_RequiredExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		_ConfigureExtensions(createInfo);

		if (Validation)
			ValidationLayers.push_back("VK_LAYER_KHRONOS_validation");
		bool wantedValidation = Validation;
		Validation = _ConfigureValidationLayers(createInfo);
		if (wantedValidation && !Validation)													// validation layers only work on locally installed VulkanSDK!
			SE_WARN(Logging::Rendering, "Couldn't enable vulkan validation layers.\n    -> Fix: Install VulkanSDK locally from https://vulkan.lunarg.com/sdk/home");

		vk::DebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{};
		if (Validation) {
			_ConfigureDebugMessengerCreateInfo(debugMessengerCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugMessengerCreateInfo;
		}
		else
			createInfo.pNext = nullptr;

		vk::Result result = vk::createInstance(&createInfo, nullptr, &Instance);
		SE_ASSERT(result == vk::Result::eSuccess, Logging::Rendering, "Failed to create vulkan instance");

		s_DispatchLoader.init(Instance, vkGetInstanceProcAddr);


		if (Validation)
			_CreateDebugMessenger(debugMessengerCreateInfo);
	}

	VulkanInstance::~VulkanInstance() {
		if (Validation)
			_DestroyDebugMessenger();

		Instance.destroy();
	}


	void VulkanInstance::_ConfigureExtensions(vk::InstanceCreateInfo& createInfo) {
		m_SupportedExtensionProperties = vk::enumerateInstanceExtensionProperties(nullptr);

		SE_ASSERT(glfwVulkanSupported(), Logging::Rendering, "GLFW doesn't support Vulkan on this platform");
		// Add glfw extensions
		uint32 glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		SE_ASSERT(glfwExtensions, Logging::Rendering, "GLFW doesn't support Vulkan surface creation");
		for (uint32 i = 0; i < glfwExtensionCount; i++)
			m_RequiredExtensions.emplace_back(glfwExtensions[i]);

		// Check if extensions are supported
		size_t unsupportedExtensionCount = 0;
		for (const char* extension : m_RequiredExtensions) {
			bool foundExtension = false;
			std::string_view extensionStr(extension);
			for (const vk::ExtensionProperties& extensionProps : m_SupportedExtensionProperties) {
				if (std::string_view(extensionProps.extensionName.data()) == extensionStr) {
					foundExtension = true;
					break;
				}
			}
			if (!foundExtension) {
				SE_ERR(Logging::Rendering, "Unsupported instane extesnion: {}", extension);
				unsupportedExtensionCount++;
			}
		}
		if (unsupportedExtensionCount != 0) {
			SE_FATAL(Logging::Rendering, "{} instance extensions aren't supported! (see logs for more detail)", unsupportedExtensionCount);
		}

		createInfo.enabledExtensionCount = (uint32)m_RequiredExtensions.size();
		createInfo.ppEnabledExtensionNames = m_RequiredExtensions.data();
	}


	bool VulkanInstance::_ConfigureValidationLayers(vk::InstanceCreateInfo& createInfo) {
		std::vector<vk::LayerProperties> supportedLayers = vk::enumerateInstanceLayerProperties();

		size_t unsupportedValidationLayerCount = 0;
		for (const char* layer : ValidationLayers) {
			bool foundLayer = false;
			std::string_view layerStr = layer;
			for (const vk::LayerProperties& supportedLayerProp : supportedLayers) {
				if (std::string_view(supportedLayerProp.layerName) == layerStr) {
					foundLayer = true;
					break;
				}
			}
			if (!foundLayer) {
				SE_WARN(Logging::Rendering, "{} validation layer isn't supported!", layer);
				unsupportedValidationLayerCount++;
			}
		}
		if (unsupportedValidationLayerCount != 0)
			return false;

		if (Validation) {
			createInfo.enabledLayerCount = (uint32)ValidationLayers.size();
			createInfo.ppEnabledLayerNames = ValidationLayers.data();
		}
		return Validation;
	}

	VKAPI_ATTR uint VKAPI_CALL
	debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			vk::DebugUtilsMessageTypeFlagsEXT /*messageType*/,
			const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData,
			void * /*pUserData*/)
	{
		if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning) {
			SE_WARN(Logging::Rendering, "[Validation Layer]: {}",
			pCallbackData->pMessage);
		} else {
			SE_ERR(Logging::Rendering, "[Validation Layer]: {}",
			pCallbackData->pMessage);
		}

		return VK_FALSE;
	}
	void VulkanInstance::_ConfigureDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
									 vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
		createInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
								 vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;
		createInfo.pfnUserCallback = debugCallback;
	}

	void VulkanInstance::_CreateDebugMessenger(const vk::DebugUtilsMessengerCreateInfoEXT& createInfo) {
		vk::Result result = Instance.createDebugUtilsMessengerEXT(&createInfo, nullptr, &m_DebugMessenger, s_DispatchLoader);
		SE_ASSERT(result == vk::Result::eSuccess, Logging::Rendering, "Failed to create debug messenger");
	}

	void VulkanInstance::_DestroyDebugMessenger() {
		Instance.destroyDebugUtilsMessengerEXT(m_DebugMessenger, nullptr, s_DispatchLoader);
	}
}