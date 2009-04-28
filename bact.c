#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define inline __inline

#ifdef ONLINE_JUDGE
#define debug(x)
#else
#define debug(x) printf x
#endif

#define MAX_SIZE 20
#define MAX_QS (MAX_SIZE-2)*(MAX_SIZE-2)

#define cell_t char
#define MAP_SIZE (MAX_SIZE*MAX_SIZE*sizeof(cell_t))

static cell_t memory_pool[MAX_QS][MAX_SIZE*MAX_SIZE];

static int cols, rows, area;
static int nqs = 0, n5s, qs[MAX_QS];
static char *solution;

#ifndef ONLINE_JUDGE
static void print_out(cell_t *map) {
    int i, j;
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++)
            printf("%d ", map[i*cols + j]);
        printf("\n");
    }
}
#endif

/* Remove a bacteria from a neighbour cell */
static inline int dec_bacteria(cell_t *mp) {
    switch (*mp) {
    case 1:
        return -1;
    default:
        (*mp)--;
    case 0:
        return 0;
    }
}

/* Revert a placement of bacteria */
static inline int untake_bacteria(cell_t *map, int pos) {
    int x = pos % cols;
    int y = pos / cols;
    map[pos] = 0;
    return (( x > 0      && dec_bacteria(map + pos - 1   ))
        ||  ( y > 0      && dec_bacteria(map + pos - cols))
        ||  ( x < cols-1 && dec_bacteria(map + pos + 1   ))
        ||  ( y < rows-1 && dec_bacteria(map + pos + cols)));
}

/* This is one step of recursion */
static int do_step(int used1s, int sdepth, int last_qnum) {
    int i, p;
    cell_t *map = memory_pool[used1s], *new_map = memory_pool[used1s+1];
    int local_depth = sdepth;

    for (i = 0; i < area; i++) {
        if (map[i] != 1)
            continue;
        if (untake_bacteria(map, i))
            return -1;
        solution[local_depth*2] =     i % cols + 1;
        solution[local_depth*2 + 1] = i / cols + 1;
        local_depth++;
        if (i > cols && map[i - cols] == 1)
            i -= cols + 1;
        else if (i > 0 && map[i - 1] == 1)
            i -= 2;
    }

/* check the results */
    if (local_depth == area) /* Solved! */
        return 0;

#ifndef ONLINE_JUDGE
    print_out(map);
    printf("  ||\n  \\/\n");
#endif
/* We did everything we can do without assigning another non-5 cell */
    local_depth++;
    for (p = last_qnum + 1; p < n5s + used1s + 1; p++) {
        i = qs[p];
        if (map[i] != 5)
            continue;

        memcpy(new_map, map, MAP_SIZE);

        solution[local_depth*2 - 2] = i % cols + 1;
        solution[local_depth*2 - 1] = i / cols + 1;
        if (untake_bacteria(new_map, i) == 0
                && do_step(used1s+1, local_depth, p) == 0) /* This worked */
            return 0;
    }

    return -1;
}

int main() {
    int i, j, sum=0, pos;
    cell_t *map = memory_pool[0];

    scanf("%d%d", &rows, &cols);
    area = cols*rows;

    solution = malloc(area*sizeof(char));

    for (i = 0, pos = 0; i < rows; i++)
        for (j = 0; j < cols; j++, pos++) {
            scanf("%d", map + pos);
            sum += map[pos];
            if (i && j && i < rows-1 && j < cols-1 && map[pos] == 1) {
                qs[nqs++] = pos;
                map[pos] = 5;
            }
        }

    n5s = area*3 - cols - rows - sum;
#ifdef ONLINE_JUDGE /* short-circuit check */
    if (n5s%4) {
        printf("No\n");
        return 0;
    }
#endif
    n5s /= 4;
    debug(("We've got %d potential 5s, %d real ones\n", nqs, n5s));

    if (do_step(0, 0, -1)) {
        printf("No\n");
    } else {
        printf("Yes\n");
        for (i = area; i--; )
            printf("%d %d\n", solution[i*2+1], solution[i*2]);
    }
}
