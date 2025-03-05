#include "dcgm_stats.h"

#include "absl/strings/str_split.h"

#include <optional>
#include <iostream>
#include "util.h"

template class GpuMetricsDCGM<atlasagent::TaggingRegistry>;
template class GpuMetricsDCGM<spectator::TestRegistry>;

bool IsServiceRunning(const char* serviceName)
{
    std::string command = "systemctl is-active --quiet " + std::string(serviceName);
    int returnCode = system(command.c_str());
    return returnCode == 0;
}

bool ParseLines(const std::vector<std::string> &lines, std::map<int, std::vector<double>> &dataMap) try
{   
    for (unsigned int i = DCGMConstants::DataStartLine; i < lines.size(); i++)
    {
        auto line = lines.at(i);
        std::vector<std::string> tokens = absl::StrSplit(line, ' ', absl::SkipWhitespace());

        if (tokens.size() != DCGMConstants::ExpectedCountOfTokens)
        {
            return false;
        }
        
        auto gpuId = std::stoi(tokens.at(DCGMConstants::GPUIdTokenIndex));
        for (unsigned int j = DCGMConstants::DataStartToken; j < tokens.size(); j++)
        {
            dataMap[gpuId].push_back(std::stod(tokens[j]));
        }

        if (dataMap[gpuId].size() != DCGMConstants::ExpectedCountOfProfileValues)
        {
            return false;
        }
    }
    return true;
}
catch(const std::exception& e)
{
    atlasagent::Logger()->error("Exception thrown in ParseLines:{}", e.what());
    return false;
}

void PrintDataMap(const std::map<int, std::vector<double>> &dataMap)
{

    // Create an ofstream to open the file for output

    // std::ofstream outFile("/opt/output.txt", std::ios::app);

    // if (false == outFile.is_open())
    // {
    //     std::cout << "Could not open log folder" << std::endl;
    // }

    // // Save the original standard output stream (optional)
    // std::streambuf* orig_cout_stream = std::cout.rdbuf();

    // // Redirect std::cout to the file
    // std::cout.rdbuf(outFile.rdbuf());

    // Print the result (for debugging or verification)
    for (const auto& [gpuId, dataLines] : dataMap)
    {
        std::cout << "GPU ID: " << gpuId << std::endl;
        for (const auto& dataLine : dataLines) 
        {
            std::cout << dataLine << " ";
        }
        std::cout << std::endl;
    }

    // Restore the original std::cout
    //std::cout.rdbuf(orig_cout_stream);
}

inline std::vector<std::string> ExecuteDCGMI()
{
    static const auto command = std::string(DCGMConstants::dcgmiPath) + " " + std::string(DCGMConstants::dcgmiArgs);
    return atlasagent::read_output_lines(command.data(), 5000);
}

template <class Reg>
bool GpuMetricsDCGM<Reg>::UpdateMetrics(std::map<int, std::vector<double>> &dataMap)
{
    if (this->registry_ == nullptr)
    {
        return false;
    }
    for (const auto& [gpuId, data] : dataMap) 
    {

        std::cout << "Updating Registry Metrics for GPU ID: " << gpuId << std::endl;
        for (unsigned int i = 0; i < data.size(); i++)
        {
            double value = data.at(i);
            switch (i)
            {
            case 0:
                detail::gauge(registry_, "gpu.dcgm.deviceTemp", gpuId)->Set(value);
                break;
            case 1:
                detail::gauge(registry_, "gpu.dcgm.powerUsage", gpuId)->Set(value);
                break;
            case 2:
                detail::gauge(registry_, "gpu.dcgm.graphicsEngineActivity", gpuId)->Set(value);
                break;
            case 3:
                detail::gauge(registry_, "gpu.dcgm.sm", gpuId, "activity")->Set(value);
                break;
            case 4:
                detail::gauge(registry_, "gpu.dcgm.sm", gpuId, "occupancy")->Set(value);
                break;
            case 5:
                detail::gauge(registry_, "gpu.dcgm.tensorCoresUtilization", gpuId)->Set(value);
                break;
            case 6:
                detail::gauge(registry_, "gpu.dcgm.memoryBandwidthUtilization", gpuId)->Set(value);
                break;
            case 7:
                detail::gauge(registry_, "gpu.dcgm.pipeUtilization", gpuId, "fp32")->Set(value);
                break;
            case 8:
                detail::gauge(registry_, "gpu.dcgm.pipeUtilization", gpuId, "fp16")->Set(value);
                break;
            case 9:
                detail::counter(registry_, "gpu.dcgm.pcie.bytes", gpuId, "out")->Set(value);
                break;
            case 10:
                detail::counter(registry_, "gpu.dcgm.pcie.bytes", gpuId, "in")->Set(value);
                break;
            case 11:
                detail::counter(registry_, "gpu.dcgm.nvlink.bytes", gpuId, "out")->Set(value);
                break;
            case 12:
                detail::counter(registry_, "gpu.dcgm.nvlink.bytes", gpuId, "in")->Set(value);
                break;
            default:
                std::cout << "Error Unknown field type.";
                break;
            }
        }
    }
    return true;
}

template<class Reg>
bool GpuMetricsDCGM<Reg>::GatherMetrics()
{
    auto lines = ExecuteDCGMI();

    std::map<int, std::vector<double>> dataMap;

    if (false == ParseLines(lines, dataMap))
    {
        return false;
    }

    PrintDataMap(dataMap);

    if (false == UpdateMetrics(dataMap))
    {
        return false;
    }

    return true;
}