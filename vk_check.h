#define VK_CHECK(call)                \
    do                                \
    {                                 \
        VkResult result_ call;        \
        assert(result_ == VK_SUCCESS) \
    } while (0)