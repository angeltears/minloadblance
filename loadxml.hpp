//
// Created by solitude on 18-6-10.
//

#ifndef MINLOADBLANCE_LOADXML_HPP
#define MINLOADBLANCE_LOADXML_HPP
#include <vector>
#include <cstdlib>
#include <memory>
#include <cassert>
#include "xml/tinyxml2.h"
#include "log.hpp"
#include "mgr.h"
using std::vector;
using std::shared_ptr;
using namespace tinyxml2;
void load_xml(char *file_name, vector<host>* balance_srv, vector<host>* logical_srv)
{

    if (file_name == nullptr)
    {
        log(LOG_ERR, __FILE__, __LINE__, "XML name is null" );
        exit(EXIT_FAILURE);
    }
    shared_ptr<XMLDocument> doc {new XMLDocument()};
    if (doc->LoadFile("config.xml") == XML_ERROR_FILE_NOT_FOUND)
    {
        log(LOG_ERR, __FILE__, __LINE__, "XML FILE is not exist" );
        exit(EXIT_FAILURE);
    }
    XMLElement* root = doc->RootElement();
    XMLElement* ser = root->FirstChildElement();
    while (ser != nullptr)
    {
        host tmp;
        memset(&tmp, 0, sizeof(tmp));
        const XMLElement* nameAttr = ser->FirstChildElement();
        assert(nameAttr != nullptr);
        if (strcmp(nameAttr->Name(),"host") == 0)
        {
            memcpy(tmp.m_hostname, nameAttr->GetText(), strlen(nameAttr->GetText()));
            nameAttr = nameAttr->NextSiblingElement();
            if (strcmp(nameAttr->Name(),"port") == 0)
            {
                tmp.m_port = atoi(nameAttr->GetText());
                balance_srv->push_back(tmp);
            }
            else
            {
                log(LOG_ERR, __FILE__, __LINE__, "XML FILE read error" );
                exit(EXIT_FAILURE);
            }
        }
        else if(strcmp(nameAttr->Name(),"name") == 0)
        {
            memcpy(tmp.m_hostname, nameAttr->GetText(), strlen(nameAttr->GetText()));
            nameAttr = nameAttr->NextSiblingElement();
            while(nameAttr != nullptr)
            {
                if (strcmp(nameAttr->Name(), "port") == 0)
                {
                    tmp.m_port = atoi(nameAttr->GetText());
                }
                else if (strcmp(nameAttr->Name(), "conns") == 0)
                {
                    tmp.m_connect =  atoi(nameAttr->GetText());
                }
                else
                {
                    log(LOG_ERR, __FILE__, __LINE__, "XML FILE read error" );
                    exit(EXIT_FAILURE);
                }
                nameAttr = nameAttr->NextSiblingElement();
            }
            logical_srv->push_back(tmp);
        }
        else
        {
            log(LOG_ERR, __FILE__, __LINE__, "XML FILE read error" );
            exit(EXIT_FAILURE);
        }
        ser = ser->NextSiblingElement();
    }

}
#endif //MINLOADBLANCE_LOADXML_HPP
