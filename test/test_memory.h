#line 2 "test_memory.h"
#include <AUnitVerbose.h>
#include "../src/memory/memory_provider_internal.h"

struct TMemory
{
    int t = 0;
};

using namespace aunit;
class TestMemoryOnce : public TestOnce
{
protected:
    void setup() override
    {
        memoryProvider = new MemoryProviderInternal();
        memoryProvider->init("testing");
        memoryProvider->factoryReset();
    }

    void teardown() override
    {
        delete memoryProvider;
    }

public:
    MemoryProvider *memoryProvider;
};

testF(TestMemoryOnce, memory_initialize_write_count)
{
    assertEqual(memoryProvider->getWriteCount(), 0);
}

testF(TestMemoryOnce, memory_write_test)
{

    assertEqual(memoryProvider->getWriteCount(), 0);
    memoryProvider->saveBool("b1", true);
    assertEqual(memoryProvider->getWriteCount(), 1);
}

testF(TestMemoryOnce, memory_write_same_key)
{

    assertEqual(memoryProvider->getWriteCount(), 0);
    assertEqual(memoryProvider->getBytesWritten(), 0);
    memoryProvider->saveBool("b1", true);
    assertEqual(memoryProvider->getWriteCount(), 1);
    assertEqual(memoryProvider->getBytesWritten(), (int)sizeof(bool));
    memoryProvider->saveBool("b1", true);
    assertEqual(memoryProvider->getWriteCount(), 1);
    assertEqual(memoryProvider->getBytesWritten(), (int)sizeof(bool));
    memoryProvider->saveBool("b1", false);
    assertEqual(memoryProvider->getWriteCount(), 2);
    assertEqual(memoryProvider->getBytesWritten(), 2 * (int)sizeof(bool));
}

testF(TestMemoryOnce, memory_write_count)
{
    int size = 0;
    assertEqual(memoryProvider->getWriteCount(), 0);
    assertEqual(memoryProvider->getBytesWritten(), 0);
    memoryProvider->saveBool("b1", true);
    size += (int)sizeof(bool);
    assertEqual(memoryProvider->getWriteCount(), 1);
    assertEqual(memoryProvider->getBytesWritten(), size);

    memoryProvider->saveFloat("f1", 1.225);
    size += (int)sizeof(float);
    assertEqual(memoryProvider->getWriteCount(), 2);
    assertEqual(memoryProvider->getBytesWritten(), size);

    memoryProvider->saveDouble("d1", 1.225);
    size += (int)sizeof(double);
    assertEqual(memoryProvider->getWriteCount(), 3);
    assertEqual(memoryProvider->getBytesWritten(), size);

    memoryProvider->saveInt("i1", 1);
    size += (int)sizeof(int);
    assertEqual(memoryProvider->getWriteCount(), 4);
    assertEqual(memoryProvider->getBytesWritten(), size);

    TMemory t;
    t.t = 2;

    memoryProvider->saveStruct("t1", &t, sizeof(TMemory));
    size += (int)sizeof(TMemory);
    assertEqual(memoryProvider->getWriteCount(), 5);
    assertEqual(memoryProvider->getBytesWritten(), size);

    memoryProvider->saveString("d1", "t");
    size += 5;
    assertEqual(memoryProvider->getWriteCount(), 6);
    assertEqual(memoryProvider->getBytesWritten(), size);
}

testF(TestMemoryOnce, bool_write_read)
{
    memoryProvider->saveBool("b1", true);
    bool b = memoryProvider->loadBool("b1", false);
    assertTrue(b);
}

testF(TestMemoryOnce, bool_default)
{
    bool b = memoryProvider->loadBool("u1", true);
    assertTrue(b);
    b = memoryProvider->loadBool("u1", false);
    assertFalse(b);
}

testF(TestMemoryOnce, float_write_read)
{
    float saved = 1.24;
    memoryProvider->saveFloat("f1", saved);
    float f = memoryProvider->loadFloat("f1", 1.22);
    assertEqual(f, saved);
}

testF(TestMemoryOnce, float_default)
{
    float saved = 1.22;
    float f = memoryProvider->loadFloat("u1", saved);
    assertEqual(f, saved);
    saved = 2.25;
    f = memoryProvider->loadFloat("u1", saved);
    assertEqual(f, saved);
}

testF(TestMemoryOnce, int_write_read)
{
    memoryProvider->saveInt("i1", 95);
    int i = memoryProvider->loadInt("i1", 1);
    assertEqual(i, 95);
}

testF(TestMemoryOnce, int_default)
{
    int i = memoryProvider->loadInt("u1", 95);
    assertEqual(i, 95);
    i = memoryProvider->loadInt("u1", 77);
    assertEqual(i, 77);
}

testF(TestMemoryOnce, string_write_read)
{
    memoryProvider->saveString("s1", "test");
    String s = memoryProvider->loadString("s1", "default");
    assertEqual(s, "test");
}

testF(TestMemoryOnce, string_default)
{
    String s = memoryProvider->loadString("s1", "default_1");
    assertEqual(s, "default_1");
    s = memoryProvider->loadString("s1", "default_2");
    assertEqual(s, "default_2");
}

testF(TestMemoryOnce, struct_write_read)
{
    TMemory t;
    t.t = 2222;

    memoryProvider->saveStruct("t1", &t, sizeof(TMemory));
    TMemory t2;
    assertTrue(memoryProvider->loadStruct("t1", &t2, sizeof(TMemory)));
    assertEqual(t.t, t2.t);
}

testF(TestMemoryOnce, struct_unknown)
{
    TMemory t;
    assertFalse(memoryProvider->loadStruct("u1", &t, sizeof(TMemory)));
}

testF(TestMemoryOnce, struct_wrong_size)
{
    TMemory t;
    t.t = 222;
    memoryProvider->saveStruct("t1", &t, sizeof(TMemory));
    assertFalse(memoryProvider->loadStruct("t1", &t, sizeof(TMemory) + 2));
}
