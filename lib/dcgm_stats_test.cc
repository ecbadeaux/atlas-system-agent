#include "dcgm_stats.h"
#include "measurement_utils.h"

#include <gtest/gtest.h>

using Registry = spectator::TestRegistry;

class DCGMTest : public GpuMetricsDCGM<Registry> {
public:
    explicit DCGMTest(Registry* registry) : GpuMetricsDCGM<Registry>(registry) {}

    bool GatherMetrics2()
    {
        return GpuMetricsDCGM<Registry>::GatherMetrics();
    }
};

TEST(EverettB, EverettB) 
{
    Registry registry;
    DCGMTest dcgmGPU(&registry);
    if (dcgmGPU.GatherMetrics2() == false)
    {
        std::cout << "failure" << std::endl;
    }
}
