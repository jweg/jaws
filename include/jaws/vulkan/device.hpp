#pragma once
#include "jaws/core.hpp"
#include "jaws/vfs/path.hpp"
#include "jaws/vulkan/fwd.hpp"
#include "jaws/vulkan/vulkan.hpp"
#include "jaws/vulkan/extension.hpp"
#include "jaws/util/misc.hpp"
#include "jaws/util/hashing.hpp"
#include "jaws/util/lru_cache.hpp"
#include "jaws/vulkan/sediment.hpp"

namespace jaws::vulkan {

class JAWS_API Device
{
public:
    struct CreateInfo
    {
        JAWS_NP_MEMBER3(uint32_t, gpu_group_index, 0);
        JAWS_NP_MEMBER2(ExtensionList, required_extensions);
        JAWS_NP_MEMBER2(ExtensionList, optional_extensions);
    };

    Device();
    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;
    ~Device();

    void create(Context*, const CreateInfo& = jaws::util::make_default<CreateInfo>());
    void destroy();


    VkInstance get_instance() const { return _vk_instance; }

    Context* get_context() const { return _context; }
    VkDevice get_device() const { return _device; }

    VkPhysicalDevice get_physical_device(uint32_t index = 0) const;

    enum class Queue
    {
        Graphics = 0,
        Present,
        Compute,
        Transfer,
        AsyncTransfer,
        AsyncCompute,
        ELEM_COUNT
    };
    VkQueue get_queue(Queue q);
    uint32_t get_queue_family(Queue q) const;

    ShaderPtr get_shader(const ShaderCreateInfo&);

    const VolkDeviceTable& vk() const { return _f; };

    Sediment* get_sediment() { return _sediment.get(); }

private:
    friend Context;
    Context* _context = nullptr;

    VkInstance _vk_instance;
    VkPhysicalDeviceGroupProperties _gpu_group;
    VkDevice _device = VK_NULL_HANDLE;

    VolkDeviceTable _f;

    VmaVulkanFunctions _vma_vulkan_functions;
    VmaAllocator _vma_allocator = VK_NULL_HANDLE;

    std::vector<VkQueue> _unique_queues;
    struct QueueInfo
    {
        uint32_t family_index = INVALID_INDEX;
        size_t unique_queue_index = 0;
    };
    std::array<QueueInfo, static_cast<size_t>(Queue::ELEM_COUNT)> _queue_infos;
    QueueInfo& get_queue_info(Queue q);
    const QueueInfo& get_queue_info(Queue q) const;

    std::unique_ptr<Sediment> _sediment;
    std::unique_ptr<ShaderSystem> _shader_system;
};

inline VkPhysicalDevice Device::get_physical_device(uint32_t index) const
{
    JAWS_ASSUME(index < _gpu_group.physicalDeviceCount);
    return _gpu_group.physicalDevices[index];
}

inline VkQueue Device::get_queue(Queue q)
{
    return _unique_queues[get_queue_info(q).unique_queue_index];
}

inline uint32_t Device::get_queue_family(Queue q) const
{
    return get_queue_info(q).family_index;
}

inline Device::QueueInfo& Device::get_queue_info(Queue q)
{
    JAWS_ASSUME(q != Queue::ELEM_COUNT);
    return _queue_infos[static_cast<size_t>(q)];
}

inline const Device::QueueInfo& Device::get_queue_info(Queue q) const
{
    JAWS_ASSUME(q != Queue::ELEM_COUNT);
    return _queue_infos[static_cast<size_t>(q)];
}

} // namespace jaws::vulkan