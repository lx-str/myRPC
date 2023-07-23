#include <tinyxml/tinyxml.h>
#include "rocket/common/config.h"

//读xml节点：节点名，父节点
#define READ_XML_NODE(name,parent)\
TiXmlElement* name##_node = parent->FirstChildElement(#name);\
    if(!name##_node){\
        printf("Start rocket server error,failed to read node [%s]\n",#name);\
        exit(0);\
    }\

//读取节点内容
#define READ_STR_FROM_XML_NODE(name,parent)\
TiXmlElement* name##_node = parent->FirstChildElement(#name); \
    if(!name##_node || !name##_node->GetText()){ \
        printf("Start rocket server error,failed to read node [%s]\n", #name);\
        exit(0); \
    } \
    std::string name##_str = std::string(name##_node->GetText());

namespace rocket{

//构造函数，读取配置文件
Config::Config(const char* xmlfile){
    TiXmlDocument* xml_document = new TiXmlDocument();
    
    bool rt = xml_document->LoadFile(xmlfile);
    if(!rt){
        printf("Start rocket server error,failed to read config file %s\n",xmlfile);
        exit(0);
    }

    READ_XML_NODE(root,xml_document);

    READ_XML_NODE(log,root_node);

    READ_STR_FROM_XML_NODE(log_level,log_node);

    m_log_level = log_level_str;

}

static Config* g_config = NULL;

Config* Config::GetGlobalConfig(){
    return g_config;
}

void Config::SetGlobalConfig(const char* xmlfile){
    if(!g_config){
        g_config = new Config(xmlfile);
    }
}

} // namespace rocket