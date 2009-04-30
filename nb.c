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

#define cell_t char

struct link {
    char h, v;
} static teh_complete_link_memory[MAX_SIZE*MAX_SIZE][MAX_SIZE*MAX_SIZE];
static struct link *teh_final_link;
static cell_t teh_complete_map_memory[MAX_SIZE*MAX_SIZE][MAX_SIZE*MAX_SIZE];
static int solution[MAX_SIZE*MAX_SIZE];

#define YOUNGER -1
#define OLDER 1

static int cols, rows, area, rowlen;

/* 
* Links.
*  v-link shows relation from THIS cell to the cell above it.
*  h-link shows relation from THIS cell to the cell to the left from it.
*  1 means that THIS cell is OLDER and bacteria moved TO here
* -1 means that THIS cell is YOUNGER and bacteria moved FROM here
*/

static char hlink[] = {'<', '?', '>'};
static char vlink[] = {'^', '?', 'v'};

#ifndef ONLINE_JUDGE
static void print_out(cell_t *map) {
    int i, j;
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++)
            printf("%d ", map[i*rowlen + j]);
        printf("\n");
    }
}

static void printme(cell_t *map, struct link *link) {
    int i, j, pos;
    for (i = 0, pos = 0; i < rows; i++) {
        for (j = 0; j < cols; j++, pos++) {
            printf("%d", map[pos]);
            if (j < cols - 1)
                printf("%c", hlink[link[pos+1].h+1]);
        }
        printf("\n");
        if (i == rows-1)
            break;
        for (j = 0; j < cols; j++) {
            printf("%c ", vlink[link[i*cols + j +cols].v+1]);
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
    int x = pos % cols;
    int y = pos /cols;
    cell_t *mp = map+pos;
    *mp = 0;
    return ((x > 0 && dec_bacteria(mp - 1))
        ||  (y > 0 && dec_bacteria(mp - rowlen))
        ||  (x < cols -1 && dec_bacteria(mp + 1))
        ||  (y < rows -1 && dec_bacteria(mp + rowlen)));
}

static int go_and_unwind_whats_left(int depth, int oldpos) {
    int i, j, pos;
    char *map = teh_complete_map_memory[depth+1], *newmap = teh_complete_map_memory[depth+2];
    struct link *link = teh_complete_link_memory[depth], *newlink = teh_complete_link_memory[depth+1];
    for (pos = oldpos+1; pos--;) {
        i = pos / cols;
        j = pos % cols;
        debug(("%d in position %d (%d,%d)\n", map[pos], pos, i+1, j+1));
        switch (map[pos]) {
        case 0:
        case 4:
            debug(("No (badnum!)\n"));
            return -1;
        case 1:
            link[pos].h = YOUNGER;
            link[pos].v = YOUNGER;
            break;
        case 3:
            if (i == 0 || j == 0) {
                debug(("No (3 in the corner)\n"));
                return -1;
            }
            link[pos].h = OLDER;
            link[pos].v = OLDER;
            break;
        case 2:
            if (i == 0 && j == 0) {
                debug(("No (2 in the corner)\n"));
                return -1;
            }
            if (j && map[pos-1] == 1) { /* this 2 was before that neighbour */
                link[pos].h = OLDER;
                link[pos].v = YOUNGER;
            } else if (!i) {                    /* Nowhere to go but to the right */
                link[pos].h = OLDER;
            } else if (!j) {                    /* Nowhere to go but up */
                link[pos].v = OLDER;
            } else {
                debug(("Ugh. Dunnae what to do. Fork?\n"));
                /* Teh path 1 */
                memcpy(newmap, map, sizeof(teh_complete_map_memory[0]));
                memcpy(newlink, link, sizeof(teh_complete_link_memory[0]));
                newmap[pos-1]--;
                if (newmap[pos-1] == 0) 
                    newmap[pos-1] = 4;
                newlink[pos].h = YOUNGER;
                newlink[pos].v = OLDER;
                if (go_and_unwind_whats_left(depth+1, pos-1) == 0)
                    goto out;
                /* Teh path 2 */
                memcpy(newmap, map, sizeof(teh_complete_map_memory[0]));
                memcpy(newlink, link, sizeof(teh_complete_link_memory[0]));
                newmap[pos-cols]--;
                if (newmap[pos-cols] == 0) 
                    newmap[pos-cols] = 4;
                newlink[pos].h = OLDER;
                newlink[pos].v = YOUNGER;
                if (go_and_unwind_whats_left(depth+1, pos-1) == 0)
                    goto out;
                return -1;
            }
            break;
        default:
            break;
        }
        if (j && link[pos].h == YOUNGER) {
            map[pos-1]--;
            if (map[pos-1] == 0) 
                map[pos-1] = 4;
        }
        if (i && link[pos].v == YOUNGER) {
            map[pos-cols]--;
            if (map[pos-cols] == 0) 
                map[pos-cols] = 4;
        }
#ifndef ONLINE_JUDGE
        printf("These be %c and %c\n", vlink[link[pos].v+1], hlink[link[pos].h+1]);
        printme(map, link);
        printf(" ||\n \\/\n");
#endif
    }
    teh_final_link = link;
#ifndef ONLINE_JUDGE
    printf("Yay! Done!\n");
    printme(teh_complete_map_memory[0], link);
#endif
out:
    return 0;
}

int main() {
    int sum, pos, local_depth, deathcount;
    cell_t *basemap = teh_complete_map_memory[0];
    cell_t *map = teh_complete_map_memory[1];
    cell_t *mp;

    scanf("%d%d", &rows, &cols);
    area = cols*rows;
    rowlen = cols;

    for (pos = 0, sum = 0; pos < area; pos++) {
        scanf("%d", &basemap[pos]);
        sum += basemap[pos];
    }

    deathcount = area*3 - cols - rows - sum;
#ifdef ONLINE_JUDGE /* short-circuit check */
    if (deathcount%4) {
        printf("No\n");
        return 0;
    }
#endif

    deathcount /= 4;

    memcpy(map, basemap, sizeof(teh_complete_map_memory[0]));
#ifndef ONLINE_JUDGE
    printme(basemap, teh_complete_link_memory[0]);
    printf(" ||\n \\/\n");
#endif
    if (go_and_unwind_whats_left(0, area-1)) {
        printf("No\n");
        return 0;
    }

    debug(("Outta unwinding!\n"));

    for (pos = 0; pos < area; pos++)
        if (basemap[pos] == 1 && teh_final_link[pos].h == OLDER) {/* One link is enough */
            basemap[pos] = 5;
            deathcount--;
        }

    if (deathcount) {
        printf("No\n");
        return 0;
    }
        
#ifndef ONLINE_JUDGE
    printf("===============\n");
    printme(basemap, teh_final_link);
    printf("===============\n");
#endif

    for (pos = 0, local_depth = 0; pos < area && local_depth < area; pos++) {
        mp = basemap + pos;
        if (*mp != 1)
            continue;
        if (untake_bacteria(basemap, pos)) {
            printf("No\n");
            return 0;
        }
        solution[local_depth++] = pos;
        if (mp[-rowlen] == 1)
            pos -= rowlen + 1;
        else if (mp[-1] == 1)
            pos -= 2;
#ifndef ONLINE_JUDGE
//        print_out(basemap);
//        printf(" ||\n \\/\n");
#endif
    }
    if (local_depth < area) {
        printf("No\n");
        return 0;
    }
    printf("Yes\n");
    for (pos = area; pos--; )
        printf("%d %d\n", solution[pos] / rowlen + 1, solution[pos] % rowlen + 1);
    return 0;
}
