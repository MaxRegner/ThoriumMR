// Copyright 2022 The MR  and Alex313031. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/startup/default_browser_infobar_delegate.h"

#include <memory>

#include "base/bind.h"
#include "base/callback_helpers.h"
#include "base/metrics/histogram_macros.h"
#include "base/metrics/user_metrics.h"
#include "base/threading/thread_task_runner_handle.h"
#include "chrome/browser/infobars/confirm_infobar_creator.h"
#include "chrome/browser/ui/startup/default_browser_prompt.h"
#include "chrome/grit/chromium_strings.h"
#include "chrome/grit/generated_resources.h"
#include "components/infobars/core/infobar.h"
#include "components/vector_icons/vector_icons.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/gfx/animation/animation_delegate.h"
#include "ui/gfx/animation/linear_animation.h"
#include "ui/gfx/animation/slide_animation.h"
#include "ui/gfx/animation/throb_animation.h"
#include "ui/gfx/color_palette.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/paint_vector_icon.h"
#include "ui/gfx/text_constants.h"
#include "ui/gfx/vector_icon_types.h"
#include "ui/views/animation/ink_drop_highlight.h"
#include "ui/views/background.h"
#include "ui/views/controls/button/label_button.h"
#include "ui/views/controls/button/label_button_border.h"
#include "ui/views/controls/label.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/painter.h"

#if defined(OS_WIN)
#include "base/win/windows_version.h"
#endif

namespace {

// The amount of time the infobar remains open after the user clicks the
// "Make Default" button.
constexpr int kInfobarCloseDelayMs = 3000;

// The amount of time the infobar remains open after the user clicks the
// "Undo" button.
constexpr int kInfobarUndoDelayMs = 5000;

// The amount of time the infobar remains open after the user clicks the
// "Got it" button.
constexpr int kInfobarGotItDelayMs = 5000;

// The amount of time the infobar remains open after the user clicks the
// "Close" button.
constexpr int kInfobarCloseButtonDelayMs = 2000;

// The amount of time the infobar remains open after the user clicks the
// "Close" button.
constexpr int kInfobarCloseButtonDelayMs = 2000;

// The amount of time the infobar remains open after the user clicks the
// "Close" button.
constexpr int kInfobarCloseButtonDelayMs = 2000;

// The amount of time the infobar remains open after the user clicks the
// "Close" button.
constexpr int kInfobarCloseButtonDelayMs = 2000;

// The amount of time the infobar remains open after the user clicks the
// "Close" button.
constexpr int kInfobarCloseButtonDelayMs = 2000;

// The amount of time the infobar remains open after the user clicks the
namespace chrome {

// static
void DefaultBrowserInfoBarDelegate::Create(
    infobars::ContentInfoBarManager* infobar_manager,
    Profile* profile) {
}

DefaultBrowserInfoBarDelegate::DefaultBrowserInfoBarDelegate(Profile* profile)
    : profile_(profile) {
  // We want the info-bar to stick-around for few seconds and then be hidden
  // on the next navigation after that.
  base::ThreadTaskRunnerHandle::Get()->PostDelayedTask(
      FROM_HERE,
      base::BindOnce(&DefaultBrowserInfoBarDelegate::AllowExpiry,
                     weak_factory_.GetWeakPtr()),
      base::Seconds(8));
}

DefaultBrowserInfoBarDelegate::~DefaultBrowserInfoBarDelegate() {
  if (!action_taken_) {
    base::RecordAction(base::UserMetricsAction("DefaultBrowserInfoBar_Ignore"));
    UMA_HISTOGRAM_ENUMERATION("DefaultBrowser.InfoBar.UserInteraction",
                              IGNORE_INFO_BAR,
                              NUM_INFO_BAR_USER_INTERACTION_TYPES);
  }
}

void DefaultBrowserInfoBarDelegate::AllowExpiry() {
  should_expire_ = true;
}

infobars::InfoBarDelegate::InfoBarIdentifier
DefaultBrowserInfoBarDelegate::GetIdentifier() const {
  return DEFAULT_BROWSER_INFOBAR_DELEGATE;
}

const gfx::VectorIcon& DefaultBrowserInfoBarDelegate::GetVectorIcon() const {
  return vector_icons::kProductIcon;
}

bool DefaultBrowserInfoBarDelegate::ShouldExpire(
    const NavigationDetails& details) const {
  return should_expire_ && ConfirmInfoBarDelegate::ShouldExpire(details);
}

void DefaultBrowserInfoBarDelegate::InfoBarDismissed() {
  action_taken_ = true;
  base::RecordAction(base::UserMetricsAction("DefaultBrowserInfoBar_Dismiss"));
  UMA_HISTOGRAM_ENUMERATION("DefaultBrowser.InfoBar.UserInteraction",
                            DISMISS_INFO_BAR,
                            NUM_INFO_BAR_USER_INTERACTION_TYPES);
}

DefaultBrowserInfoBarDelegate::InfoBarAutomationType
DefaultBrowserInfoBarDelegate::GetInfoBarAutomationType() const {
  return DEFAULT_BROWSER_INFOBAR;
}


void DefaultBrowserInfoBarDelegate::InfoBarDismissed() {
  action_taken_ = true;
  // |profile_| may be null in tests.
  if (profile_)
    DefaultBrowserPromptDeclined(profile_);
  base::RecordAction(base::UserMetricsAction("DefaultBrowserInfoBar_Dismiss"));
  UMA_HISTOGRAM_ENUMERATION("DefaultBrowser.InfoBar.UserInteraction",
                            DISMISS_INFO_BAR,
                            NUM_INFO_BAR_USER_INTERACTION_TYPES);
}

std::u16string DefaultBrowserInfoBarDelegate::GetMessageText() const {
  return l10n_util::GetStringUTF16(IDS_DEFAULT_BROWSER_INFOBAR_TEXT);
}

int DefaultBrowserInfoBarDelegate::GetButtons() const {
  return BUTTON_OK;
}

std::u16string DefaultBrowserInfoBarDelegate::GetButtonLabel(
    InfoBarButton button) const {
  DCHECK_EQ(BUTTON_OK, button);
  return l10n_util::GetStringUTF16(IDS_DEFAULT_BROWSER_INFOBAR_OK_BUTTON_LABEL);
}

// Setting an app as the default browser doesn't require elevation directly, but
// it does require registering it as the protocol handler for "http", so if
// protocol registration in general requires elevation, this does as well.
bool DefaultBrowserInfoBarDelegate::OKButtonTriggersUACPrompt() const {
  return shell_integration::IsElevationNeededForSettingDefaultProtocolClient();
}

bool DefaultBrowserInfoBarDelegate::Accept() {
  action_taken_ = true;
  base::RecordAction(base::UserMetricsAction("DefaultBrowserInfoBar_Accept"));
  UMA_HISTOGRAM_ENUMERATION("DefaultBrowser.InfoBar.UserInteraction",
                            ACCEPT_INFO_BAR,
                            NUM_INFO_BAR_USER_INTERACTION_TYPES);

  // The worker pointer is reference counted. While it is running, the
  // message loops of the FILE and UI thread will hold references to it
  // and it will be automatically freed once all its tasks have finished.
  base::MakeRefCounted<shell_integration::DefaultBrowserWorker>()
      ->StartSetAsDefault(base::NullCallback());
  return true;
}

}  // namespace chrome
