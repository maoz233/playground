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

#include <stdexcept>
#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "config.h"
#include "device.h"
#include "pipeline.h"
#include "swap_chain.h"
#include "window.h"

namespace playground {

Application::Application(const ApplicationConfig& config)
    : max_frames_in_flight_{config.max_frames_in_flight},
      current_frame_{0},
      window_{Window(config.window)},
      device_{Device(window_, config.device)},
      swap_chain_{SwapChain(window_, device_)},
      pipeline_{Pipeline(device_, swap_chain_, config.pipeline)} {
  CreateCommandBuffers();
  CreateSemaphores();
  CreateFence();
}

Application::~Application() {
  for (std::size_t i = 0; i < max_frames_in_flight_; ++i) {
    vkDestroySemaphore(device_.GetDevice(), image_available_semaphores_[i],
                       nullptr);
    vkDestroySemaphore(device_.GetDevice(), render_finished_semaphores_[i],
                       nullptr);
    vkDestroyFence(device_.GetDevice(), in_flight_fences_[i], nullptr);
  }
}

void Application::Run() {
  while (!window_.ShouldClose()) {
    ProcessInput();
    glfwPollEvents();
    DrawFrame();
  }

  vkDeviceWaitIdle(device_.GetDevice());
}

void Application::CreateCommandBuffers() {
  command_buffers_.resize(max_frames_in_flight_);

  VkCommandBufferAllocateInfo allocate_info{};
  allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocate_info.commandPool = swap_chain_.GetCommandPool();
  allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocate_info.commandBufferCount =
      static_cast<uint32_t>(command_buffers_.size());

  if (VK_SUCCESS != vkAllocateCommandBuffers(device_.GetDevice(),
                                             &allocate_info,
                                             command_buffers_.data())) {
    throw std::runtime_error(
        "----- Error::SwapChain: Failed to allocate command buffer -----");
  }
}

void Application::CreateSemaphores() {
  image_available_semaphores_.resize(max_frames_in_flight_);
  render_finished_semaphores_.resize(max_frames_in_flight_);

  VkSemaphoreCreateInfo semaphore_info{};
  semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  for (std::size_t i = 0; i < max_frames_in_flight_; ++i) {
    if (VK_SUCCESS != vkCreateSemaphore(device_.GetDevice(), &semaphore_info,
                                        nullptr,
                                        &image_available_semaphores_[i]) ||
        VK_SUCCESS != vkCreateSemaphore(device_.GetDevice(), &semaphore_info,
                                        nullptr,
                                        &render_finished_semaphores_[i])) {
      throw std::runtime_error(
          "----- Error::SwapChain: Failed to create semaphore -----");
    }
  }
}

void Application::CreateFence() {
  in_flight_fences_.resize(max_frames_in_flight_);

  VkFenceCreateInfo fence_info{};
  fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (std::size_t i = 0; i < max_frames_in_flight_; ++i) {
    if (VK_SUCCESS != vkCreateFence(device_.GetDevice(), &fence_info, nullptr,
                                    &in_flight_fences_[i])) {
      throw std::runtime_error(
          "----- Error::Application: Failed to create fence -----");
    }
  }
}

void Application::ProcessInput() {
  if (GLFW_PRESS == glfwGetKey(window_.GetWindow(), GLFW_KEY_ESCAPE))
    glfwSetWindowShouldClose(window_.GetWindow(), true);
}

void Application::DrawFrame() {
  vkWaitForFences(device_.GetDevice(), 1, &in_flight_fences_[current_frame_],
                  VK_TRUE, UINT64_MAX);
  vkResetFences(device_.GetDevice(), 1, &in_flight_fences_[current_frame_]);

  uint32_t image_index;
  vkAcquireNextImageKHR(device_.GetDevice(), swap_chain_.GetSwapChain(),
                        UINT64_MAX, image_available_semaphores_[current_frame_],
                        VK_NULL_HANDLE, &image_index);

  vkResetCommandBuffer(command_buffers_[current_frame_], 0);
  RecordCommandBuffer(command_buffers_[current_frame_], image_index);

  VkSemaphore wait_semaphores[] = {image_available_semaphores_[current_frame_]};

  VkPipelineStageFlags wait_stages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

  VkSemaphore signal_semaphores[] = {
      render_finished_semaphores_[current_frame_]};

  VkSubmitInfo submit_info{};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores = wait_semaphores;
  submit_info.pWaitDstStageMask = wait_stages;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &command_buffers_[current_frame_];
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores = signal_semaphores;

  if (VK_SUCCESS != vkQueueSubmit(device_.GetgGaphicsQueue(), 1, &submit_info,
                                  in_flight_fences_[current_frame_])) {
    throw std::runtime_error(
        "----- Error::Application: Failed to submit draw command buffer -----");
  }

  VkSwapchainKHR swap_chains[] = {swap_chain_.GetSwapChain()};

  VkPresentInfoKHR present_info{};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores = signal_semaphores;
  present_info.swapchainCount = 1;
  present_info.pSwapchains = swap_chains;
  present_info.pImageIndices = &image_index;
  present_info.pResults = nullptr;

  vkQueuePresentKHR(device_.GetPresentQueue(), &present_info);

  current_frame_ = (current_frame_ + 1) % max_frames_in_flight_;
}

void Application::RecordCommandBuffer(VkCommandBuffer command_buffer,
                                      uint32_t image_index) {
  VkCommandBufferBeginInfo begin_info{};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = 0;
  begin_info.pInheritanceInfo = nullptr;

  if (VK_SUCCESS != vkBeginCommandBuffer(command_buffer, &begin_info)) {
    throw std::runtime_error(
        "----- Error::SwapChain: Failed to record command buffer at begin "
        "-----");
  }

  VkClearValue clear_color = {{{0.f, 0.f, 0.f, 1.f}}};

  VkRenderPassBeginInfo render_pass_info{};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  render_pass_info.renderPass = swap_chain_.GetRenderPass();
  render_pass_info.framebuffer = swap_chain_.GetFrameBuffer(image_index);
  render_pass_info.renderArea.offset = {0, 0};
  render_pass_info.renderArea.extent = swap_chain_.GetSwapChainExtent();
  render_pass_info.clearValueCount = 1;
  render_pass_info.pClearValues = &clear_color;

  vkCmdBeginRenderPass(command_buffer, &render_pass_info,
                       VK_SUBPASS_CONTENTS_INLINE);

  vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pipeline_.GetGraphicsPipeline());

  VkViewport viewport{};
  viewport.x = 0.f;
  viewport.y = 0.f;
  viewport.width = static_cast<float>(swap_chain_.GetSwapChainExtent().width);
  viewport.height = static_cast<float>(swap_chain_.GetSwapChainExtent().height);
  viewport.minDepth = 0.f;
  viewport.maxDepth = 1.f;

  vkCmdSetViewport(command_buffer, 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = swap_chain_.GetSwapChainExtent();

  vkCmdSetScissor(command_buffer, 0, 1, &scissor);

  vkCmdDraw(command_buffer, 3, 1, 0, 0);

  vkCmdEndRenderPass(command_buffer);

  if (VK_SUCCESS != vkEndCommandBuffer(command_buffer)) {
    throw std::runtime_error(
        "----- Error::Application: Failed to record command buffer at end "
        "---- ");
  }
}

}  // namespace playground
