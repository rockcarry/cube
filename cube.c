#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#pragma pack(1)
typedef struct tagCUBE {
    char f[3][3];
    char b[3][3];
    char u[3][3];
    char d[3][3];
    char l[3][3];
    char r[3][3];
    char op;
    struct tagCUBE *parent;
} CUBE;
#pragma pack()

typedef struct {
    int   stride;
    char *buffer;
} LINEITEM;

static void line_rotate90(LINEITEM item[5])
{
    int i, j;
    
    for (i=0; i<5; i++) {
        int   dst_idx    = (i+0) % 5;
        int   src_idx    = (i+1) % 5;
        char *dst_buf    = item[dst_idx].buffer;
        char *src_buf    = item[src_idx].buffer;
        int   dst_stride = item[dst_idx].stride;
        int   src_stride = item[src_idx].stride;
        for (j=0; j<3; j++) {
            *dst_buf = *src_buf;
            dst_buf += dst_stride;
            src_buf += src_stride;
        }
    }
}

static void surface_rotate90(char buf[3][3])
{
    char tmp[3][3];
    int  i, j;
    memcpy(tmp, buf, sizeof(tmp));
    for (i=0; i<3; i++) {
        for (j=0; j<3; j++) {
            buf[i][j] = tmp[3-j-1][i];
        }
    }
}

static void cube_init(CUBE *c)
{
    memset(c->f, 'W', sizeof(c->f));
    memset(c->b, 'Y', sizeof(c->b));
    memset(c->u, 'G', sizeof(c->u));
    memset(c->d, 'B', sizeof(c->d));
    memset(c->l, 'R', sizeof(c->l));
    memset(c->r, 'O', sizeof(c->r));
}

static void cube_n(CUBE *c) { /* nop, do nothing */ }

static void cube_f(CUBE *c)
{
    char temp[3];
    LINEITEM lines[5] = {
        { 1,  temp          },
        { 3,  &(c->l[0][2]) },
        { 1,  &(c->d[0][0]) },
        {-3,  &(c->r[2][0]) },
        {-1,  &(c->u[2][2]) },
    };
    line_rotate90(lines);
    surface_rotate90(c->f);
}

static void cube_b(CUBE *c)
{
    char temp[3];
    LINEITEM lines[5] = {
        { 1,  temp          },
        { 3,  &(c->r[0][2]) },
        {-1,  &(c->d[2][2]) },
        {-3,  &(c->l[2][0]) },
        { 1,  &(c->u[0][0]) },
    };
    line_rotate90(lines);
    surface_rotate90(c->b);
}

static void cube_u(CUBE *c)
{
    char temp[3];
    LINEITEM lines[5] = {
        { 1,  temp          },
        { 1,  &(c->l[0][0]) },
        { 1,  &(c->f[0][0]) },
        { 1,  &(c->r[0][0]) },
        { 1,  &(c->b[0][0]) },
    };
    line_rotate90(lines);
    surface_rotate90(c->u);
}

static void cube_d(CUBE *c)
{
    char temp[3];
    LINEITEM lines[5] = {
        { 1,  temp          },
        { 1,  &(c->b[2][0]) },
        { 1,  &(c->r[2][0]) },
        { 1,  &(c->f[2][0]) },
        { 1,  &(c->l[2][0]) },
    };
    line_rotate90(lines);
    surface_rotate90(c->d);
}

static void cube_l(CUBE *c)
{
    char temp[3];
    LINEITEM lines[5] = {
        { 1,  temp          },
        { 3,  &(c->d[0][0]) },
        { 3,  &(c->f[0][0]) },
        { 3,  &(c->u[0][0]) },
        {-3,  &(c->b[2][2]) },
    };
    line_rotate90(lines);
    surface_rotate90(c->l);
}

static void cube_r(CUBE *c)
{
    char temp[3];
    LINEITEM lines[5] = {
        { 1,  temp          },
        { 3,  &(c->u[0][2]) },
        { 3,  &(c->f[0][2]) },
        { 3,  &(c->d[0][2]) },
        {-3,  &(c->b[2][0]) },
    };
    line_rotate90(lines);
    surface_rotate90(c->r);
}

enum {
    CUBE_OP_N,
    CUBE_OP_F,
    CUBE_OP_B,
    CUBE_OP_U,
    CUBE_OP_D,
    CUBE_OP_L,
    CUBE_OP_R,
};

static void (*g_op_tab[])(CUBE *c) = {
    cube_n, cube_f, cube_b, cube_u, cube_d, cube_l, cube_r,
};

static void cube_op(CUBE *c, int op)
{
    (g_op_tab[op])(c);
}

static void cube_rand(CUBE *c, int n) {
    while (n-- > 0) {
        cube_op(c, rand() % 6 + 1);
    }
}

static void cube_render(CUBE *c)
{
    char buffer[9][12] = {0};
    int  i, j;

    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);  
    WORD wOldColorAttrs;  
    CONSOLE_SCREEN_BUFFER_INFO csbiInfo; 

    // save the current color  
    GetConsoleScreenBufferInfo(h, &csbiInfo);  
    wOldColorAttrs = csbiInfo.wAttributes;

    for (i=0; i<3; i++) for (j=0; j<3; j++) buffer[3+i][3+j] = c->f[i][j];
    for (i=0; i<3; i++) for (j=0; j<3; j++) buffer[3+i][9+j] = c->b[i][j];
    for (i=0; i<3; i++) for (j=0; j<3; j++) buffer[0+i][3+j] = c->u[i][j];
    for (i=0; i<3; i++) for (j=0; j<3; j++) buffer[6+i][3+j] = c->d[i][j];
    for (i=0; i<3; i++) for (j=0; j<3; j++) buffer[3+i][0+j] = c->l[i][j];
    for (i=0; i<3; i++) for (j=0; j<3; j++) buffer[3+i][6+j] = c->r[i][j];

    for (i=0; i<9; i++) {
        for (j=0; j<12; j++) {
            switch (buffer[i][j]) {
            case 'W': SetConsoleTextAttribute(h, FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE); break;
            case 'Y': SetConsoleTextAttribute(h, FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_GREEN); break;
            case 'G': SetConsoleTextAttribute(h, FOREGROUND_INTENSITY|FOREGROUND_GREEN); break;
            case 'O': SetConsoleTextAttribute(h, FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_BLUE ); break;
            case 'B': SetConsoleTextAttribute(h, FOREGROUND_INTENSITY|FOREGROUND_BLUE ); break;
            case 'R': SetConsoleTextAttribute(h, FOREGROUND_INTENSITY|FOREGROUND_RED  ); break;
            }
            printf(buffer[i][j] ? "\2 " : "  ");
        }
        printf("\n");
    }

    // Restore the original color  
    SetConsoleTextAttribute(h, wOldColorAttrs);
}

enum {
    CUBE_STATE_CROSS,
    CUBE_STATE_SOLVED,
};

static int cube_check_color(char *buf, int *checkarray, int size)
{
    int i;
    for (i=1; i<size; i++) {
        if (buf[checkarray[i]] != buf[checkarray[0]]) {
            return 0;
        }
    }
    return 1;
}

static int cube_check_state(CUBE *cube, int state)
{
    static int cross[] = { 4, 1, 3, 5, 7 };
    static int solve[] = { 4, 0, 1, 2, 3, 5, 6, 7, 8 };
    int ret = 0;

    switch (state) {
    case CUBE_STATE_CROSS:
        ret = cube_check_color(&(cube->f[0][0]), cross, 5);
        break;
    case CUBE_STATE_SOLVED:
        ret = cube_check_color(&(cube->f[0][0]), solve, 9);
        if (!ret) break;
        ret = cube_check_color(&(cube->b[0][0]), solve, 9);
        if (!ret) break;
        ret = cube_check_color(&(cube->u[0][0]), solve, 9);
        if (!ret) break;
        ret = cube_check_color(&(cube->d[0][0]), solve, 9);
        if (!ret) break;
        ret = cube_check_color(&(cube->l[0][0]), solve, 9);
        if (!ret) break;
        ret = cube_check_color(&(cube->r[0][0]), solve, 9);
        break;
    }
    return ret;
}

static int cube_check_same(CUBE *cube1, CUBE *cube2)
{
    return memcmp(cube1, cube2, 3 * 3 * 6) == 0 ? 1 : 0;
}

static void cube_copy(CUBE *cube1, CUBE *cube2)
{
    memcpy(cube1, cube2, sizeof(CUBE));
}

typedef struct {
    int   open ;
    int   close;
    int   size ;
    CUBE *cubes;
} TABLE;

static int search_table_create(TABLE *table, int size)
{
    table->size  = size;
    table->cubes = malloc(size * sizeof(CUBE));
    return table->cubes ? 0 : -1;
}

static void search_table_destroy(TABLE *table)
{
    if (table->cubes) {
        free(table->cubes);
        table->cubes = NULL;
    }
}

static int isin_close_table(TABLE *table, CUBE *cube)
{
    int i;
    for (i=0; i<table->close; i++) {
        if (cube_check_same(cube, &table->cubes[i])) {
            return 1;
        }
    }
    return 0;
}

static CUBE* search(TABLE *table, CUBE *orgcube, int state, char *oplist, int opnum)
{
    CUBE *curcube;
    CUBE *newcube;
    int   i;

    if (cube_check_state(orgcube, state)) return orgcube;

    // init search table
    table->open  = 0;
    table->close = 0;

    // put original cube into open table
    cube_copy(&(table->cubes[table->open++]), orgcube);

    while (table->close < table->open) {
        // check memory usage
        if (table->open + 6 >= table->size - 1) {
            printf("all table memory have been used !\n");
            break;
        }

        // dequeue a cube from open table
        curcube = &(table->cubes[table->close++]);

        // extend cubes check state and put new cubes into open table
        for (i=0; i<opnum; i++) {
            newcube = &(table->cubes[table->open]);
            cube_copy(newcube, curcube  );
            cube_op  (newcube, oplist[i]);
            newcube->op     = oplist[i];
            newcube->parent = curcube;
            if (cube_check_state(newcube, state)) {
                return newcube;
            }
            if (!isin_close_table(table, newcube)) {
                table->open++;
            }
        }
    }
    return NULL;
}

static void cube_solve(CUBE *c)
{
    TABLE t;

    if (search_table_create(&t, 1024*1024*4) != 0) {
        printf("failed to create cube search table !\n");
        return;
    }

    if (1) {
        static char oplist[] = { CUBE_OP_F, CUBE_OP_U, CUBE_OP_D, CUBE_OP_L, CUBE_OP_R };
        CUBE *newcube = search(&t, c, CUBE_STATE_CROSS, oplist, 5);
        if (newcube) {
            cube_copy(c, newcube);
            printf("cube solved !\n");
        } else {
            printf("can't solve !\n");
        }
    }

    search_table_destroy(&t);
}

int main(void)
{
    char  cmd[128];
    CUBE  c;

    // init cube
    cube_init(&c);

    while (1) {
        cube_render(&c);
        printf("command: ");
        scanf("%s", cmd);
        if (strcmp(cmd, "f") == 0) {
            cube_f(&c);
        } else if (strcmp(cmd, "f2") == 0) {
            cube_f(&c); cube_f(&c);
        } else if (strcmp(cmd, "f'") == 0) {
            cube_f(&c); cube_f(&c); cube_f(&c);
        } else if (strcmp(cmd, "b" ) == 0) {
            cube_b(&c);
        } else if (strcmp(cmd, "b2") == 0) {
            cube_b(&c); cube_b(&c);
        } else if (strcmp(cmd, "b'") == 0) {
            cube_b(&c); cube_b(&c); cube_b(&c);
        } else if (strcmp(cmd, "u" ) == 0) {
            cube_u(&c);
        } else if (strcmp(cmd, "u2") == 0) {
            cube_u(&c); cube_u(&c);
        } else if (strcmp(cmd, "u'") == 0) {
            cube_u(&c); cube_u(&c); cube_u(&c);
        } else if (strcmp(cmd, "d" ) == 0) {
            cube_d(&c);
        } else if (strcmp(cmd, "d2") == 0) {
            cube_d(&c); cube_d(&c);
        } else if (strcmp(cmd, "d'") == 0) {
            cube_d(&c); cube_d(&c); cube_d(&c);
        } else if (strcmp(cmd, "l" ) == 0) {
            cube_l(&c);
        } else if (strcmp(cmd, "l2") == 0) {
            cube_l(&c); cube_l(&c);
        } else if (strcmp(cmd, "l'") == 0) {
            cube_l(&c); cube_l(&c); cube_l(&c);
        } else if (strcmp(cmd, "r" ) == 0) {
            cube_r(&c);
        } else if (strcmp(cmd, "r2") == 0) {
            cube_r(&c); cube_r(&c);
        } else if (strcmp(cmd, "r'") == 0) {
            cube_r(&c); cube_r(&c); cube_r(&c);
        } else if (strcmp(cmd, "init") == 0) {
            cube_init(&c);
        } else if (strcmp(cmd, "rand") == 0) {
            cube_rand(&c, 100);
        } else if (strcmp(cmd, "solve") == 0) {
            cube_solve(&c);
        } else if (strcmp(cmd, "exit") == 0) {
            break;
        }
        printf("\n");
    }

    return 0;
}

