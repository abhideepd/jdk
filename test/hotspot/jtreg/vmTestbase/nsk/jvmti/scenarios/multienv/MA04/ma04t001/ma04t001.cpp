/*
 * Copyright (c) 2004, 2024, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 */

#include <stdlib.h>
#include <string.h>
#include "jni_tools.h"
#include "agent_common.h"
#include "jvmti_tools.h"

#define PASSED 0
#define STATUS_FAILED 2
#define SAMPLE_TAG ((jlong) 111111)

extern "C" {

/* ========================================================================== */

/* scaffold objects */
static jlong timeout = 0;

/* test objects */
static jobject testedObject = nullptr;

/* ========================================================================== */

static int prepare(JNIEnv* jni) {
    const char* CLASS_NAME = "nsk/jvmti/scenarios/multienv/MA04/ma04t001";
    const char* FIELD_NAME = "testedObject";
    const char* FIELD_SIGNATURE = "Ljava/lang/Object;";
    jclass cls = nullptr;
    jfieldID fid = nullptr;

    NSK_DISPLAY0("Obtain tested object from a static field of debugee class\n");

    NSK_DISPLAY1("Find class: %s\n", CLASS_NAME);
    if (!NSK_JNI_VERIFY(jni, (cls = jni->FindClass(CLASS_NAME)) != nullptr))
        return NSK_FALSE;

    NSK_DISPLAY2("Find field: %s:%s\n", FIELD_NAME, FIELD_SIGNATURE);
    if (!NSK_JNI_VERIFY(jni, (fid =
            jni->GetStaticFieldID(cls, FIELD_NAME, FIELD_SIGNATURE)) != nullptr))
        return NSK_FALSE;

    if (!NSK_JNI_VERIFY(jni, (testedObject = jni->GetStaticObjectField(cls, fid)) != nullptr))
        return NSK_FALSE;

    if (!NSK_JNI_VERIFY(jni, (testedObject = jni->NewGlobalRef(testedObject)) != nullptr))
        return NSK_FALSE;

    return NSK_TRUE;
}

/* ========================================================================== */

/** Agent algorithm. */
static void JNICALL
agentProc(jvmtiEnv* jvmti, JNIEnv* jni, void* arg) {
    jlong tag = -1;
    char buffer[32];

    if (!nsk_jvmti_waitForSync(timeout))
        return;

    if (!prepare(jni)) {
        nsk_jvmti_setFailStatus();
        return;
    }

    NSK_DISPLAY0("Testcase #1: check that testedObject is not tagged \n");
    if (!NSK_JVMTI_VERIFY(jvmti->GetTag(testedObject, &tag))) {
        nsk_jvmti_setFailStatus();
        return;
    }
    NSK_DISPLAY1("tag = %s\n", jlong_to_string(tag, buffer));
    if (tag != 0) {
        NSK_COMPLAIN1("testedObject is unexpectedly tagged: %s\n",
            jlong_to_string(tag, buffer));
        nsk_jvmti_setFailStatus();
    }
    if (!NSK_JVMTI_VERIFY(jvmti->SetTag(testedObject, SAMPLE_TAG))) {
        nsk_jvmti_setFailStatus();
        return;
    }
    if (!NSK_VERIFY(nsk_jvmti_resumeSync()))
        return;
    if (!NSK_VERIFY(nsk_jvmti_waitForSync(timeout)))
        return;

    NSK_DISPLAY0("Testcase #2: check that testedObject is tagged correctly\n");
    if (!NSK_JVMTI_VERIFY(jvmti->GetTag(testedObject, &tag))) {
        nsk_jvmti_setFailStatus();
        return;
    }
    NSK_DISPLAY1("tag = %s\n", jlong_to_string(tag, buffer));
    if (tag != SAMPLE_TAG) {
        if (tag == 0) {
            NSK_COMPLAIN0("testedObject not tagged\n");
        } else {
            NSK_COMPLAIN1("testedObject tagged incorrectly, expected=%s,",
                jlong_to_string(SAMPLE_TAG, buffer));
            NSK_COMPLAIN1(" got=%s\n", jlong_to_string(tag, buffer));
        }
        nsk_jvmti_setFailStatus();
    }
    if (!NSK_VERIFY(nsk_jvmti_resumeSync()))
        return;
    if (!NSK_VERIFY(nsk_jvmti_waitForSync(timeout)))
        return;

    NSK_DISPLAY0("Testcase #3: check that testedObject is tagged correctly\n");
    if (!NSK_JVMTI_VERIFY(jvmti->GetTag(testedObject, &tag))) {
        nsk_jvmti_setFailStatus();
        return;
    }
    NSK_DISPLAY1("tag = %s\n", jlong_to_string(tag, buffer));
    if (tag != SAMPLE_TAG) {
        if (tag == 0) {
            NSK_COMPLAIN0("testedObject not tagged\n");
        } else {
            NSK_COMPLAIN1("testedObject tagged incorrectly, expected=%s,",
                jlong_to_string(SAMPLE_TAG, buffer));
            NSK_COMPLAIN1(" got=%s\n", jlong_to_string(tag, buffer));
        }
        nsk_jvmti_setFailStatus();
    }
    if (!NSK_JVMTI_VERIFY(jvmti->SetTag(testedObject, (jlong)0))) {
        nsk_jvmti_setFailStatus();
        return;
    }
    if (!NSK_VERIFY(nsk_jvmti_resumeSync()))
        return;
    if (!NSK_VERIFY(nsk_jvmti_waitForSync(timeout)))
        return;

    NSK_DISPLAY0("Testcase #4: check that testedObject is not tagged \n");
    if (!NSK_JVMTI_VERIFY(jvmti->GetTag(testedObject, &tag))) {
        nsk_jvmti_setFailStatus();
        return;
    }
    NSK_DISPLAY1("tag = %s\n", jlong_to_string(tag, buffer));
    if (tag != 0) {
        NSK_COMPLAIN1("testedObject is unexpectedly tagged: %s\n",
            jlong_to_string(tag, buffer));
        nsk_jvmti_setFailStatus();
    }
    if (!NSK_VERIFY(nsk_jvmti_resumeSync()))
        return;
    if (!NSK_VERIFY(nsk_jvmti_waitForSync(timeout)))
        return;

    NSK_DISPLAY0("Testcase #5: check that testedObject is not tagged\n");
    if (!NSK_JVMTI_VERIFY(jvmti->GetTag(testedObject, &tag))) {
        nsk_jvmti_setFailStatus();
        return;
    }
    NSK_DISPLAY1("tag = %s\n", jlong_to_string(tag, buffer));
    if (tag != 0) {
        NSK_COMPLAIN1("testedObject is unexpectedly tagged: %s\n",
            jlong_to_string(tag, buffer));
        nsk_jvmti_setFailStatus();
    }
    NSK_TRACE(jni->DeleteGlobalRef(testedObject));

    if (!nsk_jvmti_resumeSync())
        return;
}

/* ========================================================================== */

/** Agent library initialization. */
#ifdef STATIC_BUILD
JNIEXPORT jint JNICALL Agent_OnLoad_ma04t001(JavaVM *jvm, char *options, void *reserved) {
    return Agent_Initialize(jvm, options, reserved);
}
JNIEXPORT jint JNICALL Agent_OnAttach_ma04t001(JavaVM *jvm, char *options, void *reserved) {
    return Agent_Initialize(jvm, options, reserved);
}
JNIEXPORT jint JNI_OnLoad_ma04t001(JavaVM *jvm, char *options, void *reserved) {
    return JNI_VERSION_1_8;
}
#endif
jint Agent_Initialize(JavaVM *jvm, char *options, void *reserved) {
    jvmtiEnv* jvmti = nullptr;
    jvmtiEventCallbacks callbacks;
    jvmtiCapabilities caps;

    NSK_DISPLAY0("Agent_OnLoad\n");

    if (!NSK_VERIFY(nsk_jvmti_parseOptions(options)))
        return JNI_ERR;

    timeout = nsk_jvmti_getWaitTime() * 60 * 1000;

    if (!NSK_VERIFY((jvmti =
            nsk_jvmti_createJVMTIEnv(jvm, reserved)) != nullptr))
        return JNI_ERR;

    memset(&caps, 0, sizeof(caps));
    caps.can_tag_objects = 1;
    if (!NSK_JVMTI_VERIFY(jvmti->AddCapabilities(&caps))) {
        return JNI_ERR;
    }

    if (!NSK_VERIFY(nsk_jvmti_setAgentProc(agentProc, nullptr)))
        return JNI_ERR;

    memset(&callbacks, 0, sizeof(callbacks));
    if (!NSK_VERIFY(nsk_jvmti_init_MA(&callbacks)))
        return JNI_ERR;

    return JNI_OK;
}

/* ========================================================================== */

}
