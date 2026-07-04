# Hackman3D Orbit Controller - Tuning Guide

This guide explains how to adjust the **dead zones**, **speed**, **sensitivity**, and **reactivity** of the Hackman3D Orbit Controller if your controller feels too sensitive, too slow, unstable, or if it produces unwanted movements.

All settings are located near the top of the firmware file:

```cpp
// SETTINGS / PARAMÈTRES
const int DEADZONE_INPUT  = 40;
const int DEADZONE_OUTPUT = 45;
const int CENTER_SAMPLES  = 100;
const int SMOOTH_DIVISOR  = 5;

// SENSITIVITY / SENSIBILITÉ
const float GAIN_TX = 1.3;
const float GAIN_TY = 1.3;
const float GAIN_TZ = 2.3;

const float GAIN_RX = 1.8;
const float GAIN_RY = 1.8;
const float GAIN_RZ = 2.0;

const float MAX_SPEED_SCALE = 0.70;
const float RESPONSE_CURVE = 1.6;

const int SPEED_MODE_COUNT = 3;
const int DEFAULT_SPEED_MODE = 1;
const float SPEED_MODE_SCALE[SPEED_MODE_COUNT] = {
  0.50,
  MAX_SPEED_SCALE,
  1.00
};
const float SPEED_MODE_RESPONSE_CURVE[SPEED_MODE_COUNT] = {
  1.9,
  RESPONSE_CURVE,
  1.3
};

const bool DEBUG_SERIAL = false;
const unsigned long DEBUG_SERIAL_BAUD = 115200;
const unsigned long DEBUG_SERIAL_INTERVAL_MS = 100;

const float ROTATION_PRIORITY = 0.65;
const bool ENABLE_DOMINANT_AXIS_FILTER = false;

// PIN CONFIGURATION / CONFIGURATION DES PINS
const int buttonPins[3] = { 2, 3, 7 };
const int BUTTON_COUNT = 3;
const int MODE_SWITCH_BUTTONS[BUTTON_COUNT] = { 0, 1, 2 };
const int MODE_SWITCH_BUTTON_COUNT = 3;
const bool MODE_SWITCH_SUPPRESS_BUTTONS = true;
const unsigned long MODE_SWITCH_CHORD_WINDOW_MS = 250;
const unsigned long MODE_SWITCH_DEBOUNCE_MS = 500;
```

After changing a value, upload the firmware again with Arduino IDE and test the controller in your 3D/CAD software.

---

## 1. Important startup calibration note

The controller calibrates itself every time it starts.

When plugging in the Orbit Controller:

1. Place it on a stable surface.
2. Do not touch the knob.
3. Wait about one second before using it.

If the knob is touched during startup, the center position may be wrong and the controller may drift or move by itself.

If this happens, unplug the USB cable and plug it back in without touching the knob.

---

## 2. Input dead zone

```cpp
const int DEADZONE_INPUT = 40;
```

The input dead zone removes small electrical noise directly after reading the joysticks.

### Increase this value if:

- the controller moves by itself;
- small vibrations create movement;
- an axis reacts even when you are not touching the knob;
- rotation sometimes creates unwanted translation.

### Decrease this value if:

- the controller needs too much force before reacting;
- small movements are ignored;
- the controller feels less precise around the center.

### Recommended range

```cpp
30 to 70
```

### Example

More stable, less sensitive around center:

```cpp
const int DEADZONE_INPUT = 55;
```

More sensitive around center:

```cpp
const int DEADZONE_INPUT = 30;
```

---

## 3. Output dead zone

```cpp
const int DEADZONE_OUTPUT = 45;
```

The output dead zone removes very small final movement values before sending them to the computer.

This is useful to stop tiny unwanted movements after smoothing.

### Increase this value if:

- there is still movement after releasing the knob;
- the camera slowly drifts;
- the movement does not fully stop;
- the controller feels unstable at rest.

### Decrease this value if:

- very small movements are difficult to control;
- the controller feels like it has a delay before starting;
- fine navigation is not precise enough.

### Recommended range

```cpp
25 to 80
```

### Example

More stable stop:

```cpp
const int DEADZONE_OUTPUT = 60;
```

More precise small movements:

```cpp
const int DEADZONE_OUTPUT = 30;
```

---

## 4. Smoothing / reactivity

```cpp
const int SMOOTH_DIVISOR = 5;
```

This setting controls how smooth or reactive the controller feels.

A higher value makes movement smoother, but slower.
A lower value makes movement faster, but less filtered.

### Increase this value if:

- movement feels too nervous;
- the camera jumps too quickly;
- movements are not smooth enough.

### Decrease this value if:

- the controller feels too slow;
- there is too much delay;
- the movement feels soft or delayed.

### Recommended range

```cpp
3 to 8
```

### Examples

More reactive:

```cpp
const int SMOOTH_DIVISOR = 3;
```

Balanced:

```cpp
const int SMOOTH_DIVISOR = 5;
```

Smoother but slower:

```cpp
const int SMOOTH_DIVISOR = 7;
```

---

## 5. Speed scale / response curve

```cpp
const float MAX_SPEED_SCALE = 0.70;
const float RESPONSE_CURVE = 1.6;
```

These values control the global speed and the response curve of the controller.

`MAX_SPEED_SCALE` limits the maximum speed of all axes.
`RESPONSE_CURVE` changes how progressively the controller reacts near the center.

A lower `MAX_SPEED_SCALE` makes the controller slower.
A higher `RESPONSE_CURVE` makes small movements softer and more precise.

### Decrease the speed scale if:

- all axes feel too fast;
- the camera moves too quickly;
- maximum speed is too high.

### Increase the speed scale if:

- all axes feel too slow;
- maximum speed is too low;
- you need to push too far to reach full speed.

### Increase the response curve if:

- small movements are too sensitive;
- the controller feels too nervous near the center;
- you want more gradual acceleration.

### Decrease the response curve if:

- movement starts too slowly;
- the controller feels too soft or delayed.

### Recommended ranges

```cpp
MAX_SPEED_SCALE = 0.50 to 1.00
RESPONSE_CURVE  = 1.0 to 2.2
```

### Examples

Default slower profile:

```cpp
const float MAX_SPEED_SCALE = 0.70;
const float RESPONSE_CURVE = 1.6;
```

Lower maximum speed:

```cpp
const float MAX_SPEED_SCALE = 0.50;
const float RESPONSE_CURVE = 1.6;
```

Softer start near the center:

```cpp
const float MAX_SPEED_SCALE = 0.70;
const float RESPONSE_CURVE = 1.9;
```

More direct response:

```cpp
const float MAX_SPEED_SCALE = 0.70;
const float RESPONSE_CURVE = 1.0;
```

Adjust these values before changing individual axis gains.

---

## 6. Speed modes / button shortcut

```cpp
const int SPEED_MODE_COUNT = 3;
const int DEFAULT_SPEED_MODE = 1;

const float SPEED_MODE_SCALE[SPEED_MODE_COUNT] = {
  0.50,
  MAX_SPEED_SCALE,
  1.00
};

const float SPEED_MODE_RESPONSE_CURVE[SPEED_MODE_COUNT] = {
  1.9,
  RESPONSE_CURVE,
  1.3
};
```

Speed modes let the controller switch between slow, normal, and fast profiles without changing the firmware every time.

Default mode `1` is the normal profile.
By default, pressing all three buttons at the same time changes the speed mode.

The default button shortcut cycles through:

1. slow;
2. normal;
3. fast.

### Default profiles

```cpp
slow   = 50% speed, softer start
normal = 70% speed, balanced start
fast   = 100% speed, more direct start
```

### Change the startup mode if:

- you always want the controller to start slower;
- you always want the controller to start faster;
- you do not want to use the button shortcut during normal use.

### Example

Start in slow mode:

```cpp
const int DEFAULT_SPEED_MODE = 0;
```

Start in fast mode:

```cpp
const int DEFAULT_SPEED_MODE = 2;
```

### Button shortcut

```cpp
const int MODE_SWITCH_BUTTONS[BUTTON_COUNT] = { 0, 1, 2 };
const int MODE_SWITCH_BUTTON_COUNT = 3;
const bool MODE_SWITCH_SUPPRESS_BUTTONS = true;
const unsigned long MODE_SWITCH_CHORD_WINDOW_MS = 250;
const unsigned long MODE_SWITCH_DEBOUNCE_MS = 500;
```

The default shortcut uses all three buttons at the same time.

Button indexes start from `0`, so the three default buttons are:

```cpp
0, 1, 2
```

`MODE_SWITCH_BUTTON_COUNT` controls how many button indexes are used.

### Examples

Use only buttons 0 and 1 to switch speed mode:

```cpp
const int MODE_SWITCH_BUTTONS[BUTTON_COUNT] = { 0, 1, 2 };
const int MODE_SWITCH_BUTTON_COUNT = 2;
```

Disable the speed mode shortcut:

```cpp
const int MODE_SWITCH_BUTTON_COUNT = 0;
```

If `MODE_SWITCH_SUPPRESS_BUTTONS` is `true`, the shortcut is not sent to the computer as normal button presses.

`MODE_SWITCH_CHORD_WINDOW_MS` is the time allowed to complete the speed-mode button combo.
This prevents CAD software from receiving one button while you are pressing the full combo.

A quick single-button tap is sent as soon as the button is released.
A held single button is sent after the chord window expires.
A full combo switches speed mode only if all shortcut buttons are pressed within the chord window.

Keep `MODE_SWITCH_CHORD_WINDOW_MS` around `150` to `400`.
Lower it if held single buttons feel delayed.
Increase it if your software still detects one button while switching speed mode.

Keep `MODE_SWITCH_DEBOUNCE_MS` around `300` to `700` unless one press changes mode more than once.

---

## 7. Serial debug output

```cpp
const bool DEBUG_SERIAL = false;
const unsigned long DEBUG_SERIAL_BAUD = 115200;
const unsigned long DEBUG_SERIAL_INTERVAL_MS = 100;
```

Serial debug output prints the current speed mode, button mask, joystick values, and final HID output values.

Keep this disabled during normal use.
Enable it only when you need to check raw joystick movement, button states, or output values.

### Enable debug output

```cpp
const bool DEBUG_SERIAL = true;
```

Then open the Arduino Serial Monitor or Serial Plotter at:

```cpp
115200 baud
```

### Increase the interval if:

- the Serial Monitor prints too much data;
- the controller feels slower while debugging.

### Decrease the interval if:

- you need faster feedback while testing joystick movement.

---

## 8. Translation sensitivity

```cpp
const float GAIN_TX = 1.3;
const float GAIN_TY = 1.3;
const float GAIN_TZ = 2.3;
```

These values control translation sensitivity:

- `GAIN_TX` = left / right movement
- `GAIN_TY` = forward / backward movement
- `GAIN_TZ` = up / down movement

### Increase a value if:

- that axis feels too slow;
- you need to push too far to get movement;
- movement is not strong enough.

### Decrease a value if:

- that axis is too fast;
- it is difficult to make small movements;
- it triggers too easily.

### Recommended range

```cpp
1.0 to 3.0
```

### Example

If vertical movement is too strong:

```cpp
const float GAIN_TZ = 1.8;
```

If left/right movement is too slow:

```cpp
const float GAIN_TX = 1.6;
```

---

## 9. Rotation sensitivity

```cpp
const float GAIN_RX = 1.8;
const float GAIN_RY = 1.8;
const float GAIN_RZ = 2.0;
```

These values control rotation sensitivity:

- `GAIN_RX` = rotate around X
- `GAIN_RY` = rotate around Y
- `GAIN_RZ` = twist around Z

### Increase a value if:

- rotation feels too slow;
- you need too much movement to rotate;
- the axis lacks response.

### Decrease a value if:

- rotation is too fast;
- rotation is difficult to control;
- rotation triggers unwanted movement.

### Recommended range

```cpp
1.2 to 3.0
```

### Example

If Z rotation is too fast:

```cpp
const float GAIN_RZ = 1.6;
```

If rotation feels too slow:

```cpp
const float GAIN_RX = 2.1;
const float GAIN_RY = 2.1;
```

---

## 10. Rotation priority

```cpp
const float ROTATION_PRIORITY = 0.65;
```

This setting helps prevent a rotation from being interpreted as a translation.

The lower the value, the more priority rotation gets over translation.

### Lower this value if:

- rotating sometimes creates unwanted left/right movement;
- rotating sometimes creates unwanted forward/backward movement;
- rotation and translation feel mixed together.

### Increase this value if:

- translation is cancelled too easily;
- it is difficult to make translation movements;
- rotation takes priority too often.

### Recommended range

```cpp
0.45 to 1.00
```

### Examples

More rotation priority:

```cpp
const float ROTATION_PRIORITY = 0.50;
```

Less rotation priority:

```cpp
const float ROTATION_PRIORITY = 0.85;
```

---

## 11. Dominant axis filter

```cpp
const bool ENABLE_DOMINANT_AXIS_FILTER = false;
```

This setting controls whether the firmware keeps only the strongest movement axis.

When this value is `false`, several axes can be sent at the same time.
This allows combined movements such as moving and rotating together.

When this value is `true`, only the strongest axis is kept and the others are cancelled.
This can make the controller easier to control, but it also prevents natural multi-axis movement.

### Set this value to false if:

- you want smooth multi-axis navigation;
- diagonal or combined movements feel blocked;
- moving and rotating at the same time feels difficult.

### Set this value to true if:

- unwanted diagonal movements are too frequent;
- the controller feels difficult to control;
- you prefer one movement direction at a time.

### Example

Allow several axes at the same time:

```cpp
const bool ENABLE_DOMINANT_AXIS_FILTER = false;
```

Keep only the strongest axis:

```cpp
const bool ENABLE_DOMINANT_AXIS_FILTER = true;
```

---

## 12. Center calibration samples

```cpp
const int CENTER_SAMPLES = 100;
```

This value controls how many readings are used during startup calibration.

A higher value may give a more stable center but increases startup time.

### Increase this value if:

- the controller sometimes starts with a bad center;
- the neutral position is not consistent;
- your joystick readings are noisy at startup.

### Decrease this value if:

- startup feels too long.

### Recommended range

```cpp
50 to 200
```

### Example

More stable startup calibration:

```cpp
const int CENTER_SAMPLES = 150;
```

---

## 13. Common problems and suggested fixes

### The controller moves by itself

Try:

```cpp
const int DEADZONE_INPUT  = 50;
const int DEADZONE_OUTPUT = 60;
```

Also make sure you do not touch the knob during USB connection.

---

### The controller is too slow

Try:

```cpp
const float MAX_SPEED_SCALE = 0.85;
const float RESPONSE_CURVE = 1.3;
const int SMOOTH_DIVISOR = 3;
```

You can also slightly increase the gain values.

---

### The controller is too nervous or too sensitive

Try:

```cpp
const float MAX_SPEED_SCALE = 0.60;
const float RESPONSE_CURVE = 1.8;
const int SMOOTH_DIVISOR = 6;
const int DEADZONE_INPUT = 50;
```

You can also lower the gain of the axis that feels too strong.

---

### The maximum speed is too high

Try:

```cpp
const float MAX_SPEED_SCALE = 0.60;
```

Use a lower value for a slower controller.

---

### Small movements are too fast

Try:

```cpp
const float RESPONSE_CURVE = 1.8;
```

Use a higher value for more gradual acceleration from the center.

---

### A CAD app opens a menu while switching speed mode

Try:

```cpp
const unsigned long MODE_SWITCH_CHORD_WINDOW_MS = 350;
```

Use a higher value if the software still receives one of the shortcut buttons before the full combo is detected.

---

### Single shortcut buttons feel delayed

Try:

```cpp
const unsigned long MODE_SWITCH_CHORD_WINDOW_MS = 150;
```

Quick taps are sent when the button is released.
Lower this value if held single-button shortcuts feel delayed.

---

### The speed mode changes more than once

Try:

```cpp
const unsigned long MODE_SWITCH_DEBOUNCE_MS = 700;
```

Use a higher value if one shortcut press cycles through more than one speed mode.

---

### You need to check joystick or button values

Try:

```cpp
const bool DEBUG_SERIAL = true;
```

Open the Arduino Serial Monitor or Serial Plotter at `115200 baud`.
Set `DEBUG_SERIAL` back to `false` for normal use.

---

### Rotation creates unwanted translation

Try:

```cpp
const float ROTATION_PRIORITY = 0.50;
const int DEADZONE_INPUT = 50;
```

If the issue is still present, reduce translation gains slightly:

```cpp
const float GAIN_TX = 1.1;
const float GAIN_TY = 1.1;
```

---

### Multi-axis movement feels blocked

Try:

```cpp
const bool ENABLE_DOMINANT_AXIS_FILTER = false;
```

This allows several axes to be sent at the same time.

---

### Too many diagonal movements

Try:

```cpp
const bool ENABLE_DOMINANT_AXIS_FILTER = true;
```

This keeps only the strongest movement axis and cancels the others.

---

### Small movements are not detected

Try:

```cpp
const int DEADZONE_INPUT  = 30;
const int DEADZONE_OUTPUT = 30;
```

Be careful: values that are too low may cause drift.

---

### Vertical movement is too strong

Try lowering `GAIN_TZ`:

```cpp
const float GAIN_TZ = 1.8;
```

---

### Z rotation is too strong

Try lowering `GAIN_RZ`:

```cpp
const float GAIN_RZ = 1.6;
```

---

## 14. Recommended tuning method

Change only one setting at a time.

Recommended order:

1. Start with `DEADZONE_INPUT`.
2. Adjust `DEADZONE_OUTPUT`.
3. Adjust `SMOOTH_DIVISOR`.
4. Adjust `MAX_SPEED_SCALE`.
5. Adjust `RESPONSE_CURVE`.
6. Adjust `SPEED_MODE_SCALE` and `SPEED_MODE_RESPONSE_CURVE` if you use speed modes.
7. Adjust `ENABLE_DOMINANT_AXIS_FILTER` if multi-axis movement feels blocked or too loose.
8. Adjust translation and rotation gains only if one axis needs correction.
9. Adjust `ROTATION_PRIORITY` only if rotation and translation are mixed.

After each change:

1. Upload the firmware again.
2. Unplug and reconnect the controller.
3. Do not touch the knob during startup.
4. Test in your 3D/CAD software.
5. Repeat if needed.

---

## 15. Safe default values

If tuning goes wrong, you can return to these default values:

```cpp
const int DEADZONE_INPUT  = 40;
const int DEADZONE_OUTPUT = 45;
const int CENTER_SAMPLES  = 100;
const int SMOOTH_DIVISOR  = 5;

const float GAIN_TX = 1.3;
const float GAIN_TY = 1.3;
const float GAIN_TZ = 2.3;

const float GAIN_RX = 1.8;
const float GAIN_RY = 1.8;
const float GAIN_RZ = 2.0;

const float MAX_SPEED_SCALE = 0.70;
const float RESPONSE_CURVE = 1.6;

const int SPEED_MODE_COUNT = 3;
const int DEFAULT_SPEED_MODE = 1;
const float SPEED_MODE_SCALE[SPEED_MODE_COUNT] = {
  0.50,
  MAX_SPEED_SCALE,
  1.00
};
const float SPEED_MODE_RESPONSE_CURVE[SPEED_MODE_COUNT] = {
  1.9,
  RESPONSE_CURVE,
  1.3
};

const bool DEBUG_SERIAL = false;
const unsigned long DEBUG_SERIAL_BAUD = 115200;
const unsigned long DEBUG_SERIAL_INTERVAL_MS = 100;

const float ROTATION_PRIORITY = 0.65;
const bool ENABLE_DOMINANT_AXIS_FILTER = false;

const int buttonPins[3] = { 2, 3, 7 };
const int BUTTON_COUNT = 3;
const int MODE_SWITCH_BUTTONS[BUTTON_COUNT] = { 0, 1, 2 };
const int MODE_SWITCH_BUTTON_COUNT = 3;
const bool MODE_SWITCH_SUPPRESS_BUTTONS = true;
const unsigned long MODE_SWITCH_CHORD_WINDOW_MS = 250;
const unsigned long MODE_SWITCH_DEBOUNCE_MS = 500;
```

---

## 16. Final note

Small hardware differences, joystick tolerances, soldering, wire routing, and printed part tolerances can affect the final feel of the controller.

There is no single perfect setting for everyone. The values above are safe starting points, and each user can fine-tune the controller to match their own build and preferred navigation style.
