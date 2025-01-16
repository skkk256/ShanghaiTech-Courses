#pragma once
#include <string>
#include <iostream>
#include <fstream>
#ifndef NO_OMP
#include <omp.h>
#endif
#include "utils.hpp"


#ifndef NO_OMP
#define CONF_DEFUALT_NUM_THREADS omp_get_max_threads()
#else
#define CONF_DEFUALT_NUM_THREADS 1
#endif
#define CONF_DEFAULT_OUTPUT_FILE "output.png"
#define CONF_DEFAULT_OUTPUT_RESOLUTION 512
#define CONF_DEFAULT_SCENE_PREFAB 1
#define CONF_DEFAULT_STEP_LEN 0.01


struct Config
{
    /* The number of using threads */
    int num_threads;
    /* Output file */
    std::string output_file;
    /* Resolution of output */
    int output_resolution;
    /* ID of scene prefabs: 1 2 3 */
    int scene_prefab;
    /* Ray marching step length */
    float step_len;
    

    Config()
        : num_threads(CONF_DEFUALT_NUM_THREADS)
        , output_file(CONF_DEFAULT_OUTPUT_FILE)
        , output_resolution(CONF_DEFAULT_OUTPUT_RESOLUTION)
        , scene_prefab(CONF_DEFAULT_SCENE_PREFAB)
        , step_len(CONF_DEFAULT_STEP_LEN)
    {
    }

    bool parseConfigFile(std::string file_path)
    {
        std::string token;
        std::ifstream file(file_path);

        std::string line;
        int line_num = 1;
        while (std::getline(file, line))
        {
            StrUtils::trim(line);

            if (line.find("#") == 0 || line.empty())
            {
                continue;
            }

            std::stringstream line_stream(line);
            std::string key, val;
            line_stream >> key;
            if (!(line_stream >> val))
            {
                std::cout << "Config: Wrong syntax at " << file_path << ":" << line_num << ". Value field not found." << std::endl;
                return false;
            }

            if (key == "output_resolution")
            {
                output_resolution = std::stoi(val);
            }
            else if (key == "output_file")
            {
                output_file = val;
            }
            else if (key == "scene_prefab")
            {
                scene_prefab = std::stoi(val);
            }
            else if (key == "num_threads")
            {
                num_threads = std::stoi(val);
            }
            else if (key == "step_len")
            {
                step_len = std::stof(val);
            }

            if (line_stream >> key)
            {
                std::cout << "Config: Wrong syntax at " << file_path << ":" << line_num << ". Multiple values are not supported." << std::endl;
                return false;
            }

            ++line_num;
        }

        return true;
    }

    void printConfig()
    {
        std::cout << "=====================" << std::endl;
#ifndef NO_OMP
        std::cout << "max number of threads: " << omp_get_max_threads() << std::endl;
        omp_set_num_threads(num_threads);
#pragma omp parallel
        {
#pragma omp single
            {
                std::cout << "number of using threads: " << omp_get_num_threads() << std::endl;
            }
        }
#endif
        std::cout << "output file: " << output_file << std::endl;
        std::cout << "output resolution: " << output_resolution << " x " << output_resolution << std::endl;
        std::cout << "step length: " << step_len << std::endl;
        std::cout << "scene prefab: " << scene_prefab << std::endl;
        std::cout << "=====================" << std::endl;
        std::cout << std::endl;
    }

};
