#ifndef _OUTLET_CONTROLER_H_
#define _OUTLET_CONTROLER_H_

#define O1 0
#define O2 1
class OutletController
{
public:
    OutletController();
    void begin();

    void setOutlet(int, bool);
    bool isSocketOn(int);

private:
    bool _sockets[2];
};

extern OutletController outletController;

#endif