/*
 * Copyright (C) 2007, 2009 Apple Inc. All rights reserved.
 * Copyright (C) 2007 Justin Haygood (jhaygood@reaktix.com)
 * Copyright (C) 2011 Research In Motion Limited. All rights reserved.
 * Copyright (C) 2013 Igalia S.L.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer. 
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution. 
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "Threading.h"

#if USE(STDTHREAD)

#include "StdLibExtras.h"
#include "CurrentTime.h"
#include "ThreadFunctionInvocation.h"
#include <thread>
#include <wtf/OwnPtr.h>
#include <wtf/PassOwnPtr.h>
#include <wtf/WTFThreadData.h>

namespace WTF {

class StdThreadState {
    WTF_MAKE_FAST_ALLOCATED;
public:
    enum JoinableState {
        Joinable, // The default thread state. The thread can be joined on.

        Joined, // Somebody waited on this thread to exit and this thread finally exited. This state is here because there can be a 
                // period of time between when the thread exits (which causes pthread_join to return and the remainder of waitOnThreadCompletion to run) 
                // and when threadDidExit is called. We need threadDidExit to take charge and delete the thread data since there's 
                // nobody else to pick up the slack in this case (since waitOnThreadCompletion has already returned).

        Detached // The thread has been detached and can no longer be joined on. At this point, the thread must take care of cleaning up after itself.
    };

    // Currently all threads created by WTF start out as joinable.
    StdThreadState(std::thread *threadInstance)
        : m_joinableState(Joinable)
        , m_didExit(false)
        , m_threadInstance(threadInstance)
    {
        ASSERT(threadInstance);
        m_stdThreadID = threadInstance->get_id();
    }

    // Constructor when we don't have an std::thread, which would happen if we
    // didn't create the thread ourselves
    StdThreadState(std::thread::id threadID)
        : m_joinableState(Joinable)
        , m_didExit(false)
        , m_threadInstance(0)
        , m_stdThreadID(threadID)
    {
    }

    JoinableState joinableState() { return m_joinableState; }
    std::thread* threadInstance() { return m_threadInstance; }
    std::thread::id stdThreadID() { return m_stdThreadID; }
    void didBecomeDetached() { m_joinableState = Detached; }
    void didExit() { m_didExit = true; }
    void didJoin() { m_joinableState = Joined; }
    bool hasExited() { return m_didExit; }

private:
    JoinableState m_joinableState;
    bool m_didExit;
    std::thread* m_threadInstance;
    std::thread::id m_stdThreadID;
};

typedef HashMap<ThreadIdentifier, OwnPtr<StdThreadState> > ThreadMap;

static Mutex* atomicallyInitializedStaticMutex;

__declspec(thread) static ThreadIdentifier currentThreadID = 0;

static Mutex& threadMapMutex()
{
    DEFINE_STATIC_LOCAL(Mutex, mutex, ());
    return mutex;
}

static ThreadMap& threadMap()
{
    DEFINE_STATIC_LOCAL(ThreadMap, map, ());
    return map;
}

static ThreadIdentifier identifierByStdThreadID(const std::thread::id& id)
{
    MutexLocker locker(threadMapMutex());

    ThreadMap::iterator i = threadMap().begin();
    for (; i != threadMap().end(); ++i) {
        if ((i->value->stdThreadID() == id) && !i->value->hasExited())
            return i->key;
    }

    return 0;
}

static ThreadIdentifier establishIdentifierForStdThreadState(StdThreadState* threadState)
{
    ASSERT(threadState);
    ASSERT(!identifierByStdThreadID(threadState->stdThreadID()));
    MutexLocker locker(threadMapMutex());
    static ThreadIdentifier identifierCount = 1;
    threadMap().add(identifierCount, adoptPtr(threadState));
    return identifierCount++;
}

static ThreadIdentifier establishIdentifierForStdThread(std::thread* threadInstance)
{
    return establishIdentifierForStdThreadState(new StdThreadState(threadInstance));
}

static ThreadIdentifier establishIdentifierForStdThreadID(std::thread::id stdThreadID)
{
    return establishIdentifierForStdThreadState(new StdThreadState(stdThreadID));
}

static std::thread * stdThreadForIdentifierWithLockAlreadyHeld(ThreadIdentifier id)
{
    return threadMap().get(id)->threadInstance();
}

void initializeThreading()
{
    if (atomicallyInitializedStaticMutex)
        return;
    atomicallyInitializedStaticMutex = new Mutex;
    // StringImpl::empty() does not construct its static string in a threadsafe fashion,
    // so ensure it has been initialized from here.
    StringImpl::empty();
    threadMapMutex();
    /*
    WTF::double_conversion::initialize();
    initializeRandomNumberGenerator();
    StackStats::initialize();
    wtfThreadData();
    s_dtoaP5Mutex = new Mutex;
    initializeDates();
    */
}

void lockAtomicallyInitializedStaticMutex()
{
    ASSERT(atomicallyInitializedStaticMutex);
    atomicallyInitializedStaticMutex->lock();
}

void unlockAtomicallyInitializedStaticMutex()
{
    atomicallyInitializedStaticMutex->unlock();
}

static void* wtfThreadEntryPoint(void* param)
{
    // Balanced by .leakPtr() in createThreadInternal.
    OwnPtr<ThreadFunctionInvocation> invocation = adoptPtr(static_cast<ThreadFunctionInvocation*>(param));
    invocation->function(invocation->data);

    ThreadSpecificThreadExit();

    return 0;
}

ThreadIdentifier createThreadInternal(ThreadFunction entryPoint, void* data, const char*)
{
    OwnPtr<ThreadFunctionInvocation> invocation = adoptPtr(new ThreadFunctionInvocation(entryPoint, data));
    std::thread* threadInstance;
    try {
        threadInstance = new std::thread(wtfThreadEntryPoint, invocation.get());
    } catch(...) {
        LOG_ERROR("Failed to create std::thread at entry point %p with data %p", wtfThreadEntryPoint, invocation.get());
        return 0;
    }

    // Balanced by adoptPtr() in wtfThreadEntryPoint.
    ThreadFunctionInvocation* leakedInvocation = invocation.leakPtr();
    UNUSED_PARAM(leakedInvocation);

    return establishIdentifierForStdThread(threadInstance);
}

void initializeCurrentThreadInternal(const char* threadName)
{
    UNUSED_PARAM(threadName);

    currentThreadID = identifierByStdThreadID(std::this_thread::get_id());
    ASSERT(currentThreadID);
}

int waitForThreadCompletion(ThreadIdentifier threadID)
{
    std::thread* threadInstance;
    ASSERT(threadID);

    {
        // We don't want to lock across the call to join, since that can block our thread and cause deadlock.
        MutexLocker locker(threadMapMutex());
        threadInstance = stdThreadForIdentifierWithLockAlreadyHeld(threadID);
        ASSERT(threadInstance);
    }

    int joinResult = 0;
    try {
        threadInstance->join();
    } catch(...) {
        LOG_ERROR("ThreadIdentifier %u was unable to be joined. Error %d: %s", threadID);
        joinResult = 1;
    }

    MutexLocker locker(threadMapMutex());
    StdThreadState* state = threadMap().get(threadID);
    ASSERT(state);
    ASSERT(state->joinableState() == StdThreadState::Joinable);

    // The thread has already exited, so clean up after it.
    if (state->hasExited())
        threadMap().remove(threadID);
    // The thread hasn't exited yet, so don't clean anything up. Just signal
    // that we've already joined on it so that it will clean up after itself.
    else
        state->didJoin();

    return joinResult;
}

void detachThread(ThreadIdentifier threadID)
{
    ASSERT(threadID);

    MutexLocker locker(threadMapMutex());
    std::thread *threadInstance = stdThreadForIdentifierWithLockAlreadyHeld(threadID);
    ASSERT(threadInstance);

    try {
        threadInstance->detach();
    } catch(...) {
        LOG_ERROR("ThreadIdentifier %u was unable to be detached.", threadID);
    }

    StdThreadState* state = threadMap().get(threadID);
    ASSERT(state);
    if (state->hasExited())
        threadMap().remove(threadID);
    else
        threadMap().get(threadID)->didBecomeDetached();
}


void yield()
{
    std::this_thread::yield();
}

ThreadIdentifier currentThread()
{
    if (currentThreadID)
        return currentThreadID;

    // Not a WTF-created thread, and we won't have access to std::thread, but
    // the identifier can still be used for comparison purposes
    ThreadIdentifier id = identifierByStdThreadID(std::this_thread::get_id());
    if (!id) {
        id = establishIdentifierForStdThreadID(std::this_thread::get_id());
    }
    return id;
}

Mutex::Mutex()
    : m_mutex()
	, m_locker(m_mutex, std::defer_lock)
{
}


Mutex::~Mutex()
{
}

void Mutex::lock()
{
    try {
        m_mutex.lock();
    } catch(...) {
        LOG_ERROR("Failed to lock mutex.");
    }
}

bool Mutex::tryLock()
{
    return m_mutex.try_lock();
}

void Mutex::unlock()
{
    m_mutex.unlock();
}

ThreadCondition::ThreadCondition()
    try // std::condition_variable constructor could throw
       : m_condition()
{
} catch (...) {
    LOG_ERROR("Failed to create condition variable.");
}

ThreadCondition::~ThreadCondition()
{
}

void ThreadCondition::wait(Mutex& mutex)
{
	try {
		m_condition.wait(mutex.m_locker);
	} catch (...) {
        LOG_ERROR("Failed to wait for thread condition.");
	}
}

bool ThreadCondition::timedWait(Mutex& mutex, double absoluteTime)
{
    if (absoluteTime < currentTime())
        return false;

    if (absoluteTime > INT_MAX) {
        wait(mutex);
        return true;
    }

    //std::chrono::duration<double> waitDuration = currentTime() - absoluteTime;
    int durationMilliseconds = static_cast<int>((currentTime() - absoluteTime) * 1000);
    std::chrono::milliseconds waitDuration(durationMilliseconds);

    try {
        return m_condition.wait_for(mutex.m_locker, waitDuration) == std::cv_status::no_timeout;
    } catch (...) {
        LOG_ERROR("Failed when attempting a timed wait on condition.");
        return false;
    }
}

void ThreadCondition::signal()
{
    m_condition.notify_one();
}

void ThreadCondition::broadcast()
{
    m_condition.notify_all();
}

} // namespace WTF

#endif // USE(PTHREADS)
