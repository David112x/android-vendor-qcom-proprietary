/*
 * Copyright (c) 2019, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "xmlparser.h"
#include <libxml/parserInternals.h>

/* program to parse xml at compile time */

int parse_configs(char* filepath) {

    printf("Parsing config for %s\n", filepath);

    xmlDocPtr doc;
    xmlNodePtr curNode;
    xmlNodePtr curNodeTech;

    if(filepath == NULL) {
        printf("Invalid file name param\n");
        return -1;
    }

    doc = xmlReadFile(filepath, "UTF-8", XML_PARSE_RECOVER);
    if(NULL == doc) {
        printf("Document(%s) not parsed successfully.\n ", filepath);
        return -1;
    }

    curNode = xmlDocGetRootElement(doc);
    if(NULL == curNode) {
        printf("'%s' is empty document, please pass a valid xml. \n", filepath);
        xmlFreeDoc(doc);
        return -1;
    }

    if(xmlStrcmp(curNode->name, BAD_CAST "CapabilityStore")) {
        printf("document(%s) of the wrong type, root node != CapabilityStore \n", filepath);
        xmlFreeDoc(doc);
        return -1;
    }
    curNodeTech = curNode->xmlChildrenNode;
    //curNodeTech = curNodeTech->next->next;
    int count = 0;
    while(curNodeTech != NULL) {
       char * techName = (char *)xmlGetProp(curNodeTech, BAD_CAST "name");
       printf("value of techName %s", techName);
       if(curNodeTech->type != XML_TEXT_NODE){
           if(xmlStrcmp(curNodeTech->name, BAD_CAST "namespace")) {
               printf("document(%s) of the wrong type, child node != namespace , instead found %s \n", filepath, curNodeTech->name);
           }
           char * techConfig = (char*)xmlGetProp(curNodeTech, BAD_CAST "name");
       }
       curNodeTech = curNodeTech->next;
    }
    xmlFreeDoc(doc);
    return 0;
}

int main(int argc, char** argv) {
 return  parse_configs(argv[1]);
}
