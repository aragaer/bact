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

#define MAX_SIZE 20
#define MAX_AREA MAX_SIZE*MAX_SIZE
#define SAFE_SPOT MAX_AREA+1

#define MAX_QS (MAX_SIZE-2)*(MAX_SIZE-2)

#define cell_t unsigned char
#define MAP_SIZE (copied_area*sizeof(cell_t))

static cell_t memory_pool[MAX_QS][MAX_AREA + 2];

static int cols, rows, area, copied_area, rowlen;
static int nqs = 0, n5s, qs[MAX_QS];
static int solution[MAX_AREA];
static int i[MAX_AREA], j[MAX_AREA];
static int up[MAX_AREA], down[MAX_AREA], left[MAX_AREA], right[MAX_AREA];

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

struct island {
    int is_good;
    int is_linked;
} islands[MAX_AREA/2], *island_map[MAX_AREA + 2];

int sanity_check(cell_t *map) {
    int pos;
    struct island *cur_island = NULL, *last_island = islands;
    
    memset(islands, 0, sizeof islands);
    memset(island_map, 0, sizeof island_map);

    debug(("Islands!\n"));

    for (pos = 0; pos < area; pos++) {
        if (map[pos] == 0 && cur_island) {
            last_island = cur_island;
            cur_island = NULL;
        } else if (map[pos]) {
            if (!cur_island)
                cur_island = island_map[up[pos]]
                    ? island_map[up[pos]]
                    : last_island + 1;
            cur_island->is_good |= map[pos] == 1;
        }
	if (cur_island && island_map[up[pos]]) {
	    cur_island->is_linked |= cur_island != island_map[up[pos]];
	    cur_island->is_good |= island_map[up[pos]]->is_good;
	}
        island_map[pos] = cur_island;
#ifndef ONLINE_JUDGE
        if (island_map[pos])
            printf("%02d%c ", island_map[pos] - islands, island_map[pos]->is_good ? ' ' : '!');
        else
            printf("    ");
        if (right[pos] == SAFE_SPOT)
            printf("\n");
#endif
    }

    if (cur_island)
	last_island = cur_island;

    for (cur_island = islands + 1; cur_island <= last_island; cur_island++)
	if (!cur_island->is_good && !cur_island->is_linked)
	    return -1;

    return 0;
}

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

/* This is one step of recursion turning 5s into 1s*/
static int do_step5to1(int used1s, int sdepth, int last_qnum) {
    int pos, pq;
    cell_t *map = memory_pool[used1s], *new_map = memory_pool[used1s+1], *mp;
    int local_depth = sdepth;
    while (map[pos] != 1)
        pos++;

    while (pos < area) {
        if (untake_bacteria(map, pos))
            return -1;
        solution[local_depth++] = pos;
        if (map[up[pos]] == 1)
            pos = up[pos];
        else if (map[left[pos]] == 1)
            pos = left[pos];
        else
            while (map[pos] != 1)
                pos++;
    }

/* check the results */
    if (local_depth == area) /* Solved! */
        return 0;

#ifndef ONLINE_JUDGE
    print_out(map);
    printf("  ||\n  \\/\n");
#endif
    if (sanity_check(map)) {
	debug(("Sanity check failed\n"));
	return -1;
    }
/* We did everything we can do without assigning another non-5 cell */
    for (pq = last_qnum + 1; pq < n5s + used1s + 1; pq++) {
        pos = qs[pq];
        mp = map + pos;
        if (*mp != 5)
            continue;
        if (((memory_pool[0][left[pos]] == 5 || memory_pool[0][left[pos]] == 1) && map[left[pos]] == 0)
                || ((memory_pool[0][up[pos]] == 5 || memory_pool[0][up[pos]] == 1) && map[up[pos]] == 0))
            continue; /* one of elder questionable neigbours is already turned into 1 */

        memcpy(new_map, map, SAFE_SPOT * sizeof(cell_t));

        solution[local_depth] = pos;
        debug(("Assigning 1 to position %d\n", pos));
        if (untake_bacteria(new_map, pos) == 0
                && do_step5to1(used1s+1, local_depth+1, pq) == 0) {/* This worked */
            return 0;
        }
        debug(("Assigning 1 to position %d failed\n", pos));
        if ((memory_pool[0][left[pos]] == 5 && map[left[pos]] != 0)
                || (memory_pool[0][up[pos]] == 5 && map[up[pos]] != 0))
            return -1; /* one of elder questionable neigbours is already not 1 */
        if (map[left[pos]]==0 || map[up[pos]]==0 || map[down[pos]]==0 || map[right[pos]] == 0)
            return -1; /* can't be 5 */
    }

    return -1;
}

int main() {
    int sum, pos;
    cell_t *basemap = memory_pool[0], *map = memory_pool[1];

    scanf("%d%d", &rows, &cols);
    area = cols*rows;

    for (pos = 0, sum = 0; pos < area; pos++) {
        scanf("%d", map + pos);
        sum += map[pos];
    }

    n5s = area*3 - cols - rows - sum;
#ifdef ONLINE_JUDGE /* short-circuit check */
    if (n5s%4) {
        printf("No\n");
        return 0;
    }
#endif
    n5s /= 4;

    for (pos = 0; pos < area; pos++) {
        j[pos] = pos % cols;
        i[pos] = pos / cols;
        up[pos] = pos < cols ? SAFE_SPOT : pos - cols;
        down[pos] = pos >= area - cols ? SAFE_SPOT : pos + cols;
        left[pos] = pos % cols == 0 ? SAFE_SPOT : pos - 1;
        right[pos] = (pos+1) % cols == 0 ? SAFE_SPOT : pos + 1;
        if (map[pos] == 1 &&
                up[pos] != SAFE_SPOT && left[pos] != SAFE_SPOT &&
                up[pos] != SAFE_SPOT && down[pos] != SAFE_SPOT) {
            map[pos] = 5;
            qs[nqs++] = pos;
        }
    }

    debug(("We've got %d potential 5s, %d real ones\n", nqs, n5s));

    map[MAX_AREA] = 1;

    memcpy(basemap, map, SAFE_SPOT*sizeof(cell_t));

    if (do_step5to1(0, 0, -1)) {
        printf("No\n");
    } else {
        printf("Yes\n");
        for (pos = area; pos--; )
            printf("%d %d\n", i[pos] + 1, j[pos] + 1);
    }
}
