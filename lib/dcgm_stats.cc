#include "dcgm_stats.h"

#include "absl/strings/str_split.h"

#include <optional>
#include <iostream>
#include "util.h"

void printTokens(std::vector<std::string> tokens)
{
    for (unsigned int i = 0; i < tokens.size(); i++)
    {
        std::cout << tokens.at(i) << " ";
    }
    std::cout << std::endl;
}

bool ParseLines(const std::vector<std::string> &lines, std::map<int, std::vector<double>> &dataMap) try
{   
    std::cout << "TokensCheck" << std::endl;
    for (unsigned int i = 2; i < lines.size(); i++)
    {
        auto line = lines.at(i);
        std::vector<std::string> tokens = absl::StrSplit(line, ' ', absl::SkipWhitespace());

        printTokens(tokens);

        // if (tokens.size() != 5)
        // {
        //     atlasagent::Logger()->error("CSV data not in valid form");
        //     return false;
        // }
        int gpuId = std::stoi(tokens[1]);
        std::cout << "GpuID:" << gpuId << std::endl;
        for (unsigned int j = 2; j < tokens.size(); j++)
        {
            std::cout << "CurrentToken:" << tokens[j] << std::endl;
            dataMap[gpuId].push_back(std::stod(tokens[j]));
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

void DCGMExecutor<Reg>::UpdateMetrics(std::map<int, std::vector<DataLine>> &dataMap)
{
    for (const auto& [gpuId, dataLines] : dataMap) 
    {

        std::cout << "Updating Registry Metrics for GPU ID: " << gpuId << std::endl;
        for (const auto& dataLine : dataLines)
        {
            switch (dataLine.fieldId)
            {
            case 0:
                detail::gauge(registry_, "gpu.dcgm.powerUsage", gpuId)->Set(dataLine.value);
                break;
            case 1:
                detail::gauge(registry_, "gpu.dcgm.deviceTemp", gpuId)->Set(dataLine.value);
                break;
            case 2:
                detail::gauge(registry_, "gpu.dcgm.sm", gpuId, "activity")->Set(dataLine.value);
                break;
            case 3:
                detail::gauge(registry_, "gpu.dcgm.sm", gpuId, "occupancy")->Set(dataLine.value);
                break;
            case 4:
                detail::gauge(registry_, "gpu.dcgm.tensorCoresUtilization", gpuId)->Set(dataLine.value);
                break;
            case 5:
                detail::gauge(registry_, "gpu.dcgm.memoryBandwidthUtilization", gpuId)->Set(dataLine.value);
                break;
            case 6:
                detail::gauge(registry_, "gpu.dcgm.pipeUtilization", gpuId, "fp32")->Set(dataLine.value);
                break;
            case 7:
                detail::gauge(registry_, "gpu.dcgm.pipeUtilization", gpuId, "fp16")->Set(dataLine.value);
                break;
            case 8:
                detail::counter(registry_, "gpu.dcgm.pcie.bytes", gpuId, "out")->Set(dataLine.value);
                break;
            case 9:
                detail::counter(registry_, "gpu.dcgm.pcie.bytes", gpuId, "in")->Set(dataLine.value);
                break;
            case 10:
                detail::counter(registry_, "gpu.dcgm.nvlink.bytes", gpuId, "in")->Set(dataLine.value);
                break;
            case 11:
                detail::counter(registry_, "gpu.dcgm.nvlink.bytes", gpuId, "out")->Set(dataLine.value);
                break;
            case 12:
                detail::gauge(registry_, "gpu.dcgm.graphicsEngineActivity", gpuId)->Set(dataLine.value);
                break;
            default:
                std::cout << "Error Unknown field type.";
                break;
            }
        }
    }
}

void GpuMetricsDCGM::Driver()
{

    // Path to the binary in your CMake build folder
    const char* binaryPath = "/usr/bin/dcgmi dmon -e 1001,1002,1003,1004,1005,1007,1008,1009,1010,1011,1012 -c 1";

    auto lines = atlasagent::read_output_lines(binaryPath, 50000);

    std::cout << "Lines Check" <<std::endl;
    std::cout << "Lines Size" << lines.size() << std::endl;
    for(unsigned int i = 0; i < lines.size(); i++)
    {
        std::cout << lines[i] << std::endl;
    }

    std::map<int, std::vector<double>> dataMap;

    ParseLines(lines, dataMap);

    std::cout << "MapCheck" << std::endl;

    PrintDataMap(dataMap);

    return;
}