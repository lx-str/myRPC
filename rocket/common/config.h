#ifndef ROCKET_COMMON_CONFIG_H
#define ROCKET_COMMON_CONFIG_H  

#include<map>
#include<string>
namespace rocket{

class Config{
public:
    Config(const char* xmlfile);

    
    static Config* GetGlobalConfig();
    static void SetGlobalConfig(const char* xmlfile);
    std::string m_log_level;

private:
    
};



} // namespace rocket

#endif
