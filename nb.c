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

struct link {
    char h, v;
} static teh_complete_link_memory[MAX_SIZE*MAX_SIZE][MAX_SIZE*MAX_SIZE];
static char teh_complete_map_memory[MAX_SIZE*MAX_SIZE][MAX_SIZE*MAX_SIZE];

static int cols, rows, area;

/* Links.
*  v-link shows relation from THIS cell to the cell above it.
*  h-link shows relation from THIS cell to the cell to the left from it.
*  1 means that THIS cell is OLDER and bacteria moved TO here
* -1 means that THIS cell is YOUNGER and bacteria moved FROM here
*/

static char hlink[] = {'<', '?', '>'};
static char vlink[] = {'^', '?', 'v'};

static void printme(char *map, struct link *link) {
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

static int go_and_unwind_whats_left(char *map, struct link *link, int oldpos) {
    int i, j, pos;
    for (i = oldpos / cols + 1, pos = oldpos; i--;) {
        for (j = oldpos % cols + 1; j--; pos--) {
            printf("Position %d (%d,%d)\n", pos, i, j);
            switch (map[pos]) {
            case 0:
            case 4:
                printf("No (badnum!)\n");
                return -1;
            case 1:
                if (j) {
                    map[pos-1]--;
                    if (map[pos-1] == 0) 
                        map[pos-1] = 4;
                    link[pos].h = -1;
                }
                if (i) {
                    map[pos-cols]--;
                    if (map[pos-cols] == 0) 
                        map[pos-cols] = 4;
                    link[pos].v = -1;
                }
                break;
            case 3:
                if (i == 0 || j == 0) {
                    printf("No (3 in the corner)\n");
                    return -1;
                }
                link[pos].h = link[pos].v = 1;
                break;
            case 2:
                if (i == 0 && j == 0) {
                    printf("No (2 in the corner)\n");
                    return -1;
                }

                printf("Woot, looking at 2 at position %d\n", pos);

                if (j && map[pos-1] == 1) { /* this 2 was before that neighbour */
                    link[pos].h = 1;
                    if (i) {
                        map[pos-cols]--;
                        if (map[pos-cols] == 0) 
                            map[pos-cols] = 4;
                        link[pos].v = -1;
                    }
                } else if (i && map[pos-cols] == 1) { /* this 2 was before that another neighbour */
                    link[pos].v = 1;
                    if (j) {
                        map[pos-1]--;
                        link[pos].h = -1;
                    }
                } else if (j && map[pos-1] > 2) { /* this 2 was after that neighbour */
                    map[pos-1]--;
                    link[pos].h = 1;
                    if (i) {
                        link[pos].v = -1;
                    }
#if 0
                } else if (i && map[pos-cols] > 2) { /* this 2 was after that another neighbour */
                    map[pos-cols]--;
                    if (map[pos-cols] == 0) 
                        map[pos-cols] = 4;
                    link[pos].v = -1;
                    if (j) {
                        link[pos].h = 1;
                    }
#endif
                } else if (!i) {                    /* Nowhere to go but to the right */
                    map[pos-1]--;
                    link[pos].h = -1;
                } else if (!j) {                    /* Nowhere to go but up */
                    map[pos-cols]--;
                    if (map[pos-cols] == 0) 
                        map[pos-cols] = 4;
                    link[pos].v = -1;
                } else {
                    printf("Ugh. Dunnae what to do. Fork?\n");
                    return -1;
                }
                break;
            default:
                break;
            }
        printf("These be %c and %c\n", vlink[link[pos].v+1], hlink[link[pos].h+1]);
        printme(map, link);
        printf(" ||\n \\/\n");
        }
    }
    printf("Yay, done!\n");
    return 0;
}

int main() {
    int i, j, sum=0, pos;
    char *basemap = teh_complete_map_memory[0];
    char *map = teh_complete_map_memory[1];

    scanf("%d%d", &rows, &cols);
    area = cols*rows;

    for (i = 0, pos = 0; i < rows; i++)
        for (j = 0; j < cols; j++, pos++) {
            scanf("%d", &basemap[pos]);
            map[pos] = basemap[pos];
            sum += basemap[pos];
        }

#ifdef ONLINE_JUDGE /* short-circuit check */
    if ((area*3 - cols - rows - sum)%4) {
        printf("No\n");
        return 0;
    }
#endif

   printme(map, teh_complete_link_memory[0]);
   printf(" ||\n \\/\n");
   go_and_unwind_whats_left(map, teh_complete_link_memory[0], area-1);
}
