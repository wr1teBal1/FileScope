#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#ifndef FILE_WATCHER_DEBOUNCE_MAX
#define FILE_WATCHER_DEBOUNCE_MAX 256
#endif

typedef struct {
    char key[256]; // path + '#' + type
    uint64_t last_ms;
    int in_use;
} DebounceEntry;

typedef struct {
    DebounceEntry entries[FILE_WATCHER_DEBOUNCE_MAX];
} DebounceStore;

static void debounce_init(DebounceStore *s) {
    if (!s) return;
    memset(s, 0, sizeof(*s));
}

static void make_key(char *dst, size_t cap, const char *path, int type) {
    if (!dst || cap == 0) return;
    dst[0] = '\0';
    if (path) {
        size_t n = strlen(path);
        if (n > cap - 4) n = cap - 4;
        memcpy(dst, path, n);
        dst[n] = '\0';
    }
    size_t cur = strlen(dst);
    if (cur + 3 < cap) {
        dst[cur++] = '#';
        dst[cur++] = (char)('0' + (type % 10));
        dst[cur] = '\0';
    }
}

static bool debounce_accept(DebounceStore *s, const char *path, int type, uint64_t now_ms, uint32_t window_ms) {
    if (!s) return true;
    char key[256];
    make_key(key, sizeof(key), path, type);
    int free_idx = -1;
    for (int i = 0; i < FILE_WATCHER_DEBOUNCE_MAX; ++i) {
        if (s->entries[i].in_use) {
            if (strcmp(s->entries[i].key, key) == 0) {
                if (now_ms - s->entries[i].last_ms < (uint64_t)window_ms) {
                    return false;
                }
                s->entries[i].last_ms = now_ms;
                return true;
            }
        } else if (free_idx < 0) {
            free_idx = i;
        }
    }
    if (free_idx < 0) free_idx = 0;
    s->entries[free_idx].in_use = 1;
    strncpy(s->entries[free_idx].key, key, sizeof(s->entries[free_idx].key) - 1);
    s->entries[free_idx].key[sizeof(s->entries[free_idx].key) - 1] = '\0';
    s->entries[free_idx].last_ms = now_ms;
    return true;
} 