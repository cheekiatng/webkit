/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
 * FIXME: add copyright
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */


#include "config.h"
#include "JSManagedValuecpp.h"


#include "APICast.h"
#include "Heap.h"
#include "Weak.h"
#include "WeakHandleOwner.h"
#include "Operations.h"

class JSManagedValueHandleOwner : public JSC::WeakHandleOwner {
public:
    virtual void finalize(JSC::Handle<JSC::Unknown>, void* context);
    virtual bool isReachableFromOpaqueRoots(JSC::Handle<JSC::Unknown>, void* context, JSC::SlotVisitor&);
};

static JSManagedValueHandleOwner* managedValueHandleOwner()
{
    DEFINE_STATIC_LOCAL(JSManagedValueHandleOwner, jsManagedValueHandleOwner, ());
    return &jsManagedValueHandleOwner;
}


JSManagedValue::JSManagedValue(JSContextRef context, JSValueRef value)
{
    if (!value || !JSValueIsObject(context, value)) {
        JSC::Weak<JSC::JSObject> weak;
        m_value.swap(weak);
    } else {
        JSC::JSObject* object = toJS(const_cast<OpaqueJSValue*>(value));
        JSC::Weak<JSC::JSObject> weak(object, managedValueHandleOwner(), this);
        m_value.swap(weak);
    }
}


JSValueRef
JSManagedValue::value()
{
    if (!m_value)
        return 0;
    JSC::JSObject* object = m_value.get();
    return toRef(object);
}

void JSManagedValue::disconnectValue()
{
    m_value.clear();
}


bool JSManagedValueHandleOwner::isReachableFromOpaqueRoots(JSC::Handle<JSC::Unknown>, void* context, JSC::SlotVisitor& visitor)
{
    JSManagedValue *managedValue = static_cast<JSManagedValue *>(context);
    return visitor.containsOpaqueRoot(managedValue);
}

void JSManagedValueHandleOwner::finalize(JSC::Handle<JSC::Unknown>, void* context)
{
    JSManagedValue *managedValue = static_cast<JSManagedValue *>(context);
    managedValue->disconnectValue();
}

