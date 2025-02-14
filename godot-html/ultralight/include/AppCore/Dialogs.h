/******************************************************************************
 *  This file is a part of Ultralight, an ultra-portable web-browser engine.  *
 *                                                                            *
 *  See <https://ultralig.ht> for licensing and more.                         *
 *                                                                            *
 *  (C) 2024 Ultralight, Inc.                                                 *
 ******************************************************************************/

#pragma once

#include "Defines.h"
#include <Ultralight/String.h>

namespace ultralight {

///
/// Enum representing the icon to be displayed in a dialog.
///
enum class DialogIcon {
  Info,     ///< Information icon
  Warning,  ///< Warning icon
  Error,    ///< Error icon
  Question  ///< Question icon
};

///
/// Enum representing the button types in a dialog.
///
enum class ButtonType {
  OK,        ///< Single "OK" button
  OKCancel,  ///< "OK" and "Cancel" buttons
  YesNo      ///< "Yes" and "No" buttons
};

///
/// Enum representing the result of a dialog button press.
///
enum class ButtonResult {
  OK,      ///< "OK" button was pressed
  Cancel,  ///< "Cancel" button was pressed
  Yes,     ///< "Yes" button was pressed
  No       ///< "No" button was pressed
};

///
/// Shows a modal message box with the specified title, message, icon, and buttons.
///
/// @param  title    The title of the message box.
/// @param  message  The message to display in the message box.
/// @param  icon     The icon to display in the message box (default: DialogIcon::Info).
/// @param  buttons  The button type to display in the message box (default: ButtonType::OK).
///
/// @return  The result of the button press.
///
AExport ButtonResult ShowMessageBox(const String& title, const String& message,
                                    DialogIcon icon = DialogIcon::Info,
                                    ButtonType buttons = ButtonType::OK);

} // namespace ultralight