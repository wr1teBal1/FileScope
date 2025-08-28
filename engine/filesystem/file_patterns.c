#include <stdbool.h>
#include <ctype.h>
#include <string.h>

static int tolower_if_needed(int c, bool insensitive) {
    return insensitive ? tolower(c) : c;
}

// 简单通配：'*' 多字符，'?' 单字符；大小写策略外部决定
static bool wildcard_match_core(const char *pattern, const char *text, bool insensitive) {
    const char *star = NULL, *saved = NULL;
    while (*text) {
        if (*pattern == '*') { star = pattern++; saved = text; continue; }
        if (*pattern == '?' || tolower_if_needed(*pattern, insensitive) == tolower_if_needed(*text, insensitive)) {
            pattern++; text++; continue;
        }
        if (star) { pattern = star + 1; text = ++saved; continue; }
        return false;
    }
    while (*pattern == '*') pattern++;
    return *pattern == '\0';
}

bool file_patterns_match(const char *pattern, const char *filename, bool case_insensitive) {
    if (!pattern || !filename) return false;
    return wildcard_match_core(pattern, filename, case_insensitive);
} 