#ifndef _V_DEBUG_H_
#define _V_DEBUG_H_

class MemoryProvider;
class Auth;
class WaterTempModule;
class Vivarium;

namespace Debugger
{
    void setupDebug();
    void addProvider(MemoryProvider *);
    void addAuth(Auth *);
    void addWaterTemp(WaterTempModule *);
    void addVivarium(Vivarium *);
    void debugDelay();
}

#endif