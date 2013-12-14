//
//  JavaScriptCoreJNI.h
//  JavaScriptCoreJNI
//
//  Created by Kota Iguchi on 12/5/13.
//  Copyright (c) 2013 Appcelerator, Inc. All rights reserved.
//

#ifndef JavaScriptCoreJNI_JavaScriptCoreJNI_h
#define JavaScriptCoreJNI_JavaScriptCoreJNI_h

#include <jni.h>

/* Private object for JSObjectRef */
typedef struct {
    // Java Object for callback
    jobject callback;
    jclass  callbackClass;
    // Java Object associated with jsobject
    jobject object;
    // Reserved data
    void* reserved;
    // Can be used from outside of the JavaScriptCore
    void* data;
} JSObjectPrivateData;

#endif
