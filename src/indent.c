#include <string.h>

#include "ring.h"

#include "indent.h"

indent_flag indent_mode      = 0;
int         indent_tab_width = 4;
int         indent_lvl_width = 4;
char       *indent_tab_text  = ";";
col_desc    indent_tab_col   = { .fg = col_blue, .bg = col_none };

void indent_print_tab(size_t ind, FILE *f, col fnt)
{
    char *str;
    size_t len, width;
    col tabcol;

    width = indent_get_width(&CHR("\t"), ind);
    len   = strlen(indent_tab_text);

    tabcol = col_update(fnt, indent_tab_col);
    col_print(tabcol, f);

    if (width > len)
    {
        size_t n;
        n = width - len;

        while (n--) fputs(" ", f);

        str = indent_tab_text;
    }
    else
    {
        str = indent_tab_text + len - width;
    }

    fputs(str, f);
    col_print(fnt, f);

}

int indent_get_width(chr *c, size_t ind)
{
    if (!c) return 0;

    if (chr_is_blank(c))
        return 1;

    if (strcmp(c->utf8, "\t") == 0)
        return indent_tab_width - (ind % indent_tab_width);

    return 1;
}

void indent_add_blanks_buf(buf *b)
{
    vec *lines;
    size_t ln, maxln;

    lines = &(b->lines);
    maxln = vec_len(lines);
    for (ln = 0; ln < maxln; ln++)
    {
        vec *l;
        l = vec_get(lines, ln);

        indent_add_blanks_line(l, 0);
    }
}

void indent_add_blanks_line(vec *line, size_t ind)
{
    for (; ind < vec_len(line); ind++)
    {
        chr *c;
        c = vec_get(line, ind);

        if (!c) break;

        if (indent_get_width(c, ind) != 1 || strcmp(c->utf8, "\t") == 0)
            indent_add_blanks_chr(line, ind);
    }
}

void indent_add_blanks_chr(vec *line, size_t ind)
{
    size_t blanks, width, blankind, len;
    chr   *c;

    c   = vec_get(line, ind);
    len = vec_len(line);

    if (!c) return;

    width = indent_get_width(c, ind);

    for (blankind = ind + 1; blankind < len; blankind++)
    {
        c = vec_get(line, blankind);

        if (!c || !chr_is_blank(c)) break;
    }

    blanks = blankind - ind - 1;

    if (blanks > width - 1)
    {
        size_t n;
        n = blanks - width + 1;
        vec_del(line, blankind - n, n);
    }
    else if (blanks < width - 1)
    {
        size_t n, off;
        n = width - 1 - blanks;
        vec_ins(line, blankind, n, NULL);

        for (off = 0; off < n; off++)
        {
            chr *c;
            c = vec_get(line, blankind + off);
            chr_blankify(c);
        }
    }
}

size_t indent_get_depth(buf *b, cur c)
{
    vec *line;
    size_t ind, len;

    line = vec_get(&(b->lines), c.ln);
    if (!line) return 0;

    len = vec_len(line);
    for (ind = 0; ind < len; ind++)
    {
        chr *c;
        c = vec_get(line, ind);
        if (!c || !chr_is_whitespace(c)) break;
    }

    return ind;
}

void indent_set_depth(buf *b, cur c, size_t depth)
{
    vec *line;
    size_t orig;

    if (b->flags & buf_readonly) return;
    b->flags |= buf_modified;

    line = vec_get(&(b->lines), c.ln);
    if (!line) return;

    orig = indent_get_depth(b, c);

    if (orig)
        vec_del(line, 0, orig);

    if (depth)
    {
        size_t tabs, spaces;
        chr tab   = CHR("\t");
        chr space = CHR(" ");

        if (indent_mode & indent_spacify)
        {
            spaces = depth;
            tabs   = 0;
        }
        else
        {
            spaces = depth % indent_tab_width;
            tabs   = depth / indent_tab_width;
        }

        vec_rep(line, 0, 1, &space, spaces);
        vec_rep(line, 0, 1, &tab,   tabs);
    }

    indent_add_blanks_line(line, 0);
}

cur indent_incr_depth(buf *b, cur c)
{
    size_t depth;
    depth = indent_get_depth(b, c) + indent_lvl_width;

    indent_set_depth(b, c, depth);
    c.cn += indent_lvl_width;

    c = cur_check_bounds(c, b);
    c = cur_check_blank(c, b, (cur){ .cn = 1 });

    return c;
}

cur indent_decr_depth(buf *b, cur c)
{
    ssize_t depth;
    depth = indent_get_depth(b, c) - indent_lvl_width;
    if (depth < 0) depth = 0;

    indent_set_depth(b, c, depth);

    c.cn -= indent_lvl_width;
    if (c.cn < 0) c.cn = 0;

    c = cur_check_bounds(c, b);
    c = cur_check_blank(c, b, (cur){ .cn = -1 });

    return c;
}

cur indent_auto_depth(buf *b, cur c)
{
    ssize_t depth;

    if (!(indent_mode & indent_auto))
        return c;

    depth = indent_get_depth(b, (cur){ .ln = c.ln - 1 });
    if (depth < 0) depth = 0;

    indent_set_depth(b, c, depth);
    c.cn = depth;

    return c;
}

void indent_set_tab_width(size_t width)
{
    size_t ind, len;
    indent_tab_width = width;

    len = vec_len(&ring_bufs);
    for (ind = 0; ind < len; ind++)
    {
        buf **b;
        b = vec_get(&ring_bufs, ind);
        indent_add_blanks_buf(*b);
    }

    win_out_all(stdout);
}

