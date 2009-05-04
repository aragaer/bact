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
#define MAX_LEVELS MAX_AREA - 2*MAX_SIZE + 1

#define cell_t char
#define link_t char

#define UNWIND_THRESHOLD 10

struct level_data {
    cell_t map[MAX_AREA], unwinded[MAX_AREA+1];
    int solution_depth, position, found5s, last_unmake;
} static start[MAX_LEVELS];

static int solution[MAX_AREA];

static int cols, rows, area, real_deathcount;

/* 
* Links.
*  bit 0 shows relation from THIS cell to the cell above it.
*  bit 1 shows relation from THIS cell to the cell to the left from it.
*  0 means that THIS cell is OLDER and bacteria moved TO here
*  1 means that THIS cell is YOUNGER and bacteria moved FROM here
*/

#define IS_H_YOUNGER(x) (x & 2)
#define IS_V_YOUNGER(x) (x & 1)

#define LINK_OO 0
#define LINK_OY 1
#define LINK_YO 2
#define LINK_YY 3

#ifndef ONLINE_JUDGE
static char hlink[] = ">><<";
static char vlink[] = "v^v^";

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
    int x = pos % cols;
    int y = pos /cols;
    cell_t *mp = map+pos;
    *mp = 0;
    return ((x > 0      && dec_bacteria(mp - 1))
        ||  (y > 0      && dec_bacteria(mp - cols))
        ||  (x < cols-1 && dec_bacteria(mp + 1))
        ||  (y < rows-1 && dec_bacteria(mp + cols)));
}

/* Take out a bacteria. If it makes cell empty, add 4 and increase number of found deads. */
static inline int careful_dec(struct level_data *level, int pos) {
    cell_t *map = level->map;
    if (--map[pos] == 0) {
        map[pos] = 4;
        level->found5s++;
        level->unwinded[pos] = 5;
    }
    return 0;
}

/* 0 if everything is solved, -1 on error, 1 on partial success */
static inline int unmake(struct level_data *level) {
    cell_t *map = level->unwinded;
    const int limit = level->position;
    int pos = limit, local_depth = level->solution_depth;

#ifndef ONLINE_JUDGE
        print_out(map);
        printf(" ||%d\n \\/\n", local_depth);
#endif
    debug(("Unwinding from below %d\n", limit));
    while (map[pos] != 1)
        pos++;
    pos--;
    debug(("First 1 located at %d\n", pos));
    while (++pos < area) {
        if (untake_bacteria(map, pos))
            return -1;
        solution[local_depth++] = pos;
        if (local_depth == area)
            break;
#ifndef ONLINE_JUDGE
//        print_out(map);
//        printf(" ||%d (%d)\n \\/\n", local_depth, pos);
#endif
        if (pos >= limit + cols && map[pos-cols] == 1)
            pos -= cols + 1;
        else if (pos > limit && map[pos-1] == 1)
            pos -= 2;
        else /* fast-forward to next 1 */
            while (map[pos+1] != 1)
                pos++;
        debug(("Next 1 located at %d\n", pos));
    }
    level->last_unmake = limit;
    level->solution_depth = local_depth;
    debug(("Unmade %d of %d below position %d\n", local_depth, area, level->position));
    return local_depth != area;
}

static int find_order(struct level_data *level) {
    int i, j, pos, *deadsfound = &level->found5s;
    cell_t *map = level->map;
    link_t link;

    if (*deadsfound == real_deathcount) { /* Unlikely but who knows */
        level->position = 0;
        return unmake(level);
    }

    i = level->position / cols;
    j = level->position % cols;
    for (pos = level->position; pos--;) {
        if (!j--) {
            j = cols-1;
            if (!--i && *deadsfound < real_deathcount) /* we won't find any dead in last line */
                return -1;
        }
        debug(("%d in position %d (%d,%d)\n", map[pos], pos, i+1, j+1));
        switch (map[pos]) {
        case 0:
        case 4:
            return -1;                          /* Not supposed to be here */
        case 1:
            link = LINK_YY;
            break;
        case 3:
            if (i == 0 || j == 0)               /* 3 in the corner */
                return -1;
            link = LINK_OO;
            continue;
        case 2:
            if (i == 0 && j == 0)               /* 2 in the end */
                return -1;
            if (j && map[pos-1] == 1) {         /* this 2 was before that neighbour */
                link = LINK_OY;
            } else if (j && map[pos-1] == 4) {  /* this 2 was after that neighbour */
                link = LINK_YO;
            } else if (i && j == cols-1 && map[pos-cols] == 1) {
                link = LINK_YO;
            } else if (i && j == cols-1 && map[pos-cols] == 4) {
                link = LINK_OY;
            } else if (!i || !j) {                    /* Nowhere to go, just one direction */
                link = LINK_OO;
                continue;
            } else {
                debug(("Forking. Do some unwinding first\n"));
                level->position = pos;
                if (level->last_unmake - pos > UNWIND_THRESHOLD && unmake(level) == -1)
                    return -1;
                debug(("Forking now\n"));
                /* Path 1 uses new level, path 2 uses current level */
                memcpy(level+1, level, sizeof(struct level_data));
                link = LINK_OY;
                careful_dec(level+1, pos-1);
                if (find_order(level+1) == 0) /* Path 1 succeeded */
                    return 0;
                debug(("Path 1 failed\n"));
            }
            break;
        default:
            return -1; /* What? */
        }

#ifndef ONLINE_JUDGE
        printf("These be %c and %c\n", hlink[link], vlink[link]);
        printlevel(level);
        printf(" ||\n \\/\n");
#endif
        if ((j && IS_H_YOUNGER(link) && careful_dec(level, pos-1))
                || (i && IS_V_YOUNGER(link) && careful_dec(level, pos-cols)))
            return -1;
        if (*deadsfound < real_deathcount)
            continue;
        if (*deadsfound == real_deathcount) {
            level->position = 0;
            return unmake(level);
        }
        return -1; /* deadsfound > real_deathcount */
    }

    debug(("Nah, we lost some deads\n"));
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

    memcpy(start->unwinded, start->map, sizeof(start->map));
    start->unwinded[area] = 1;
    start->position = area;
    start->last_unmake = area;

    if (find_order(start) == 0) {
        printf("Yes\n");
        for (pos = area; pos--; )
            printf("%d %d\n", solution[pos] / cols + 1, solution[pos] % cols + 1);
    } else
        printf("No\n");
}
