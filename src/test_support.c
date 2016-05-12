#include <stdio.h>

#include "test_support.h"

void testSupportPrintStart(char* description) {
  printf("TEST\t: %s ... ", description);
}

bool testSupportReport(bool* overallResult, bool currentResult) {
  if (currentResult) {
    printf("[OK]\r\n");
  } else {
    printf("[FAIL]\r\n");
  }

  *overallResult = *overallResult && currentResult;
  return *overallResult;
}
