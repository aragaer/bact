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

#define UNWIND_THRESHOLD 10

#define MAX_SIZE        20
#define MAX_AREA        MAX_SIZE*MAX_SIZE
#define MAX_LEVELS      MAX_AREA
#define MAX_AREA_W_SAFE MAX_AREA + 2
#define SAFE_SPOT       MAX_AREA + 1

/* This struct is the portion of data analyzed within one level of recursion */
struct level {
    int level;				// Recursion depth
    cell_t map[MAX_AREA_W_SAFE];	// Map
    cell_t unwinded[MAX_AREA_W_SAFE];
    int found5s;
    int solution_depth;
    int last_unmake;
    int position, tension1, tension2, surface1, surface2;
} static start[MAX_LEVELS];

static int i[MAX_AREA], j[MAX_AREA], is_unk[MAX_AREA];
static int up[MAX_AREA], down[MAX_AREA], left[MAX_AREA], right[MAX_AREA];
static int solution[MAX_AREA];
struct q {
    int pos;
    struct q *next;
} static qs[MAX_LEVELS];

const int f[] = {-4, 4, 2, 0, -2, -4};

static int cols, rows, area, n5s, nqs;

#ifndef ONLINE_JUDGE

static void print_out(cell_t *map) {
    int i, j;
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            printf("%d ", map[i*cols + j]);
        }
        printf("\n");
    }
}

static void print_sliced(cell_t *map,   /* the map */
                         int pos,       /* current position */
                         int t1,        /* tension up */
                         int t2) {      /* tension down */
    int i, j, k, p;
    for (i = 0, p = 0; i < rows; i++) {
        for (j = 0; j < cols; j++, p++) {
            if (p == pos) {
                for (k = j; k < cols; k++)
                    printf("  ");
                printf("  v  %d\n", t1);
                for (k = 0; k < j; k++)
                    printf("  ");
            }
            printf("%d ", map[p]);
            if (p == pos) {
                for (k = j+1; k < cols; k++)
                    printf("  ");
                printf(" < > %d\n", f[map[pos]]);
                for (k = 0; k <= j; k++)
                    printf("  ");
            }
        }
        if (i == pos/cols) {
            for (k = j+1; k < cols; k++)
                printf("  ");
            printf("  ^  %d", t2);
        }
        printf("\n");
    }
}
#else
#define print_out(map)
#define print_sliced(map, pos, t1, t2)
#endif

/* Take out a bacteria. If it makes cell empty, add 4 and increase number of found deads. */
static inline void careful_dec(struct level *level, int pos) {
    cell_t *map = level->map;
    if (--map[pos] == 0) {
        map[pos] = 4;
        level->found5s++;
        level->unwinded[pos] = 5;
    }
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

/* 0 if everything is solved, -1 on error, 1 on partial success */
static inline int unmake(struct level *level) {
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
//        printf(" ||%d (%d:%d)\n \\/\n", local_depth, i[pos] + 1, j[pos] + 1);
#endif
        if (map[up[pos]] == 1)
            pos = up[pos];
        else if (map[left[pos]] == 1)
            pos = left[pos];
        else /* fast-forward to next 1 */
            while (map[pos] != 1)
                pos++;
    }
    level->last_unmake = limit;
    level->solution_depth = local_depth;
    debug(("Unmade %d of %d below position %d\n", local_depth, area, level->position));
    return local_depth != area;
}

static int pre_analyze(struct level *level) {
    int curpos, pos, qnum, tension1 = 0, tension2 = level->tension2;
    int up5s_max, up5s_min, down5s_max, down5s_min;
    cell_t *map = level->map;

    /* first line */
    for (curpos = 0; curpos < cols; curpos++)
        tension1 += f[map[curpos]];
    tension1 -= cols + 2; /* Tension goes up and to both sides */
    tension2 -= tension1; /* Total tension stays the same */
    level->found5s = 0;

    up5s_max = up5s_min = 0;
    down5s_max = down5s_min = n5s;
    qnum = 0;
    for (;curpos < area - cols; curpos++) {
        int max5s, min5s;

        debug(("looking at position %d:%d\n", i[curpos], j[curpos]));
        tension2 -= f[map[curpos]];
        if (right[curpos] == SAFE_SPOT)
            tension2 += 2;

        max5s = ((cols + 1 + tension2) >> 3);
        min5s = -((cols + 1 - tension2) >> 3);

        if (max5s < down5s_max) {
            debug(("@@@At least %d 5s are not in down now!\n", down5s_max - max5s));
            down5s_max = max5s;
            if (down5s_min > down5s_max)
                down5s_min = down5s_max;
        }
        if (min5s > down5s_min) {
            debug(("@@@At least %d 5s are in down now!\n", min5s - down5s_min));
            down5s_min = min5s;
            if (down5s_min > down5s_max)
                down5s_max = down5s_min;
        }

        if (!is_unk[curpos] && down5s_max + up5s_min < n5s - level->found5s) {
            debug(("We've missed at least one unknown\n"));
            up5s_min = n5s - level->found5s - down5s_max;
            if (up5s_min > up5s_max)
                up5s_max = up5s_min;
        }

        if (is_unk[curpos]) {
            qnum++;
            debug(("^^^^Passing an unknown\n"));
        }

        debug(("%d to %d in up, %d to %d in down. Possible from %d-%d to %d-%d\n",
                up5s_min, up5s_max, down5s_min, down5s_max,
                up5s_min, down5s_max, up5s_max, down5s_min));

        {
            int is5_1 = (down5s_min + up5s_max < n5s - level->found5s);
            int is5_2 = (down5s_max + up5s_min < n5s - level->found5s);
            if (is5_1 || is5_2) {
                if (is5_1 && is5_2 && is_unk[curpos]) {
                    debug(("!!!!Found 5 in position %d:%d\n", i[curpos] + 1, j[curpos] + 1));
                    map[curpos] = 5;
                    level->found5s++;
                } else {
                    debug(("DESYNC (%d -> %d:%d at %d)\n", n5s - level->found5s, down5s_min + up5s_max, down5s_max + up5s_min, map[curpos]));
                    print_sliced(map, curpos, tension1, tension2);
                    debug(("Total tension %d+%d+%d = %d\n", tension1, tension2, f[map[curpos]], tension1+tension2+f[map[curpos]]));
                    return -1;
                }
            }
        }

        tension1 += f[map[curpos]];
        max5s = ((cols + 1 + tension1) >> 3);
        min5s = -((cols + 1 - tension1) >> 3);
        if (right[curpos] == SAFE_SPOT)
            tension1 -= 2;
        if (max5s < up5s_max) {
            debug(("###At least %d 5s are not in up now!\n", up5s_max - max5s));
            up5s_max = max5s;
            if (up5s_min > up5s_max)
                up5s_min = up5s_max;
        }
        if (min5s > up5s_min) {
            debug(("###At least %d 5s are in up now!\n", min5s - up5s_min));
            up5s_min = min5s;
            if (up5s_min > up5s_max)
                up5s_max = up5s_min;
        }
        debug(("Tensions are %d + %d = %d\n", tension1, tension2, tension1+tension2));
    }

    print_out(map);
    return 0;
}

static int analyze(struct level *level) {
    int pos, *deadsfound = &level->found5s;
    cell_t *map = level->map;

    if (*deadsfound == n5s) { /* Unlikely but who knows */
        level->position = 0;
        return unmake(level);
    }

    for (pos = level->position - 1; pos > cols; pos--) {
        debug(("%d in position %d (%d,%d)\n", map[pos], pos, i[pos] + 1, j[pos] + 1));
        switch (map[pos]) {
        case 1:
            careful_dec(level, up[pos]);
            careful_dec(level, left[pos]);
            if (*deadsfound > n5s) /* the only case when we can get too much deads */
                return -1;
            break;
        case 2:
            if (j[pos] == 0)                         /* Nowhere to go, just one direction */
                continue;
            if (right[pos] == SAFE_SPOT)
                switch (map[up[pos]]) {
                case 1:
                    careful_dec(level, left[pos]);
                    goto switch_exit;
                case 4:
                    careful_dec(level, up[pos]);
                    goto switch_exit;
                default:
                    break;
                }

            switch (map[left[pos]]) {
            case 1:
                careful_dec(level, up[pos]);
                break;
            case 4:
                careful_dec(level, left[pos]);
                break;
            default:
                if (map[up[pos]] == 5) {// this can happen!
                    careful_dec(level, up[pos]);
                    break;
                }
                debug(("Forking. Do some unwinding first\n"));
                level->position = pos;
                if (level->last_unmake - pos > UNWIND_THRESHOLD && unmake(level) == -1)
                    /*return -1*/;
                debug(("Forking now\n"));
                /* Path 1 uses new level, path 2 uses current level */
                memcpy(level+1, level, sizeof(struct level));
                careful_dec(level, up[pos]);
                careful_dec(level+1, left[pos]);
                if (analyze(level+1) == 0) /* Path 1 succeeded */
                    return 0;
                debug(("Path 1 failed\n"));
                break;
            }
            break;
        case 3:
            if (j[pos] == 0)                         /* 3 in the corner */
                return -1;
            continue;
/*      case 4: Not supposed to be here */
        default:
            return -1; /* What? */
        }

switch_exit:
#ifndef ONLINE_JUDGE
        print_out(level->unwinded);
        printf(" ||\n \\/\n");
#endif
        if (*deadsfound == n5s) {
            debug(("All deads found!\n"));
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

    /* Fill the arrays used for map transition */
    for (pos = 0; pos < area; pos++) {
        j[pos] = pos % cols;
        i[pos] = pos / cols;
        up[pos] = pos < cols ? SAFE_SPOT : pos - cols;
        down[pos] = pos >= area - cols ? SAFE_SPOT : pos + cols;
        left[pos] = pos % cols == 0 ? SAFE_SPOT : pos - 1;
        right[pos] = (pos+1) % cols == 0 ? SAFE_SPOT : pos + 1;
    }

    /* Find all undefined 1's and calculate the total tension */
    debug(("List of LoR's: "));
    for (pos = 0, sum = 0; pos < area; pos++) {
        scanf("%d", start->map + pos);
        sum += start->map[pos];
        if (start->map[pos] == 1 && i[pos] && j[pos] && i[pos] < rows-1 && j[pos] < cols-1) {
            is_unk[pos] = 1;
            qs[nqs].pos = pos;
            qs[nqs].next = qs + nqs + 1;
            nqs++;
            debug(("[%d:%d],", i[pos], j[pos]));
        }
    }
    qs[nqs-1].next = NULL;
    debug(("\b \n"));

    start->level = 0;
    start->map[area] = 1; /* stopper for unmake */

    n5s = area*3 - cols - rows - sum;
#ifdef ONLINE_JUDGE /* short-circuit check */
    if (n5s%4) {
        printf("No\n");
        return 0;
    }
#endif
    n5s /= 4;

    start->tension2 = n5s*8;
    debug(("Total number of 5's = %d, initial tension = %d\n", n5s, n5s*8));
    if (pre_analyze(start)) {
        printf("No\n");
        return 0;
    }
    n5s -= start->found5s;
    debug(("Preanalysis removed %d 5s, new number is %d\n", start->found5s, n5s)); 
    start->found5s = 0;

    memcpy(start->unwinded, start->map, sizeof(start->map));
    start->unwinded[area] = 1;
    start->position = area;
    start->last_unmake = area;

    if (analyze(start) == 0) {
        printf("Yes\n");
        for (pos = area; pos--; )
            printf("%d %d\n", i[solution[pos]] + 1, j[solution[pos]] + 1);
    } else {
        printf("No\n");
    }
}

