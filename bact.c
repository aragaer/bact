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
#define MAP_SIZE (copied_area*sizeof(cell_t))

static cell_t memory_pool[MAX_QS][(MAX_SIZE+2)*(MAX_SIZE+2)];
static char counted_already[(MAX_SIZE+2)*(MAX_SIZE+2)];

static int cols, rows, area, copied_area, rowlen;
static int nqs = 0, n5s, qs[MAX_QS];
static char solution[2*MAX_SIZE*MAX_SIZE];

#ifndef ONLINE_JUDGE
static void print_out(cell_t *map) {
    int i, j;
    for (i = 0; i < rows+2; i++) {
        for (j = 0; j < cols+2; j++)
            printf("%d ", map[i*rowlen + j]);
        printf("\n");
    }
}
#endif

/* Remove a bacteria from a neighbour cell */
static int dec_bacteria(cell_t *mp) {
    if (*mp == 0)
        return 0;
    if (*mp == 1)
        return -1;
    (*mp)--;
    return 0;
}

/* Revert a placement of bacteria */
static int untake_bacteria(cell_t *mp) {
    *mp = 0;
    return (dec_bacteria(mp - 1)
        ||  dec_bacteria(mp - rowlen)
        ||  dec_bacteria(mp + 1)
        ||  dec_bacteria(mp + rowlen));
}

/* check if number of non-zero neighbours is too low */
static int low_neigh_count(cell_t *mp) {
    return *mp > (mp[-1]!=0) + (mp[1]!=0) + (mp[rowlen]!=0) + (mp[-rowlen]) + 1;
}

/* This is one step of recursion turning 5s into 1s*/
static int do_step5to1(int used1s, int sdepth, int last_qnum) {
    int i, pq;
    cell_t *map = memory_pool[used1s], *new_map = memory_pool[used1s+1], *mp;
    int local_depth = sdepth;
    memset(counted_already+rowlen, copied_area, 0);
    for (i = rowlen + 1; i < copied_area + rowlen - 1; i++) {
        mp = map + i;
        switch (*mp) {
        case 0:
            continue;
        case 1:
            if (untake_bacteria(mp))
                return -1;
            solution[local_depth*2] =     i % rowlen;
            solution[local_depth*2 + 1] = i / rowlen;
            local_depth++;
            if (mp[-rowlen] == 1)
                i -= rowlen + 1;
            else if (mp[-1] == 1)
                i -= 2;
            break;
        default:
            if (!counted_already[i] && low_neigh_count(mp))
                return -1;
            counted_already[i] = 1;
            break;
        }
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
    for (pq = last_qnum + 1; pq < n5s + used1s + 1; pq++) {
        i = qs[pq];
        if (map[i] != 5)
            continue;
        if ((memory_pool[0][i-1] == 5 && map[i-1] == 0)
                || (memory_pool[0][i-rowlen] == 5 && map[i-rowlen] == 0))
            continue; /* one of elder questionable neigbours is already turned into 1 */

        memcpy(new_map+rowlen, map+rowlen, copied_area);

        solution[local_depth*2 - 2] = i % rowlen;
        solution[local_depth*2 - 1] = i / rowlen;
        debug(("Assigning 1 to position %d\n", i));
        if (untake_bacteria(new_map + i) == 0
                && do_step5to1(used1s+1, local_depth, pq) == 0) {/* This worked */
            return 0;
        }
        debug(("Assigning 1 to position %d failed\n", i));
        if ((memory_pool[0][i-1] == 5 && map[i-1] != 0)
                || (memory_pool[0][i-rowlen] == 5 && map[i-rowlen] != 0))
            return -1; /* one of elder questionable neigbours is already not 1 */
    }

    return -1;
}

int main() {
    int i, j, sum=0, pos;
    cell_t *map = memory_pool[0];

    scanf("%d%d", &rows, &cols);
    rowlen = cols + 2;
    area = cols*rows;
    copied_area = rowlen*rows;

    for (i = 1, pos = rowlen + 1; i <= rows; i++) {
        for (j = 1; j <= cols; j++, pos++) {
            scanf("%d", map + pos);
            sum += map[pos];
            if (i>1 && j>1 && i < rows && j < cols && map[pos] == 1) {
                qs[nqs++] = pos;
                map[pos] = 5;
            }
        }
        pos+=2;
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

    if (do_step5to1(0, 0, -1)) {
        printf("No\n");
    } else {
        printf("Yes\n");
        for (i = area; i--; )
            printf("%d %d\n", solution[i*2+1], solution[i*2]);
    }
}
