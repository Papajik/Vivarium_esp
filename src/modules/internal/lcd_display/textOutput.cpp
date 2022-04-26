/**
* @file textOutput.cpp
* @author Michal Papaj (papaj.mich@gmail.com)
* @brief 
* @version 1.0
* @date 2021-10-03
* 
* @copyright Copyright (c) 2021
* 
*/

#include "textOutput.h"

void TextOutput::setText(const std::vector<String> &texts, int delay)
{
    _time_of_show = millis();
    _delay = delay;
    setText(texts);
}

void TextOutput::checkLockedTextDelay()
{
    if (!_textLocked || _delay == 0)
        return;
    if (millis() > _time_of_show + _delay)
    {
        _delay = 0;
        _time_of_show = 0;
        unlockText();
    }
}

void TextOutput::setText(const std::vector<String> &texts)
{
    printlnA("Set text: " + String(texts.size()));

    lockText();
    displayText(texts);
}
void TextOutput::unlockText()
{
    _textLocked = false;
    onTextUnlocked();
}

void TextOutput::lockText()
{
    printlnA("Lock text");
    _textLocked = true;
}