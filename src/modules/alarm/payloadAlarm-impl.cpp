/**
* @file payloadAlarm-impl.cpp
* @author Michal Papaj (papaj.mich@gmail.com)
* @brief Forward declaration of abstract template PayloadAlarm class. Necessary for compiler to handle compilation. Ł
* @date 2021-10-24
* 
* @copyright Copyright (c) 2021
* 
*/
#include "payloadAlarm.cpp"

template class PayloadAlarm<double>;
template class PayloadAlarm<uint32_t>;
template class PayloadAlarm<int>;