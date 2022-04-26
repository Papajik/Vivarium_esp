/**
* @file watchdog.h
* @author Michal Papaj (papaj.mich@gmail.com)
* @brief 
* @version 1.0
* @date 2021-09-20
* 
* @copyright Copyright (c) 2021
* 
*/

#ifndef V_WATCHDOG_H_
#define V_WATCHDOG_H_

class Vivarium;
typedef void (*callback)(void);

/**
* @brief Used to control freeze status of all "classState" class 
* 
*/
class Watchdog
{

public:
    /**
    * @brief Construct a new Watchdog object
    * y
    */
    Watchdog();

    /**
    * @brief 
    * 
    */
    void addVivarium(Vivarium *);

    /**
    * @brief Get the Callback object
    * 
    * @return callback 
    */
    callback getCallback();

    /**
    * @brief Detects task freeze and restarts ESP if so
    * 
    */
    void checkDeadlock();

private:
    Vivarium *_vivarium;
    unsigned long lastDelayTime = 0;
};

extern Watchdog watchdog;

#endif