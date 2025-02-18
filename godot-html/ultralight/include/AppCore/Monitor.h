/******************************************************************************
 *  This file is a part of Ultralight, an ultra-portable web-browser engine.  *
 *                                                                            *
 *  See <https://ultralig.ht> for licensing and more.                         *
 *                                                                            *
 *  (C) 2023 Ultralight, Inc.                                                 *
 *****************************************************************************/
#pragma once
#include "Defines.h"

namespace ultralight {

///
/// Monitor class, represents a platform monitor.
///
class AExport Monitor {
public:
  virtual ~Monitor() {}

  ///
  /// Get the unique display ID of the monitor.
  /// 
  /// The renderer uses this ID to identify which monitor a View is on (ViewConfig::display_id).
  /// 
  /// AppCore internally tracks the display's hardware refresh event and automatically calls
  /// Renderer::RefreshDisplay(id) to drive animation in all corresponding Views.
  /// 
  virtual uint32_t display_id() const = 0;

  ///
  /// Get the DPI scale (1.0 = 100%)
  ///
  virtual double scale() const = 0;

  ///
  /// Get the width of the monitor (in pixels).
  ///
  virtual uint32_t width() const = 0;

  ///
  /// Get the height of the monitor (in pixels).
  ///
  virtual uint32_t height() const = 0;

  ///
  /// Get the refresh rate of the monitor (in Hz).
  /// 
  virtual uint32_t refresh_rate() const = 0;
};

}  // namespace ultralight
