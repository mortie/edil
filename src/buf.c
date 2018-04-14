#include <string.h>

#include "buf.h"
#include "indent.h"

void buf_init(buf *b)
{
    vec_init(&(b->lines), sizeof(vec));
    vec_init(&(b->fname), sizeof(char));

    b->flags = 0;

    buf_ins_line(b, (cur){0, 0});
}

void buf_kill(buf *b)
{
    size_t ln;

    for (ln = 0; ln < buf_len(b); ln++)
        vec_kill(vec_get(&(b->lines), ln));

    vec_kill(&(b->lines));
    vec_kill(&(b->fname));
}

void buf_setcol(buf *b, cur loc, size_t n, col_desc col)
{
    size_t cn, len;
    vec *line;

    line = vec_get(&(b->lines), loc.ln);
    if (!line) return;

    len = vec_len(line);

    if (n + loc.cn > len) n = len - loc.cn;

    for (cn = loc.cn; cn < n + loc.cn; cn++)
    {
        chr *c;
        c = vec_get(line, cn);
        if (!c) break;

        chr_set_cols(c, col);
    }
}

void buf_ins(buf *b, cur loc, chr *chrs, size_t n)
{
    vec *line;

    line = vec_get(&(b->lines), loc.ln);
    if (!line) return; 

    vec_ins(line, loc.cn, n, chrs);
    indent_add_blanks_line(line, loc.cn);
}

void buf_del(buf *b, cur loc, size_t n)
{
    size_t len;
    vec *line;
    chr *c;

    line = vec_get(&(b->lines), loc.ln);
    len  = vec_len(line);

    if (loc.cn     < 0)   loc.cn = 0;
    if (loc.cn + n > len) loc.cn = len - n;

    if (len == 0) return;

    if (n == 0) return;

    while (loc.cn > 0)
    {
        c = vec_get(line, loc.cn);
        if (!c) return;
        if (!chr_is_blank(c)) break;
        loc.cn -= 1;
        n      += 1;
    }

    while (loc.cn + n < len)
    {
        c = vec_get(line, loc.cn + n);
        if (!chr_is_blank(c)) break;
        n += 1;
    }

    if (!line) return;

    vec_del(line, loc.cn, n);
    indent_add_blanks_line(line, loc.cn);
}

void buf_ins_nl(buf *b, cur loc)
{
    size_t num;
    cur  newloc;
    vec *line;

    newloc = (cur){ .ln = loc.ln + 1 };

    line = vec_get(&(b->lines), loc.ln);
    if (!line) return;
    num  = vec_len(line) - loc.cn;

    buf_ins_line(b, newloc);

    if (num > 0)
    {
        chr *text;
        text = vec_get(line, loc.cn);

        buf_ins(b, newloc, text, num);
        buf_del(b, loc, num);
    }
}

void buf_ins_line(buf *b, cur loc)
{
    vec *line;

    line = vec_ins(&(b->lines), loc.ln, 1, NULL);
    if (!line) return;

    vec_init(line, sizeof(chr));
}

void buf_del_line(buf *b, cur loc)
{
    vec *line;

    line = vec_get(&(b->lines), loc.ln);
    if (!line) return;

    vec_kill(line);
    vec_del(&(b->lines), loc.ln, 1);

    if (vec_len(&(b->lines)) == 0)
    {
        line = vec_ins(&(b->lines), 0, 1, NULL);
        if (!line) return;

        vec_init(line, sizeof(chr));
    }
}

size_t buf_len(buf *b)
{
    return vec_len(&(b->lines));
}

size_t buf_line_len(buf *b, cur loc)
{
    vec *line;

    line = vec_get(&(b->lines), loc.ln);
    if (!line) return 0;

    return vec_len(line);
}

vec *buf_line(buf *b, cur loc)
{
    vec *line;

    line = vec_get(&(b->lines), loc.ln);
    if (!line) return 0;

    return line;
}

chr *buf_chr(buf *b, cur loc)
{
    vec *line;
    chr *c;

    line = vec_get(&(b->lines), loc.ln);
    if (!line)
        return NULL;

    c = vec_get(line, loc.cn);

    return c;
}

cur buf_last_cur(buf *b)
{
    cur rtn;
    rtn.ln = buf_len(b) - 1;
    rtn.cn = buf_line_len(b, rtn);

    return rtn;
}

void buf_ins_buf(buf *b, cur *c, buf *other, cur loc, cur end)
{
    for (; loc.ln < end.ln; loc = (cur){ .ln = loc.ln + 1 })
    {
        ssize_t linelen, num;
        vec *line;
        line    = buf_line(other, loc);
        linelen = vec_len(line);

        num = linelen - loc.cn;

        if (num > 0)
        {
            buf_ins(b, *c, vec_get(line, loc.cn), num);
            c->cn += linelen;
        }

        buf_ins_nl(b, *c);
        *c = (cur){ .ln = c->ln + 1 };
    }

    if (loc.ln == end.ln)
    {
        ssize_t num;
        vec *line;
        line    = buf_line(other, loc);

        num = end.cn - loc.cn;

        if (num > 0)
            buf_ins(b, *c, vec_get(line, loc.cn), num);
    }
}

