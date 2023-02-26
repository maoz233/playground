/**
 * @file swap_chain.cc
 * @author Mao Zhang (mao.zhang233@gmail.com)
 * @brief
 * @version 1.0
 * @date 2023-02-25
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "swap_chain.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>

#include "device.h"
#include "window.h"

namespace playground {
SwapChain::SwapChain(Window& window, Device& device)
    : window_{window}, device_{device} {
  CreateSwapChain();
  CreateImageViews();
  CreateRenderPass();
}

SwapChain::~SwapChain() {
  for (auto framebuffer : frame_buffers_) {
    vkDestroyFramebuffer(device_.GetDevice(), framebuffer, nullptr);
  }
  vkDestroyRenderPass(device_.GetDevice(), render_pass_, nullptr);
  for (auto image_view : swap_chain_image_views_) {
    vkDestroyImageView(device_.GetDevice(), image_view, nullptr);
  }
  vkDestroySwapchainKHR(device_.GetDevice(), swap_chain_, nullptr);
}

void SwapChain::CreateSwapChain() {
  SwapChainSupportDetails swap_chain_support_details =
      device_.QuerySwapChainSupport(device_.GetPhysicalDevice());

  VkExtent2D extent = ChooseSwapExtent(swap_chain_support_details.capabilities);
  VkSurfaceFormatKHR surface_format =
      ChooseSwapSufaceFormat(swap_chain_support_details.formats);
  VkPresentModeKHR present_mode =
      ChooseSwapPresentMode(swap_chain_support_details.present_modes);

  uint32_t image_cnt =
      swap_chain_support_details.capabilities.minImageCount + 1;
  if (swap_chain_support_details.capabilities.maxImageCount > 0 &&
      image_cnt > swap_chain_support_details.capabilities.maxImageCount) {
    image_cnt = swap_chain_support_details.capabilities.maxImageCount;
  }

  QueueFamilies indices =
      device_.FindQueueFaimilies(device_.GetPhysicalDevice());
  uint32_t queue_family_indices[]{indices.graphics_family.value(),
                                  indices.present_family.value()};

  VkSwapchainCreateInfoKHR create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  create_info.surface = device_.GetSurface();
  create_info.minImageCount = image_cnt;
  create_info.imageFormat = surface_format.format;
  create_info.imageColorSpace = surface_format.colorSpace;
  create_info.imageExtent = extent;
  create_info.imageArrayLayers = 1;
  create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  create_info.preTransform =
      swap_chain_support_details.capabilities.currentTransform;
  create_info.presentMode = present_mode;
  if (indices.graphics_family != indices.graphics_family) {
    create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    create_info.queueFamilyIndexCount = 2;
    create_info.pQueueFamilyIndices = queue_family_indices;
  } else {
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.queueFamilyIndexCount = 1;
    create_info.pQueueFamilyIndices = nullptr;
  }
  create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  create_info.oldSwapchain = VK_NULL_HANDLE;

  if (VK_SUCCESS != vkCreateSwapchainKHR(device_.GetDevice(), &create_info,
                                         nullptr, &swap_chain_)) {
    throw std::runtime_error(
        "----- Error::SwapChain: Failed to create swap chain -----");
  }

  vkGetSwapchainImagesKHR(device_.GetDevice(), swap_chain_, &image_cnt,
                          nullptr);
  swap_chain_images_.resize(image_cnt);
  vkGetSwapchainImagesKHR(device_.GetDevice(), swap_chain_, &image_cnt,
                          swap_chain_images_.data());
  swap_chain_image_format_ = surface_format.format;
  swap_chain_extent_ = extent;
}

void SwapChain::CreateImageViews() {
  swap_chain_image_views_.resize(swap_chain_images_.size());
  for (std::size_t i = 0; i < swap_chain_images_.size(); ++i) {
    VkImageViewCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    create_info.image = swap_chain_images_[i];
    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    create_info.format = swap_chain_image_format_;
    create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    create_info.subresourceRange.baseMipLevel = 0;
    create_info.subresourceRange.levelCount = 1;
    create_info.subresourceRange.baseArrayLayer = 0;
    create_info.subresourceRange.layerCount = 1;

    if (VK_SUCCESS != vkCreateImageView(device_.GetDevice(), &create_info,
                                        nullptr,
                                        swap_chain_image_views_.data())) {
      throw std::runtime_error(
          "----- Error::SwapChain: Failed to create image view -----");
    }
  }
}

void SwapChain::CreateRenderPass() {
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
  color_attachment_ref.layout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_attachment_ref;

  VkRenderPassCreateInfo render_pass_info{};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  render_pass_info.attachmentCount = 1;
  render_pass_info.pAttachments = &color_attachment;
  render_pass_info.subpassCount = 1;
  render_pass_info.pSubpasses = &subpass;

  if (VK_SUCCESS != vkCreateRenderPass(device_.GetDevice(), &render_pass_info,
                                       nullptr, &render_pass_)) {
    throw std::runtime_error(
        "----- Error::SwapChain: Failed to create render pass -----");
  }
}

void SwapChain::CreateFrameBuffers() {
  frame_buffers_.resize(swap_chain_image_views_.size());

  for (std::size_t i = 0; i < swap_chain_image_views_.size(); ++i) {
    VkImageView attachments[] = {swap_chain_image_views_[i]};

    VkFramebufferCreateInfo frame_buffer_info{};
    frame_buffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frame_buffer_info.renderPass = render_pass_;
    frame_buffer_info.attachmentCount = 1;
    frame_buffer_info.pAttachments = attachments;
    frame_buffer_info.width = swap_chain_extent_.width;
    frame_buffer_info.height = swap_chain_extent_.height;
    frame_buffer_info.layers = 1;

    if (VK_SUCCESS != vkCreateFramebuffer(device_.GetDevice(),
                                          &frame_buffer_info, nullptr,
                                          &frame_buffers_[i])) {
      throw std::runtime_error(
          "----- Error::SwapChain: Failed to create framebuffer -----");
    }
  }
}

VkExtent2D SwapChain::GetSwapChainExtent() { return swap_chain_extent_; }

VkSwapchainKHR SwapChain::GetSwapChain() { return swap_chain_; }

std::vector<VkImageView> SwapChain::GetSwapChainImageViews() {
  return swap_chain_image_views_;
}

VkRenderPass SwapChain::GetRenderPass() { return render_pass_; }

VkExtent2D SwapChain::ChooseSwapExtent(
    const VkSurfaceCapabilitiesKHR& capabilities) {
  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    int width = 0;
    int height = 0;
    window_.GetFramebufferSize(width, height);

    VkExtent2D actual_extent{static_cast<uint32_t>(width),
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

VkSurfaceFormatKHR SwapChain::ChooseSwapSufaceFormat(
    const std::vector<VkSurfaceFormatKHR>& available_formats) {
  for (const auto& available : available_formats) {
    if (VK_FORMAT_B8G8R8_SRGB == available.format &&
        VK_COLOR_SPACE_SRGB_NONLINEAR_KHR == available.colorSpace) {
      return available;
    }
  }

  return available_formats[0];
}

VkPresentModeKHR SwapChain::ChooseSwapPresentMode(
    const std::vector<VkPresentModeKHR>& available_modes) {
  for (const auto& available : available_modes) {
    if (VK_PRESENT_MODE_MAILBOX_KHR == available) {
      std::clog << "----- Present Mode: Mailbox -----" << std::endl;
      return available;
    }
  }

  std::clog << "----- Present Mode: " << available_modes[0] << std::endl;
  return available_modes[0];
}

}  // namespace playground
