/**
 * @file application.cc
 * @author Mao Zhang (mao.zhang233@gmail.com)
 * @brief
 * @version 1.0
 * @date 2023-02-20
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "application.h"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <stdexcept>
#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

namespace playground {

bool QueueFamilies::IsCompleted() {
  return graphics_family.has_value() && present_family.has_value();
}

bool SwapChainSupportDetails::IsAdequate() {
  return capabilities.minImageCount > 0 && !formats.empty() &&
         !present_modes.empty();
}

Application::Application() {
  CreateWindow();
  CreateInstance();

  if (ENABLE_VALIDATION_LAYER) {
    SetupDebugMessenger();
  }

  CreateSurface();
  PickPhysicalDevice();
  CreateLogicalDevice();
  CreateSwapChain();
  CreateRenderPass();
  CreateGraphicsPipeline();
  CreateFrameBuffers();
}

Application::~Application() {
  for (auto framebuffer : swap_chain_framebuffers_) {
    vkDestroyFramebuffer(device_, framebuffer, nullptr);
  }

  vkDestroyPipeline(device_, graphics_pipeline_, nullptr);
  vkDestroyPipelineLayout(device_, pipeline_layout_, nullptr);
  vkDestroyRenderPass(device_, render_pass_, nullptr);

  for (auto image_view : swap_chain_image_views_) {
    vkDestroyImageView(device_, image_view, nullptr);
  }

  vkDestroySwapchainKHR(device_, swap_chain_, nullptr);
  vkDestroyDevice(device_, nullptr);
  vkDestroySurfaceKHR(instance_, surface_, nullptr);

  if (ENABLE_VALIDATION_LAYER) {
    DestroyDebugUtilsMessengerEXT(instance_, debug_messenger_, nullptr);
  }

  vkDestroyInstance(instance_, nullptr);

  glfwDestroyWindow(window_);
  glfwTerminate();
}

void Application::Run() {
  while (!glfwWindowShouldClose(window_)) {
    glfwPollEvents();
  }
}

void Application::CreateWindow() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  window_ = glfwCreateWindow(WIDTH, HEIGHT, TITLE.c_str(), nullptr, nullptr);

  if (!window_) {
    throw std::runtime_error(
        "----- Error::Window: Failed to create the GLFW window -----");
  }
}

void Application::CreateInstance() {
  VkInstanceCreateInfo instance_info{};
  instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

  VkApplicationInfo app_info{};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "Playground";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "No Engine";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion = VK_API_VERSION_1_0;

  instance_info.pApplicationInfo = &app_info;

#ifdef __APPLE__
  instance_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

  // required extensions
  std::vector<const char*> required_extensions{};
  FindInstanceExtensions(required_extensions);

  instance_info.enabledExtensionCount =
      static_cast<uint32_t>(required_extensions.size());
  instance_info.ppEnabledExtensionNames = required_extensions.data();

  // layers
  std::vector<const char*> required_layers{};
  FindInstanceLayers(required_layers);

  if (ENABLE_VALIDATION_LAYER) {
    instance_info.enabledLayerCount =
        static_cast<uint32_t>(required_layers.size());
    instance_info.ppEnabledLayerNames = required_layers.data();
  } else {
    instance_info.enabledLayerCount = 0;
  }

  if (VK_SUCCESS != vkCreateInstance(&instance_info, nullptr, &instance_)) {
    throw std::runtime_error(
        "----- Error:Vulkan: Failed to create instance -----");
  }
}

void Application::SetupDebugMessenger() {
  VkDebugUtilsMessengerCreateInfoEXT create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  create_info.messageSeverity =
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  create_info.pfnUserCallback = DebugCallBack;
  create_info.pUserData = nullptr;  // optional

  if (CreateDebugUtilsMessengerEXT(instance_, &create_info, nullptr,
                                   &debug_messenger_) != VK_SUCCESS) {
    throw std::runtime_error(
        "----- Error::Device: Failed to set up debug messenger -----");
  }
}

void Application::CreateSurface() {
  glfwCreateWindowSurface(instance_, window_, nullptr, &surface_);
}

void Application::PickPhysicalDevice() {
  uint32_t device_cnt = 0;
  vkEnumeratePhysicalDevices(instance_, &device_cnt, nullptr);
  if (0 == device_cnt) {
    throw std::runtime_error(
        "----- Error::Device: Failed to find GPUs with Vulkan support -----");
  }

  std::vector<VkPhysicalDevice> devices(device_cnt);
  vkEnumeratePhysicalDevices(instance_, &device_cnt, devices.data());

  std::multimap<int, VkPhysicalDevice> candidates{};
  for (const auto& device : devices) {
    int score = EvaluateDevice(device);
    candidates.insert(std::make_pair(score, device));
  }

  if (candidates.rbegin()->first > 0) {
    physical_device_ = candidates.rbegin()->second;
  }

  if (VK_NULL_HANDLE == physical_device_) {
    throw std::runtime_error(
        "----- Error::Device: Failed to find a suitable GPU -----");
  }
}

void Application::CreateLogicalDevice() {
  VkDeviceCreateInfo device_info{};
  device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

  queue_faimlies_ = FindQueueFaimilies(physical_device_);

  std::vector<VkDeviceQueueCreateInfo> queue_infos;
  std::set<uint32_t> unique_queue_families{
      queue_faimlies_.graphics_family.value(),
      queue_faimlies_.present_family.value()};

  float queue_priority = 1.f;
  for (const auto& family : unique_queue_families) {
    VkDeviceQueueCreateInfo queue_info{};
    queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info.queueFamilyIndex = family;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = &queue_priority;

    queue_infos.push_back(queue_info);
  }

  device_info.queueCreateInfoCount = static_cast<uint32_t>(queue_infos.size());
  device_info.pQueueCreateInfos = queue_infos.data();

  // physical device features
  VkPhysicalDeviceFeatures physical_device_features{};

  device_info.pEnabledFeatures = &physical_device_features;

  // required physical device extensions
  std::vector<const char*> required_extensions{};
  FindDeviceExtensions(physical_device_, required_extensions);

  device_info.enabledExtensionCount =
      static_cast<uint32_t>(required_extensions.size());
  device_info.ppEnabledExtensionNames = required_extensions.data();

  // required layers
  std::vector<const char*> required_layers{};
  FindInstanceLayers(required_layers);

  if (ENABLE_VALIDATION_LAYER) {
    device_info.enabledLayerCount =
        static_cast<uint32_t>(required_layers.size());
    device_info.ppEnabledLayerNames = required_layers.data();
  } else {
    device_info.enabledLayerCount = 0;
  }

  if (VK_SUCCESS !=
      vkCreateDevice(physical_device_, &device_info, nullptr, &device_)) {
    throw std::runtime_error("Error: Failed to create logical device");
  }

  // graphics queue
  vkGetDeviceQueue(device_, queue_faimlies_.graphics_family.value(), 0,
                   &graphics_queue_);
  // present queue
  vkGetDeviceQueue(device_, queue_faimlies_.present_family.value(), 0,
                   &present_queue_);
}

void Application::CreateSwapChain() {
  SwapChainSupportDetails swap_chain_support =
      QuerySwapChainSupoort(physical_device_);

  VkSurfaceFormatKHR surface_format =
      ChooseSwapSurfaceFormat(swap_chain_support.formats);
  VkPresentModeKHR present_mode =
      ChooseSwapPresentMode(swap_chain_support.present_modes);
  VkExtent2D extent = ChooseSwapExtent(swap_chain_support.capabilities);

  uint32_t image_count = swap_chain_support.capabilities.minImageCount + 1;
  if (swap_chain_support.capabilities.maxImageCount > 0 &&
      image_count > swap_chain_support.capabilities.maxImageCount) {
    image_count = swap_chain_support.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR swap_chain_info{};
  swap_chain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swap_chain_info.surface = surface_;
  swap_chain_info.minImageCount = image_count;
  swap_chain_info.imageFormat = surface_format.format;
  swap_chain_info.imageColorSpace = surface_format.colorSpace;
  swap_chain_info.imageExtent = extent;
  swap_chain_info.imageArrayLayers = 1;
  swap_chain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  QueueFamilies indices = FindQueueFaimilies(physical_device_);
  uint32_t queue_family_indices[] = {indices.graphics_family.value(),
                                     indices.present_family.value()};

  if (indices.graphics_family != indices.present_family) {
    swap_chain_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    swap_chain_info.queueFamilyIndexCount = 2;
    swap_chain_info.pQueueFamilyIndices = queue_family_indices;
  } else {
    swap_chain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swap_chain_info.queueFamilyIndexCount = 0;      // Optional
    swap_chain_info.pQueueFamilyIndices = nullptr;  // Optional
  }

  swap_chain_info.preTransform =
      swap_chain_support.capabilities.currentTransform;
  swap_chain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swap_chain_info.presentMode = present_mode;
  swap_chain_info.clipped = VK_TRUE;
  swap_chain_info.oldSwapchain = VK_NULL_HANDLE;

  if (VK_SUCCESS !=
      vkCreateSwapchainKHR(device_, &swap_chain_info, nullptr, &swap_chain_)) {
    throw std::runtime_error(
        "----- Error::Vulkan: Failed to create swap chain -----");
  }

  vkGetSwapchainImagesKHR(device_, swap_chain_, &image_count, nullptr);
  swap_chain_images_.resize(image_count);
  vkGetSwapchainImagesKHR(device_, swap_chain_, &image_count,
                          swap_chain_images_.data());
  swap_chain_image_format_ = surface_format.format;
  swap_chain_extent_ = extent;
}

void Application::CreateImageViews() {
  swap_chain_image_views_.resize(swap_chain_images_.size());

  for (size_t i = 0; i < swap_chain_images_.size(); i++) {
    VkImageViewCreateInfo image_view_info{};
    image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_info.image = swap_chain_images_[i];
    image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_info.format = swap_chain_image_format_;
    image_view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_view_info.subresourceRange.baseMipLevel = 0;
    image_view_info.subresourceRange.levelCount = 1;
    image_view_info.subresourceRange.baseArrayLayer = 0;
    image_view_info.subresourceRange.layerCount = 1;

    if (VK_SUCCESS != vkCreateImageView(device_, &image_view_info, nullptr,
                                        &swap_chain_image_views_[i])) {
      throw std::runtime_error(
          "----- Error::Vulkan: Failed to create image views -----");
    }
  }
}

void Application::CreateRenderPass() {
  VkAttachmentDescription color_attachment{};
  color_attachment.format = swap_chain_image_format_;
  color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference color_attachment_ref{};
  color_attachment_ref.attachment = 0;
  color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass_desc{};
  subpass_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass_desc.colorAttachmentCount = 1;
  subpass_desc.pColorAttachments = &color_attachment_ref;

  VkRenderPassCreateInfo render_pass_info{};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  render_pass_info.attachmentCount = 1;
  render_pass_info.pAttachments = &color_attachment;
  render_pass_info.subpassCount = 1;
  render_pass_info.pSubpasses = &subpass_desc;

  if (vkCreateRenderPass(device_, &render_pass_info, nullptr, &render_pass_) !=
      VK_SUCCESS) {
    throw std::runtime_error(
        "---- Error::Vulkan: Failed to create render pass -----");
  }
}

void Application::CreateGraphicsPipeline() {
  auto vert_shader_code = ReadFile(VERT_SHADER_FILEPATH);
  auto frag_sahder_code = ReadFile(FRAG_SHADER_FILEPATH);

  VkShaderModule vert_shader_moudle = CreateShaderMoudle(vert_shader_code);
  VkShaderModule frag_shader_moudle = CreateShaderMoudle(frag_sahder_code);

  // shader stage creation
  VkPipelineShaderStageCreateInfo vert_shader_stage_info{};
  vert_shader_stage_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vert_shader_stage_info.module = vert_shader_moudle;
  vert_shader_stage_info.pName = "main";

  VkPipelineShaderStageCreateInfo frag_shader_stage_info{};
  frag_shader_stage_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  frag_shader_stage_info.module = frag_shader_moudle;
  frag_shader_stage_info.pName = "main";

  VkPipelineShaderStageCreateInfo shader_stage_infos[] = {
      vert_shader_stage_info, frag_shader_stage_info};

  // dynamic state
  std::vector<VkDynamicState> dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT,
                                                VK_DYNAMIC_STATE_SCISSOR};

  VkPipelineDynamicStateCreateInfo dynamicState{};
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
  dynamicState.pDynamicStates = dynamic_states.data();

  // vertex input
  VkPipelineVertexInputStateCreateInfo vertex_input_info{};
  vertex_input_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertex_input_info.vertexBindingDescriptionCount = 0;
  vertex_input_info.pVertexBindingDescriptions = nullptr;  // Optional
  vertex_input_info.vertexAttributeDescriptionCount = 0;
  vertex_input_info.pVertexAttributeDescriptions = nullptr;  // Optional

  // input assembly
  VkPipelineInputAssemblyStateCreateInfo input_assembly_info{};
  input_assembly_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  input_assembly_info.primitiveRestartEnable = VK_FALSE;

  // viewport and scissor
  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(swap_chain_extent_.width);
  viewport.height = static_cast<float>(swap_chain_extent_.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = swap_chain_extent_;

  VkPipelineViewportStateCreateInfo viewport_state_info{};
  viewport_state_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_state_info.viewportCount = 1;
  viewport_state_info.pViewports = &viewport;
  viewport_state_info.scissorCount = 1;
  viewport_state_info.pScissors = &scissor;

  // rasterizer
  VkPipelineRasterizationStateCreateInfo rasterizer_state_info{};
  rasterizer_state_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer_state_info.depthClampEnable = VK_FALSE;
  rasterizer_state_info.rasterizerDiscardEnable = VK_FALSE;
  rasterizer_state_info.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer_state_info.lineWidth = 1.0f;
  rasterizer_state_info.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer_state_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterizer_state_info.depthBiasEnable = VK_FALSE;
  rasterizer_state_info.depthBiasConstantFactor = 0.0f;  // Optional
  rasterizer_state_info.depthBiasClamp = 0.0f;           // Optional
  rasterizer_state_info.depthBiasSlopeFactor = 0.0f;     // Optional

  // multisampling
  VkPipelineMultisampleStateCreateInfo multisample_state_info{};
  multisample_state_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisample_state_info.sampleShadingEnable = VK_FALSE;
  multisample_state_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisample_state_info.minSampleShading = 1.0f;           // Optional
  multisample_state_info.pSampleMask = nullptr;             // Optional
  multisample_state_info.alphaToCoverageEnable = VK_FALSE;  // Optional
  multisample_state_info.alphaToOneEnable = VK_FALSE;       // Optional

  // color blending
  VkPipelineColorBlendAttachmentState color_blend_attachment{};
  color_blend_attachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  color_blend_attachment.blendEnable = VK_FALSE;
  color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
  color_blend_attachment.dstColorBlendFactor =
      VK_BLEND_FACTOR_ZERO;                                          // Optional
  color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;             // Optional
  color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
  color_blend_attachment.dstAlphaBlendFactor =
      VK_BLEND_FACTOR_ZERO;                               // Optional
  color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;  // Optional

  VkPipelineColorBlendStateCreateInfo color_blend_state_info{};
  color_blend_state_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  color_blend_state_info.logicOpEnable = VK_FALSE;
  color_blend_state_info.logicOp = VK_LOGIC_OP_COPY;  // Optional
  color_blend_state_info.attachmentCount = 1;
  color_blend_state_info.pAttachments = &color_blend_attachment;
  color_blend_state_info.blendConstants[0] = 0.0f;  // Optional
  color_blend_state_info.blendConstants[1] = 0.0f;  // Optional
  color_blend_state_info.blendConstants[2] = 0.0f;  // Optional
  color_blend_state_info.blendConstants[3] = 0.0f;  // Optional

  // pipeline layout
  VkPipelineLayoutCreateInfo pipeline_layout_info{};
  pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_info.setLayoutCount = 0;             // Optional
  pipeline_layout_info.pSetLayouts = nullptr;          // Optional
  pipeline_layout_info.pushConstantRangeCount = 0;     // Optional
  pipeline_layout_info.pPushConstantRanges = nullptr;  // Optional

  if (VK_SUCCESS != vkCreatePipelineLayout(device_, &pipeline_layout_info,
                                           nullptr, &pipeline_layout_)) {
    throw std::runtime_error(
        "----- Error::Vulkan: Failed to create pipeline layout -----");
  }

  VkGraphicsPipelineCreateInfo pipeline_info{};
  pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_info.stageCount = 2;
  pipeline_info.pStages = shader_stage_infos;
  pipeline_info.pVertexInputState = &vertex_input_info;
  pipeline_info.pInputAssemblyState = &input_assembly_info;
  pipeline_info.pViewportState = &viewport_state_info;
  pipeline_info.pRasterizationState = &rasterizer_state_info;
  pipeline_info.pMultisampleState = &multisample_state_info;
  pipeline_info.pDepthStencilState = nullptr;  // Optional
  pipeline_info.pColorBlendState = &color_blend_state_info;
  pipeline_info.pDynamicState = &dynamicState;
  pipeline_info.layout = pipeline_layout_;
  pipeline_info.renderPass = render_pass_;
  pipeline_info.subpass = 0;
  pipeline_info.basePipelineHandle = VK_NULL_HANDLE;  // Optional
  pipeline_info.basePipelineIndex = -1;               // Optional

  if (VK_SUCCESS != vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1,
                                              &pipeline_info, nullptr,
                                              &graphics_pipeline_)) {
    throw std::runtime_error("failed to create graphics pipeline!");
  }

  vkDestroyShaderModule(device_, vert_shader_moudle, nullptr);
  vkDestroyShaderModule(device_, frag_shader_moudle, nullptr);
}

void Application::CreateFrameBuffers() {
  swap_chain_framebuffers_.resize(swap_chain_image_views_.size());

  for (size_t i = 0; i < swap_chain_image_views_.size(); i++) {
    VkImageView attachments[] = {swap_chain_image_views_[i]};

    VkFramebufferCreateInfo framebuffer_info{};
    framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_info.renderPass = render_pass_;
    framebuffer_info.attachmentCount = 1;
    framebuffer_info.pAttachments = attachments;
    framebuffer_info.width = swap_chain_extent_.width;
    framebuffer_info.height = swap_chain_extent_.height;
    framebuffer_info.layers = 1;

    if (VK_SUCCESS != vkCreateFramebuffer(device_, &framebuffer_info, nullptr,
                                          &swap_chain_framebuffers_[i])) {
      throw std::runtime_error(
          "----- Error::Vulkan: Failed to create framebuffer -----");
    }
  }
}

void Application::FindInstanceExtensions(
    std::vector<const char*>& required_extensions) {
  // glfw required extensions
  uint32_t glfw_ext_cnt = 0;
  const char** glfw_exts = glfwGetRequiredInstanceExtensions(&glfw_ext_cnt);

  for (uint32_t i = 0; i < glfw_ext_cnt; ++i) {
    required_extensions.push_back(glfw_exts[i]);
  }

#ifdef __APPLE__
  required_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
  required_extensions.push_back("VK_KHR_get_physical_device_properties2");
#endif

  required_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

  // available instance extensions
  uint32_t available_extension_cnt = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &available_extension_cnt,
                                         nullptr);
  std::vector<VkExtensionProperties> available_extensions(
      available_extension_cnt);
  vkEnumerateInstanceExtensionProperties(nullptr, &available_extension_cnt,
                                         available_extensions.data());

  if (!CheckExtensionSupport(available_extensions, required_extensions)) {
    throw std::runtime_error(
        "----- Error::Vulkan: Find not supported instance extension(s) "
        "-----");
  }
}

void Application::FindInstanceLayers(
    std::vector<const char*>& required_layers) {
  // add validation layer
  required_layers.push_back("VK_LAYER_KHRONOS_validation");

  // available instance layers
  uint32_t available_layer_cnt = 0;
  vkEnumerateInstanceLayerProperties(&available_layer_cnt, nullptr);
  std::vector<VkLayerProperties> available_layers(available_layer_cnt);
  vkEnumerateInstanceLayerProperties(&available_layer_cnt,
                                     available_layers.data());

  if (true != CheckLayersSupport(available_layers, required_layers)) {
    throw std::runtime_error(
        "----- Error::Vulkan: Find not supported layer(s) -----");
  }
}

VkResult Application::CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* create_info,
    const VkAllocationCallbacks* allocator,
    VkDebugUtilsMessengerEXT* debug_messenger) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugUtilsMessengerEXT");

  if (func) {
    return func(instance, create_info, allocator, debug_messenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void Application::DestroyDebugUtilsMessengerEXT(
    VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger,
    const VkAllocationCallbacks* allocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkDestroyDebugUtilsMessengerEXT");

  if (func) {
    func(instance, debug_messenger, allocator);
  } else {
    throw std::runtime_error(
        "----- Error::Vualkan: Failed to destroy debug utils messenger "
        "-----");
  }
}

int Application::EvaluateDevice(VkPhysicalDevice device) {
  // physical device properties
  VkPhysicalDeviceProperties device_properties{};
  vkGetPhysicalDeviceProperties(device, &device_properties);

  // physical device features
  VkPhysicalDeviceFeatures device_features{};
  vkGetPhysicalDeviceFeatures(device, &device_features);

  int score = 0;

  if (VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU == device_properties.deviceType) {
    score += 1000;
  }
  score += device_properties.limits.maxImageDimension2D;

  if (!device_features.geometryShader) {
    score = 1;
  }

  // physical device queue family support
  QueueFamilies queue_faimlies = FindQueueFaimilies(device);
  if (!queue_faimlies.IsCompleted()) {
    score = 0;
  }

  // physical device extension support
  std::vector<const char*> required_extensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
#ifdef __APPLE__
  required_extensions.push_back("VK_KHR_portability_subset");
#endif

  uint32_t available_extension_cnt = 0;
  vkEnumerateDeviceExtensionProperties(device, nullptr,
                                       &available_extension_cnt, nullptr);
  std::vector<VkExtensionProperties> available_extensions(
      available_extension_cnt);
  vkEnumerateDeviceExtensionProperties(
      device, nullptr, &available_extension_cnt, available_extensions.data());

  if (!CheckExtensionSupport(available_extensions, required_extensions)) {
    score = 0;
  }

  // physical device swap chain support
  SwapChainSupportDetails details = QuerySwapChainSupoort(device);

  if (!details.IsAdequate()) {
    score = 0;
  }

  std::clog << "----- Physical Device: " << device_properties.deviceName
            << ", score: " << score << " -----\n";

  return score;
}

void Application::FindDeviceExtensions(
    VkPhysicalDevice device, std::vector<const char*>& required_extensions) {
  required_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

#ifdef __APPLE__
  required_extensions.push_back("VK_KHR_portability_subset");
#endif

  uint32_t available_extension_cnt = 0;
  vkEnumerateDeviceExtensionProperties(device, nullptr,
                                       &available_extension_cnt, nullptr);
  std::vector<VkExtensionProperties> available_extensions(
      available_extension_cnt);
  vkEnumerateDeviceExtensionProperties(
      device, nullptr, &available_extension_cnt, available_extensions.data());

  if (!CheckExtensionSupport(available_extensions, required_extensions)) {
    throw std::runtime_error(
        "----- Error::Vulkan: Find not supported device extension(s) -----");
  }
}

QueueFamilies Application::FindQueueFaimilies(VkPhysicalDevice device) {
  QueueFamilies indices{};

  // queue families
  uint32_t queue_family_cnt = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_cnt, nullptr);
  std::vector<VkQueueFamilyProperties> queue_families(queue_family_cnt);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_cnt,
                                           queue_families.data());

  // get index of required queue family
  for (uint32_t i = 0; i < queue_family_cnt; ++i) {
    if (queue_families[i].queueCount > 0 &&
        (VK_QUEUE_GRAPHICS_BIT & queue_families[i].queueFlags)) {
      indices.graphics_family = i;
    }

    VkBool32 present_support = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &present_support);
    if (queue_families[i].queueCount > 0 && present_support) {
      indices.present_family = i;
    }

    if (indices.IsCompleted()) {
      break;
    }
  }

  return indices;
}

SwapChainSupportDetails Application::QuerySwapChainSupoort(
    VkPhysicalDevice device) {
  SwapChainSupportDetails details{};

  // basic surface capabilities
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_,
                                            &details.capabilities);

  // surface formats
  uint32_t format_cnt = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &format_cnt, nullptr);

  if (format_cnt) {
    details.formats.resize(format_cnt);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &format_cnt,
                                         details.formats.data());
  }

  // available present modes
  uint32_t present_mode_cnt;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &present_mode_cnt,
                                            nullptr);

  if (present_mode_cnt) {
    details.present_modes.resize(present_mode_cnt);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, surface_, &present_mode_cnt, details.present_modes.data());
  }

  return details;
}

VkSurfaceFormatKHR Application::ChooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& available_formats) {
  for (const auto& available : available_formats) {
    if (available.format == VK_FORMAT_B8G8R8A8_SRGB &&
        available.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return available;
    }
  }

  return available_formats[0];
}

VkPresentModeKHR Application::ChooseSwapPresentMode(
    const std::vector<VkPresentModeKHR>& available_present_modes) {
  for (const auto& available : available_present_modes) {
    if (available == VK_PRESENT_MODE_MAILBOX_KHR) {
      return available;
    }
  }

  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Application::ChooseSwapExtent(
    const VkSurfaceCapabilitiesKHR& capabilities) {
  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    int width, height;
    glfwGetFramebufferSize(window_, &width, &height);

    VkExtent2D actual_extent = {static_cast<uint32_t>(width),
                                static_cast<uint32_t>(height)};
    actual_extent.width =
        std::clamp(actual_extent.width, capabilities.minImageExtent.width,
                   capabilities.maxImageExtent.width);
    actual_extent.height =
        std::clamp(actual_extent.height, capabilities.minImageExtent.height,
                   capabilities.maxImageExtent.height);

    return actual_extent;
  }
}

VkShaderModule Application::CreateShaderMoudle(const std::vector<char>& code) {
  VkShaderModuleCreateInfo shader_module_info{};
  shader_module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  shader_module_info.codeSize = code.size();
  shader_module_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

  VkShaderModule shader_module;
  if (vkCreateShaderModule(device_, &shader_module_info, nullptr,
                           &shader_module) != VK_SUCCESS) {
    throw std::runtime_error(
        "----- Error::Vulkan: Failed to create shader module -----");
  }

  return shader_module;
}

bool Application::CheckExtensionSupport(
    std::vector<VkExtensionProperties>& available_extensions,
    std::vector<const char*>& required_extensions) {
  std::set<std::string> extensions(required_extensions.begin(),
                                   required_extensions.end());

  for (const auto& extension : available_extensions) {
    extensions.erase(extension.extensionName);
  }

  return extensions.empty();
}

bool Application::CheckLayersSupport(
    std::vector<VkLayerProperties>& available_layers,
    std::vector<const char*>& required_layers) {
  std::set<std::string> layers(required_layers.begin(), required_layers.end());

  for (const auto& layer : available_layers) {
    layers.erase(layer.layerName);
  }

  return layers.empty();
}

VKAPI_ATTR VkBool32 VKAPI_CALL Application::DebugCallBack(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data) {
  if (message_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    std::clog << "----- Validation Layer: "
              << "\n\t\tSeverity: " << message_severity
              << "\n\t\tType: " << message_type
              << "\n\t\tMessage: " << callback_data->pMessage
              << "\n\t\tUser Data Address: " << user_data << std::endl;
  }

  return VK_FALSE;
}

std::vector<char> Application::ReadFile(const std::string& filename) {
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    throw std::runtime_error("----- Error::File: Failed to open file -----");
  }

  size_t file_size = static_cast<size_t>(file.tellg());
  std::vector<char> buffer(file_size);

  file.seekg(0);
  file.read(buffer.data(), file_size);

  file.close();

  return buffer;
}

}  // namespace playground
