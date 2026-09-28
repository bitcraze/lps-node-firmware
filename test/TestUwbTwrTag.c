#include "unity.h"

#define _DEFAULT_SOURCE
#include <strings.h>

// The module under test has no public header for its internal state, include the source
// to get access to static variables such as curr_anchor and txPacket.
#include "../src/uwb_twr_tag.c"

// Stubs for the radio and LED drivers used by the module under test
void dwIdle(dwDevice_t* dev) {}
void dwNewReceive(dwDevice_t* dev) {}
void dwStartReceive(dwDevice_t* dev) {}
void dwNewTransmit(dwDevice_t* dev) {}
void dwStartTransmit(dwDevice_t* dev) {}
void dwWaitForResponse(dwDevice_t* dev, bool val) {}
void dwSetDefaults(dwDevice_t* dev) {}
void dwSetData(dwDevice_t* dev, uint8_t data[], unsigned int n) {}
unsigned int dwGetDataLength(dwDevice_t* dev) { return 0; }
void dwGetData(dwDevice_t* dev, uint8_t data[], unsigned int n) {}
void dwGetTransmitTimestamp(dwDevice_t* dev, dwTime_t* time) {}
void dwGetReceiveTimestamp(dwDevice_t* dev, dwTime_t* time) {}
void ledOn(led_e led) {}
void vAssertCalled(unsigned long ulLine, const char * const pcFileName) {}

#define UNUSED_SLOT 0xAA

static dwDevice_t dev;

static void setAnchorList(const uint8_t anchors[], uint8_t size) {
  memset(config.anchors, UNUSED_SLOT, sizeof(config.anchors));
  memcpy(config.anchors, anchors, size);
  config.anchorListSize = size;
}

static uint8_t interrogatedAnchor() {
  return txPacket.destAddress[0];
}

void setUp(void) {
  memset(&config, 0, sizeof(config));
  curr_anchor = 0;
}

void tearDown(void) {
}


void test_initiateRangingShouldInterrogateAllAnchorsInOrderAndWrapAround() {
  // Fixture
  const uint8_t anchors[] = {1, 2, 3};
  setAnchorList(anchors, sizeof(anchors));
  const uint8_t expected[] = {1, 2, 3, 1, 2, 3};

  for (unsigned int i = 0; i < sizeof(expected); i++) {
    // Test
    initiateRanging(&dev);

    // Assert
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(expected[i], interrogatedAnchor(), "Wrong anchor interrogated");
  }
}

void test_initiateRangingShouldNeverInterrogateAnchorOutsideList() {
  // Fixture
  const uint8_t anchors[] = {1, 2, 3};
  setAnchorList(anchors, sizeof(anchors));

  for (int i = 0; i < 20; i++) {
    // Test
    initiateRanging(&dev);

    // Assert
    TEST_ASSERT_NOT_EQUAL(UNUSED_SLOT, interrogatedAnchor());
  }
}

void test_initiateRangingShouldKeepAnchorIndexWithinArrayForFullList() {
  // Fixture
  const uint8_t anchors[MAX_ANCHORS] = {1, 2, 3, 4, 5, 6};
  setAnchorList(anchors, sizeof(anchors));

  for (int i = 0; i < 3 * MAX_ANCHORS; i++) {
    // Test
    initiateRanging(&dev);

    // Assert
    TEST_ASSERT_TRUE_MESSAGE(curr_anchor < MAX_ANCHORS, "Anchor index outside of anchor array");
    TEST_ASSERT_EQUAL_UINT8(anchors[i % MAX_ANCHORS], interrogatedAnchor());
  }
}
