/*
 * Copyright (C) 2026 The Android Open Source Project
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

#define LOG_TAG "BtAudioModuleSysbta"

#include <algorithm>
#include <set>
#include <string>

#include <android/binder_manager.h>
#include <utils/Log.h>

#include "core-impl/ChildInterface.h"
#include "core-impl/Configuration.h"
#include "core-impl/DevicePortProxy.h"
#include "core-impl/ModuleBluetoothBase.h"

using ::aidl::android::hardware::audio::core::ChildInterface;
using ::aidl::android::hardware::audio::core::Module;
using ::aidl::android::hardware::audio::core::ModuleBluetoothBase;
using ::android::bluetooth::audio::aidl::BluetoothAudioPort;
using ::android::bluetooth::audio::aidl::BluetoothAudioPortAidlIn;
using ::android::bluetooth::audio::aidl::BluetoothAudioPortAidlOut;

// Referenced by the AOSP ModuleBluetooth, the sysbta provider factory is registered in main()
extern "C" binder_status_t createIBluetoothAudioProviderFactory() { return STATUS_OK; }

namespace {

class ModuleSysbta final : public ModuleBluetoothBase {
 public:
  explicit ModuleSysbta(std::unique_ptr<Module::Configuration>&& config)
      : ModuleBluetoothBase(std::move(config)) {}

 protected:
  std::shared_ptr<BluetoothAudioPort> createProxyInstance(bool isInput) override {
    if (isInput) return std::make_shared<BluetoothAudioPortAidlIn>();
    return std::make_shared<BluetoothAudioPortAidlOut>();
  }
};

// Same ports as sysbta_audio_policy_configuration.xml: A2DP and hearing aid, no LE audio
std::unique_ptr<Module::Configuration> getSysbtaConfiguration() {
  static const std::set<std::string> kLeAudioPorts = {
      "BLE Headset Out", "BLE Headset In", "le audio output", "le audio input"};
  auto config = ::aidl::android::hardware::audio::core::internal::getConfiguration(
      Module::Type::BLUETOOTH);
  std::set<int32_t> erased;
  std::erase_if(config->ports, [&](const auto& port) {
    if (kLeAudioPorts.count(port.name) == 0) return false;
    erased.insert(port.id);
    config->connectedProfiles.erase(port.id);
    return true;
  });
  std::erase_if(config->routes, [&](const auto& route) {
    return erased.count(route.sinkPortId) != 0 ||
           std::any_of(route.sourcePortIds.begin(), route.sourcePortIds.end(),
                       [&](int32_t id) { return erased.count(id) != 0; });
  });
  return config;
}

}  // namespace

void registerAudioModule() {
  const std::string vendor_name = std::string() + Module::descriptor + "/default";
  if (!AServiceManager_isDeclared(vendor_name.c_str())) return;
  static ChildInterface<Module> module;
  std::shared_ptr<Module> instance =
      ::ndk::SharedRefBase::make<ModuleSysbta>(getSysbtaConfiguration());
  module = std::move(instance);
  const std::string instance_name = std::string() + Module::descriptor + "/sysbta";
  binder_status_t status =
      AServiceManager_addService(module.getBinder(), instance_name.c_str());
  ALOGW_IF(status != STATUS_OK, "Could not register %s, status=%d",
           instance_name.c_str(), status);
}
