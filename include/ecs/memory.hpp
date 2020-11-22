#ifndef ECS_STD_MEMORY
#define ECS_STD_MEMORY

#include <utility>
#include <memory>

namespace ecs {

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

}

#endif