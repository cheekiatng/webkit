#include "config.h"
#include "ThreadSpecific.h"

namespace WTF {

static __declspec(thread) HashMap<ThreadSpecificKey, ThreadSpecificData>* g_threadLocalHashMap = 0;

HashMap<ThreadSpecificKey, ThreadSpecificData>& threadLocalHashMap()
{
    if (!g_threadLocalHashMap)
        g_threadLocalHashMap = new HashMap<ThreadSpecificKey, ThreadSpecificData>();
    return *g_threadLocalHashMap;
}

void ThreadSpecificThreadExit()
{
    HashMap<ThreadSpecificKey, ThreadSpecificData>& map = threadLocalHashMap();
    HashMap<ThreadSpecificKey, ThreadSpecificData>::iterator i = map.begin();

    while (i != map.end()) {
        i->value.first(i->value.second);
    }
}

std::atomic<long>& tlsKeyCount()
{
    static std::atomic<long> count = 1;
    return count;
}

inline void threadSpecificKeyCreate(ThreadSpecificKey* key, Destructor destructor)
{
    *key = tlsKeyCount()++;
    threadLocalHashMap().set(*key, ThreadSpecificData(destructor, 0));
}

inline void threadSpecificKeyDelete(ThreadSpecificKey key)
{
    HashMap<ThreadSpecificKey, ThreadSpecificData>& map = threadLocalHashMap();
    map.remove(key);
}

inline void threadSpecificSet(ThreadSpecificKey key, void* value)
{
    HashMap<ThreadSpecificKey, ThreadSpecificData>& map = threadLocalHashMap();
    map.set(key, ThreadSpecificData(map.get(key).first, value));
}

inline void* threadSpecificGet(ThreadSpecificKey key)
{
    return threadLocalHashMap().get(key).second;
}

}
