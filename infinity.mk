$(call inherit-product, vendor/infinity/config/common.mk)
$(call inherit-product, vendor/infinity/config/common_full_phone.mk)
$(call inherit-product, vendor/infinity/config/BoardConfigSoong.mk)
$(call inherit-product, vendor/infinity/config/BoardConfigLineage.mk)
$(call inherit-product, device/lineage/sepolicy/common/sepolicy.mk)
-include vendor/infinity/build/core/config.mk

TARGET_DISABLE_EPPE := true
BYPASS_CHARGE_SUPPORTED := true

INFINITY_MAINTAINER := Sibieeeee
# Keep the default UNOFFICIAL build type: OFFICIAL hard-includes
# vendor/infinity-priv/keys/keys.mk (the Infinity team's private signing keys)
# and adds UpdaterGMSOverlay (OTA from the official Infinity servers).
INFINITY_BUILD_TYPE := UNOFFICIAL
