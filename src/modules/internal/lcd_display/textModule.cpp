/**
* @file textModule.cpp
* @author Michal Papaj (papaj.mich@gmail.com)
* @brief 
* @version 1.0
* @date 2021-10-03
* 
* @copyright Copyright (c) 2021
* 
*/
#include "textModule.h"

TextModule::TextModule() {}
TextModule::~TextModule() {}

void TextModule::addTextOutput(TextOutput *t)
{
    _textOutput = t;
}

void TextModule::printText(const std::vector<String> &texts, int duration)
{
    if (_textOutput != nullptr)
    {
        if (duration != 0)
        {
            _textOutput->setText(texts, duration);
        }
        else
        {
            _textOutput->setText(texts);
        }
    }
}