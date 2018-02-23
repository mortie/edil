#include <string.h>

#include "vec.h"
#include "bar.h"
#include "cli.h"

#include "files.h"

void files_load_query(text_buf *b)
{
    bar_query("Load: ", files_load_confirm);
}

void files_load_confirm(char *str)
{
    FILE *f;
    vec   name;

    vec_init(&name, sizeof(char));
    vec_ins(&name, 0, strlen(str), str);

    text_buf_setname(text_cur_buf, &name);

    if (strlen(str) == 0) return;

    f = fopen(str, "r");

    if (f)
    {
        files_load(text_cur_buf, f);

        cli_lines_after(text_cur_buf, 0);
    
        fclose(f);
    }
}    

void files_dump_query(text_buf *b)
{
}

void files_dump_confirm(char *str)
{
    FILE *f;

    if (strlen(str) == 0) return;

    f = fopen(str, "w");

    if (f)
    {
        files_dump(text_cur_buf, f);

        fclose(f);
    }
}

void files_load(text_buf *b, FILE *f)
{
    int chr;
    size_t ln;
    vec *line;
    text_cmd cmd;

    ln = text_buf_len(b);

    while (ln--)
    {
        cmd.type = text_cmd_del_line;
        cmd.args.del_line.ln = ln;

        text_buf_cmd(b, &cmd);
    }

    line = &(cmd.args.ins.chrs);
    vec_init(line, sizeof(text_char));
    cmd.type = text_cmd_ins;
    cmd.args.ins.cn = 0;
    cmd.args.ins.ln = 0;
 
    ln = 0;
    while ((chr = getc(f)) != EOF)
    {
        if (chr != '\n')
        {
            text_char chrval;
            chrval.utf8[0] = chr;
            chrval.utf8[1] = '\0';
            chrval.fg      = text_col_none;
            vec_ins(line, vec_len(line), 1, &chrval);
        }
        else
        {
            text_cmd lcmd;

            text_buf_cmd(b, &cmd);
            ln += 1;

            vec_del(line, 0, vec_len(line));
            lcmd.type = text_cmd_ins_line;
            lcmd.args.ins_line.ln = ln;

            cmd.args.ins.ln = ln;
 
            text_buf_cmd(b, &lcmd);
        }
    }

    text_buf_cmd(b, &cmd);
}

void files_dump(text_buf *b, FILE *f)
{
    size_t maxln, ln;

    maxln = text_buf_len(b);

    for (ln = 0; ln < maxln; ln++)
    {
        vec    line;
        size_t maxcn, cn;

        text_buf_get(b, ln, &line);
        maxcn = vec_len(&line);

        for (cn = 0; cn < maxcn; cn++)
        {
            text_char *chr;
            chr = vec_get(&line, cn);
            fputs(chr->utf8, f);
        }
        
        if (ln < maxln - 1)
        {
            fputc('\n', f);
        }
    }
}

