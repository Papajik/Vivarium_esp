#ifndef _V_DEBUG_H_
#define _V_DEBUG_H_

class MemoryProvider;

namespace Debugger
{
    void setupDebug();
    void addProvider(MemoryProvider *);
}

#endif