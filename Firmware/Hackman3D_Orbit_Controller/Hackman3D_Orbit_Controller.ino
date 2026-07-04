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

bool modeSwitchComboWasPressed = false;
bool modeSwitchChordActive = false;
bool modeSwitchChordComboTriggered = false;
bool modeSwitchButtonsForwarded = false;
uint32_t modeSwitchPendingButtons = 0;
unsigned long modeSwitchChordStartedAt = 0;
unsigned long lastModeSwitchAt = 0;


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

  if (comboPressed && now - modeSwitchChordStartedAt <= MODE_SWITCH_CHORD_WINDOW_MS) {
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
  bool modeSwitchComboAccepted = false;
  uint32_t hidButtonMask = filterModeSwitchButtons(buttonMask,
                                                  modeSwitchComboPressed,
                                                  modeSwitchComboAccepted);

  updateSpeedMode(modeSwitchComboAccepted);

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
  sendCommand(
    outputRX,
    outputRZ,
    outputRY,
    outputTX,
    outputTZ,
    outputTY
  );

  sendButtons(hidButtonMask);
  debugPrintState(v, outputTX, outputTY, outputTZ, outputRX, outputRY, outputRZ, buttonMask);
}
