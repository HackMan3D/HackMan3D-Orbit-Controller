#include "HID.h"
#include <math.h>

// ============================================================================
// Hackman3D DIY SpaceMouse Firmware
// Firmware for Arduino Pro Micro / ATmega32U4
//
// EN: This firmware turns an Arduino Pro Micro into a 6-axis HID SpaceMouse.
// FR: Ce firmware transforme un Arduino Pro Micro en souris 3D HID 6 axes.
//
// Author / Auteur: Hackman3D
// ============================================================================


// ============================================================================
// SETTINGS / PARAMÈTRES
// ============================================================================

// EN: Input deadzone applied directly after reading the joysticks.
// FR: Zone morte appliquée juste après la lecture des joysticks.
const int DEADZONE_INPUT  = 40;

// EN: Output deadzone applied before sending values to the computer.
// FR: Zone morte appliquée avant l’envoi des valeurs au PC.
const int DEADZONE_OUTPUT = 45;

// EN: Number of samples used at startup to calculate joystick center positions.
// FR: Nombre d’échantillons utilisés au démarrage pour calibrer les centres.
const int CENTER_SAMPLES  = 100;

// EN: Smoothing divisor. Higher value = smoother but slower response.
// FR: Diviseur de lissage. Plus la valeur est haute, plus c’est doux mais lent.
const int SMOOTH_DIVISOR  = 5;


// ============================================================================
// SENSITIVITY / SENSIBILITÉ
// ============================================================================

// EN: Translation sensitivity.
// FR: Sensibilité des translations.
const float GAIN_TX = 1.3;
const float GAIN_TY = 1.3;
const float GAIN_TZ = 2.3;

// EN: Rotation sensitivity.
// FR: Sensibilité des rotations.
const float GAIN_RX = 1.8;
const float GAIN_RY = 1.8;
const float GAIN_RZ = 2.0;

// EN: Global maximum speed scale. 0.70 means 30% slower than the base firmware.
// FR: Échelle globale de vitesse maximale. 0.70 signifie 30 % plus lent.
const float MAX_SPEED_SCALE = 0.70;

// EN: Response curve. 1.0 is linear; higher values make small movements slower.
// FR: Courbe de réponse. 1.0 est linéaire ; plus haut adoucit les petits mouvements.
const float RESPONSE_CURVE = 1.6;

// EN: Speed profiles. Default mode 1 keeps the values above.
// FR: Profils de vitesse. Le mode par défaut 1 garde les valeurs ci-dessus.
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

// EN: Serial debug output. Keep disabled during normal HID use.
// FR: Sortie debug série. Garder désactivé pendant l’utilisation HID normale.
const bool DEBUG_SERIAL = false;
const unsigned long DEBUG_SERIAL_BAUD = 115200;
const unsigned long DEBUG_SERIAL_INTERVAL_MS = 100;

// EN: Lower value gives rotation more priority over translation.
// FR: Plus la valeur est basse, plus la rotation est prioritaire.
const float ROTATION_PRIORITY = 0.65;

// EN: If false, multiple axes can be sent at the same time.
// FR: Si faux, plusieurs axes peuvent être envoyés en même temps.
const bool ENABLE_DOMINANT_AXIS_FILTER = false;

// EN: Experimental mode for slicers without native SpaceMouse support.
// FR: Mode expérimental pour les slicers sans support SpaceMouse natif.
const bool ENABLE_SLICER_MOUSE_MODE = true;
const bool DEFAULT_SLICER_MOUSE_MODE = false;
const bool ENABLE_SLICER_KEYBOARD_SHORTCUTS = true;
const int SLICER_MOUSE_MOVE_DIVISOR = 120;
const int SLICER_MOUSE_WHEEL_THRESHOLD = 90;
const int SLICER_MOUSE_WHEEL_FULL_SCALE = 700;
const int SLICER_MOUSE_MAX_MOVE = 12;
const int SLICER_MOUSE_MAX_WHEEL = 1;
const unsigned long SLICER_MOUSE_WHEEL_MIN_INTERVAL_MS = 45;
const unsigned long SLICER_MOUSE_WHEEL_MAX_INTERVAL_MS = 125;
const bool SLICER_MOUSE_AUTO_DRAG = true;
const uint8_t SLICER_MOUSE_BUTTON_LEFT = 0x01;
const uint8_t SLICER_MOUSE_BUTTON_RIGHT = 0x02;
const uint8_t SLICER_MOUSE_BUTTON_MIDDLE = 0x04;
const uint8_t SLICER_MOUSE_DRAG_BUTTON = SLICER_MOUSE_BUTTON_LEFT;
const uint8_t SLICER_SHORTCUT_MODIFIER_PRIMARY = 0x08; // macOS Command
const uint8_t SLICER_SHORTCUT_MODIFIER_SHIFT = 0x02;
const uint8_t SLICER_SHORTCUT_KEY_0 = 0x27;
const uint8_t SLICER_SHORTCUT_KEY_A = 0x04;
const uint8_t SLICER_SHORTCUT_KEY_G = 0x0A;
const uint8_t SLICER_SHORTCUT_KEY_L = 0x0F;
const uint8_t SLICER_SHORTCUT_KEY_N = 0x11;
const uint8_t SLICER_SHORTCUT_KEY_TAB = 0x2B;
const int SLICER_BUTTON_ACTION_HOME = 1;
const int SLICER_BUTTON_ACTION_PAINT = 2;
const int SLICER_BUTTON_ACTION_TAB_SEND = 3;
const unsigned long SLICER_BUTTON_LONG_PRESS_MS = 650;
const int SLICER_BUTTON_ACTIONS[3] = {
  SLICER_BUTTON_ACTION_TAB_SEND,
  SLICER_BUTTON_ACTION_PAINT,
  SLICER_BUTTON_ACTION_HOME
};


// ============================================================================
// AXIS INVERSION / INVERSION DES AXES
// ============================================================================

// EN: Set to true to reverse an axis.
// FR: Mettre sur true pour inverser un axe.
bool invX  = false;
bool invY  = false;
bool invZ  = false;
bool invRX = true;
bool invRY = true;
bool invRZ = true;


// ============================================================================
// PIN CONFIGURATION / CONFIGURATION DES PINS
// ============================================================================

// EN: Analog pins used by the 4 Hall effect joysticks.
// FR: Pins analogiques utilisées par les 4 joysticks à effet Hall.
const int pins[8] = {
  A1, A0,
  A3, A2,
  A7, A6,
  A9, A8
};

// EN: Button pins. Buttons must be wired between pin and GND.
// FR: Pins des boutons. Les boutons doivent être câblés entre la pin et GND.
const int buttonPins[3] = { 2, 3, 7 };
const int BUTTON_COUNT = 3;

// EN: Button indexes used to switch speed mode. Default = all three buttons.
// FR: Index des boutons utilisés pour changer de mode. Défaut = trois boutons.
const int MODE_SWITCH_BUTTONS[BUTTON_COUNT] = { 0, 1, 2 };
const int MODE_SWITCH_BUTTON_COUNT = 3;

// EN: If true, the mode-switch button combo is not sent as normal HID buttons.
// FR: Si vrai, la combinaison de changement de mode n’est pas envoyée en HID.
const bool MODE_SWITCH_SUPPRESS_BUTTONS = true;

// EN: Time to wait for the full combo before sending individual combo buttons.
// FR: Temps d’attente de la combinaison avant d’envoyer les boutons séparés.
const unsigned long MODE_SWITCH_CHORD_WINDOW_MS = 250;

// EN: Minimum time between two speed mode changes.
// FR: Temps minimum entre deux changements de mode de vitesse.
const unsigned long MODE_SWITCH_DEBOUNCE_MS = 500;

// EN: Button indexes used to toggle CAD / slicer mouse mode. Default = buttons 2 + 3.
// FR: Index des boutons utilisés pour basculer CAD / souris slicer. Défaut = boutons 2 + 3.
const int SLICER_MODE_BUTTONS[BUTTON_COUNT] = { 1, 2, 0 };
const int SLICER_MODE_BUTTON_COUNT = 2;
const unsigned long SLICER_MODE_HOLD_MS = 250;
const unsigned long SLICER_MODE_DEBOUNCE_MS = 500;


// ============================================================================
// GLOBAL VARIABLES / VARIABLES GLOBALES
// ============================================================================

// EN: Stores the neutral center value for each analog input.
// FR: Stocke la valeur neutre de chaque entrée analogique.
int center[8];

// EN: Smoothed values sent to the computer.
// FR: Valeurs lissées envoyées à l’ordinateur.
int16_t smoothTX = 0;
int16_t smoothTY = 0;
int16_t smoothTZ = 0;
int16_t smoothRX = 0;
int16_t smoothRY = 0;
int16_t smoothRZ = 0;

// EN: Current speed profile index.
// FR: Index du profil de vitesse actuel.
int currentSpeedMode = DEFAULT_SPEED_MODE;
bool slicerMouseModeEnabled = ENABLE_SLICER_MOUSE_MODE && DEFAULT_SLICER_MOUSE_MODE;

bool modeSwitchComboWasPressed = false;
bool modeSwitchChordActive = false;
bool modeSwitchChordComboTriggered = false;
bool modeSwitchButtonsForwarded = false;
uint32_t modeSwitchPendingButtons = 0;
unsigned long modeSwitchChordStartedAt = 0;
unsigned long lastModeSwitchAt = 0;

bool slicerModeComboWasPressed = false;
bool slicerModeToggleHandled = false;
uint8_t slicerMouseButtonMask = 0;
bool slicerButtonWasPressed[3] = { false, false, false };
bool slicerButtonLongHandled[3] = { false, false, false };
unsigned long slicerButtonPressedAt[3] = { 0, 0, 0 };
unsigned long slicerModeComboStartedAt = 0;
unsigned long lastSlicerModeSwitchAt = 0;
unsigned long lastSlicerMouseWheelAt = 0;
int8_t lastSlicerMouseWheelDirection = 0;


// ============================================================================
// HID DESCRIPTOR / DESCRIPTEUR HID
// ============================================================================
//
// EN:
// This HID descriptor defines a multi-axis controller compatible with
// 3Dconnexion-style input reports:
// Report 1 = Translation X/Y/Z
// Report 2 = Rotation RX/RY/RZ
// Report 3 = Buttons
//
// FR:
// Ce descripteur HID définit un contrôleur multi-axes compatible avec des
// rapports de type 3Dconnexion :
// Rapport 1 = Translation X/Y/Z
// Rapport 2 = Rotation RX/RY/RZ
// Rapport 3 = Boutons
// ============================================================================

static const uint8_t hidReportDescriptor[] PROGMEM = {
  0x05, 0x01, 0x09, 0x08, 0xA1, 0x01,

  // Translation report / Rapport de translation
  0xA1, 0x00,
  0x85, 0x01,
  0x16, 0x00, 0x80,
  0x26, 0xFF, 0x7F,
  0x36, 0x00, 0x80,
  0x46, 0xFF, 0x7F,
  0x09, 0x30, 0x09, 0x31, 0x09, 0x32,
  0x75, 0x10,
  0x95, 0x03,
  0x81, 0x02,
  0xC0,

  // Rotation report / Rapport de rotation
  0xA1, 0x00,
  0x85, 0x02,
  0x16, 0x00, 0x80,
  0x26, 0xFF, 0x7F,
  0x36, 0x00, 0x80,
  0x46, 0xFF, 0x7F,
  0x09, 0x33, 0x09, 0x34, 0x09, 0x35,
  0x75, 0x10,
  0x95, 0x03,
  0x81, 0x02,
  0xC0,

  // Button report / Rapport des boutons
  0xA1, 0x00,
  0x85, 0x03,
  0x15, 0x00,
  0x25, 0x01,
  0x75, 0x01,
  0x95, 32,
  0x05, 0x09,
  0x19, 1,
  0x29, 32,
  0x81, 0x02,
  0xC0,

  0xC0
};

static const uint8_t mouseReportDescriptor[] PROGMEM = {
  0x05, 0x01,        // Usage Page (Generic Desktop)
  0x09, 0x02,        // Usage (Mouse)
  0xA1, 0x01,        // Collection (Application)
  0x85, 0x01,        //   Report ID (1)
  0x09, 0x01,        //   Usage (Pointer)
  0xA1, 0x00,        //   Collection (Physical)
  0x05, 0x09,        //     Usage Page (Button)
  0x19, 0x01,        //     Usage Minimum (Button 1)
  0x29, 0x03,        //     Usage Maximum (Button 3)
  0x15, 0x00,        //     Logical Minimum (0)
  0x25, 0x01,        //     Logical Maximum (1)
  0x95, 0x03,        //     Report Count (3)
  0x75, 0x01,        //     Report Size (1)
  0x81, 0x02,        //     Input (Data, Variable, Absolute)
  0x95, 0x01,        //     Report Count (1)
  0x75, 0x05,        //     Report Size (5)
  0x81, 0x03,        //     Input (Constant, Variable, Absolute)
  0x05, 0x01,        //     Usage Page (Generic Desktop)
  0x09, 0x30,        //     Usage (X)
  0x09, 0x31,        //     Usage (Y)
  0x09, 0x38,        //     Usage (Wheel)
  0x15, 0x81,        //     Logical Minimum (-127)
  0x25, 0x7F,        //     Logical Maximum (127)
  0x75, 0x08,        //     Report Size (8)
  0x95, 0x03,        //     Report Count (3)
  0x81, 0x06,        //     Input (Data, Variable, Relative)
  0xC0,              //   End Collection
  0xC0               // End Collection
};

static const uint8_t keyboardReportDescriptor[] PROGMEM = {
  0x05, 0x01,        // Usage Page (Generic Desktop)
  0x09, 0x06,        // Usage (Keyboard)
  0xA1, 0x01,        // Collection (Application)
  0x85, 0x02,        //   Report ID (2)
  0x05, 0x07,        //   Usage Page (Keyboard)
  0x19, 0xE0,        //   Usage Minimum (Keyboard LeftControl)
  0x29, 0xE7,        //   Usage Maximum (Keyboard Right GUI)
  0x15, 0x00,        //   Logical Minimum (0)
  0x25, 0x01,        //   Logical Maximum (1)
  0x75, 0x01,        //   Report Size (1)
  0x95, 0x08,        //   Report Count (8)
  0x81, 0x02,        //   Input (Data, Variable, Absolute)
  0x95, 0x01,        //   Report Count (1)
  0x75, 0x08,        //   Report Size (8)
  0x81, 0x03,        //   Input (Constant, Variable, Absolute)
  0x95, 0x06,        //   Report Count (6)
  0x75, 0x08,        //   Report Size (8)
  0x15, 0x00,        //   Logical Minimum (0)
  0x25, 0x73,        //   Logical Maximum (115)
  0x05, 0x07,        //   Usage Page (Keyboard)
  0x19, 0x00,        //   Usage Minimum (Reserved)
  0x29, 0x73,        //   Usage Maximum (Keyboard Application)
  0x81, 0x00,        //   Input (Data, Array, Absolute)
  0xC0               // End Collection
};


// ============================================================================
// SlicerMouseHID_
// EN: Separate USB HID mouse interface with keyboard report for slicer mode.
// FR: Interface USB HID souris séparée avec rapport clavier pour le mode slicer.
// ============================================================================

class SlicerMouseHID_ : public PluggableUSBModule {
public:
  SlicerMouseHID_(void);
  int sendReport(uint8_t buttons, int8_t x, int8_t y, int8_t wheel);
  int sendKeyboardReport(uint8_t modifiers, uint8_t key);

protected:
  int getInterface(uint8_t* interfaceCount);
  int getDescriptor(USBSetup& setup);
  bool setup(USBSetup& setup);
  uint8_t getShortName(char* name);

private:
  uint8_t epType[1];
  uint8_t protocol;
  uint8_t idle;
};

SlicerMouseHID_::SlicerMouseHID_(void)
  : PluggableUSBModule(1, 1, epType),
    protocol(HID_REPORT_PROTOCOL),
    idle(1) {
  epType[0] = EP_TYPE_INTERRUPT_IN;

  HID();

  if (ENABLE_SLICER_MOUSE_MODE) {
    PluggableUSB().plug(this);
  }
}

int SlicerMouseHID_::getInterface(uint8_t* interfaceCount) {
  *interfaceCount += 1;
  uint16_t descriptorSize = sizeof(mouseReportDescriptor);

  if (ENABLE_SLICER_KEYBOARD_SHORTCUTS) {
    descriptorSize += sizeof(keyboardReportDescriptor);
  }

  HIDDescriptor hidInterface = {
    D_INTERFACE(pluggedInterface, 1, USB_DEVICE_CLASS_HUMAN_INTERFACE,
                HID_SUBCLASS_BOOT_INTERFACE, HID_PROTOCOL_MOUSE),
    D_HIDREPORT(descriptorSize),
    D_ENDPOINT(USB_ENDPOINT_IN(pluggedEndpoint), USB_ENDPOINT_TYPE_INTERRUPT,
               USB_EP_SIZE, 0x01)
  };

  return USB_SendControl(0, &hidInterface, sizeof(hidInterface));
}

int SlicerMouseHID_::getDescriptor(USBSetup& setup) {
  if (setup.bmRequestType != REQUEST_DEVICETOHOST_STANDARD_INTERFACE) {
    return 0;
  }

  if (setup.wValueH != HID_REPORT_DESCRIPTOR_TYPE) {
    return 0;
  }

  if (setup.wIndex != pluggedInterface) {
    return 0;
  }

  protocol = HID_REPORT_PROTOCOL;

  int mouseResult = USB_SendControl(TRANSFER_PGM, mouseReportDescriptor, sizeof(mouseReportDescriptor));

  if (mouseResult < 0 || !ENABLE_SLICER_KEYBOARD_SHORTCUTS) {
    return mouseResult;
  }

  int keyboardResult = USB_SendControl(TRANSFER_PGM, keyboardReportDescriptor,
                                       sizeof(keyboardReportDescriptor));

  if (keyboardResult < 0) {
    return keyboardResult;
  }

  return mouseResult + keyboardResult;
}

bool SlicerMouseHID_::setup(USBSetup& setup) {
  if (setup.wIndex != pluggedInterface) {
    return false;
  }

  uint8_t request = setup.bRequest;
  uint8_t requestType = setup.bmRequestType;

  if (requestType == REQUEST_DEVICETOHOST_CLASS_INTERFACE) {
    if (request == HID_GET_REPORT || request == HID_GET_PROTOCOL) {
      return true;
    }

    if (request == HID_GET_IDLE) {
      return true;
    }
  }

  if (requestType == REQUEST_HOSTTODEVICE_CLASS_INTERFACE) {
    if (request == HID_SET_PROTOCOL) {
      protocol = setup.wValueL;
      return true;
    }

    if (request == HID_SET_IDLE) {
      idle = setup.wValueL;
      return true;
    }

    if (request == HID_SET_REPORT) {
      return true;
    }
  }

  return false;
}

uint8_t SlicerMouseHID_::getShortName(char* name) {
  name[0] = 'M';
  name[1] = 'O';
  name[2] = 'U';
  return 3;
}

int SlicerMouseHID_::sendReport(uint8_t buttons, int8_t x, int8_t y, int8_t wheel) {
  uint8_t reportId = 1;
  uint8_t report[4] = {
    buttons,
    (uint8_t)x,
    (uint8_t)y,
    (uint8_t)wheel
  };

  int reportIdResult = USB_Send(pluggedEndpoint, &reportId, 1);

  if (reportIdResult < 0) {
    return reportIdResult;
  }

  int reportResult = USB_Send(pluggedEndpoint | TRANSFER_RELEASE, report, 4);

  if (reportResult < 0) {
    return reportResult;
  }

  return reportIdResult + reportResult;
}

int SlicerMouseHID_::sendKeyboardReport(uint8_t modifiers, uint8_t key) {
  if (!ENABLE_SLICER_KEYBOARD_SHORTCUTS) {
    return 0;
  }

  uint8_t reportId = 2;
  uint8_t report[8] = {
    modifiers,
    0,
    key,
    0,
    0,
    0,
    0,
    0
  };

  int reportIdResult = USB_Send(pluggedEndpoint, &reportId, 1);

  if (reportIdResult < 0) {
    return reportIdResult;
  }

  int reportResult = USB_Send(pluggedEndpoint | TRANSFER_RELEASE, report, 8);

  if (reportResult < 0) {
    return reportResult;
  }

  return reportIdResult + reportResult;
}

SlicerMouseHID_ SlicerMouseHID;


// ============================================================================
// readAxes()
// EN: Reads all analog inputs.
// FR: Lit toutes les entrées analogiques.
// ============================================================================

void readAxes(int* values) {
  for (int i = 0; i < 8; i++) {
    values[i] = analogRead(pins[i]);
  }
}


// ============================================================================
// calibrateCenter()
// EN: Calculates the neutral center value of every joystick at startup.
// FR: Calcule la position neutre de chaque joystick au démarrage.
//
// Important:
// EN: Do not touch the SpaceMouse during startup calibration.
// FR: Ne touchez pas la souris 3D pendant la calibration au démarrage.
// ============================================================================

void calibrateCenter() {
  long sum[8] = {0};

  for (int n = 0; n < CENTER_SAMPLES; n++) {
    int temp[8];
    readAxes(temp);

    for (int i = 0; i < 8; i++) {
      sum[i] += temp[i];
    }

    delay(5);
  }

  for (int i = 0; i < 8; i++) {
    center[i] = sum[i] / CENTER_SAMPLES;
  }
}


// ============================================================================
// applyInputDeadzone()
// EN: Removes small joystick noise before calculations.
// FR: Supprime les petits bruits des joysticks avant les calculs.
// ============================================================================

void applyInputDeadzone(int* values) {
  for (int i = 0; i < 8; i++) {
    if (abs(values[i]) < DEADZONE_INPUT) {
      values[i] = 0;
    }
  }
}


// ============================================================================
// applyOutputDeadzone()
// EN: Removes tiny final output values to avoid drift.
// FR: Supprime les très petites valeurs finales pour éviter la dérive.
// ============================================================================

void applyOutputDeadzone(int16_t &x, int16_t &y, int16_t &z,
                         int16_t &rx, int16_t &ry, int16_t &rz) {
  if (abs(x)  < DEADZONE_OUTPUT) x  = 0;
  if (abs(y)  < DEADZONE_OUTPUT) y  = 0;
  if (abs(z)  < DEADZONE_OUTPUT) z  = 0;
  if (abs(rx) < DEADZONE_OUTPUT) rx = 0;
  if (abs(ry) < DEADZONE_OUTPUT) ry = 0;
  if (abs(rz) < DEADZONE_OUTPUT) rz = 0;
}


// ============================================================================
// countPositive4()
// EN: Counts how many of 4 values are above a threshold.
// FR: Compte combien de 4 valeurs sont au-dessus d’un seuil.
// ============================================================================

int countPositive4(int a, int b, int c, int d, int threshold) {
  int n = 0;

  if (a > threshold) n++;
  if (b > threshold) n++;
  if (c > threshold) n++;
  if (d > threshold) n++;

  return n;
}


// ============================================================================
// countNegative4()
// EN: Counts how many of 4 values are below a negative threshold.
// FR: Compte combien de 4 valeurs sont sous un seuil négatif.
// ============================================================================

int countNegative4(int a, int b, int c, int d, int threshold) {
  int n = 0;

  if (a < -threshold) n++;
  if (b < -threshold) n++;
  if (c < -threshold) n++;
  if (d < -threshold) n++;

  return n;
}


// ============================================================================
// smoothValue()
// EN: Smooths movement to avoid harsh jumps.
// FR: Lisse le mouvement pour éviter les changements trop brusques.
// ============================================================================

int16_t smoothValue(int16_t current, int16_t target) {
  int16_t delta = target - current;

  if (delta == 0) {
    return current;
  }

  int16_t step = delta / SMOOTH_DIVISOR;

  if (step == 0) {
    step = (delta > 0) ? 1 : -1;
  }

  return current + step;
}


// ============================================================================
// applyGain()
// EN: Applies sensitivity gain to an axis value.
// FR: Applique la sensibilité à la valeur d’un axe.
// ============================================================================

int16_t applyGain(int16_t value, float gain) {
  return (int16_t)(value * gain);
}


// ============================================================================
// applyResponseCurve()
// EN: Scales maximum speed and makes small movements easier to control.
// FR: Réduit la vitesse maximale et rend les petits mouvements plus contrôlables.
// ============================================================================

int16_t applyResponseCurve(int16_t value, float inputMax,
                           float speedScale, float responseCurve) {
  if (value == 0) {
    return 0;
  }

  float magnitude = abs(value);
  float maxMagnitude = inputMax;

  if (maxMagnitude < DEADZONE_OUTPUT + 1.0) {
    maxMagnitude = DEADZONE_OUTPUT + 1.0;
  }

  if (magnitude < DEADZONE_OUTPUT) {
    return 0;
  }

  if (magnitude > maxMagnitude) {
    magnitude = maxMagnitude;
  }

  float normalized = (magnitude - DEADZONE_OUTPUT) / (maxMagnitude - DEADZONE_OUTPUT);
  float maxOutputMagnitude = maxMagnitude * speedScale;

  if (maxOutputMagnitude < DEADZONE_OUTPUT) {
    maxOutputMagnitude = DEADZONE_OUTPUT;
  }

  float curved = DEADZONE_OUTPUT +
                  pow(normalized, responseCurve) *
                  (maxOutputMagnitude - DEADZONE_OUTPUT);

  if (value < 0) {
    curved = -curved;
  }

  return (int16_t)curved;
}


// ============================================================================
// resetSmoothing()
// EN: Clears smoothed axis memory after mode changes.
// FR: Réinitialise le lissage après un changement de mode.
// ============================================================================

void resetSmoothing() {
  smoothTX = 0;
  smoothTY = 0;
  smoothTZ = 0;

  smoothRX = 0;
  smoothRY = 0;
  smoothRZ = 0;
}


// ============================================================================
// resetModeSwitchChord()
// EN: Clears temporary state used while detecting a speed-mode button chord.
// FR: Réinitialise l’état temporaire de détection de la combinaison de boutons.
// ============================================================================

void resetModeSwitchChord() {
  modeSwitchChordActive = false;
  modeSwitchChordComboTriggered = false;
  modeSwitchButtonsForwarded = false;
  modeSwitchPendingButtons = 0;
}


// ============================================================================
// keepOnlyDominantAxis()
// EN:
// Keeps only the strongest axis and cancels the others.
// This makes the SpaceMouse easier to control and reduces unwanted diagonal
// movements.
//
// FR:
// Garde uniquement l’axe dominant et annule les autres.
// Cela rend la souris 3D plus facile à contrôler et réduit les mouvements
// parasites en diagonale.
// ============================================================================

void keepOnlyDominantAxis(int16_t &tx, int16_t &ty, int16_t &tz,
                          int16_t &rx, int16_t &ry, int16_t &rz) {
  int16_t values[6] = { tx, ty, tz, rx, ry, rz };

  int maxIndex = 0;
  int maxValue = abs(values[0]);

  for (int i = 1; i < 6; i++) {
    if (abs(values[i]) > maxValue) {
      maxValue = abs(values[i]);
      maxIndex = i;
    }
  }

  tx = (maxIndex == 0) ? tx : 0;
  ty = (maxIndex == 1) ? ty : 0;
  tz = (maxIndex == 2) ? tz : 0;
  rx = (maxIndex == 3) ? rx : 0;
  ry = (maxIndex == 4) ? ry : 0;
  rz = (maxIndex == 5) ? rz : 0;
}


// ============================================================================
// sendCommand()
// EN:
// Sends translation and rotation reports to the computer.
//
// FR:
// Envoie les rapports de translation et de rotation à l’ordinateur.
// ============================================================================

void sendCommand(int16_t rx, int16_t ry, int16_t rz,
                 int16_t x, int16_t y, int16_t z) {
  uint8_t trans[6] = {
    (uint8_t)(x & 0xFF), (uint8_t)(x >> 8),
    (uint8_t)(y & 0xFF), (uint8_t)(y >> 8),
    (uint8_t)(z & 0xFF), (uint8_t)(z >> 8)
  };

  uint8_t rot[6] = {
    (uint8_t)(rx & 0xFF), (uint8_t)(rx >> 8),
    (uint8_t)(ry & 0xFF), (uint8_t)(ry >> 8),
    (uint8_t)(rz & 0xFF), (uint8_t)(rz >> 8)
  };

  HID().SendReport(1, trans, 6);
  HID().SendReport(2, rot, 6);
}


// ============================================================================
// readButtonMask()
// EN: Reads physical buttons into a bit mask.
// FR: Lit les boutons physiques dans un masque de bits.
// ============================================================================

uint32_t readButtonMask() {
  uint32_t buttons = 0;

  for (int i = 0; i < BUTTON_COUNT; i++) {
    if (digitalRead(buttonPins[i]) == LOW) {
      buttons |= (1UL << i);
    }
  }

  return buttons;
}


// ============================================================================
// getModeSwitchButtonMask()
// EN: Builds a bit mask from the configured mode-switch buttons.
// FR: Crée un masque à partir des boutons configurés pour changer de mode.
// ============================================================================

uint32_t getModeSwitchButtonMask() {
  if (MODE_SWITCH_BUTTON_COUNT <= 0 || MODE_SWITCH_BUTTON_COUNT > BUTTON_COUNT) {
    return 0;
  }

  uint32_t comboMask = 0;

  for (int i = 0; i < MODE_SWITCH_BUTTON_COUNT; i++) {
    int buttonIndex = MODE_SWITCH_BUTTONS[i];

    if (buttonIndex < 0 || buttonIndex >= BUTTON_COUNT) {
      return 0;
    }

    comboMask |= (1UL << buttonIndex);
  }

  return comboMask;
}


// ============================================================================
// getSlicerModeButtonMask()
// EN: Builds a bit mask from the configured slicer-mode buttons.
// FR: Crée un masque à partir des boutons configurés pour le mode slicer.
// ============================================================================

uint32_t getSlicerModeButtonMask() {
  if (SLICER_MODE_BUTTON_COUNT <= 0 || SLICER_MODE_BUTTON_COUNT > BUTTON_COUNT) {
    return 0;
  }

  uint32_t comboMask = 0;

  for (int i = 0; i < SLICER_MODE_BUTTON_COUNT; i++) {
    int buttonIndex = SLICER_MODE_BUTTONS[i];

    if (buttonIndex < 0 || buttonIndex >= BUTTON_COUNT) {
      return 0;
    }

    comboMask |= (1UL << buttonIndex);
  }

  return comboMask;
}


// ============================================================================
// isModeSwitchComboPressed()
// EN: Checks if the configured speed mode button combo is pressed.
// FR: Vérifie si la combinaison de changement de mode est appuyée.
// ============================================================================

bool isModeSwitchComboPressed(uint32_t buttonMask) {
  uint32_t comboMask = getModeSwitchButtonMask();

  if (comboMask == 0) {
    return false;
  }

  return (buttonMask & comboMask) == comboMask;
}


// ============================================================================
// isSlicerModeComboPressed()
// EN: Checks if only the configured slicer-mode combo is pressed.
// FR: Vérifie si seule la combinaison du mode slicer est appuyée.
// ============================================================================

bool isSlicerModeComboPressed(uint32_t buttonMask) {
  uint32_t comboMask = getSlicerModeButtonMask();

  if (!ENABLE_SLICER_MOUSE_MODE || comboMask == 0) {
    return false;
  }

  return buttonMask == comboMask;
}


// ============================================================================
// updateSpeedMode()
// EN: Cycles through slow / normal / fast speed profiles.
// FR: Change de profil lent / normal / rapide.
// ============================================================================

void updateSpeedMode(bool comboPressed) {
  unsigned long now = millis();

  if (comboPressed &&
      !modeSwitchComboWasPressed &&
      now - lastModeSwitchAt >= MODE_SWITCH_DEBOUNCE_MS) {
    currentSpeedMode++;

    if (currentSpeedMode >= SPEED_MODE_COUNT) {
      currentSpeedMode = 0;
    }

    lastModeSwitchAt = now;
    resetSmoothing();
  }

  modeSwitchComboWasPressed = comboPressed;
}


// ============================================================================
// updateSlicerMouseMode()
// EN: Toggles CAD / slicer mouse emulation mode.
// FR: Bascule entre CAD et émulation souris pour slicer.
// ============================================================================

void updateSlicerMouseMode(bool comboPressed) {
  unsigned long now = millis();

  if (comboPressed && !slicerModeComboWasPressed) {
    slicerModeComboStartedAt = now;
    slicerModeToggleHandled = false;
  }

  if (comboPressed &&
      !slicerModeToggleHandled &&
      now - slicerModeComboStartedAt >= SLICER_MODE_HOLD_MS &&
      now - lastSlicerModeSwitchAt >= SLICER_MODE_DEBOUNCE_MS) {
    slicerMouseModeEnabled = !slicerMouseModeEnabled;
    lastSlicerModeSwitchAt = now;
    slicerModeToggleHandled = true;
    releaseSlicerMouseButtons();
    resetSmoothing();
  }

  if (!comboPressed) {
    slicerModeToggleHandled = false;
  }

  slicerModeComboWasPressed = comboPressed;
}


// ============================================================================
// filterModeSwitchButtons()
// EN: Suppresses shortcut buttons while a speed-mode chord is being detected.
// FR: Bloque les boutons du raccourci pendant la détection de la combinaison.
// ============================================================================

uint32_t filterModeSwitchButtons(uint32_t buttonMask, bool comboPressed, bool &comboAccepted) {
  comboAccepted = false;

  if (!MODE_SWITCH_SUPPRESS_BUTTONS) {
    comboAccepted = comboPressed;
    return buttonMask;
  }

  uint32_t switchMask = getModeSwitchButtonMask();

  if (switchMask == 0) {
    return buttonMask;
  }

  unsigned long now = millis();
  uint32_t nonSwitchButtons = buttonMask & ~switchMask;
  uint32_t activeSwitchButtons = buttonMask & switchMask;

  if (activeSwitchButtons == 0) {
    modeSwitchChordActive = false;

    if (!modeSwitchChordComboTriggered &&
        !modeSwitchButtonsForwarded &&
        modeSwitchPendingButtons != 0) {
      uint32_t releasedButtons = modeSwitchPendingButtons;
      resetModeSwitchChord();

      return nonSwitchButtons | releasedButtons;
    }

    resetModeSwitchChord();
    return nonSwitchButtons;
  }

  if (!modeSwitchChordActive) {
    modeSwitchChordActive = true;
    modeSwitchChordComboTriggered = false;
    modeSwitchButtonsForwarded = false;
    modeSwitchPendingButtons = activeSwitchButtons;
    modeSwitchChordStartedAt = now;
  } else {
    modeSwitchPendingButtons |= activeSwitchButtons;
  }

  if (comboPressed) {
    modeSwitchChordComboTriggered = true;
    comboAccepted = true;
    return nonSwitchButtons;
  }

  if (modeSwitchChordComboTriggered) {
    comboAccepted = true;
    return nonSwitchButtons;
  }

  if (now - modeSwitchChordStartedAt < MODE_SWITCH_CHORD_WINDOW_MS) {
    return nonSwitchButtons;
  }

  modeSwitchButtonsForwarded = true;
  return buttonMask;
}


// ============================================================================
// filterSlicerModeButtons()
// EN: Suppresses slicer-mode combo buttons while toggling mode.
// FR: Bloque les boutons du raccourci slicer pendant le basculement de mode.
// ============================================================================

uint32_t filterSlicerModeButtons(uint32_t buttonMask, bool comboPressed) {
  if (!ENABLE_SLICER_MOUSE_MODE || !comboPressed) {
    return buttonMask;
  }

  uint32_t comboMask = getSlicerModeButtonMask();

  if (comboMask == 0) {
    return buttonMask;
  }

  return buttonMask & ~comboMask;
}


// ============================================================================
// scaleMouseAxis()
// EN: Converts HID axis values to small relative mouse movements.
// FR: Convertit les axes HID en petits mouvements relatifs de souris.
// ============================================================================

int8_t scaleMouseAxis(int16_t value, int divisor, int maxValue) {
  if (abs(value) < DEADZONE_OUTPUT) {
    return 0;
  }

  int scaled = value / divisor;

  if (scaled == 0) {
    scaled = (value > 0) ? 1 : -1;
  }

  if (scaled > maxValue) {
    scaled = maxValue;
  }

  if (scaled < -maxValue) {
    scaled = -maxValue;
  }

  return (int8_t)scaled;
}


// ============================================================================
// scaleMouseWheel()
// EN: Sends wheel impulses slowly enough for slicer zoom control.
// FR: Envoie les impulsions de molette assez lentement pour le zoom slicer.
// ============================================================================

int8_t scaleMouseWheel(int16_t value) {
  int magnitude = abs(value);

  if (magnitude < SLICER_MOUSE_WHEEL_THRESHOLD) {
    lastSlicerMouseWheelDirection = 0;
    return 0;
  }

  unsigned long now = millis();
  int cappedMagnitude = magnitude;

  if (cappedMagnitude > SLICER_MOUSE_WHEEL_FULL_SCALE) {
    cappedMagnitude = SLICER_MOUSE_WHEEL_FULL_SCALE;
  }

  int wheelRange = SLICER_MOUSE_WHEEL_FULL_SCALE - SLICER_MOUSE_WHEEL_THRESHOLD;

  if (wheelRange < 1) {
    wheelRange = 1;
  }

  unsigned long intervalRange = SLICER_MOUSE_WHEEL_MAX_INTERVAL_MS -
                                SLICER_MOUSE_WHEEL_MIN_INTERVAL_MS;
  unsigned long interval = SLICER_MOUSE_WHEEL_MAX_INTERVAL_MS -
                           ((unsigned long)(cappedMagnitude - SLICER_MOUSE_WHEEL_THRESHOLD) *
                            intervalRange / wheelRange);
  int8_t direction = (value > 0) ? -SLICER_MOUSE_MAX_WHEEL : SLICER_MOUSE_MAX_WHEEL;

  if (direction == lastSlicerMouseWheelDirection &&
      now - lastSlicerMouseWheelAt < interval) {
    return 0;
  }

  lastSlicerMouseWheelAt = now;
  lastSlicerMouseWheelDirection = direction;

  return direction;
}


// ============================================================================
// sendSlicerMouseReport()
// EN: Sends a relative USB mouse report for slicer mouse emulation.
// FR: Envoie un rapport souris USB relatif pour l’émulation slicer.
// ============================================================================

void sendSlicerMouseReport(int8_t x, int8_t y, int8_t wheel) {
  SlicerMouseHID.sendReport(slicerMouseButtonMask, x, y, wheel);
}


// ============================================================================
// setSlicerMouseButton()
// EN: Updates one mouse button used by slicer mouse emulation.
// FR: Met à jour un bouton souris utilisé par l’émulation slicer.
// ============================================================================

void setSlicerMouseButton(uint8_t button, bool pressed) {
  uint8_t previousMask = slicerMouseButtonMask;

  if (pressed) {
    slicerMouseButtonMask |= button;
  } else {
    slicerMouseButtonMask &= ~button;
  }

  if (slicerMouseButtonMask != previousMask) {
    sendSlicerMouseReport(0, 0, 0);
  }
}


// ============================================================================
// releaseSlicerMouseButtons()
// EN: Releases mouse buttons used by slicer mouse emulation.
// FR: Relâche les boutons souris utilisés par l’émulation slicer.
// ============================================================================

void releaseSlicerMouseButtons() {
  if (slicerMouseButtonMask == 0) {
    return;
  }

  slicerMouseButtonMask = 0;
  sendSlicerMouseReport(0, 0, 0);
}


// ============================================================================
// sendSlicerKeyboardShortcut()
// EN: Sends one keyboard shortcut through the slicer keyboard interface.
// FR: Envoie un raccourci clavier via l’interface clavier slicer.
// ============================================================================

void sendSlicerKeyboardShortcut(uint8_t modifiers, uint8_t key) {
  if (!ENABLE_SLICER_KEYBOARD_SHORTCUTS) {
    return;
  }

  SlicerMouseHID.sendKeyboardReport(modifiers, key);
  delay(20);
  SlicerMouseHID.sendKeyboardReport(0, 0);
}


// ============================================================================
// resetSlicerButtonActions()
// EN: Clears slicer button shortcut state without sending actions.
// FR: Réinitialise l’état des raccourcis sans envoyer d’action.
// ============================================================================

void resetSlicerButtonActions() {
  for (int i = 0; i < BUTTON_COUNT; i++) {
    slicerButtonWasPressed[i] = false;
    slicerButtonLongHandled[i] = false;
    slicerButtonPressedAt[i] = 0;
  }

  if (ENABLE_SLICER_KEYBOARD_SHORTCUTS) {
    SlicerMouseHID.sendKeyboardReport(0, 0);
  }
}


// ============================================================================
// runSlicerButtonAction()
// EN: Runs one configured slicer button shortcut.
// FR: Exécute un raccourci configuré pour un bouton slicer.
// ============================================================================

void runSlicerButtonAction(int action, bool longPress) {
  releaseSlicerMouseButtons();

  if (action == SLICER_BUTTON_ACTION_HOME) {
    if (longPress) {
      sendSlicerKeyboardShortcut(0, SLICER_SHORTCUT_KEY_A);
    } else {
      sendSlicerKeyboardShortcut(SLICER_SHORTCUT_MODIFIER_PRIMARY, SLICER_SHORTCUT_KEY_0);
    }

    return;
  }

  if (action == SLICER_BUTTON_ACTION_PAINT) {
    if (longPress) {
      sendSlicerKeyboardShortcut(0, SLICER_SHORTCUT_KEY_L);
    } else {
      sendSlicerKeyboardShortcut(0, SLICER_SHORTCUT_KEY_N);
    }

    return;
  }

  if (action == SLICER_BUTTON_ACTION_TAB_SEND) {
    if (longPress) {
      sendSlicerKeyboardShortcut(SLICER_SHORTCUT_MODIFIER_PRIMARY | SLICER_SHORTCUT_MODIFIER_SHIFT,
                                 SLICER_SHORTCUT_KEY_G);
    } else {
      sendSlicerKeyboardShortcut(0, SLICER_SHORTCUT_KEY_TAB);
    }
  }
}


// ============================================================================
// updateSlicerMouseButtons()
// EN: Maps physical buttons to slicer shortcuts while slicer mode is active.
// FR: Associe les boutons physiques aux raccourcis en mode slicer.
// ============================================================================

void updateSlicerMouseButtons(uint32_t buttonMask, bool suppressButtons) {
  if (suppressButtons) {
    releaseSlicerMouseButtons();
    resetSlicerButtonActions();
    return;
  }

  if (!ENABLE_SLICER_KEYBOARD_SHORTCUTS) {
    return;
  }

  unsigned long now = millis();

  for (int i = 0; i < BUTTON_COUNT; i++) {
    bool pressed = (buttonMask & (1UL << i)) != 0;
    int action = SLICER_BUTTON_ACTIONS[i];

    if (pressed && !slicerButtonWasPressed[i]) {
      slicerButtonWasPressed[i] = true;
      slicerButtonLongHandled[i] = false;
      slicerButtonPressedAt[i] = now;
    }

    if (pressed &&
        (action == SLICER_BUTTON_ACTION_PAINT ||
         action == SLICER_BUTTON_ACTION_TAB_SEND ||
         action == SLICER_BUTTON_ACTION_HOME) &&
        !slicerButtonLongHandled[i] &&
        now - slicerButtonPressedAt[i] >= SLICER_BUTTON_LONG_PRESS_MS) {
      runSlicerButtonAction(action, true);
      slicerButtonLongHandled[i] = true;
    }

    if (!pressed && slicerButtonWasPressed[i]) {
      if (!slicerButtonLongHandled[i]) {
        runSlicerButtonAction(action, false);
      }

      slicerButtonWasPressed[i] = false;
      slicerButtonLongHandled[i] = false;
      slicerButtonPressedAt[i] = 0;
    }
  }
}


// ============================================================================
// sendSlicerMouse()
// EN: Sends mouse drag / wheel events for slicers without SpaceMouse support.
// FR: Envoie drag souris / molette pour les slicers sans support SpaceMouse.
// ============================================================================

void sendSlicerMouse(int16_t tx, int16_t ty, int16_t tz,
                     int16_t rx, int16_t ry, int16_t rz) {
  int translationPower = abs(tx) + abs(ty);
  int rotationPower = abs(rx) + abs(ry) + abs(rz);
  bool zoomActive = abs(tz) >= SLICER_MOUSE_WHEEL_THRESHOLD;

  int8_t wheel = scaleMouseWheel(tz);
  int8_t mouseX = 0;
  int8_t mouseY = 0;

  if (zoomActive) {
    releaseSlicerMouseButtons();

    if (wheel != 0) {
      sendSlicerMouseReport(0, 0, wheel);
    }

    return;
  }

  if (!SLICER_MOUSE_AUTO_DRAG) {
    if (rotationPower > translationPower && rotationPower > DEADZONE_OUTPUT) {
      mouseX = scaleMouseAxis(ry + rz, SLICER_MOUSE_MOVE_DIVISOR, SLICER_MOUSE_MAX_MOVE);
      mouseY = scaleMouseAxis(rx, SLICER_MOUSE_MOVE_DIVISOR, SLICER_MOUSE_MAX_MOVE);
    } else if (translationPower > DEADZONE_OUTPUT) {
      mouseX = scaleMouseAxis(tx, SLICER_MOUSE_MOVE_DIVISOR, SLICER_MOUSE_MAX_MOVE);
      mouseY = scaleMouseAxis(ty, SLICER_MOUSE_MOVE_DIVISOR, SLICER_MOUSE_MAX_MOVE);
    }

    if (mouseX != 0 || mouseY != 0 || wheel != 0) {
      sendSlicerMouseReport(mouseX, mouseY, wheel);
    }

    return;
  }

  if (rotationPower > translationPower && rotationPower > DEADZONE_OUTPUT) {
    setSlicerMouseButton(SLICER_MOUSE_DRAG_BUTTON, true);

    mouseX = scaleMouseAxis(ry + rz, SLICER_MOUSE_MOVE_DIVISOR, SLICER_MOUSE_MAX_MOVE);
    mouseY = scaleMouseAxis(rx, SLICER_MOUSE_MOVE_DIVISOR, SLICER_MOUSE_MAX_MOVE);
  } else if (translationPower > DEADZONE_OUTPUT) {
    setSlicerMouseButton(SLICER_MOUSE_DRAG_BUTTON, true);

    mouseX = scaleMouseAxis(tx, SLICER_MOUSE_MOVE_DIVISOR, SLICER_MOUSE_MAX_MOVE);
    mouseY = scaleMouseAxis(ty, SLICER_MOUSE_MOVE_DIVISOR, SLICER_MOUSE_MAX_MOVE);
  } else {
    releaseSlicerMouseButtons();
  }

  if (mouseX != 0 || mouseY != 0 || wheel != 0) {
    sendSlicerMouseReport(mouseX, mouseY, wheel);
  }
}


// ============================================================================
// sendButtons()
// EN:
// Reads the physical buttons and sends them as HID button states.
//
// FR:
// Lit les boutons physiques et les envoie comme états de boutons HID.
// ============================================================================

void sendButtons(uint32_t buttonMask) {
  uint8_t buttons[4] = { 0, 0, 0, 0 };

  for (int i = 0; i < BUTTON_COUNT; i++) {
    if ((buttonMask & (1UL << i)) != 0) {
      buttons[i / 8] |= (1 << (i % 8));
    }
  }

  HID().SendReport(3, buttons, 4);
}


// ============================================================================
// debugPrintState()
// EN: Optional Serial Plotter friendly debug output.
// FR: Sortie debug optionnelle compatible Serial Plotter.
// ============================================================================

void debugPrintState(const int* values,
                     int16_t tx, int16_t ty, int16_t tz,
                     int16_t rx, int16_t ry, int16_t rz,
                     uint32_t buttonMask) {
  if (!DEBUG_SERIAL) {
    return;
  }

  static unsigned long lastDebugAt = 0;
  unsigned long now = millis();

  if (now - lastDebugAt < DEBUG_SERIAL_INTERVAL_MS) {
    return;
  }

  lastDebugAt = now;

  Serial.print("mode:");
  Serial.print(currentSpeedMode);

  Serial.print(" buttons:");
  Serial.print(buttonMask);

  for (int i = 0; i < 8; i++) {
    Serial.print(" v");
    Serial.print(i);
    Serial.print(":");
    Serial.print(values[i]);
  }

  Serial.print(" tx:");
  Serial.print(tx);
  Serial.print(" ty:");
  Serial.print(ty);
  Serial.print(" tz:");
  Serial.print(tz);

  Serial.print(" rx:");
  Serial.print(rx);
  Serial.print(" ry:");
  Serial.print(ry);
  Serial.print(" rz:");
  Serial.println(rz);
}


// ============================================================================
// setup()
// EN:
// Initializes HID, buttons, then calibrates the joystick center positions.
//
// FR:
// Initialise le HID, les boutons, puis calibre la position centrale des joysticks.
// ============================================================================

void setup() {
  static HIDSubDescriptor node(hidReportDescriptor, sizeof(hidReportDescriptor));
  HID().AppendDescriptor(&node);

  if (DEBUG_SERIAL) {
    Serial.begin(DEBUG_SERIAL_BAUD);
  }

  for (int i = 0; i < BUTTON_COUNT; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  // EN: Short delay to let the board and sensors stabilize.
  // FR: Petit délai pour laisser la carte et les capteurs se stabiliser.
  delay(800);

  calibrateCenter();
}


// ============================================================================
// loop()
// EN:
// Main processing loop:
// 1. Read joystick values.
// 2. Subtract calibrated centers.
// 3. Apply input deadzone.
// 4. Calculate translation and rotation.
// 5. Detect Z push/pull and Z rotation.
// 6. Apply anti-drift rotation priority.
// 7. Apply gains, dominant axis filtering, deadzone, inversion and smoothing.
// 8. Send HID reports.
//
// FR:
// Boucle principale :
// 1. Lecture des joysticks.
// 2. Soustraction des centres calibrés.
// 3. Application de la zone morte d’entrée.
// 4. Calcul des translations et rotations.
// 5. Détection de la montée/descente Z et de la rotation Z.
// 6. Application de la priorité rotation anti-dérive.
// 7. Application des gains, axe dominant, zone morte, inversion et lissage.
// 8. Envoi des rapports HID.
// ============================================================================

void loop() {
  int raw[8];
  int v[8];
  uint32_t buttonMask = readButtonMask();
  bool modeSwitchComboPressed = isModeSwitchComboPressed(buttonMask);
  bool slicerModeComboPressed = isSlicerModeComboPressed(buttonMask);
  bool modeSwitchComboAccepted = false;
  uint32_t hidButtonMask = filterModeSwitchButtons(buttonMask,
                                                  modeSwitchComboPressed,
                                                  modeSwitchComboAccepted);
  hidButtonMask = filterSlicerModeButtons(hidButtonMask, slicerModeComboPressed);

  updateSpeedMode(modeSwitchComboAccepted);
  updateSlicerMouseMode(slicerModeComboPressed);

  // EN: Read raw joystick values.
  // FR: Lecture des valeurs brutes des joysticks.
  readAxes(raw);

  // EN: Convert raw values into movement values around zero.
  // FR: Convertit les valeurs brutes en valeurs centrées autour de zéro.
  for (int i = 0; i < 8; i++) {
    v[i] = raw[i] - center[i];
  }

  applyInputDeadzone(v);

  // EN: Basic translation calculation.
  // FR: Calcul de base des translations.
  int16_t transX = (v[5] - v[1]);
  int16_t transY = (v[7] - v[3]);
  int16_t transZ = 0;

  // EN: Basic rotation calculation.
  // FR: Calcul de base des rotations.
  int16_t rotX = (v[4] - v[0]);
  int16_t rotY = (v[2] - v[6]);
  int16_t rotZ = 0;

  // --------------------------------------------------------------------------
  // Z PUSH / PULL DETECTION
  // DÉTECTION MONTÉE / DESCENTE Z
  // --------------------------------------------------------------------------

  // EN:
  // If at least 3 sensors move in the same direction, the firmware considers
  // it a vertical push/pull movement.
  //
  // FR:
  // Si au moins 3 capteurs bougent dans le même sens, le firmware considère
  // que c’est un mouvement vertical haut/bas.
  int zPushPull = v[0] + v[2] + v[4] + v[6];

  int zPosCount = countPositive4(v[0], v[2], v[4], v[6], DEADZONE_INPUT);
  int zNegCount = countNegative4(v[0], v[2], v[4], v[6], DEADZONE_INPUT);

  bool zDetected = ((zPosCount >= 3) || (zNegCount >= 3)) &&
                   (abs(zPushPull) > DEADZONE_INPUT * 2);

  if (zDetected) {
    transZ = -zPushPull;

    // EN: Cancel X/Y translation to avoid unwanted side movement.
    // FR: Annule X/Y pour éviter un mouvement latéral parasite.
    transX = 0;
    transY = 0;
  }

  // --------------------------------------------------------------------------
  // Z ROTATION DETECTION
  // DÉTECTION ROTATION Z
  // --------------------------------------------------------------------------

  // EN:
  // If at least 3 side sensors move in the same direction, the firmware
  // interprets it as a twist around the Z axis.
  //
  // FR:
  // Si au moins 3 capteurs latéraux bougent dans le même sens, le firmware
  // l’interprète comme une rotation autour de l’axe Z.
  int zTwist = v[1] + v[3] + v[5] + v[7];

  int rzPosCount = countPositive4(v[1], v[3], v[5], v[7], DEADZONE_INPUT);
  int rzNegCount = countNegative4(v[1], v[3], v[5], v[7], DEADZONE_INPUT);

  bool rzDetected = ((rzPosCount >= 3) || (rzNegCount >= 3)) &&
                    (abs(zTwist) > DEADZONE_INPUT * 3);

  if (rzDetected) {
    rotZ = zTwist / 2;

    // EN: Cancel X/Y rotation to avoid unwanted mixed rotation.
    // FR: Annule RX/RY pour éviter les rotations mélangées parasites.
    rotX = 0;
    rotY = 0;
  }

  // --------------------------------------------------------------------------
  // ROTATION PRIORITY / PRIORITÉ ROTATION
  // --------------------------------------------------------------------------

  // EN:
  // This reduces the issue where a rotation is accidentally interpreted as a
  // left/right or forward/back translation.
  //
  // FR:
  // Cela réduit le problème où une rotation est parfois interprétée comme une
  // translation gauche/droite ou avant/arrière.
  int rotationPower = abs(rotX) + abs(rotY) + abs(rotZ);
  int translationPower = abs(transX) + abs(transY) + abs(transZ);

  bool rotationMode = rotationPower > 80 &&
                      rotationPower > translationPower * ROTATION_PRIORITY;

  if (rotationMode) {
    transX = 0;
    transY = 0;
    transZ = 0;

    // EN: Reset translation smoothing to avoid ghost movement.
    // FR: Réinitialise le lissage translation pour éviter les mouvements fantômes.
    smoothTX = 0;
    smoothTY = 0;
    smoothTZ = 0;
  }

  // --------------------------------------------------------------------------
  // GAINS / SENSIBILITÉS
  // --------------------------------------------------------------------------

  transX = applyGain(transX, GAIN_TX);
  transY = applyGain(transY, GAIN_TY);
  transZ = applyGain(transZ, GAIN_TZ);

  rotX = applyGain(rotX, GAIN_RX);
  rotY = applyGain(rotY, GAIN_RY);
  rotZ = applyGain(rotZ, GAIN_RZ);

  // --------------------------------------------------------------------------
  // DOMINANT AXIS FILTER / FILTRE D’AXE DOMINANT
  // --------------------------------------------------------------------------

  if (ENABLE_DOMINANT_AXIS_FILTER) {
    keepOnlyDominantAxis(transX, transY, transZ, rotX, rotY, rotZ);
  }

  // --------------------------------------------------------------------------
  // RESPONSE CURVE / COURBE DE RÉPONSE
  // --------------------------------------------------------------------------

  float speedScale = SPEED_MODE_SCALE[currentSpeedMode];
  float responseCurve = SPEED_MODE_RESPONSE_CURVE[currentSpeedMode];

  transX = applyResponseCurve(transX, 1024.0 * GAIN_TX, speedScale, responseCurve);
  transY = applyResponseCurve(transY, 1024.0 * GAIN_TY, speedScale, responseCurve);
  transZ = applyResponseCurve(transZ, 2048.0 * GAIN_TZ, speedScale, responseCurve);

  rotX = applyResponseCurve(rotX, 1024.0 * GAIN_RX, speedScale, responseCurve);
  rotY = applyResponseCurve(rotY, 1024.0 * GAIN_RY, speedScale, responseCurve);
  rotZ = applyResponseCurve(rotZ, 1024.0 * GAIN_RZ, speedScale, responseCurve);

  // --------------------------------------------------------------------------
  // OUTPUT DEADZONE / ZONE MORTE DE SORTIE
  // --------------------------------------------------------------------------

  applyOutputDeadzone(transX, transY, transZ, rotX, rotY, rotZ);

  // --------------------------------------------------------------------------
  // AXIS INVERSION / INVERSION DES AXES
  // --------------------------------------------------------------------------

  if (invX)  transX = -transX;
  if (invY)  transY = -transY;
  if (invZ)  transZ = -transZ;
  if (invRX) rotX   = -rotX;
  if (invRY) rotY   = -rotY;
  if (invRZ) rotZ   = -rotZ;

  // --------------------------------------------------------------------------
  // SMOOTHING / LISSAGE
  // --------------------------------------------------------------------------

  smoothTX = smoothValue(smoothTX, transX);
  smoothTY = smoothValue(smoothTY, transY);
  smoothTZ = smoothValue(smoothTZ, transZ);

  smoothRX = smoothValue(smoothRX, rotX);
  smoothRY = smoothValue(smoothRY, rotY);
  smoothRZ = smoothValue(smoothRZ, rotZ);

  // --------------------------------------------------------------------------
  // FINAL DEADZONE / ZONE MORTE FINALE
  // --------------------------------------------------------------------------

  int16_t outputTX = smoothTX;
  int16_t outputTY = smoothTY;
  int16_t outputTZ = smoothTZ;

  int16_t outputRX = smoothRX;
  int16_t outputRY = smoothRY;
  int16_t outputRZ = smoothRZ;

  applyOutputDeadzone(outputTX, outputTY, outputTZ, outputRX, outputRY, outputRZ);

  // --------------------------------------------------------------------------
  // SEND HID REPORTS / ENVOI DES RAPPORTS HID
  // --------------------------------------------------------------------------

  // EN:
  // Axis order is adjusted here to match the 3Dconnexion driver behavior.
  //
  // FR:
  // L’ordre des axes est ajusté ici pour correspondre au comportement du driver
  // 3Dconnexion.
  if (slicerMouseModeEnabled) {
    sendCommand(0, 0, 0, 0, 0, 0);
    sendButtons(0);
    updateSlicerMouseButtons(buttonMask, modeSwitchComboPressed || slicerModeComboPressed);
    sendSlicerMouse(outputTX, outputTY, outputTZ, outputRX, outputRY, outputRZ);
  } else {
    releaseSlicerMouseButtons();

    sendCommand(
      outputRX,
      outputRZ,
      outputRY,
      outputTX,
      outputTZ,
      outputTY
    );

    sendButtons(hidButtonMask);
  }

  debugPrintState(v, outputTX, outputTY, outputTZ, outputRX, outputRY, outputRZ, buttonMask);
}
