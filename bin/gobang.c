#include "stdio.h"
#include "usr.h"
#include "string.h"


#define SIZE 16
#define inside(a, b) 	((a) >= 0 && (a) < SIZE && (b) >= 0 && (b) < SIZE)

enum Color {
    WHITE = 2, BLACK = 1
};
enum Direction {
    LU = 0, RU, L, U
};

int table[SIZE][SIZE];
int t_weight[SIZE][SIZE];
int chess();
int judge(int i, int j);
void print();
int ai();

int
main(void)
{
    int time = 1;

    ttymode(0);
    print();

    while (1) {
        int state;
        
        state = (time++ & 1) ?  chess() : ai();

        if (state == 0)
            continue;

        goto_xy(0, SIZE + 2);
        printf(state == BLACK ? 
                "Sorry, you lose! = = " : "You Win! T T");
        break;
    }

    fflush(stdin);
    getchar();
    ttymode(TTY_LINE | TTY_ECHO);
    return 0;
}

void
print()
{
    int i, j;

    cls();
    printf("  ");
    for (i = 0; i < SIZE; i++)
        printf("%-2X", i);
    putchar('\n');
    for (i = 0; i < SIZE; i++){
        printf("%-2X", i);
        for (j = 0; j < SIZE; j++)
                printf("%-2c", table[j][i] ? table[j][i] : ' ');
        putchar('\n');
    }
    printf("Press wsad to move, enter to chess, q to quit...\n");
}

int
chess()
{
    int c;
    static int x = 0, y = 0;
begin:
    while ((c = getchar()) != '\n') {
        if (c == 'q') {
            ttymode(TTY_LINE | TTY_ECHO);
            exit(0);
        }

        goto_xy((x + 1) * 2, y + 1);
        putchar(table[x][y] == 0 ? ' ' : table[x][y]);
        switch(c) {
            case 'w':
                if (inside(x, y - 1))	y--;
                else	y = SIZE - 1;
                break;
            case 's':
                if (inside(x, y + 1))	y++;
                else 	y = 0;
                break;
            case 'a':
                if (inside(x - 1, y))	x--;
                else 	x = SIZE - 1;
                break;
            case 'd':
                if (inside(x + 1, y))	x++;
                else	x = 0;
                break;
        }
        goto_xy((x + 1) * 2, y + 1);
        putchar('\xb');
        fflush(stdout);
    }
    
    if (table[x][y]) {
        goto_xy(0, SIZE + 2);
        printf("What the hell are you doing?  >_<|||");
        getchar();
        goto_xy(0, SIZE + 2);
        printf("                                               ");
        goto begin;
    }
    table[x][y] = WHITE;
    goto_xy((x + 1) * 2, y + 1);
    putchar(WHITE);
    return judge(x, y);
}



int
_judge(int x, int y, int dir)
{
    int op1, op2, color = table[x][y], n = 0;
    int i, j;

    switch (dir) {
        case LU: 	op1 =  -1; 	op2 =  -1; 	break;
        case RU: 	op1 =  +1; 	op2 =  -1; 	break;
        case L: 	op1 = 1; 	op2 = 0;	break;
        case U:		op1 = 0;	op2 = 1;	break;
    }
    for (i = x, j = y; inside(i, j) && table[i][j] == color; i += op1, j += op2, n++)
        ;
    op1 =  -op1;
    op2 =  -op2;
    n--;
    for (i = x, j = y; inside(i, j) && table[i][j] == color; i += op1, j += op2, n++)
        ;

    return n >= 5 ? color : 0;
}

int
judge(int x, int y)
{
    int t, dir;

    for (dir = 0; dir < 4; dir++) {
        t = _judge(x, y, dir);
        if (t)
            return t;
    }
    return 0;
}

int
_log(int x, int y, int dir)
{
    int op1, op2, color = table[x][y], n = 0, side1 = 0, side2 = 0, i, j;
    switch (dir) {
        case LU: 	op1 =  -1; 	op2 =  -1; 	break;
        case RU: 	op1 =  +1; 	op2 =  -1; 	break;
        case L: 	op1 = 1; 	op2 = 0;	break;
        case U:		op1 = 0;	op2 = 1;	break;
    }
    for (i = x, j = y; inside(i, j) && table[i][j] == color; i += op1, j += op2, n++)
        ;
    if (!inside(i, j) || (inside(i, j) && table[i][j] != 0 && table[i][j] != color))
        side1 = 1;
    op1 =  -op1;
    op2 =  -op2;
    n--;
    for (i = x, j = y; inside(i, j) && table[i][j] == color; i += op1, j += op2, n++)
        ;

    if (!inside(i, j) || (inside(i, j) && table[i][j] != 0 && table[i][j] != color))
        side2 = 1;
    if (n >= 5)
        return 100;
    if (n == 4 && side1 == 0 && side2 == 0)
        return 20;
    if (side1 && side2)
        n = 0;
    else if (n!=4 && (side1 || side2))
        n--;
    return n;
}

int
log(int x, int y)
{
    int dir, all = 0;

    for (dir = 0; dir < 4; dir++)
        all += _log(x, y, dir);

    return all;
}

int
weight(int x, int y)
{
    int wei, t;
    table[x][y] = BLACK;
    wei = log(x, y) * 11;
    table[x][y] = WHITE;
    t = log(x, y) * 10;
    if (wei < t)
        wei = t;
    table[x][y] = 0;
    return wei;
}

void
compute_wei()
{
    int i, j;

    memset(t_weight, 0, sizeof(t_weight));
    for (i = 0; i < SIZE; i++)
        for (j = 0; j < SIZE; j++)
            if (table[i][j] == 0)
                t_weight[i][j] = weight(i, j);
}

int
ai()
{
    compute_wei();
    int n, mi, mj, max = 0, i, j;

    for (i = 0; i < SIZE; i++)
        for (j = 0; j < SIZE; j++)
            if (t_weight[i][j] > max)
                max = t_weight[i][j];

    n = 1;
    while (n) {
        for (i = 0; i < SIZE; i++)
            for (j = 0; j < SIZE; j++)
                if (t_weight[i][j] == max) {
                    mi = i;
                    mj = j;
                    if (!--n)
                        goto done;
                }
    }
done:
    table[mi][mj] = BLACK;
    goto_xy((mi + 1) * 2, mj + 1);
    putchar(BLACK);
    return judge(mi, mj);
}
