
/**
* @file baseAlarm-impl.cpp
* @author Michal Papaj (papaj.mich@gmail.com)
* @brief 
* @version 1.0
* @date 2021-10-24
* 
* @copyright Copyright (c) 2021
* 
*/
#include "baseAlarm.h"
#include "baseAlarm.cpp"

template struct PayloadTrigger<double>;
template struct PayloadTrigger<uint32_t>;
template struct PayloadTrigger<int>;
template class BaseAlarm<Trigger>;
template class BaseAlarm<PayloadTrigger<double>>;
template class BaseAlarm<PayloadTrigger<uint32_t>>;
template class BaseAlarm<PayloadTrigger<int>>;