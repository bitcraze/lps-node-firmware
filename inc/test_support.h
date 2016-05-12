#ifndef _TEST_SUPPORT_H_
#define _TEST_SUPPORT_H_

#include <stdbool.h>

void testSupportPrintStart(char* description);
bool testSupportReport(bool* overallResult, bool currentResult);

#endif // _TEST_SUPPORT_H_
