#ifndef _V_DEBUG_H_
#define _V_DEBUG_H_

class MemoryProvider;
class Auth;
class WaterTempModule;
class Vivarium;
class WiFiProvider;
class FirebaseService;
class ModuleControl;

namespace Debugger
{
    void setupDebug();
    void addProvider(MemoryProvider *);
    void addModuleControl(ModuleControl *);
    void addAuth(Auth *);
    void addFirebase(FirebaseService *);
    void addWaterTemp(WaterTempModule *);
    void addVivarium(Vivarium *);
    void debugDelay();
}

#endif