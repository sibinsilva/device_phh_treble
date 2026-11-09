/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "android.hardware.biometrics.fingerprint@2.3-service.anakin"

#include <android-base/logging.h>
#include <hidl/HidlTransportSupport.h>
#include <unistd.h>

#include "BiometricsFingerprint.h"

using android::hardware::biometrics::fingerprint::V2_3::IBiometricsFingerprint;
using android::hardware::biometrics::fingerprint::V2_3::implementation::BiometricsFingerprint;
using VendorBiometricsFingerprint = android::hardware::biometrics::fingerprint::V2_1::IBiometricsFingerprint;
using vendor::goodix::hardware::biometrics::fingerprint::V2_1::IGoodixFingerprintDaemon;
using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using android::OK;
using android::sp;
using android::status_t;

// The vendor @2.1-service publishes both IGoodixFingerprintDaemon (which we
// wrap) and its own IBiometricsFingerprint@2.1/default. hwservicemanager keeps
// whichever registers "default" last and drops the earlier one entirely, so we
// must register after the vendor service or the framework only ever sees the
// vendor's 2.1 object (UdfpsHelper then fails to cast it to V2_3). Its TEE init
// takes a few seconds, so poll for both instead of racing it. Bounded so a
// missing vendor service can never stall us forever.
template <typename T>
static bool waitForVendorService(const char* name) {
    constexpr int kPollMs = 200;
    constexpr int kTimeoutMs = 30000;
    for (int waited = 0; waited < kTimeoutMs; waited += kPollMs) {
        if (T::tryGetService() != nullptr) {
            LOG(INFO) << name << " is available after " << waited << "ms.";
            return true;
        }
        usleep(kPollMs * 1000);
    }
    LOG(ERROR) << name << " did not appear within " << kTimeoutMs << "ms, continuing anyway.";
    return false;
}

int main() {
    sp<BiometricsFingerprint> biometricsFingerprint;
    status_t status;

    LOG(INFO) << "Fingerprint HAL Adapter service is starting for ASUS ROG Phone 5 (ANAKIN).";

    waitForVendorService<IGoodixFingerprintDaemon>("IGoodixFingerprintDaemon");
    waitForVendorService<VendorBiometricsFingerprint>("vendor IBiometricsFingerprint@2.1");

    biometricsFingerprint = new BiometricsFingerprint();
    if (biometricsFingerprint == nullptr) {
        LOG(ERROR) << "Can not create an instance of Fingerprint HAL Adapter BiometricsFingerprint Iface, exiting.";
        goto shutdown;
    }

    configureRpcThreadpool(1, true /*callerWillJoin*/);

    status = biometricsFingerprint->registerAsService();
    if (status != OK) {
        LOG(ERROR) << "Could not register service for Fingerprint HAL Adapter BiometricsFingerprint Iface ("
                   << status << ")";
        goto shutdown;
    }

    LOG(INFO) << "Fingerprint HAL Adapter service is ready.";
    joinRpcThreadpool();
    // Should not pass this line

shutdown:
    LOG(ERROR) << "Fingerprint HAL Adapter service is shutting down.";
    return 1;
}
