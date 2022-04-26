/**
* @file textModule.h
* @author Michal Papaj (papaj.mich@gmail.com)
* @brief 
* @version 1.0
* @date 2021-10-03
* 
* @copyright Copyright (c) 2021
* 
*/
#ifndef _TEXT_MODULE_H
#define _TEXT_MODULE_H

#include <vector>
#include <WString.h>

#include "textOutput.h"

class TextModule
{
public:
    TextModule();
    virtual ~TextModule();
    virtual std::vector<String> getText() = 0;
    void printText(const std::vector<String> &texts, int delay = 0);
    void addTextOutput(TextOutput *);

private:
    TextOutput *_textOutput = nullptr;
};

#endif