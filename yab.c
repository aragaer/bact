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
typedef int knowledge_t;

#define MAX_SIZE        20
#define MAX_AREA        MAX_SIZE*MAX_SIZE
#define MAX_LEVELS      MAX_AREA
#define MAX_AREA_W_SAFE MAX_AREA + 2
#define SAFE_SPOT       MAX_AREA + 1

#define EARLY   1
#define LATE    8
#define UNKNOWN 0

#define KNOWLEDGE_AROUND(kmap,pos) (kmap)[up[pos]] + (kmap)[down[pos]] + (kmap)[left[pos]] + (kmap)[right[pos]]
#define LATE_NEIGH_COUNT(kmap,pos) ((KNOWLEDGE_AROUND((kmap),(pos)) & 0x78) >> 4)
#define EARLY_NEIGH_COUNT(kmap,pos) (KNOWLEDGE_AROUND((kmap),(pos)) & 0x4)

struct level {
    int level;
    cell_t map[MAX_AREA_W_SAFE];
    knowledge_t knowledge[MAX_AREA_W_SAFE];
    int solution_depth;
    int lor_num, early_count;
} static start[MAX_LEVELS];

static int i[MAX_AREA], j[MAX_AREA];
static int up[MAX_AREA], down[MAX_AREA], left[MAX_AREA], right[MAX_AREA];
static int solution[MAX_AREA];
int late_or_early[MAX_AREA];

static int cols, rows, area, real_deathcount, lor_count;

#ifndef ONLINE_JUDGE

static void print_out(cell_t *map, knowledge_t *kmap) {
    int i, j;
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            switch (kmap[i*cols + j]) {
            case 1:
                printf(">");
                break;
            case 8:
                printf("<");
                break;
            default:
                printf(" ");
                break;
            }
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
        print_out(map, level->knowledge);
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
        print_out(map, level->knowledge);
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
    int lor, pos, i;
#ifndef ONLINE_JUDGE
        printf("============================\n");
        print_out(level->map, level->knowledge);
        printf(" ||\n \\/\n");
#endif
    debug(("Going to browse for lors from %d to %d\n", level->lor_num + 1, lor_count + level->level));
    for (lor = level->lor_num + 1; lor < lor_count; lor++) {
        pos = late_or_early[lor];
#ifndef ONLINE_JUDGE
        printf("Looking at LoR at %d now. %d/%d earlies, %d/%d lates so far\n",
            pos, level->early_count, real_deathcount, lor - level->early_count, lor_count-real_deathcount);
        print_out(level->map, level->knowledge);
        printf(" ||\n \\/\n");
#endif

        if (lor - level->early_count == lor_count - real_deathcount /* all lates */
                || level->early_count == real_deathcount) {         /* all earlies */
            for (i = lor + 1; i < lor_count; i++)
                if (level->map[late_or_early[i]] == 5)
                    level->map[late_or_early[i]] = 1;
            debug(("Found everything, let's solve it now\n"));
            return unmake(level);
        }

        if (level->map[pos] != 5) {
            level->knowledge[pos] = EARLY;
            level->early_count++;
        } else
        switch (KNOWLEDGE_AROUND(level->knowledge,pos)) {
        case 0:
        /* nothing known */
            debug(("Forking. Unmake as much as possible\n"));
            switch (unmake(level)) {
            case 0:
                return 0;
            case -1:
                return -1;
            default:
                break;
            }
            level->lor_num = lor;
            memcpy(level+1, level, sizeof(*level));
            (level+1)->knowledge[pos] = LATE;
            (level+1)->map[pos] = 1;
            if (solve(level+1) == 0)
                return 0;
            debug(("Failed. Fallback\n"));
            level->knowledge[pos] = EARLY;
            level->early_count++;
            break;
        case 1:
        case 2:
        case 3:
        case 4:
        /* lotta earlies around, no lates. Good */
            debug(("Is late\n"));
            level->map[pos] = 1;
            level->knowledge[pos] = LATE;
            break;
        case 8:
        case 16:
        case 24:
        case 32:
        /* lotta lates around, no earlies. Good */
            debug(("Is early\n"));
            level->knowledge[pos] = EARLY;
            level->early_count++;
            break;
        default:
        /* everything else is bad */
            return -1;
        }

        /* one more is set, check for neighbours now */
#define test(x) do {                                                        \
    if (LATE_NEIGH_COUNT(level->knowledge, x) > start->map[x] + 1           \
        || EARLY_NEIGH_COUNT(level->knowledge, x) > 5 - start->map[x]) {    \
        debug(("Unfit at %d", x));                                          \
        return -1;                                                          \
    }                                                                       \
} while (0)

        test(up[pos]);
        test(down[pos]);
        test(left[pos]);
        test(right[pos]);
    }
    
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
            late_or_early[lor_count++] = pos;
            debug(("%d,", pos));
        }
    }
    debug(("\b \n"));

    start->level = 0;
    start->lor_num = -1;
    start->map[area] = 1; /* stopper for unmake */

    real_deathcount = area*3 - cols - rows - sum;
#ifdef ONLINE_JUDGE /* short-circuit check */
    if (real_deathcount%4) {
        printf("No\n");
        return 0;
    }
#endif
    real_deathcount /= 4;

#define check_corner(a) do {                                                \
    switch (start->map[a] == 1) {                                           \
    case 4:                                                                 \
        goto no;                                                            \
    case 3:                                                                 \
        start->knowledge[a] = EARLY;                                        \
        break;                                                              \
    case 1:                                                                 \
        start->knowledge[a] = LATE;                                         \
        break;                                                              \
    default:                                                                \
        break;                                                              \
    }                                                                       \
} while (0);

    check_corner(0);
    check_corner(cols-1);
    check_corner(area-1);
    check_corner(area-cols);

#define check_side(a) do {                                                  \
    switch (start->map[a]) {                                           \
    case 4:                                                                 \
        start->knowledge[a] = EARLY;                                        \
        break;                                                              \
    case 1:                                                                 \
        start->knowledge[a] = LATE;                                         \
        break;                                                              \
    default:                                                                \
        break;                                                              \
    }                                                                       \
} while (0);

    /* first and last rows */
    for (pos = 1; pos < cols - 1; pos++) {
        check_side(pos);
        if (start->knowledge[pos] == EARLY && EARLY_NEIGH_COUNT(start->knowledge,pos)
                || start->knowledge[pos] == LATE && LATE_NEIGH_COUNT(start->knowledge,pos))
            goto no;

        check_side(area - pos);
        if (start->knowledge[area-pos] == EARLY && EARLY_NEIGH_COUNT(start->knowledge,area-pos)
                || start->knowledge[area-pos] == LATE && LATE_NEIGH_COUNT(start->knowledge,area-pos))
            goto no;
    }

    for (pos = cols; pos < area - cols; pos+=cols) {
        check_side(pos);
        if (start->knowledge[pos] == EARLY && EARLY_NEIGH_COUNT(start->knowledge,pos)
                || start->knowledge[pos] == LATE && LATE_NEIGH_COUNT(start->knowledge,pos))
            goto no;

        check_side(area - pos);
        if (start->knowledge[area-pos] == EARLY && EARLY_NEIGH_COUNT(start->knowledge,area-pos)
                || start->knowledge[area-pos] == LATE && LATE_NEIGH_COUNT(start->knowledge,area-pos))
            goto no;
    }

    if (solve(start) == 0) {
        printf("Yes\n");
        for (pos = area; pos--; )
            printf("%d %d\n", solution[pos] / cols + 1, solution[pos] % cols + 1);
    } else {
no:
        printf("No\n");
    }
}
