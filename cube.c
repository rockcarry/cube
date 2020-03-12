#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#pragma pack(1)
typedef struct tagCUBE {
    struct tagCUBE *parent;
    char f[3][3];
    char b[3][3];
    char u[3][3];
    char d[3][3];
    char l[3][3];
    char r[3][3];
    char op;
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
    memset(c->f, 'w', sizeof(c->f));
    memset(c->b, 'y', sizeof(c->b));
    memset(c->u, 'b', sizeof(c->u));
    memset(c->d, 'g', sizeof(c->d));
    memset(c->l, 'o', sizeof(c->l));
    memset(c->r, 'r', sizeof(c->r));
}

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
    CUBE_OP_F,
    CUBE_OP_B,
    CUBE_OP_U,
    CUBE_OP_D,
    CUBE_OP_L,
    CUBE_OP_R,
};

static void (*g_op_tab[])(CUBE *c) = { cube_f, cube_b, cube_u, cube_d, cube_l, cube_r };
static void cube_op  (CUBE *c, int op) { (g_op_tab[op])(c); }
static void cube_rand(CUBE *c, int n ) { while (n-- > 0) cube_op(c, rand() % 6); }

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
            case 'w': case 'W': SetConsoleTextAttribute(h, FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE); break;
            case 'y': case 'Y': SetConsoleTextAttribute(h, FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_GREEN); break;
            case 'b': case 'B': SetConsoleTextAttribute(h, FOREGROUND_INTENSITY|FOREGROUND_BLUE ); break;
            case 'g': case 'G': SetConsoleTextAttribute(h, FOREGROUND_INTENSITY|FOREGROUND_GREEN); break;
            case 'o': case 'O': SetConsoleTextAttribute(h, FOREGROUND_RED|FOREGROUND_BLUE       ); break;
            case 'r': case 'R': SetConsoleTextAttribute(h, FOREGROUND_INTENSITY|FOREGROUND_RED  ); break;
            }
            printf(buffer[i][j] ? "#" : "  ");
        }
        printf("\n");
    }

    // restore the original color
    SetConsoleTextAttribute(h, wOldColorAttrs);
}

static int cube_check_fcross0(CUBE *cube)
{
    int checklist[][2] = {
        { cube->f[1][1], cube->f[0][1] },
        { cube->f[1][1], cube->f[1][0] },
        { cube->f[1][1], cube->f[1][2] },
        { cube->f[1][1], cube->f[2][1] },
    };
    int value = 0, i;
    for (i=0; i<4; i++) {
        value += (checklist[i][0] == checklist[i][1]);
    }
    return value;
}

static int cube_check_fcross1(CUBE *cube)
{
    int checklist[][2] = {
        { cube->l[1][1], cube->l[1][2] },
        { cube->u[1][1], cube->u[2][1] },
        { cube->r[1][1], cube->r[1][0] },
        { cube->d[1][1], cube->d[0][1] },
    };
    int value = 0, i;
    for (i=0; i<4; i++) {
        value += (checklist[i][0] == checklist[i][1]);
    }
    return value;
}

static int cube_check_fcorners(CUBE *cube)
{
    int checklist[][3][2] = {
        {
            { cube->f[1][1], cube->f[0][0] },
            { cube->l[1][1], cube->l[0][2] },
            { cube->u[1][1], cube->u[2][0] },
        },
        {
            { cube->f[1][1], cube->f[0][2] },
            { cube->u[1][1], cube->u[2][2] },
            { cube->r[1][1], cube->r[0][0] },
        },
        {
            { cube->f[1][1], cube->f[2][0] },
            { cube->l[1][1], cube->l[2][2] },
            { cube->d[1][1], cube->d[0][0] },
        },
        {
            { cube->f[1][1], cube->f[2][2] },
            { cube->r[1][1], cube->r[2][0] },
            { cube->d[1][1], cube->d[0][2] },
        },
    };
    int value = 0, i, j;
    for (j=0; j<4; j++) {
        value++;
        for (i=0; i<3; i++) {
            if (checklist[j][i][0] != checklist[j][i][1]) { value--; break; }
        }
    }
    return value;
}

static int cube_check_medges(CUBE *cube)
{
    int checklist[][2][2] = {
        {
            { cube->l[1][1], cube->l[0][1] },
            { cube->u[1][1], cube->u[1][0] },
        },
        {
            { cube->u[1][1], cube->u[1][2] },
            { cube->r[1][1], cube->r[0][1] },
        },
        {
            { cube->l[1][1], cube->l[2][1] },
            { cube->d[1][1], cube->d[1][0] },
        },
        {
            { cube->r[1][1], cube->r[2][1] },
            { cube->d[1][1], cube->d[1][2] },
        },
    };
    int value = 0, i, j;
    for (j=0; j<4; j++) {
        value++;
        for (i=0; i<2; i++) {
            if (checklist[j][i][0] != checklist[j][i][1]) { value--; break; }
        }
    }
    return value;
}

static int cube_check_bcross(CUBE *cube)
{
    int checklist[][2][2] = {
        {
            { cube->b[1][1], cube->b[0][1] },
            { cube->b[1][1], cube->b[2][1] },
        },
        {
            { cube->b[1][1], cube->b[1][0] },
            { cube->b[1][1], cube->b[1][2] },
        },
    };
    int value = 0, i, j;
    for (j=0; j<2; j++) {
        value += 2;
        for (i=0; i<2; i++) {
            if (checklist[j][i][0] != checklist[j][i][1]) { value -= 2; break; }
        }
    }
    return value;
}

static int cube_check_bsurface(CUBE *cube)
{
    int checklist[][2] = {
        { cube->b[1][1], cube->b[0][0] },
        { cube->b[1][1], cube->b[0][2] },
        { cube->b[1][1], cube->b[2][0] },
        { cube->b[1][1], cube->b[2][2] },
    };
    int value = 0, i;
    for (i=0; i<4; i++) {
        value += (checklist[i][0] == checklist[i][1]);
    }
    if (value == 2) value = 0;
    return value;
}

static int cube_check_bcorners(CUBE *cube)
{
    int checklist[][3][2] = {
        {
            { cube->b[1][1], cube->b[0][2] },
            { cube->l[1][1], cube->l[0][0] },
            { cube->u[1][1], cube->u[0][0] },
        },
        {
            { cube->b[1][1], cube->b[0][0] },
            { cube->u[1][1], cube->u[0][2] },
            { cube->r[1][1], cube->r[0][2] },
        },
        {
            { cube->b[1][1], cube->b[2][0] },
            { cube->r[1][1], cube->r[2][2] },
            { cube->d[1][1], cube->d[2][2] },
        },
        {
            { cube->b[1][1], cube->b[2][2] },
            { cube->l[1][1], cube->l[2][0] },
            { cube->d[1][1], cube->d[2][0] },
        },
    };
    int returned[4] = { 1, 1, 1, 1 };
    int value = 0, flag = 0, i, j;
    for (j=0; j<4; j++) {
        value++;
        for (i=0; i<3; i++) {
            if (checklist[j][i][0] != checklist[j][i][1]) { value--; returned[j] = 0; break; }
        }
    }
    for (i=0; i<4; i++) {
        if (returned[i] && returned[(i+1)%4]) flag = 1;
    }
    return flag ? value : 0;
}

static int cube_check_bedges(CUBE *cube)
{
    int checklist[][2][2] = {
        {
            { cube->u[1][1], cube->u[0][1] },
            { cube->b[1][1], cube->b[0][1] },
        },
        {
            { cube->d[1][1], cube->d[2][1] },
            { cube->b[1][1], cube->b[2][1] },
        },
        {
            { cube->l[1][1], cube->l[1][0] },
            { cube->b[1][1], cube->b[1][2] },
        },
        {
            { cube->r[1][1], cube->r[1][2] },
            { cube->b[1][1], cube->b[1][0] },
        },
    };
    int value = 0, i, j;
    for (j=0; j<4; j++) {
        value++;
        for (i=0; i<2; i++) {
            if (checklist[j][i][0] != checklist[j][i][1]) { value--; break; }
        }
    }
    return value;
}

static int cube_check_state(CUBE *cube, int flag)
{
    int (*pfn_check_tab[])(CUBE* cube) = {
        cube_check_fcross0 ,
        cube_check_fcross1 ,
        cube_check_fcorners,
        cube_check_medges  ,
        cube_check_bcross  ,
        cube_check_bsurface,
        cube_check_bcorners,
        cube_check_bedges  ,
        NULL,
    };
    int value = 0, cur, i;
    for (i=0; pfn_check_tab[i]; i++) {
        if (flag) {
            value += pfn_check_tab[i](cube);
            cur   += 4;
            if (cur >= flag) break;
        } else {
            cur    = pfn_check_tab[i](cube);
            value += cur;
            if (cur != 4) break;
        }
    }
    return value;
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

static int is_4same_ops(CUBE *cube)
{
    int curop = cube->op;
    int n     = 0;
    while (cube) {
        if (cube->op == curop) {
            if (++n == 4) {
                return 1;
            }
            cube = cube->parent;
        } else break;
    }
    return 0;
}

static int cut_branch(int newval, int cutval)
{
    return newval < cutval;
}

static CUBE* search(TABLE *table, CUBE *start, int state, char *oplist, int opnum, int cutval)
{
    CUBE *curcube ,*newcube;
    int   newstate, newvalue, i;

    start->parent = NULL;
    start->op     = -1;
    if (cube_check_state(start, 0) >= state) return start;

    // init search table
    table->open  = 0;
    table->close = 0;

    // put original cube into open table
    table->cubes[table->open] = *start;
    table->open++;

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
            memcpy (newcube, curcube, sizeof(CUBE));
            cube_op(newcube, oplist[i]);
            newcube->op     = oplist[i];
            newcube->parent = curcube;
            newstate = cube_check_state(newcube, 0    );
            newvalue = cube_check_state(newcube, state);
            if (newstate >= state) { // found
                return newcube;
            }
            if (is_4same_ops(newcube)) {
                continue;
            }
            if (cut_branch(newvalue, cutval)) {
                continue;
            }
            table->open++;
        }
    }
    return NULL;
}

static void print_solve_oplist(CUBE *cube)
{
    static char* optab[] = {
        "F" , "B" , "U" , "D" , "L" , "R" ,
        "F2", "B2", "U2", "D2", "L2", "R2",
        "F'", "B'", "U'", "D'", "L'", "R'",
    };
    char *oplist[256];
    int   last = -1, times = 0, i = 0, n = 0;
    while (cube) {
        if (cube->op >= 0) {
            if (last != cube->op) {
                if (last != -1) {
                    oplist[i++] = optab[last + times * 6];
                }
                last = cube->op;
                times= 0;
            } else {
                times++;
            }
        } else {
            if (last != -1) oplist[i++] = optab[last + times * 6];
        }
        cube = cube->parent;
    }
    printf("\noperation list:\n");
    while (--i >= 0) {
        printf("%s%s%s", oplist[i], i == 0 ? "" : " -> ", ++n % 12 == 0 ? "\n" : "");
    }
    printf("\n");
}

static void cube_solve(CUBE *c)
{
    TABLE t;

    if (search_table_create(&t, 1024*1024*16) != 0) {
        printf("failed to create cube search table !\n");
        return;
    }

    if (1) {
        static char oplisttab[][6] = {
            { CUBE_OP_F, CUBE_OP_U, CUBE_OP_D, CUBE_OP_L, CUBE_OP_R },
            { CUBE_OP_U, CUBE_OP_D, CUBE_OP_L, CUBE_OP_R, CUBE_OP_B },
            { CUBE_OP_B, CUBE_OP_R, CUBE_OP_U },
        };
        static int stepparams[][4] = {
            { 2 , 0, 5, 0 }, //+ fcross0
            { 4 , 0, 5, 2 }, //- fcross0
            { 8 , 0, 5, 2 }, //+-fcross1
            { 9 , 0, 5, 3 }, //+ fcorners
            { 10, 0, 5, 4 }, //..
            { 11, 0, 5, 5 }, //..
            { 12, 0, 5, 5 }, //- fcorners
            { 13, 1, 5, 6 }, //+ medges
            { 14, 1, 5, 6 }, //..
            { 15, 1, 5, 7 }, //..
            { 16, 1, 5, 8 }, //- medges
            { 18, 1, 5, 10}, //+ bcross
            { 20, 1, 5, 11}, //- bcross
            { 21, 1, 5, 11}, //+ bsurface
            { 24, 1, 5, 11}, //- bsurface
            { 26, 2, 3, 12}, //+ bcorners
            { 28, 2, 3, 12}, //- bcorners
            { 32, 2, 2, 12}, //+-bedges
            { 0 , 0, 0, 0 },
        };
        CUBE *start = c;
        CUBE *find  = NULL;
        int   i;
        for (i=0; stepparams[i][0]; i++) {
            find = search(&t, start, stepparams[i][0], oplisttab[stepparams[i][1]], stepparams[i][2], stepparams[i][3]);
            if (find) {
                if (find != start) {
                    start = find;
                    *c    =*find;
                    print_solve_oplist(find);
                    if (stepparams[i][0] != 32) cube_render(c);
                }
            } else {
                printf("can't solve !\n");
                goto done;
            }
        }
        printf("\ncube solved !\n");
    }

done:
    search_table_destroy(&t);
}

static void cube_input(CUBE *c)
{
    char str[256];

    printf("please input F surface of cube:\n");
    scanf("%c %c %c %c %c %c %c %c %c",
        &(c->f[0][0]), &(c->f[0][1]), &(c->f[0][2]),
        &(c->f[1][0]), &(c->f[1][1]), &(c->f[1][2]),
        &(c->f[2][0]), &(c->f[2][1]), &(c->f[2][2]));
    gets(str);

    printf("please input U surface of cube:\n");
    scanf("%c %c %c %c %c %c %c %c %c",
        &(c->u[0][0]), &(c->u[0][1]), &(c->u[0][2]),
        &(c->u[1][0]), &(c->u[1][1]), &(c->u[1][2]),
        &(c->u[2][0]), &(c->u[2][1]), &(c->u[2][2]));
    gets(str);

    printf("please input D surface of cube:\n");
    scanf("%c %c %c %c %c %c %c %c %c",
        &(c->d[0][0]), &(c->d[0][1]), &(c->d[0][2]),
        &(c->d[1][0]), &(c->d[1][1]), &(c->d[1][2]),
        &(c->d[2][0]), &(c->d[2][1]), &(c->d[2][2]));
    gets(str);

    printf("please input L surface of cube:\n");
    scanf("%c %c %c %c %c %c %c %c %c",
        &(c->l[0][0]), &(c->l[0][1]), &(c->l[0][2]),
        &(c->l[1][0]), &(c->l[1][1]), &(c->l[1][2]),
        &(c->l[2][0]), &(c->l[2][1]), &(c->l[2][2]));
    gets(str);

    printf("please input R surface of cube:\n");
    scanf("%c %c %c %c %c %c %c %c %c",
        &(c->r[0][0]), &(c->r[0][1]), &(c->r[0][2]),
        &(c->r[1][0]), &(c->r[1][1]), &(c->r[1][2]),
        &(c->r[2][0]), &(c->r[2][1]), &(c->r[2][2]));
    gets(str);

    printf("please input B surface of cube:\n");
    scanf("%c %c %c %c %c %c %c %c %c",
        &(c->b[0][0]), &(c->b[0][1]), &(c->b[0][2]),
        &(c->b[1][0]), &(c->b[1][1]), &(c->b[1][2]),
        &(c->b[2][0]), &(c->b[2][1]), &(c->b[2][2]));
    gets(str);
}

static void show_help(void)
{
    printf(
        "\n"
        "available commands:\n\n"
        "f f2 f' b b2 b' u u2 u' d d2 d' l l2 l' r r2 r'\n"
        "init rand input solve help exit\n\n"
        "note: all command is case sensitive.\n"
    );
}

int main(void)
{
    char cmd[128];
    char str[256];
    CUBE c;

    // init cube
    cube_init(&c);

    show_help();
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
            gets(str);
            cube_rand(&c, atoi(str) == 0 ? 100 : atoi(str));
        } else if (strcmp(cmd, "input") == 0) {
            cube_input(&c);
        } else if (strcmp(cmd, "solve") == 0) {
            cube_solve(&c);
        } else if (strcmp(cmd, "help") == 0) {
            show_help();
        } else if (strcmp(cmd, "exit") == 0) {
            break;
        } else {
            printf("unsupported command !\n");
        }
        printf("\n");
    }

    return 0;
}

