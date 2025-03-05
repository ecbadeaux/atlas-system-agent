#include "dcgm_agent.h"
#include "dcgm_structs.h"
#include "string.h"
#include "unistd.h"
#include <iostream>
#include <map>
#include <unordered_map>
#include <time.h>
#include <vector>
#include <optional>
#include <array>

enum class ErrorCode
{
    SUCCESS = 0,
    INITIALIZATION_FAILED = 1,
    EMBEDDED_MODE_FAILED_TO_START = 2,
    FAILED_TO_GATHER_NUMBER_OF_DEVICES = 3,
    FAILED_TO_CREATE_FIELD_GROUP = 4,
    FAILED_TO_WATCH_FIELDS = 5,
    FAILED_TO_GET_LATEST_VALUES = 6,
    NO_DEVICES_TO_PROFILE = 7,
};

namespace detail 
{
    template <typename Reg>
    inline auto gauge(Reg* registry, const char* name, unsigned gpu, const char* id = nullptr)
    {
      auto tags = spectator::Tags{{"gpu", fmt::format("gpu-{}", gpu)}};
      if (id != nullptr) 
      {
        tags.add("id", id);
      }
      return registry->GetGauge(name, tags);
    }

    template <typename Reg>
    inline auto counter(Reg* registry, const char* name, unsigned gpu, const char* id = nullptr)
    {
        auto tags = spectator::Tags{{"gpu", fmt::format("gpu-{}", gpu)}};
        if (id != nullptr) 
        {
            tags.add("id", id);
        }
        return registry->GetMonotonicCounter(name, tags);
    }
}

template <typename Reg = atlasagent::TaggingRegistry>
class GpuMetricsDCGM
{
private:
    Reg* registry_;

    static constexpr std::array<unsigned short, 13> fieldIds{
        155,
        150,
        1001,
        1002,
        1003,
        1004,
        1005,
        1007,
        1008,
        1009,
        1010,
        1011,
        1012,
    };

public:
    GpuMetricsDCGM(Reg* registry) : registry_{registry} {};

    ~GpuMetricsDCGM() {};

    // Abide by the C++ rule of 5
    GpuMetricsDCGM(const GpuMetricsDCGM &other) = delete;
    GpuMetricsDCGM &operator=(const GpuMetricsDCGM &other) = delete;
    GpuMetricsDCGM(GpuMetricsDCGM &&other) noexcept = delete;
    GpuMetricsDCGM &operator=(GpuMetricsDCGM &&other) noexcept = delete;

    void Driver();
};