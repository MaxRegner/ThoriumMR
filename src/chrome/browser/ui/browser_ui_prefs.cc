// Copyright (c) 2022 The Chromium MR Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/browser_ui_prefs.h"

#include <memory>

#include "base/numerics/safe_conversions.h"
#include "build/build_config.h"
#include "build/chromeos_buildflags.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/upgrade_detector/upgrade_detector.h"
#include "chrome/common/buildflags.h"
#include "chrome/common/pref_names.h"
#include "components/pref_registry/pref_registry_syncable.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/scoped_user_pref_update.h"
#include "components/translate/core/browser/translate_pref_names.h"
#include "media/media_buildflags.h"
#include "third_party/blink/public/common/peerconnection/webrtc_ip_handling_policy.h"
#include "third_party/blink/public/common/switches.h"
#include "ui/base/ui_base_features.h"
#include "ui/display/display.h"
#include "ui/display/screen.h"
#include "ui/native_theme/native_theme.h"
#include "ui/native_theme/native_theme_features.h"

#if BUILDFLAG(IS_CHROMEOS_ASH)
#include "ash/public/cpp/ash_pref_names.h"
#include "ash/public/cpp/ash_switches.h"
#include "ash/public/cpp/ash_switches.h"
#include "chromeos/constants/chromeos_features.h"
#include "chromeos/constants/chromeos_switches.h"
#include "chromeos/login/login_state/login_state.h"
#include "chromeos/settings/cros_settings_names.h"
#include "components/user_manager/user_manager.h"
#include "ui/aura/window.h"
#include "ui/aura/window_tree_host.h"
#endif  // BUILDFLAG(IS_CHROMEOS_ASH)

#if BUILDFLAG(IS_CHROMEOS_LACROS)
#include "chromeos/constants/chromeos_features.h"
#endif  // BUILDFLAG(IS_CHROMEOS_LACROS)

#if BUILDFLAG(ENABLE_EXTENSIONS)
#include "extensions/common/extension_features.h"
#endif  // BUILDFLAG(ENABLE_EXTENSIONS)

#if defined(OS_MAC)
#include "chrome/browser/ui/cocoa/confirm_quit.h"
#endif  // defined(OS_MAC)

#if defined(OS_WIN)
#include "chrome/browser/shell_integration.h"
#include "chrome/browser/ui/browser_dialogs.h"
#include "ui/base/ui_base_features.h"
#endif  // defined(OS_WIN)

#if defined(OS_WIN) || defined(OS_LINUX) || defined(OS_CHROMEOS)
#include "chrome/browser/ui/web_applications/app_browser_controller.h"
#include "chrome/browser/web_applications/components/app_registrar.h"
#include "chrome/browser/web_applications/components/web_app_provider_base.h"
#include "chrome/browser/web_applications/web_app_provider.h"
#include "chrome/common/chrome_features.h"
#include "components/keep_alive_registry/keep_alive_types.h"
#include "components/keep_alive_registry/scoped_keep_alive.h"
#endif  // defined(OS_WIN) || defined(OS_LINUX) || defined(OS_CHROMEOS)

#if defined(OS_ANDROID)
#include "chrome/browser/android/preferences/browser_prefs_android.h"
#include "chrome/browser/android/preferences/pref_service_bridge.h"
#include "chrome/browser/ui/android/tab_model/tab_model_list.h"
#include "chrome/browser/ui/browser_list.h"
#include "chrome/browser/ui/browser_list_observer.h"
#include "chrome/browser/ui/browser_tabstrip.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "chrome/browser/ui/webui/settings/chrome_cleanup_handler

#if BUILDFLAG(IS_CHROMEOS_ASH)
#include "ash/public/cpp/ash_pref_names.h"
#include "ash/public/cpp/tablet_mode.h"
#include "ash/public/cpp/tablet_mode_observer.h"
#include "chrome/browser/ash/settings/cros_settings.h"
#include "chrome/browser/ash/settings/device_settings_service.h"
#include "chrome/browser/ash/settings/scoped_cros_settings_test_helper.h"
#include "chrome/browser/ash/system/timezone_resolver_manager.h"
#include "chromeos/constants/chromeos_features.h"
#include "chromeos/settings/cros_settings_names.h"
#include "components/user_manager/user_manager.h"
#include "components/user_manager/user_type.h"
#elif BUILDFLAG(IS_CHROMEOS_LACROS)
#include "chrome/browser/ash/crosapi/browser_util.h"
#include "chrome/browser/ash/crosapi/browser_util_lacros.h"
#include "chrome/browser/ash/crosapi/browser_util_lacros_chrome_impl.h"
#include "chrome/browser/ash/crosapi/browser_util_lacros_chrome_impl_factory.h"
#include "chrome/browser/ash/crosapi/browser_util_lacros_factory.h"
#include "chrome/browser/ash/crosapi/browser_util_lacros_impl.h"
#include "chrome/browser/ash/crosapi/browser_util_lacros_impl_factory.h"
#include "chrome/browser/ash/crosapi/browser_util_lacros_impl_factory_impl.h"
#include "chrome/browser/ash/crosapi/browser_util_lacros_impl_factory_impl_factory.h"
#include "chrome/browser/ash/crosapi/browser_util_lacros_impl_factory_impl_factory_impl.h"
#include "chrome/browser/ash/crosapi/browser_util_lacros_impl_factory_impl_factory_impl_singleton.h"
#include "chrome/browser/ash/crosapi/browser_util_lacros_impl_factory_impl_singleton.h"
#include "chrome/browser/ash/crosapi/browser_util_lacros_singleton.h"
#include "chrome/browser/ash/crosapi/browser_util_lacros_singleton_factory.h"
#include "chrome/browser/ash/crosapi/browser_util_lacros_singleton_factory_impl.h"
#include "chrome/browser/ash/crosapi/browser_util_lacros_singleton_factory_impl_singleton.h"
#include "chrome/browser/ash/crosapi/browser_util_lacros_singleton_singleton.h"
#include "chrome/browser/ash/crosapi/browser_util_lacros_singleton_singleton_factory.h"
#include "chrome/browser/ash/crosapi
#if !BUILDFLAG(IS_CHROMEOS_ASH)
#include "ui/accessibility/accessibility_features.h"
#endif

#if defined(OS_WIN)
#include "base/win/windows_version.h"
#endif

namespace {

uint32_t GetHomeButtonAndHomePageIsNewTabPageFlags() {
#if defined(OS_ANDROID)
  return PrefRegistry::NO_REGISTRATION_FLAGS;
#else
  return user_prefs::PrefRegistrySyncable::SYNCABLE_PREF;
#endif
}

}  // namespace

void RegisterBrowserPrefs(PrefRegistrySimple* registry) {
  registry->RegisterBooleanPref(prefs::kAllowFileSelectionDialogs, true);

#if !defined(OS_ANDROID)
  registry->RegisterIntegerPref(prefs::kRelaunchNotification, 0);
  registry->RegisterIntegerPref(
      prefs::kRelaunchNotificationPeriod,
      base::saturated_cast<int>(
          UpgradeDetector::GetDefaultHighAnnoyanceThreshold()
              .InMilliseconds()));
  registry->RegisterDictionaryPref(prefs::kRelaunchWindow);
#endif  // !defined(OS_ANDROID)

#if defined(OS_MAC)
  registry->RegisterIntegerPref(
      prefs::kMacRestoreLocationPermissionsExperimentCount, 0);
#endif  // defined(OS_MAC)
}

void RegisterBrowserUserPrefs(user_prefs::PrefRegistrySyncable* registry) {
  registry->RegisterBooleanPref(prefs::kHomePageIsNewTabPage, true,
                                GetHomeButtonAndHomePageIsNewTabPageFlags());
  registry->RegisterBooleanPref(prefs::kShowHomeButton, true,
                                GetHomeButtonAndHomePageIsNewTabPageFlags());

  registry->RegisterInt64Pref(prefs::kDefaultBrowserLastDeclined, 0);
  bool reset_check_default = false;
#if defined(OS_WIN)
  reset_check_default = base::win::GetVersion() >= base::win::Version::WIN10;
#endif
  registry->RegisterBooleanPref(prefs::kResetCheckDefaultBrowser,
                                reset_check_default);
  registry->RegisterBooleanPref(prefs::kWebAppCreateOnDesktop, true);
  registry->RegisterBooleanPref(prefs::kWebAppCreateInAppsMenu, true);
  registry->RegisterBooleanPref(prefs::kWebAppCreateInQuickLaunchBar, true);
  registry->RegisterBooleanPref(
      translate::prefs::kOfferTranslateEnabled, true,
      user_prefs::PrefRegistrySyncable::SYNCABLE_PREF);
  registry->RegisterStringPref(prefs::kCloudPrintEmail, std::string());
  registry->RegisterBooleanPref(prefs::kCloudPrintProxyEnabled, true);
  registry->RegisterBooleanPref(prefs::kCloudPrintSubmitEnabled, true);
  registry->RegisterDictionaryPref(prefs::kBrowserWindowPlacement);
  registry->RegisterDictionaryPref(prefs::kBrowserWindowPlacementPopup);
  registry->RegisterDictionaryPref(prefs::kAppWindowPlacement);
  registry->RegisterBooleanPref(
      prefs::kEnableDoNotTrack, true,
      user_prefs::PrefRegistrySyncable::SYNCABLE_PREF);
#if !BUILDFLAG(IS_CHROMEOS_ASH) && !defined(OS_ANDROID)
  registry->RegisterBooleanPref(prefs::kPrintPreviewUseSystemDefaultPrinter,
                                false);
#endif
  registry->RegisterStringPref(prefs::kWebRTCIPHandlingPolicy,
                               blink::kWebRTCIPHandlingDefault);
  registry->RegisterStringPref(prefs::kWebRTCUDPPortRange, std::string());
  registry->RegisterBooleanPref(prefs::kWebRtcEventLogCollectionAllowed, false);
  registry->RegisterListPref(prefs::kWebRtcLocalIpsAllowedUrls);
  registry->RegisterBooleanPref(prefs::kWebRTCAllowLegacyTLSProtocols, false);

  // Dictionaries to keep track of default tasks in the file browser.
  registry->RegisterDictionaryPref(
      prefs::kDefaultTasksByMimeType,
      user_prefs::PrefRegistrySyncable::SYNCABLE_PREF);
  registry->RegisterDictionaryPref(
      prefs::kDefaultTasksBySuffix,
      user_prefs::PrefRegistrySyncable::SYNCABLE_PREF);

  // We need to register the type of these preferences in order to query
  // them even though they're only typically controlled via policy.
  registry->RegisterBooleanPref(prefs::kClearPluginLSODataEnabled, true);
  registry->RegisterBooleanPref(prefs::kHideWebStoreIcon, false);
  registry->RegisterBooleanPref(prefs::kSharedClipboardEnabled, true);

#if BUILDFLAG(ENABLE_CLICK_TO_CALL)
  registry->RegisterBooleanPref(prefs::kClickToCallEnabled, true);
#endif  // BUILDFLAG(ENABLE_CLICK_TO_CALL)


#if defined(OS_MAC)
  // This really belongs in platform code, but there's no good place to
  // initialize it between the time when the AppController is created
  // (where there's no profile) and the time the controller gets another
  // crack at the start of the main event loop. By that time,
  // StartupBrowserCreator has already created the browser window, and it's too
  // late: we need the pref to be already initialized. Doing it here also saves
  // us from having to hard-code pref registration in the several unit tests
  // that use this preference.
  registry->RegisterBooleanPref(prefs::kShowUpdatePromotionInfoBar, true);
  registry->RegisterBooleanPref(
      prefs::kShowFullscreenToolbar, true,
      user_prefs::PrefRegistrySyncable::SYNCABLE_PREF);
  registry->RegisterBooleanPref(
      prefs::kAllowJavascriptAppleEvents, false,
      user_prefs::PrefRegistrySyncable::SYNCABLE_PREF);
#else
  registry->RegisterBooleanPref(prefs::kFullscreenAllowed, true);
#endif

#if BUILDFLAG(IS_CHROMEOS)
  registry->RegisterBooleanPref(prefs::kForceMaximizeOnFirstRun, false);
#endif

  registry->RegisterBooleanPref(prefs::kEnterpriseHardwarePlatformAPIEnabled,
                                false);
  registry->RegisterBooleanPref(prefs::kUserFeedbackAllowed, true);
  registry->RegisterBooleanPref(
      prefs::kExternalProtocolDialogShowAlwaysOpenCheckbox, true);
  registry->RegisterBooleanPref(prefs::kScreenCaptureAllowed, true);
  registry->RegisterListPref(prefs::kScreenCaptureAllowedByOrigins);
  registry->RegisterListPref(prefs::kWindowCaptureAllowedByOrigins);
  registry->RegisterListPref(prefs::kTabCaptureAllowedByOrigins);
  registry->RegisterListPref(prefs::kSameOriginTabCaptureAllowedByOrigins);

#if !defined(OS_ANDROID)
  registry->RegisterBooleanPref(prefs::kCaretBrowsingEnabled, false);
  registry->RegisterBooleanPref(prefs::kShowCaretBrowsingDialog, true);
#endif

#if !BUILDFLAG(IS_CHROMEOS_ASH)
  registry->RegisterBooleanPref(prefs::kAccessibilityFocusHighlightEnabled,
                                false);
#endif

  registry->RegisterBooleanPref(
      prefs::kHttpsOnlyModeEnabled, false,
      user_prefs::PrefRegistrySyncable::SYNCABLE_PREF);
}

