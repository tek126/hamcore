#include <gtest/gtest.h>

#include <helpers/HamRadio.h>

using mesh::HamRadio;

TEST(HamRadio, AcceptsTypicalUSCallsigns) {
  EXPECT_TRUE(HamRadio::isValidCallsign("W1AW"));     // 1x2
  EXPECT_TRUE(HamRadio::isValidCallsign("K1A"));      // 1x1 special event
  EXPECT_TRUE(HamRadio::isValidCallsign("N0AX"));     // 1x2
  EXPECT_FALSE(HamRadio::isValidCallsign("N0CALL"));  // the classic placeholder is 1x4 -- not a real format
  EXPECT_TRUE(HamRadio::isValidCallsign("KD2ABC"));   // 2x3
  EXPECT_TRUE(HamRadio::isValidCallsign("AA7X"));     // 2x1, A-block prefix
  EXPECT_TRUE(HamRadio::isValidCallsign("AL7Q"));     // AL prefix (Alaska)
  EXPECT_TRUE(HamRadio::isValidCallsign("w1aw"));     // case-insensitive
}

TEST(HamRadio, RejectsInvalidCallsigns) {
  EXPECT_FALSE(HamRadio::isValidCallsign("NONAME"));  // no digit in a valid position
  EXPECT_FALSE(HamRadio::isValidCallsign("1ABC"));    // starts with digit
  EXPECT_FALSE(HamRadio::isValidCallsign("B1AW"));    // B not a US prefix
  EXPECT_FALSE(HamRadio::isValidCallsign("AM1AB"));   // AM outside AA-AL block
  EXPECT_FALSE(HamRadio::isValidCallsign("W1"));      // no suffix
  EXPECT_FALSE(HamRadio::isValidCallsign("W1ABCD"));  // suffix too long
  EXPECT_FALSE(HamRadio::isValidCallsign(""));
  EXPECT_FALSE(HamRadio::isValidCallsign("W1A W"));   // embedded space
  EXPECT_FALSE(HamRadio::isValidCallsign("DEADBEEF"));
}

TEST(HamRadio, ExtractsCallsignFromNodeName) {
  char cs[CALLSIGN_BUF_SIZE];

  EXPECT_EQ(4, HamRadio::extractCallsign(cs, "W1AW"));
  EXPECT_STREQ("W1AW", cs);

  EXPECT_EQ(4, HamRadio::extractCallsign(cs, "w1aw-2"));      // SSID suffix, uppercased
  EXPECT_STREQ("W1AW", cs);

  EXPECT_EQ(6, HamRadio::extractCallsign(cs, "KD2ABC Base")); // space-separated description
  EXPECT_STREQ("KD2ABC", cs);

  EXPECT_EQ(0, HamRadio::extractCallsign(cs, "NONAME"));
  EXPECT_EQ(0, HamRadio::extractCallsign(cs, "Alice's node"));
  EXPECT_EQ(0, HamRadio::extractCallsign(cs, "-W1AW"));
}

TEST(HamRadio, ValidatesNodeNames) {
  EXPECT_TRUE(HamRadio::isValidNodeName("W1AW"));
  EXPECT_TRUE(HamRadio::isValidNodeName("W1AW-2"));
  EXPECT_TRUE(HamRadio::isValidNodeName("K5XYZ Mobile"));
  EXPECT_FALSE(HamRadio::isValidNodeName("NONAME"));
  EXPECT_FALSE(HamRadio::isValidNodeName("Repeater 1"));
}

TEST(HamRadio, BandLimits) {
  EXPECT_TRUE(HamRadio::isHamFrequency(906.875f));   // 33cm default
  EXPECT_TRUE(HamRadio::isHamFrequency(433.5f));     // 70cm preset
  EXPECT_TRUE(HamRadio::isHamFrequency(902.0f));
  EXPECT_TRUE(HamRadio::isHamFrequency(928.0f));
  EXPECT_TRUE(HamRadio::isHamFrequency(420.0f));
  EXPECT_TRUE(HamRadio::isHamFrequency(450.0f));
  EXPECT_FALSE(HamRadio::isHamFrequency(869.618f));  // EU ISM (stock MeshCore default)
  EXPECT_TRUE(HamRadio::isHamFrequency(915.5f));     // mid-band 33cm
  EXPECT_FALSE(HamRadio::isHamFrequency(901.9f));
  EXPECT_FALSE(HamRadio::isHamFrequency(928.1f));
  EXPECT_FALSE(HamRadio::isHamFrequency(146.52f));   // 2m (not supported by this hardware)

  EXPECT_TRUE(HamRadio::isHamFrequencyKhz(906875));
  EXPECT_TRUE(HamRadio::isHamFrequencyKhz(433500));
  EXPECT_FALSE(HamRadio::isHamFrequencyKhz(869618));
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
