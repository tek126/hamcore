#pragma once

#include <MeshCore.h>

// HamCore login authentication (replaces plaintext passwords over RF).
//
// The client proves knowledge of the server's password without transmitting it:
//   auth_tag = HMAC-SHA256(key = password,
//                          msg = timestamp_le(4) || server_pub(32) || client_pub(32))[0..15]
// The login payload carries LOGIN_AUTH_MARKER followed by the 16-byte tag in the
// position where the plaintext password used to be. Binding both public keys
// prevents cross-server replay/reflection; the timestamp plus the server's
// existing "sender_timestamp must increase" check rejects replays of captured
// logins. This is an authentication code, not content encryption, and so is
// permissible under FCC Part 97.113(a)(4).
#define LOGIN_AUTH_MARKER  0x04   // must stay outside 0 (ACL re-login), 0x01-0x03 (ANON_REQ_TYPE_*), and >= ' ' (legacy password)
#define LOGIN_AUTH_SIZE      16

namespace mesh {

class AuthHelpers {
public:
  /**
   * \brief  computes the 16-byte login proof-of-password tag.
   * \param dest  destination buffer, LOGIN_AUTH_SIZE bytes
   */
  static void computeLoginAuth(uint8_t dest[LOGIN_AUTH_SIZE], const char* password,
                               uint32_t timestamp,
                               const uint8_t server_pub[PUB_KEY_SIZE],
                               const uint8_t client_pub[PUB_KEY_SIZE]);

  /**
   * \brief  verifies a received tag against one candidate password.
   * \param tag  the LOGIN_AUTH_SIZE-byte tag from the login payload
   */
  static bool verifyLoginAuth(const uint8_t tag[LOGIN_AUTH_SIZE], const char* password,
                              uint32_t timestamp,
                              const uint8_t server_pub[PUB_KEY_SIZE],
                              const uint8_t client_pub[PUB_KEY_SIZE]);
};

}
