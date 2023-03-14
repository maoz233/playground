/**
 * @file application.h
 * @author Mao Zhang (mao.zhang233@gmail.com)
 * @brief
 * @version 1.0
 * @date 2023-02-20
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef PLAYGROUND_INCLUDE_APPLICATION_H_
#define PLAYGROUND_INCLUDE_APPLICATION_H_
#include <array>
#include <optional>
#include <string>
#include <vector>

#define PLAYGROUND_IMGUI_
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#define PLAYGROUND_GLM_
#include <glm/glm.hpp>

namespace playground {

const int WIDTH = 800;
const int HEIGHT = 600;
const std::string TITLE{"Playground"};

const int MAX_FRAMES_IN_FLIGHT = 2;

#ifdef NDEBUG
const bool ENABLE_VALIDATION_LAYER = false;
#else
const bool ENABLE_VALIDATION_LAYER = true;
#endif

#ifdef _WIN32
const std::string VERT_SHADER_FILEPATH{"../../shaders/triangle.vert.spv"};
const std::string FRAG_SHADER_FILEPATH{"../../shaders/triangle.frag.spv"};
#else
const std::string VERT_SHADER_FILEPATH{"../shaders/triangle.vert.spv"};
const std::string FRAG_SHADER_FILEPATH{"../shaders/triangle.frag.spv"};
#endif

struct QueueFamilies {
  std::optional<uint32_t> graphics_family;
  std::optional<uint32_t> present_family;

  inline bool IsCompleted();
};

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> present_modes;

  inline bool IsAdequate();
};

struct Vertex {
  glm::vec2 pos;
  glm::vec3 color;

  static VkVertexInputBindingDescription GetBindingDescription();
  static std::array<VkVertexInputAttributeDescription, 2>
  GetAttributeDescriptions();
};

class Application {
 public:
  Application();
  Application(const Application&) = delete;
  ~Application();

  Application& operator=(const Application&) = delete;

  void Run();

  void CreateWindow();
  void CreateInstance();
  void SetupDebugMessenger();
  void CreateSurface();
  void PickPhysicalDevice();
  void CreateLogicalDevice();
  void CreateSwapChain();
  void CreateImageViews();
  void CreateFrameBuffers();
  void CreateRenderPass();
  void CreatePipelineLayout();
  void CreateGraphicsPipeline();
  void CreateCommandPool();
  void CreateCommandBuffers();
  void CreateVertexBuffer();
  void CreateIndexBuffer();
  void CreateDescriptorPool();
  void CreateSyncObjects();

  void DrawFrame();
  void RecordCommandBuffer(VkCommandBuffer command_buffer,
                           uint32_t image_index);
  void RecreateSwapChain();
  void CleanupSwapChain();

  void FindInstanceExtensions(std::vector<const char*>& required_extensions);
  void FindInstanceLayers(std::vector<const char*>& required_layers);

  VkResult CreateDebugUtilsMessengerEXT(
      VkInstance instance,
      const VkDebugUtilsMessengerCreateInfoEXT* create_info,
      const VkAllocationCallbacks* allocator,
      VkDebugUtilsMessengerEXT* debug_messenger);
  void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                     VkDebugUtilsMessengerEXT debug_messenger,
                                     const VkAllocationCallbacks* allocator);

  int EvaluateDevice(VkPhysicalDevice device);

  void FindDeviceExtensions(VkPhysicalDevice device,
                            std::vector<const char*>& required_extensions);

  QueueFamilies FindQueueFaimilies(VkPhysicalDevice device);

  SwapChainSupportDetails QuerySwapChainSupoort(VkPhysicalDevice device);
  VkSurfaceFormatKHR ChooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR>& available_formats);
  VkPresentModeKHR ChooseSwapPresentMode(
      const std::vector<VkPresentModeKHR>& available_present_modes);
  VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

  VkShaderModule CreateShaderMoudle(const std::vector<char>& code);

  bool CheckExtensionSupport(
      std::vector<VkExtensionProperties>& available_extensions,
      std::vector<const char*>& required_extensions);
  bool CheckLayersSupport(std::vector<VkLayerProperties>& available_layers,
                          std::vector<const char*>& required_layers);

  VkCommandBuffer BeginSingleTimeCommands();
  void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

  uint32_t FindMemoryType(uint32_t type_filter,
                          VkMemoryPropertyFlags properties);

  void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                    VkMemoryPropertyFlags properties, VkBuffer& buffer,
                    VkDeviceMemory& buffer_memory);
  void CopyBuffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);

  static void FramebufferResizeCallback(GLFWwindow* window, int width,
                                        int height);
  static VKAPI_ATTR VkBool32 VKAPI_CALL
  DebugCallBack(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                VkDebugUtilsMessageTypeFlagsEXT message_type,
                const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                void* user_data);

  static std::vector<char> ReadFile(const std::string& filename);

 private:
  GLFWwindow* window_;

  VkInstance instance_;
  VkDebugUtilsMessengerEXT debug_messenger_;
  VkSurfaceKHR surface_;
  VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
  VkDevice device_;

  QueueFamilies queue_faimlies_;
  VkQueue graphics_queue_;
  VkQueue present_queue_;

  VkSwapchainKHR swap_chain_;
  std::vector<VkImage> swap_chain_images_;
  VkFormat swap_chain_image_format_;
  VkExtent2D swap_chain_extent_;

  std::vector<VkImageView> swap_chain_image_views_;

  VkRenderPass render_pass_;

  VkPipelineLayout pipeline_layout_;
  VkPipeline graphics_pipeline_;

  std::vector<VkFramebuffer> swap_chain_framebuffers_;

  VkCommandPool command_pool_;
  std::vector<VkCommandBuffer> command_buffers_;

  VkBuffer vertex_buffer_;
  VkDeviceMemory vertex_buffer_memory_;
  VkBuffer index_buffer_;
  VkDeviceMemory index_buffer_memory_;

  VkDescriptorPool descriptor_pool_;

  std::vector<VkSemaphore> image_available_semaphores_;
  std::vector<VkSemaphore> render_finished_semaphores_;
  std::vector<VkFence> in_flight_fences_;

  int current_frame = 0;
  bool framebuffer_resized = false;

  const std::vector<Vertex> vertices_ = {{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                                         {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
                                         {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
                                         {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}};
  const std::vector<uint16_t> indices_ = {0, 1, 2, 2, 3, 0};
};

}  // namespace playground

#endif  // PLAYGROUND_INCLUDE_APPLICATION_H_
