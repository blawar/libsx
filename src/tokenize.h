#ifndef SX_TOKENIZE_H
#define SX_TOKENIZE_H
#include <vector>
#include "strings.h"

bool tokenize(const char* pszStr, STRINGS &tokens, bool ignore_newline = true);
char isOp(char c);

#endif
