
#include "baseAlarm.h"
#include "baseAlarm.cpp"

template struct PayloadTrigger<double>;
template class BaseAlarm<Trigger>;
template class BaseAlarm<PayloadTrigger<double>>;