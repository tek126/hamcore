#pragma once

#include <MeshCore.h>

namespace mesh {

/**
 * HamCore helpers for FCC Part 97 compliance:
 *  - US amateur callsign validation and extraction (§97.119 station identification)
 *  - amateur band limits for the frequency ranges HamCore supports (§97.301)
 */
class HamRadio {
public:
  /**
   * \brief  validates a US amateur callsign: 1-2 letter prefix (K/N/W, or AA-AL,
   *      KA-KZ, NA-NZ, WA-WZ), one digit, 1-3 letter suffix. Case-insensitive.
   * \param  len  length to check, or -1 to use strlen
   */
  static bool isValidCallsign(const char* s, int len = -1);

  /**
   * \brief  extracts the callsign prefix of a node name (chars up to the first
   *      '-', ' ' or end), uppercased, into dest.
   * \param  dest  destination buffer, CALLSIGN_BUF_SIZE bytes, NUL-terminated on success
   * \returns  callsign length, or 0 if the name does not begin with a valid callsign
   */
  static int extractCallsign(char dest[CALLSIGN_BUF_SIZE], const char* name);

  /**
   * \brief  a node name is valid when it begins with a valid callsign, optionally
   *      followed by '-' or ' ' and an SSID/description (eg. "W1AW", "W1AW-2", "W1AW Base").
   */
  static bool isValidNodeName(const char* name);

  /**
   * \brief  true if freq (MHz) is inside a US amateur band HamCore supports:
   *      70cm (420-450 MHz) or 33cm (902-928 MHz).
   */
  static bool isHamFrequency(float freq_mhz);
  static bool isHamFrequencyKhz(uint32_t freq_khz);
};

}
