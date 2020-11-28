
#include <fstream>
#include <iostream>
#ifdef _WIN32
    #include <direct.h>
#else
    #include <unistd.h>
#endif

#include "ConfigFileHelper.h"
#include "OPSConfigRepository.h"

void setup_alt_config(std::string const cfg_rel_ops4)
{
    // First, if CWD contains an ops_config.xml, use it
    bool exist = false;
    {
        std::ifstream inStream("ops_config.xml");
        exist = inStream.is_open();
    }
    if (exist) {
        std::cout << "Using config file in CWD\n";
        // not needed (default behaviour): ops::OPSConfigRepository::Instance()->Add("ops_config.xml")

    } else {
        // Second, check if we are in the OPS4 Examples hierarchy and then use the given cfg
        std::string path;
        char* buffer = nullptr;
#ifdef _WIN32
        buffer = _getcwd(nullptr, 0);
#else
        buffer = getcwd(nullptr, 0);
#endif
        if (buffer != nullptr) {
            path = buffer;
            free(buffer);
        }
        const auto pos = path.find("Examples");
        if (pos != path.npos) {
            path = path.substr(0, pos);
            path += cfg_rel_ops4;
            std::cout << "Using config file: " << path << "\n";
            ops::OPSConfigRepository::Instance()->Add(path);
        }
    }
}
