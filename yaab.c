#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef NO_INLINE
#define inline
#else
#define inline __inline
#endif

#ifdef ONLINE_JUDGE
#define debug(x)
#else
#define debug(x) printf x
#endif

typedef int cell_t;

#define MAX_SIZE        20
#define MAX_AREA        MAX_SIZE*MAX_SIZE
#define MAX_LEVELS      MAX_AREA
#define MAX_AREA_W_SAFE MAX_AREA + 2
#define SAFE_SPOT       MAX_AREA + 1

struct level {
    int level;
    cell_t map[MAX_AREA_W_SAFE];
    int solution_depth;
    int lor_num, early_count;
} static start[MAX_LEVELS];

static int i[MAX_AREA], j[MAX_AREA];
static int up[MAX_AREA], down[MAX_AREA], left[MAX_AREA], right[MAX_AREA];
static int solution[MAX_AREA];
static int qs[MAX_LEVELS];

static int cols, rows, area, n5s, nqs;

#ifndef ONLINE_JUDGE

static void print_out(cell_t *map) {
    int i, j;
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            printf("%d", map[i*cols + j]);
        }
        printf("\n");
    }
}
#endif

/* Remove a bacteria from a neighbour cell */
static inline int dec_bacteria(cell_t *mp) {
    if (*mp == 0)
        return 0;
    if (*mp == 1)
        return -1;
    (*mp)--;
    return 0;
}

/* Revert a placement of bacteria */
static inline int untake_bacteria(cell_t *map, int pos) {
    map[pos] = 0;
    return (dec_bacteria(map + up[pos])
        ||  dec_bacteria(map + down[pos])
        ||  dec_bacteria(map + left[pos])
        ||  dec_bacteria(map + right[pos]));
}

/* 0 if everything is solved, -1 on error, 1 on partial success */
static inline int unmake(struct level *level) {
    cell_t *map = level->map;
    int pos = 0, local_depth = level->solution_depth;

#ifndef ONLINE_JUDGE
        print_out(map);
        printf(" ||%d\n \\/\n", local_depth);
#endif
    while (map[pos] != 1)
        pos++;
    while (pos < area) {
        if (untake_bacteria(map, pos))
            return -1;
        solution[local_depth++] = pos;
        if (local_depth == area)
            return 0;
#ifndef ONLINE_JUDGE
        print_out(map);
        printf(" ||%d (%d)\n \\/\n", local_depth, pos);
#endif
        if (map[up[pos]] == 1)
            pos = up[pos];
        else if (map[left[pos]] == 1)
            pos = left[pos];
        else /* fast-forward to next 1 */
            while (map[pos] != 1)
                pos++;
    }
    level->solution_depth = local_depth;
    return local_depth != area;
}

static int solve(struct level *level) {
    return -1;
}

int main() {
    int sum, pos;

    scanf("%d%d", &rows, &cols);
    area = cols*rows;

    for (pos = 0; pos < area; pos++) {
        j[pos] = pos % cols;
        i[pos] = pos / cols;
        up[pos] = pos < cols ? SAFE_SPOT : pos - cols;
        down[pos] = pos >= area - cols ? SAFE_SPOT : pos + cols;
        left[pos] = pos % cols == 0 ? SAFE_SPOT : pos - 1;
        right[pos] = (pos+1) % cols == 0 ? SAFE_SPOT : pos + 1;
    }

    debug(("List of LoR's:"));
    for (pos = 0, sum = 0; pos < area; pos++) {
        scanf("%d", start->map + pos);
        sum += start->map[pos];
        if (start->map[pos] == 1 && i[pos] && j[pos] && i[pos] < rows-1 && j[pos] < cols-1) {
            start->map[pos] = 5;
            qs[lor_count++] = pos;
            debug(("%d,", pos));
        }
    }
    debug(("\b \n"));

    start->level = 0;
    start->map[area] = 1; /* stopper for unmake */

    n5s = area*3 - cols - rows - sum;
#ifdef ONLINE_JUDGE /* short-circuit check */
    if (real_deathcount%4) {
        printf("No\n");
        return 0;
    }
#endif
    n5s /= 4;

    if (solve(start) == 0) {
        printf("Yes\n");
        for (pos = area; pos--; )
            printf("%d %d\n", solution[pos] / cols + 1, solution[pos] % cols + 1);
    } else {
        printf("No\n");
    }
}
