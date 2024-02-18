#pragma once
#include <string>
#include <unordered_map>
#include <fstream>

std::unordered_map<std::string, int> parse_config_file(std::string file)
{
    const std::unordered_map<std::string, int> defaults = {
        {"WINDOW_RESOLUTION_X", 640},
        {"WINDOW_RESOLUTION_Y", 480},
        {"WINDOW_MODE", 0},
        {"SCALING_MODE", 0},
        {"VSYNC", 1},
        {"MAX_FPS", 60}
    };

    std::unordered_map<std::string, int> config = {};
    std::ifstream config_file {file};
    if(config_file.is_open())
    {
        std::string line;
        while(std::getline(config_file, line))
        {
            if(!line.empty())
            {
                if(line.front() != '/')
                {
                    std::string key = line.substr(0, line.find("="));
                    int value = std::stoi(line.substr(line.find("=")+1, line.length()));
                    config.insert(std::pair<std::string, int>(key, value));
                }
            }
        }
    }

    if(!(defaults.size() == config.size() && std::equal(defaults.begin(), defaults.end(), config.begin(), [](auto a, auto b){return a.first == b.first;})))
    {
        return defaults;
    }
    
    if(config.at("WINDOW_RESOLUTION_X") <= 0)
    {
        return defaults;
    }
    if(config.at("WINDOW_RESOLUTION_Y") <= 0)
    {
        return defaults;
    }
    if(config.at("WINDOW_MODE") != 0 && config.at("WINDOW_MODE") != 1 && config.at("WINDOW_MODE") != 2)
    {
        return defaults;
    }
    if(config.at("SCALING_MODE") != 0 && config.at("SCALING_MODE") != 1)
    {
        return defaults;
    }
    if(config.at("VSYNC") != 0 && config.at("VSYNC") != 1)
    {
        return defaults;
    }
    if(config.at("MAX_FPS") <= 0)
    {
        return defaults;
    }
    
    return config;
}