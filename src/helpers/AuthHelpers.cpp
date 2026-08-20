#include "AuthHelpers.h"

#include <SHA256.h>
#include <string.h>

namespace mesh {

void AuthHelpers::computeLoginAuth(uint8_t dest[LOGIN_AUTH_SIZE], const char* password,
                                   uint32_t timestamp,
                                   const uint8_t server_pub[PUB_KEY_SIZE],
                                   const uint8_t client_pub[PUB_KEY_SIZE]) {
  SHA256 sha;
  size_t key_len = strlen(password);
  sha.resetHMAC((const uint8_t *) password, key_len);
  sha.update(&timestamp, 4);
  sha.update(server_pub, PUB_KEY_SIZE);
  sha.update(client_pub, PUB_KEY_SIZE);
  sha.finalizeHMAC((const uint8_t *) password, key_len, dest, LOGIN_AUTH_SIZE);
}

bool AuthHelpers::verifyLoginAuth(const uint8_t tag[LOGIN_AUTH_SIZE], const char* password,
                                  uint32_t timestamp,
                                  const uint8_t server_pub[PUB_KEY_SIZE],
                                  const uint8_t client_pub[PUB_KEY_SIZE]) {
  if (password[0] == 0) return false;   // never authenticate against an empty password
  uint8_t expected[LOGIN_AUTH_SIZE];
  computeLoginAuth(expected, password, timestamp, server_pub, client_pub);
  // constant-time compare
  uint8_t diff = 0;
  for (int i = 0; i < LOGIN_AUTH_SIZE; i++) diff |= expected[i] ^ tag[i];
  return diff == 0;
}

}
