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

static char link[MAX_SIZE][MAX_SIZE][4];

static char map[MAX_SIZE*MAX_SIZE], tmap[MAX_SIZE*MAX_SIZE];

static int cols, rows, area;

static char hlink[] = {'<', '?', '>'};
static char vlink[] = {'^', '?', 'v'};

void printme(char *map) {
    int i, j, pos;
    for (i = 0, pos = 0; i < rows; i++) {
        for (j = 0; j < cols; j++, pos++) {
            printf("%d", map[pos]);
            if (j < cols - 1)
                printf("%c", hlink[link[i][j][0]+1]);
        }
        printf("\n");
        if (i == rows-1)
            break;
        for (j = 0; j < cols; j++) {
            printf("%c ", vlink[link[i][j][1]+1]);
        }
        printf("\n");
    }
}

int main() {
    int i, j, sum=0, pos;
    struct solution *sol;

    scanf("%d%d", &rows, &cols);
    area = cols*rows;

    for (i = 0, pos = 0; i < rows; i++)
        for (j = 0; j < cols; j++, pos++) {
            scanf("%d", &map[pos]);
            tmap[pos] = map[pos];
            sum += map[pos];
        }

#ifdef ONLINE_JUDGE /* short-circuit check */
    if ((area*3 - cols - rows - sum)%4) {
        printf("No\n");
        return 0;
    }
#endif

    for (i = rows, pos = area-1; i--;) {
        for (j = cols; j--; pos--) {
            printf("Position %d\n", pos);
            switch (map[pos]) {
            case 0:
            case 4:
                printf("No (badnum!)\n");
                return 0;
            case 1:
                if (j) {
                    map[pos-1]--;
                    if (map[pos-1] == 0) 
                        map[pos-1] = 4;
                    link[i][j][2] = 1;
                    link[i][j-1][0] = -1;
                }
                if (i) {
                    map[pos-cols]--;
                    if (map[pos-cols] == 0) 
                        map[pos-cols] = 4;
                    link[i][j][3] = 1;
                    link[i-1][j][1] = -1;
                }
                break;
            case 3:
                if (i == 0 || j == 0) {
                    printf("No (3 in the corner)\n");
                    return 0;
                }
                link[i][j][2] = -1;
                link[i][j-1][0] = 1;
                link[i][j][3] = -1;
                link[i-1][j][1] = 1;
                break;
            case 2:
                if (i == 0 && j == 0) {
                    printf("No (2 in the corner)\n");
                    return 0;
                }

                printf("Woot, looking at 2 at position %d\n", pos);

                if (j && map[pos-1] == 1) { /* this 2 was before that neighbour */
                    link[i][j][2] = -1;
                    link[i][j-1][0] = 1;
                    if (i) {
                        map[pos-cols]--;
                        if (map[pos-cols] == 0) 
                            map[pos-cols] = 4;
                        link[i][j][3] = 1;
                        link[i-1][j][1] = -1;
                    }
                } else if (i && map[pos-cols] == 1) { /* this 2 was before that another neighbour */
                    link[i][j][3] = -1;
                    link[i-1][j][1] = 1;
                    if (j) {
                        map[pos-1]--;
                        link[i][j][2] = 1;
                        link[i][j-1][0] = -1;
                    }
                } else if (j && map[pos-1] > 2) { /* this 2 was after that neighbour */
                    map[pos-1]--;
                    link[i][j][2] = 1;
                    link[i][j-1][0] = -1;
                    if (i) {
                        link[i][j][3] = -1;
                        link[i-1][j][1] = 1;
                    }
                } else if (i && map[pos-cols] > 2) { /* this 2 was after that another neighbour */
                    map[pos-cols]--;
                    if (map[pos-cols] == 0) 
                        map[pos-cols] = 4;
                    link[i][j][3] = 1;
                    link[i-1][j][1] = -1;
                    if (j) {
                        link[i][j][2] = -1;
                        link[i][j-1][0] = 1;
                    }
                } else if (!i) {                    /* Nowhere to go but to the right */
                    map[pos-1]--;
                    link[i][j][2] = 1;
                    link[i][j-1][0] = -1;
                } else if (!j) {                    /* Nowhere to go but up */
                    map[pos-cols]--;
                    if (map[pos-cols] == 0) 
                        map[pos-cols] = 4;
                    link[i][j][3] = 1;
                    link[i-1][j][1] = -1;
                } else {
                    printf("No (Lots of 2's!)\n");
                    return 0;
                }
                break;
            default:
                break;
            }
        printme(map);
        printf(" ||\n \\/\n");
        }
    }
    memcpy(map, tmap, MAX_SIZE*MAX_SIZE);

/* let's try drawing this.. */
    for (i = 0, pos = 0; i < rows; i++) {
        for (j = 0; j < cols; j++, pos++) {
            printf("%d", tmap[pos]);
            if (j < cols - 1)
                printf("%c", hlink[link[i][j][0]+1]);
        }
        printf("\n");
        if (i == rows-1)
            break;
        for (j = 0; j < cols; j++) {
            printf("%c ", vlink[link[i][j][1]+1]);
        }
        printf("\n");
    }
}
