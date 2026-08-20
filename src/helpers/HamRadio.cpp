#include "HamRadio.h"

#include <string.h>
#include <ctype.h>

namespace mesh {

static bool isValidPrefix(char a, char b, int prefix_len) {
  if (prefix_len == 1) {
    return a == 'K' || a == 'N' || a == 'W';
  }
  if (a == 'A') return b >= 'A' && b <= 'L';
  return (a == 'K' || a == 'N' || a == 'W') && (b >= 'A' && b <= 'Z');
}

bool HamRadio::isValidCallsign(const char* s, int len) {
  if (len < 0) len = strlen(s);
  if (len < 3 || len > 6) return false;

  char up[6];
  for (int i = 0; i < len; i++) up[i] = toupper((unsigned char) s[i]);

  int digit_pos = -1;
  for (int i = 0; i < len; i++) {
    if (up[i] >= '0' && up[i] <= '9') { digit_pos = i; break; }
    if (up[i] < 'A' || up[i] > 'Z') return false;
  }
  if (digit_pos < 1 || digit_pos > 2) return false;   // prefix is 1-2 letters

  int suffix_len = len - digit_pos - 1;
  if (suffix_len < 1 || suffix_len > 3) return false;
  for (int i = digit_pos + 1; i < len; i++) {
    if (up[i] < 'A' || up[i] > 'Z') return false;
  }
  return isValidPrefix(up[0], digit_pos == 2 ? up[1] : 0, digit_pos);
}

int HamRadio::extractCallsign(char dest[CALLSIGN_BUF_SIZE], const char* name) {
  int len = 0;
  while (name[len] && name[len] != '-' && name[len] != ' ') len++;
  if (!isValidCallsign(name, len)) return 0;

  for (int i = 0; i < len; i++) dest[i] = toupper((unsigned char) name[i]);
  dest[len] = 0;
  return len;
}

bool HamRadio::isValidNodeName(const char* name) {
  char callsign[CALLSIGN_BUF_SIZE];
  return extractCallsign(callsign, name) > 0;
}

bool HamRadio::isHamFrequency(float freq_mhz) {
  return (freq_mhz >= 420.0f && freq_mhz <= 450.0f)
      || (freq_mhz >= 902.0f && freq_mhz <= 928.0f);
}

bool HamRadio::isHamFrequencyKhz(uint32_t freq_khz) {
  return (freq_khz >= 420000 && freq_khz <= 450000)
      || (freq_khz >= 902000 && freq_khz <= 928000);
}

}
