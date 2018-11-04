#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#pragma pack(1)
typedef struct {
    char f[3][3];
    char b[3][3];
    char u[3][3];
    char d[3][3];
    char l[3][3];
    char r[3][3];
} CUBE;
#pragma pack()

static void cube_init(CUBE *c)
{
#if 1
    memset(c->f, 'W', sizeof(c->f));
    memset(c->b, 'Y', sizeof(c->b));
    memset(c->u, 'G', sizeof(c->u));
    memset(c->d, 'B', sizeof(c->d));
    memset(c->l, 'R', sizeof(c->l));
    memset(c->r, 'O', sizeof(c->r));
#else
    int i, j, n = 1;
    for (i=0; i<3; i++) {
        for (j=0; j<3; j++) {
            c->f[i][j] = n++;
        }
    }
    for (i=0; i<3; i++) {
        for (j=0; j<3; j++) {
            c->b[i][j] = n++;
        }
    }
    for (i=0; i<3; i++) {
        for (j=0; j<3; j++) {
            c->u[i][j] = n++;
        }
    }
    for (i=0; i<3; i++) {
        for (j=0; j<3; j++) {
            c->d[i][j] = n++;
        }
    }
    for (i=0; i<3; i++) {
        for (j=0; j<3; j++) {
            c->l[i][j] = n++;
        }
    }
    for (i=0; i<3; i++) {
        for (j=0; j<3; j++) {
            c->r[i][j] = n++;
        }
    }
#endif
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
    surface_rotate90(c->f);
    line_rotate90(lines);
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
    surface_rotate90(c->b);
    line_rotate90(lines);
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
    surface_rotate90(c->u);
    line_rotate90(lines);
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
    surface_rotate90(c->d);
    line_rotate90(lines);
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
    surface_rotate90(c->l);
    line_rotate90(lines);
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
    surface_rotate90(c->r);
    line_rotate90(lines);
}

static void cube_rand(CUBE *c, int n) {
    int i, r;
    for (i=0; i<n; i++) {
        r = rand() % 6;
        switch (r) {
        case 0 : cube_f(c); break;
        case 1 : cube_b(c); break;
        case 2 : cube_u(c); break;
        case 3 : cube_d(c); break;
        case 4 : cube_l(c); break;
        case 5 : cube_r(c); break;
        }
    }
}

static void cube_solve(CUBE *c) {}

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

    for (i=0; i<3; i++) {
        for (j=0; j<3; j++) {
            buffer[3+i][3+j] = c->f[i][j];
        }
    }
    for (i=0; i<3; i++) {
        for (j=0; j<3; j++) {
            buffer[3+i][9+j] = c->b[i][j];
        }
    }
    for (i=0; i<3; i++) {
        for (j=0; j<3; j++) {
            buffer[0+i][3+j] = c->u[i][j];
        }
    }
    for (i=0; i<3; i++) {
        for (j=0; j<3; j++) {
            buffer[6+i][3+j] = c->d[i][j];
        }
    }
    for (i=0; i<3; i++) {
        for (j=0; j<3; j++) {
            buffer[3+i][0+j] = c->l[i][j];
        }
    }
    for (i=0; i<3; i++) {
        for (j=0; j<3; j++) {
            buffer[3+i][6+j] = c->r[i][j];
        }
    }

    for (i=0; i<9; i++) {
        for (j=0; j<12; j++) {
#if 1
            switch (buffer[i][j]) {
            case 'W': SetConsoleTextAttribute(h, FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE); break;
            case 'Y': SetConsoleTextAttribute(h, FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_GREEN); break;
            case 'G': SetConsoleTextAttribute(h, FOREGROUND_INTENSITY|FOREGROUND_GREEN); break;
            case 'O': SetConsoleTextAttribute(h, FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_BLUE ); break;
            case 'B': SetConsoleTextAttribute(h, FOREGROUND_INTENSITY|FOREGROUND_BLUE ); break;
            case 'R': SetConsoleTextAttribute(h, FOREGROUND_INTENSITY|FOREGROUND_RED  ); break;
            }
            printf(buffer[i][j] ? "\2 " : "  ");
#else
            if (buffer[i][j]) printf("%02d ", buffer[i][j]);
            else printf("   ");
#endif
        }
        printf("\n");
    }

    // Restore the original color  
    SetConsoleTextAttribute(h, wOldColorAttrs);
}

int main(void)
{
    char cmd[128];
    CUBE c;

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
}




