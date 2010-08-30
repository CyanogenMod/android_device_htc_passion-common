# Input Device Calibration File

# Touch Area
touch.touchArea.calibration = pressure

# Tool Area
#   Raw width field measures approx. 1 unit per millimeter
#   of tool area on the surface where a raw width of 1 corresponds
#   to about 17mm of physical size.  Given that the display resolution
#   is 10px per mm we obtain a scale factor of 10 pixels / unit and
#   a bias of 160 pixels.  In addition, the raw width represents a
#   sum of all contact area so we note this fact in the calibration.
touch.toolArea.calibration = linear
touch.toolArea.linearScale = 10
touch.toolArea.linearBias = 160
touch.toolArea.isSummed = 1

# Pressure
#   Driver reports signal strength as pressure.
#   A normal thumb touch while touching the back of the device
#   typically registers about 100 signal strength units although
#   this value is highly variable and is sensitive to contact area,
#   manner of contact and environmental conditions.  We set the
#   scale so that a normal touch with good signal strength will be
#   reported as having a pressure somewhere in the vicinity of 1.0,
#   a featherlight touch will be below 1.0 and a heavy or large touch
#   will be above 1.0.  We don't expect these values to be accurate.
touch.pressure.calibration = amplitude
touch.pressure.source = default
touch.pressure.scale = 0.01

# Size
touch.size.calibration = normalized

# Orientation
touch.orientation.calibration = none

