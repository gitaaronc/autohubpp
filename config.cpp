#include <iostream>
#include <fstream>

#include "include/config.hpp"

namespace ace
{
    namespace utils
    {

        bool Config::Init(const std::string& file_name)
        {
            try {
                node_config_ = YAML::LoadFile(file_name);
                file_name_ = file_name;
                isLoaded_ = true;
            }
            catch (const std::exception& e) {
                std::cout << e.what() << "\n";
                isLoaded_ = false;
            }
            return isLoaded_;
        }

        YAML::Node Config::Get()
        {
            if (!isLoaded_)
                throw;
            return node_config_;
        }
        
        void Config::Save(){
            if (!isLoaded_)
                throw;
            std::ofstream ofs(file_name_);
            ofs << node_config_;
            ofs.close();
            
        }

    }// namespace utils
} // namespace config

