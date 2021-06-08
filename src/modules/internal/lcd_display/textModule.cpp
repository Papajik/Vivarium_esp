#include "textModule.h"

TextModule::TextModule() {}
TextModule::~TextModule() {}

void TextModule::addTextOutput(TextOutput *t)
{
    _textOutput = t;
}

void TextModule::printText(const std::vector<String> &texts, int delay)
{
    if (_textOutput != nullptr)
    {
        if (delay != 0)
        {
            _textOutput->setText(texts, delay);
        }
        else
        {
            _textOutput->setText(texts);
        }
    }
}