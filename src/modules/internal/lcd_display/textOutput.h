/**
* @file textOutput.h
* @author Michal Papaj (papaj.mich@gmail.com)
* @brief 
* @version 1.0
* @date 2021-10-03
* 
* @copyright Copyright (c) 2021
* 
*/
#ifndef TEXT_OUTPUT_H
#define TEXT_OUTPUT_H

#include <vector>

#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

class TextOutput
{
public:
    TextOutput() {}
    ~TextOutput() {}

    void setText(const std::vector<String> &texts, int delay);

    void checkLockedTextDelay();

    void setText(const std::vector<String> &texts);
    void unlockText();

protected:
    unsigned long _time_of_display_start = 0;
    int _delay = 0;
    void lockText();
    virtual void onTextUnlocked() = 0;
    virtual void displayText(const std::vector<String> &) = 0;
    bool _textLocked = false;
};

#endif