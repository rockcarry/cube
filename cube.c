#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct tagCUBE {
    char f[3][3];
    char b[3][3];
    char u[3][3];
    char d[3][3];
    char l[3][3];
    char r[3][3];
} CUBE;

//++ ZUBE is data compressed of CUBE
#pragma pack(1)
typedef struct tagZUBE {
    struct tagZUBE *parent;
    char f[3];
    char b[3];
    char u[3];
    char d[3];
    char l[3];
    char r[3];
    char op  ;
} ZUBE;
#pragma pack()

static void encode_surface(char *dst, char *src)
{
    int i, n;
    dst[0] = dst[1] = dst[2] = 0;
    for (i=0,n=0; i<9; i++) {
        if (i == 4) continue;
        dst[0] |= ((src[i] >> 0) & 1) << n;
        dst[1] |= ((src[i] >> 1) & 1) << n;
        dst[2] |= ((src[i] >> 2) & 1) << n;
        n++;
    }
}

static void decode_surface(char *dst, char *src)
{
    int i, n;
    for (i=0,n=0; i<9; i++) {
        if (i == 4) continue;
        dst[i] = (((src[0] >> n) & 1) << 0)
               | (((src[1] >> n) & 1) << 1)
               | (((src[2] >> n) & 1) << 2);
        n++;
    }
}

static void cube2zube(ZUBE *z, CUBE *c, char *center)
{
    encode_surface(z->f, (char*)c->f);
    encode_surface(z->b, (char*)c->b);
    encode_surface(z->u, (char*)c->u);
    encode_surface(z->d, (char*)c->d);
    encode_surface(z->l, (char*)c->l);
    encode_surface(z->r, (char*)c->r);
    if (center) {
        center[0] = c->f[1][1];
        center[1] = c->b[1][1];
        center[2] = c->u[1][1];
        center[3] = c->d[1][1];
        center[4] = c->l[1][1];
        center[5] = c->r[1][1];
    }
}

static void zube2cube(CUBE *c, ZUBE *z, char *center)
{
    decode_surface((char*)c->f, z->f);
    decode_surface((char*)c->b, z->b);
    decode_surface((char*)c->u, z->u);
    decode_surface((char*)c->d, z->d);
    decode_surface((char*)c->l, z->l);
    decode_surface((char*)c->r, z->r);
    if (center) {
        c->f[1][1] = center[0];
        c->b[1][1] = center[1];
        c->u[1][1] = center[2];
        c->d[1][1] = center[3];
        c->l[1][1] = center[4];
        c->r[1][1] = center[5];
    }
}
//-- ZUBE is data compressed of CUBE

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
    memset(c->f, 1, sizeof(c->f));
    memset(c->b, 2, sizeof(c->b));
    memset(c->u, 3, sizeof(c->u));
    memset(c->d, 4, sizeof(c->d));
    memset(c->l, 5, sizeof(c->l));
    memset(c->r, 6, sizeof(c->r));
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
            case 1: SetConsoleTextAttribute(h, FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE); break;
            case 2: SetConsoleTextAttribute(h, FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_GREEN); break;
            case 3: SetConsoleTextAttribute(h, FOREGROUND_INTENSITY|FOREGROUND_BLUE ); break;
            case 4: SetConsoleTextAttribute(h, FOREGROUND_INTENSITY|FOREGROUND_GREEN); break;
            case 5: SetConsoleTextAttribute(h, FOREGROUND_RED|FOREGROUND_BLUE       ); break;
            case 6: SetConsoleTextAttribute(h, FOREGROUND_INTENSITY|FOREGROUND_RED  ); break;
            }
            printf(buffer[i][j] ? "\2 " : "  ");
        }
        printf("\n");
    }

    // Restore the original color
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
    int value = cube_check_fcross0(cube), i;
    if (value < 4) return value;
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
    int value = cube_check_fcross1(cube), i, j;
    if (value < 8) return value;
    for (j=0; j<4; j++) {
        value++;
        for (i=0; i<3; i++) {
            if (checklist[j][i][0] != checklist[j][i][1]) { value--; break; }
        }
    }
    return value;
}

static int cube_check_edges(CUBE *cube)
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
    int value = cube_check_fcorners(cube), i, j;
    if (value < 12) return value;
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
    int value = cube_check_edges(cube), i, j;
    if (value < 16) return value;
    for (j=0; j<2; j++) {
        value += 2;
        for (i=0; i<2; i++) {
            if (checklist[j][i][0] != checklist[j][i][1]) { value -= 2; break; }
        }
    }
    return value;
}

static int cube_check_back(CUBE *cube)
{
    int checklist[][2] = {
        { cube->b[1][1], cube->b[0][0] },
        { cube->b[1][1], cube->b[0][2] },
        { cube->b[1][1], cube->b[2][0] },
        { cube->b[1][1], cube->b[2][2] },
    };
    int value = cube_check_bcross(cube), i;
    if (value < 20) return value;
    for (i=0; i<4; i++) {
        value += (checklist[i][0] == checklist[i][1]);
    }
    if (value == 22) value = 20;
    return value;
}

static int cube_check_state(CUBE *cube)
{
    return cube_check_back(cube);
}

typedef struct {
    int   open ;
    int   close;
    int   size ;
    ZUBE *zubes;
    char  center[6];
} TABLE;

static int search_table_create(TABLE *table, int size)
{
    table->size  = size;
    table->zubes = malloc(size * sizeof(ZUBE));
    return table->zubes ? 0 : -1;
}

static void search_table_destroy(TABLE *table)
{
    if (table->zubes) {
        free(table->zubes);
        table->zubes = NULL;
    }
}

static int is_4same_ops(ZUBE *zube)
{
    int curop = zube->op;
    int n     = 0;
    while (zube) {
        if (zube->op == curop) {
            if (++n == 4) {
                return 1;
            }
            zube = zube->parent;
        } else break;
    }
    return 0;
}

static int cut_branch(int type, int curval, int newval)
{
    switch (type) {
    case 0 : return 0;
    case 1 : return newval < 2;
    case 2 : return newval < 3;
    default: return 0;
    }
}

static ZUBE* search(TABLE *table, ZUBE *start, int state, char *oplist, int opnum, int cuttype)
{
    CUBE  curcube, newcube;
    ZUBE *curzube,*newzube;
    int   curval , newval, i;

    start->parent = NULL;
    start->op     = -1;
    zube2cube(&curcube, start, table->center);
    if (cube_check_state(&curcube) >= state) return start;

    // init search table
    table->open  = 0;
    table->close = 0;

    // put original cube into open table
    table->zubes[table->open] = *start;
    table->open++;

    while (table->close < table->open) {
        // check memory usage
        if (table->open + 6 >= table->size - 1) {
            printf("all table memory have been used !\n");
            break;
        }

        // dequeue a cube from open table
        curzube = &(table->zubes[table->close++]);
        zube2cube(&curcube, curzube, NULL);
        curval  = cube_check_state(&curcube);

        // extend cubes check state and put new cubes into open table
        for (i=0; i<opnum; i++) {
            newzube = &(table->zubes[table->open]);
            newcube = curcube;
            cube_op  (&newcube, oplist[i]);
            cube2zube(newzube, &newcube, NULL);
            newzube->op     = oplist[i];
            newzube->parent = curzube;
            newval = cube_check_state(&newcube);
            if (newval >= state) { // found
                return newzube;
            }
            if (is_4same_ops(newzube)) {
                continue;
            }
            if (cut_branch(cuttype, curval, newval)) {
                continue;
            }
            table->open++;
        }
//      printf("%d %d\n", table->close, table->open);
    }
    return NULL;
}

static void print_solve_oplist(ZUBE *zube)
{
    char oplist[256];
    int  i = 0, n = 0;
    while (zube) {
        static char optab[] = { 'F', 'B', 'U', 'D', 'L', 'R' };
        if (zube->op >= 0) {
            oplist[i++] = optab[(int)zube->op];
        }
        zube = zube->parent;
    }
    printf("\noperation list:\n");
    while (--i >= 0) {
        printf("%c%s%s", oplist[i], i == 0 ? "" : " -> ", ++n % 12 == 0 ? "\n" : "");
    }
    printf("\n");
}

static void cube_solve(CUBE *c)
{
    TABLE t;

    if (search_table_create(&t, 1024*1024*82) != 0) {
        printf("failed to create cube search table !\n");
        return;
    }

    if (1) {
        static char oplisttab[][6] = {
            { CUBE_OP_F, CUBE_OP_U, CUBE_OP_D, CUBE_OP_L, CUBE_OP_R },
            { CUBE_OP_B, CUBE_OP_U, CUBE_OP_D, CUBE_OP_L, CUBE_OP_R },
        };
        static int stepparams[][4] = {
            { 4 , 0, 5, 0 },
            { 8 , 0, 5, 1 },
            { 9 , 1, 5, 2 },
            { 10, 1, 5, 2 },
            { 11, 1, 5, 2 },
            { 12, 1, 5, 2 },
            { 13, 1, 5, 2 },
            { 14, 1, 5, 2 },
            { 15, 1, 5, 2 },
            { 16, 1, 5, 2 },
            { 18, 1, 5, 2 },
            { 20, 1, 5, 2 },
            { 21, 1, 5, 2 },
            { 24, 1, 5, 2 },
            { 0 , 0, 0, 0 },
        };
        ZUBE  start = {0};
        ZUBE *find  = NULL;
        int   i;
        cube2zube(&start, c, t.center);
        for (i=0; stepparams[i][0]; i++) {
            find = search(&t, &start, stepparams[i][0], oplisttab[stepparams[i][1]], stepparams[i][2], stepparams[i][3]);
            if (find) {
                start = *find;
                print_solve_oplist(find);
                zube2cube(c, find, t.center);
                if (stepparams[i][0] != 24) cube_render(c);
            } else {
                printf("can't solve !\n");
                goto done;
            }
        }
    }

done:
    search_table_destroy(&t);
}

static void convert_input(char s[3][3])
{
    int i, j;
    for (i=0; i<3; i++) {
        for (j=0; j<3; j++) {
            switch (s[i][j]) { // W Y B G O R
            case 'w': case 'W': s[i][j] = 1; break;
            case 'y': case 'Y': s[i][j] = 2; break;
            case 'b': case 'B': s[i][j] = 3; break;
            case 'g': case 'G': s[i][j] = 4; break;
            case 'o': case 'O': s[i][j] = 5; break;
            case 'r': case 'R': s[i][j] = 6; break;
            }
        }
    }
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

    convert_input(c->f);
    convert_input(c->u);
    convert_input(c->d);
    convert_input(c->l);
    convert_input(c->r);
    convert_input(c->b);
}

static void show_help(void)
{
    printf(
        "\n"
        "available commands:\n\n"
        "f f2 f' b b2 b' u u2 u' d d2 d' l l2 l' r r2 r'\n"
        "init rand solve help exit\n\n"
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
            gets(str);
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

