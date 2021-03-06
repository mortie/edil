#if !defined(BIND_CMDBIND_H)
# define BIND_CMDBIND_H

#define BIND_CMD_MAP_ALL(mode) \
    BIND_MAP(mode, cmd_new,  inp_key_esc | 'n') \
    BIND_MAP(mode, cmd_load, inp_key_esc | 'a') \
    BIND_MAP(mode, cmd_save, inp_key_esc | 's') \
    BIND_MAP(mode, cmd_goto, inp_key_esc | 'g') \
    \
    BIND_MAP(mode, cmd_quit, inp_key_esc | inp_key_ctrl | 'K') \
    \
    BIND_MAP(mode, cmd_swap, inp_key_ctrl | 'C') \
    BIND_MAP(mode, cmd_snap, inp_key_ctrl | 'Z') \
    \
    BIND_MAP(mode, cmd_next, inp_key_ctrl | 'N') \
    BIND_MAP(mode, cmd_prev, inp_key_ctrl | 'V') \
    \
    BIND_MAP(mode, cmd_copy,  inp_key_ctrl | 'Y') \
    BIND_MAP(mode, cmd_paste, inp_key_ctrl | 'P') \
    \
    BIND_MAP(mode, cmd_incrindent, inp_key_tab) \
    BIND_MAP(mode, cmd_decrindent, inp_key_shifttab) \

void bind_cmdbind_init(void);

#endif