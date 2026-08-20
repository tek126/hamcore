#include <gtest/gtest.h>

#include <helpers/AuthHelpers.h>
#include <string.h>

using mesh::AuthHelpers;

static void fillKey(uint8_t key[PUB_KEY_SIZE], uint8_t seed) {
  for (int i = 0; i < PUB_KEY_SIZE; i++) key[i] = seed + i;
}

TEST(AuthHelpers, ClientAndServerAgree) {
  uint8_t server_pub[PUB_KEY_SIZE], client_pub[PUB_KEY_SIZE];
  fillKey(server_pub, 0x10);
  fillKey(client_pub, 0x80);

  uint8_t tag[LOGIN_AUTH_SIZE];
  AuthHelpers::computeLoginAuth(tag, "hunter2", 1723456789, server_pub, client_pub);
  EXPECT_TRUE(AuthHelpers::verifyLoginAuth(tag, "hunter2", 1723456789, server_pub, client_pub));
}

TEST(AuthHelpers, WrongPasswordRejected) {
  uint8_t server_pub[PUB_KEY_SIZE], client_pub[PUB_KEY_SIZE];
  fillKey(server_pub, 0x10);
  fillKey(client_pub, 0x80);

  uint8_t tag[LOGIN_AUTH_SIZE];
  AuthHelpers::computeLoginAuth(tag, "hunter2", 1723456789, server_pub, client_pub);
  EXPECT_FALSE(AuthHelpers::verifyLoginAuth(tag, "hunter3", 1723456789, server_pub, client_pub));
  EXPECT_FALSE(AuthHelpers::verifyLoginAuth(tag, "", 1723456789, server_pub, client_pub));
}

TEST(AuthHelpers, TimestampBound) {
  // a replayed tag with a different timestamp must not verify
  uint8_t server_pub[PUB_KEY_SIZE], client_pub[PUB_KEY_SIZE];
  fillKey(server_pub, 0x10);
  fillKey(client_pub, 0x80);

  uint8_t tag[LOGIN_AUTH_SIZE];
  AuthHelpers::computeLoginAuth(tag, "hunter2", 1723456789, server_pub, client_pub);
  EXPECT_FALSE(AuthHelpers::verifyLoginAuth(tag, "hunter2", 1723456790, server_pub, client_pub));
}

TEST(AuthHelpers, ServerBound) {
  // a tag captured for one server must not verify on another (cross-server replay)
  uint8_t server_a[PUB_KEY_SIZE], server_b[PUB_KEY_SIZE], client_pub[PUB_KEY_SIZE];
  fillKey(server_a, 0x10);
  fillKey(server_b, 0x11);
  fillKey(client_pub, 0x80);

  uint8_t tag[LOGIN_AUTH_SIZE];
  AuthHelpers::computeLoginAuth(tag, "hunter2", 1723456789, server_a, client_pub);
  EXPECT_FALSE(AuthHelpers::verifyLoginAuth(tag, "hunter2", 1723456789, server_b, client_pub));
}

TEST(AuthHelpers, ClientBound) {
  // a tag from one client must not verify presented as another client
  uint8_t server_pub[PUB_KEY_SIZE], client_a[PUB_KEY_SIZE], client_b[PUB_KEY_SIZE];
  fillKey(server_pub, 0x10);
  fillKey(client_a, 0x80);
  fillKey(client_b, 0x81);

  uint8_t tag[LOGIN_AUTH_SIZE];
  AuthHelpers::computeLoginAuth(tag, "hunter2", 1723456789, server_pub, client_a);
  EXPECT_FALSE(AuthHelpers::verifyLoginAuth(tag, "hunter2", 1723456789, server_pub, client_b));
}

TEST(AuthHelpers, EmptyPasswordNeverAuthenticates) {
  // an unset (empty) server password must not be a wildcard
  uint8_t server_pub[PUB_KEY_SIZE], client_pub[PUB_KEY_SIZE];
  fillKey(server_pub, 0x10);
  fillKey(client_pub, 0x80);

  uint8_t tag[LOGIN_AUTH_SIZE];
  AuthHelpers::computeLoginAuth(tag, "", 1723456789, server_pub, client_pub);
  EXPECT_FALSE(AuthHelpers::verifyLoginAuth(tag, "", 1723456789, server_pub, client_pub));
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
