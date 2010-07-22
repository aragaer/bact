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

#define UNWIND_THRESHOLD 10
#define MAX_SIZE 20
#define MAX_AREA MAX_SIZE*MAX_SIZE
#define SAFE_SPOT MAX_AREA+1
#define MAX_LEVELS MAX_AREA - 2*MAX_SIZE + 1

#define cell_t unsigned char

#define UNWIND_THRESHOLD 10

struct level_data {
    cell_t map[MAX_AREA+2], unwinded[MAX_AREA+2];
    int solution_depth, position, found5s, last_unmake;
} static start[MAX_LEVELS];


static int i[MAX_AREA], j[MAX_AREA];
static int up[MAX_AREA], down[MAX_AREA], left[MAX_AREA], right[MAX_AREA];
static int solution[MAX_AREA];

static int cols, rows, area, real_deathcount;

struct {
	int forks;
	int brokens;
        int failed_forks;
        int prevented;
} stat = {0,0,0,0};

#ifndef ONLINE_JUDGE

#define printlevel(l) print_out(l->unwinded)

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

/* Take out a bacteria. If it makes cell empty, add 4 and increase number of found deads. */
static inline void careful_dec(struct level_data *level, int pos) {
    cell_t *map = level->map;
    if (--map[pos] == 0) {
        map[pos] = 4;
        level->found5s++;
        level->unwinded[pos] = 5;
    }
}

/* only when we are sure it is not 1 */
static inline void dumb_dec(struct level_data *level, int pos) {
    level->map[pos]--;
}

/* only when we are sure it IS 1 */
static inline dumb_dec_to_5(struct level_data *level, int pos) {
    level->map[pos] = 4;
    level->found5s++;
    level->unwinded[pos] = 5;
}

/* 0 if everything is solved, -1 on error, 1 on partial success */
static inline int unmake(struct level_data *level) {
    cell_t *map = level->unwinded;
    int limit = level->position;
    int pos = limit, local_depth = level->solution_depth;

#ifndef ONLINE_JUDGE
        print_out(map);
        printf(" ||%d\n \\/\n", local_depth);
#endif
    debug(("Unwinding from below %d\n", limit));
    while (map[pos] != 1)
        pos++;
    while (pos < area) {
        if (untake_bacteria(map, pos))
            return -1;
        solution[local_depth++] = pos;
        if (local_depth == area)
            return 0;
#ifndef ONLINE_JUDGE
//        print_out(map);
//        printf(" ||%d (%d)\n \\/\n", local_depth, pos);
#endif
        if (pos >= limit + cols && map[up[pos]] == 1)
            pos = up[pos];
        else if (pos > limit && map[left[pos]] == 1)
            pos = left[pos];
        else /* fast-forward to next 1 */
            while (map[pos] != 1)
                pos++;
    }
    level->last_unmake = level->position;
    level->solution_depth = local_depth;
    debug(("Unmade %d of %d below position %d\n", local_depth, area, level->position));
    return local_depth != area;
}

static int find_order(struct level_data *level) {
    int pos, *deadsfound = &level->found5s;
    cell_t *map = level->map;

    if (*deadsfound == real_deathcount) { /* Unlikely but who knows */
        level->position = 0;
        return unmake(level);
    }

    for (pos = level->position - 1; pos > cols; pos--) {
        debug(("%d in position %d (%d,%d)\n", map[pos], pos, i[pos] + 1, j[pos] + 1));
        switch (map[pos]) {
        case 1:
            if (map[left[pos]] == 1) {
                stat.prevented++;
                return -1;
            }
            careful_dec(level, up[pos]);
            dumb_dec(level, left[pos]);
            if (*deadsfound > real_deathcount) /* the only case when we can get too much deads */
                return -1;
            break;
        case 2:
            if (j[pos] == 0)                         /* Nowhere to go, just one direction */
                continue;
            if (right[pos] == SAFE_SPOT)
                switch (map[up[pos]]) {
                case 1:
                    if (map[left[pos]] == 1) {
                        stat.prevented++;
                        return -1;
                    }
                    dumb_dec(level, left[pos]);
                    goto switch_exit;
                case 4:
                    if (map[left[pos]] == 4
                        || (j[pos] == 1 && map[left[pos]] == 3)) {
                        stat.prevented++;
                        return -1;
                    }
                    dumb_dec(level, up[pos]);
                    goto switch_exit;
                default:
                    break;
                }

            switch (map[left[pos]]) {
            case 1:
                careful_dec(level, up[pos]);
                break;
            case 4:
                dumb_dec(level, left[pos]);
                break;
            default:
                debug(("Forking. Do some unwinding first\n"));
                level->position = pos;
                if (level->last_unmake - pos > UNWIND_THRESHOLD) {
                    int res = unmake(level);
                    if (res != 1) /* Not PARTIAL success */
                        return res;
                }
                debug(("Forking now\n"));
                stat.forks++;
                /* Path 1 uses new level, path 2 uses current level */
                memcpy(level+1, level, sizeof(struct level_data));
                if (map[left[pos]] == 2) {
                    dumb_dec(level+1, left[pos]);
                    careful_dec(level, up[pos]);
                } else { //left is 3
                    dumb_dec(level, left[pos]);
                    careful_dec(level+1, up[pos]);
                }
                if (find_order(level+1) == 0) /* Path 1 succeeded */
                    return 0;
                stat.failed_forks++;
                debug(("Path 1 failed\n"));
                break;
            }
            break;
        case 3:
            if (j[pos] == 0) {                          /* 3 in the corner */
		stat.brokens++;
                return -1;
            }
            continue;
/*      case 4: Not supposed to be here */
        default:
            stat.brokens++;
            return -1; /* What? */
        }

switch_exit:
#ifndef ONLINE_JUDGE
        printlevel(level);
        printf(" ||\n \\/\n");
#endif
        if (*deadsfound == real_deathcount) {
            level->position = 0;
            return unmake(level);
        } /* else deadsfound < real_deathcount */
    }

    debug(("We lost some deads\n"));
    return -1;
}

int main() {
    int sum, pos;

    scanf("%d%d", &rows, &cols);
    area = cols*rows;

    for (pos = 0, sum = 0; pos < area; pos++) {
        scanf("%d", start->map + pos);
        sum += start->map[pos];
    }

    real_deathcount = area*3 - cols - rows - sum;
#ifdef ONLINE_JUDGE /* short-circuit check */
    if (real_deathcount%4) {
        printf("No\n");
        return 0;
    }
#endif
    real_deathcount /= 4;

    for (pos = 0; pos < area; pos++) {
        j[pos] = pos % cols;
        i[pos] = pos / cols;
        up[pos] = pos < cols ? SAFE_SPOT : pos - cols;
        down[pos] = pos >= area - cols ? SAFE_SPOT : pos + cols;
        left[pos] = pos % cols == 0 ? SAFE_SPOT : pos - 1;
        right[pos] = (pos+1) % cols == 0 ? SAFE_SPOT : pos + 1;
    }

    memcpy(start->unwinded, start->map, sizeof(start->map));
    start->unwinded[area] = 1;
    start->position = area;
    start->last_unmake = area;

    if (find_order(start) == 0) {
        printf("Yes\n");
        for (pos = area; pos--; )
            printf("%d %d\n", i[solution[pos]] + 1, j[solution[pos]] + 1);
    } else
        printf("No\n");

    debug(("Stat: forks: %d, broken: %d, failed forks: %d, prevented brokens: %d\n",
        stat.forks, stat.brokens, stat.failed_forks, stat.prevented));
}

