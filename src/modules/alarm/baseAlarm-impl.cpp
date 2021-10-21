
#include "baseAlarm.h"
#include "baseAlarm.cpp"

template struct PayloadTrigger<double>;
template struct PayloadTrigger<uint32_t>;
template class BaseAlarm<Trigger>;
template class BaseAlarm<PayloadTrigger<double>>;
template class BaseAlarm<PayloadTrigger<uint32_t>>;
