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
    int direction, downpos, uppos;
} static start[MAX_LEVELS];


static int i[MAX_AREA], j[MAX_AREA];
static int up[MAX_AREA], down[MAX_AREA], left[MAX_AREA], right[MAX_AREA];
static int solution[MAX_AREA];
static int *solution_tail;

static int cols, rows, area, real_deathcount;

#ifdef ONLINE_JUDGE
#define statup(field)
#else

struct {
    int forks;
    int fork_depth;
    int brokens1;
    int brokens2;
    int failed_forks;
    int prevented;
    int sanity
} stat = {0,0,0,0,0,0,0};

#define statup(field) stat.field++
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

struct island {
    int is_good;
    int is_linked;
} islands[MAX_AREA/2], *island_map[MAX_AREA + 2];

int sanity_check(struct level_data *level) {
    int pos;
    cell_t *map = level->unwinded;
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

/* Take out a bacteria. If it makes cell empty, add 4 and increase number of found deads. */
static inline void careful_dec(struct level_data *level, int pos) {
    cell_t *map = level->map;
    if (--map[pos] == 0) {
        map[pos] = 4;
        level->found5s++;
        level->unwinded[pos] = 5;
//        solution_tail[-level->found5s] = pos;
        debug(("Added %d:%d to tail on careful dec\n", i[pos]+1, j[pos]+1));
    }
}

/* only when we are sure it is not 1 */
static inline void dumb_dec(struct level_data *level, int pos) {
    level->map[pos]--;
}

/* 0 if everything is solved, -1 on error, 1 on partial success */
static inline int unmake(struct level_data *level) {
    cell_t *map = level->unwinded;
    int limit = level->position;
    int pos = limit, local_depth = level->solution_depth;
    int to_unwind = area/* - level->found5s*/;

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
        if (local_depth == to_unwind)
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
    debug(("Unmade %d of %d below position %d\n", local_depth, to_unwind, level->position));

#ifndef ONLINE_JUDGE
    print_out(map);
#endif
    if (sanity_check(level)) {
	statup(sanity);
	return -1;
    }

    return local_depth != to_unwind;
}

static int find_order(struct level_data *level) {
    int pos, *deadsfound = &level->found5s;
    cell_t *map = level->map;
    int *walk_next = left, *walk_in = up, *walk_prev = right;

    if (*deadsfound == real_deathcount) { /* Unlikely but who knows */
        level->position = 0;
        return unmake(level);
    }

    for (pos = level->position - 1;
                walk_in == up ? pos > cols : pos < level->position;
                walk_in == up ? pos-- : pos++) {
        debug(("%d in position %d (%d,%d)\n", map[pos], pos, i[pos] + 1, j[pos] + 1));
        switch (map[pos]) {
        case 1:
/* Not really a huge optimization
            switch (map[left[pos]]) {
            case 4:
                if (j[pos] == 1) {
            case 1:
                    stat.prevented++;
                    return -1;
                }
                break;
            default:
                break;
            }
*/
            if ((walk_prev[pos] == SAFE_SPOT || walk_next[pos] == SAFE_SPOT)
                    && map[walk_in[pos]] == 1) {
                statup(prevented);
                return -1;
            }
            careful_dec(level, walk_in[pos]);
            dumb_dec(level, walk_next[pos]);
            break;
        case 2:
            if (walk_next[pos] == SAFE_SPOT) {               /* Nowhere to go, just one direction */
                if (map[walk_in[pos]] == 4) {
                    statup(prevented);
                    return -1;
                }
                break;
            }
            if (walk_prev[pos] == SAFE_SPOT)
                switch (map[walk_in[pos]]) {
                case 1:
/* Not really a huge optimization
                    if (map[left[pos]] == 1) {
                        stat.prevented++;
                        return -1;
                    }
*/
                    dumb_dec(level, walk_next[pos]);
                    goto switch_exit;
                case 4:
/* Not really a huge optimization
                    if (map[left[pos]] == 4) {
                        stat.prevented++;
                        return -1;
                    }
*/
                    dumb_dec(level, walk_in[pos]);
                    goto switch_exit;
                default:
                    break;
                }

            switch (map[walk_next[pos]]) {
            case 1:
                careful_dec(level, walk_in[pos]);
                break;
            case 4:
                dumb_dec(level, walk_next[pos]);
                break;
            default:
                if (level->uppos == -1)
                    if (walk_in == up) {
                        debug(("Flip!\n"));
                        walk_in = down;
                        walk_next = right;
                        walk_prev = left;
                        level->downpos = pos;
                        pos = level->uppos;
                        break;
                    } else {
                        debug(("Unflip!\n"));
                        pos = level->position;
                        walk_next = left;
                        walk_in = up;
                        walk_prev = right;
                        level->uppos = pos;
                        pos = level->downpos;
                    }

                debug(("Forking. Do some unwinding first\n"));
                level->position = pos;
                if (level->last_unmake - pos > UNWIND_THRESHOLD) {
                    int res = unmake(level);
                    if (res != 1) /* Not PARTIAL success */
                        return res;
                }
                debug(("Forking now\n"));
                statup(forks);
                /* Path 1 uses new level, path 2 uses current level */
                memcpy(level+1, level, sizeof(struct level_data));
                if (map[walk_next[pos]] == 2) {
                    dumb_dec(level+1, walk_next[pos]);
                    careful_dec(level, walk_in[pos]);
                } else { //left is 3
                    dumb_dec(level, walk_next[pos]);
                    careful_dec(level+1, walk_in[pos]);
                }
#ifndef ONLINE_JUDGE
		if (stat.fork_depth < level - start + 1)
			stat.fork_depth = level - start + 1;
#endif
                if (find_order(level+1) == 0) /* Path 1 succeeded */
                    return 0;
                statup(failed_forks);
                debug(("Path 1 failed\n"));
                break;
            }
            break;
        case 3:
            if (walk_next[pos] == SAFE_SPOT) {                          /* 3 in the corner */
		statup(brokens1);
                return -1;
            }
            if (walk_prev[pos] == SAFE_SPOT && map[walk_in[pos]] == 4) {
                statup(prevented);
                return -1;
            }
/*          Not really a huge optimization
            if ((j[pos] == 1 && map[left[pos]] == 3)
                    || map[left[pos]] == 4) {
                stat.prevented++;
                return -1;
            }
*/
            break;
/*      case 4: Not supposed to be here */
        default:
            statup(brokens2);
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

    solution_tail = solution + area;

    memcpy(start->unwinded, start->map, sizeof(start->map));
    start->unwinded[area] = 1;
    start->position = area;
    start->last_unmake = area;
    start->uppos = -1;

    if (find_order(start) == 0) {
        printf("Yes\n");
        for (pos = area; pos--; )
            printf("%d %d\n", i[solution[pos]] + 1, j[solution[pos]] + 1);
    } else
        printf("No\n");

    debug(("Stat: forks: %d, depth %d, broken: %d/%d, failed forks: %d, prevented brokens: %d, failed sanity checks: %d\n",
	stat.forks,
        stat.fork_depth, stat.brokens1, stat.brokens2, stat.failed_forks, stat.prevented, stat.sanity));
}

