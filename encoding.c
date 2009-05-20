#include <stdio.h>

#ifdef ONLINE_JUDGE
#define debug(x)
#else
#define debug(x) printf x
#endif

static char message[200001];

int main() {
    int c = 0;
    scanf("%s", message);

    while (c < strlen(message)) {
        if (message[c] == message[c+1]) {
            memmove(message+c, message+c+2, strlen(message+c) + 1);
            if (--c < 0)
                c = 0;
            debug(("%s\n", message));
        } else {
            c++;
        }
    }
    printf("%s\n", message);
    exit(0);
}
