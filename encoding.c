#include <stdio.h>

#ifdef ONLINE_JUDGE
#define debug(x)
#else
#define debug(x) printf x
#endif

static char message[200002];

int main() {
    char *src, *dst;
    scanf("%s", message+1);
    src = message + 2;
    dst = message + 1;

    while (*src) {
        debug(("[%s] src = %d, dst = %d\n", message+1, src-message, dst-message));
        if (*src == *dst) {
            --dst;
        } else {
            *++dst = *src;
        }
        src++;
    }
    *++dst = 0;
    printf("%s\n", message+1);
    exit(0);
}
