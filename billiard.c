#include <stdio.h>
#include <string.h>

static unsigned long int stack[100000];
static unsigned long int next[1000002], prev[1000002]; /* untaken lists */

int main() {
    unsigned long int n, curr, *sp = stack;

    memset(stack, 0, sizeof stack);

    for (curr = 1; curr <= sizeof(stack); curr++) {
        next[curr] = curr + 1;
        prev[curr] = curr - 1;
    }

    scanf("%ld", &n);
    while (n--) {
        scanf("%ld", &curr);
        prev[next[curr]] = prev[curr];
        next[prev[curr]] = next[curr];

        if (curr < *sp) { /* This one should be deep inside */
            printf("Cheater\n");
            return 0;
        }
/* We didn't know this ball was inside, assume all lower balls are there */
/* OR this one is on top of our stack. Doesn't matter */
        curr = prev[curr];
        sp += curr - *sp;
        *sp = curr;
    }
    printf("Not a proof\n");
    return 0;
}
