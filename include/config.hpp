/* 
 * File:   config.hpp
 * Author: aaronc
 *
 * Created on July 7, 2016, 7:07 PM
 */

#ifndef CONFIG_HPP
#define	CONFIG_HPP

#include <memory>
#include <iostream>
#include <yaml-cpp/yaml.h>

namespace ace {
namespace config {
    
extern int command_delay; // to to wait between sending commands
extern int worker_threads; // the number of worker threads
}
namespace utils {
    class Config {
    public:
        static Config&
        Instance() {
            static Config m_pInstance;
            return m_pInstance;
        }
        bool Init(const std::string& file_name);
        YAML::Node Get();
        void Save();
        
    private:
        bool isLoaded_;
        std::string file_name_;
        YAML::Node node_config_;
        Config() : isLoaded_(false){
        }
        
        ~Config(){
        }
    };
} // namespace utils

} // namespace ace

#endif	/* CONFIG_HPP */

