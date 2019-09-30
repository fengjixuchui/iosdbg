#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "regcmd.h"

#include "../debuggee.h"
#include "../reg.h"
#include "../strext.h"
#include "../thread.h"

enum cmd_error_t cmdfunc_register_view(struct cmd_args *args,
        int arg1, char **outbuffer, char **error){
    struct machthread *focused = get_focused_thread();

    /* If there were no arguments, print every register. */
    if(args->num_args == 0){
        get_thread_state(focused);

        for(int i=0; i<29; i++){        
            char *regstr = NULL;
            concat(&regstr, "x%d", i);

            concat(outbuffer, "%10s = 0x%16.16llx\n", regstr, 
                    focused->thread_state.__x[i]);

            free(regstr);
        }
        
        concat(outbuffer, "%10s = 0x%16.16llx\n", "fp", focused->thread_state.__fp);
        concat(outbuffer, "%10s = 0x%16.16llx\n", "lr", focused->thread_state.__lr);
        concat(outbuffer, "%10s = 0x%16.16llx\n", "sp", focused->thread_state.__sp);
        concat(outbuffer, "%10s = 0x%16.16llx\n", "pc", focused->thread_state.__pc);
        concat(outbuffer, "%10s = 0x%8.8x\n", "cpsr", focused->thread_state.__cpsr);

        return CMD_SUCCESS;
    }

    char *curreg = argcopy(args, REGISTER_VIEW_COMMAND_REGEX_GROUPS[0]);

    while(curreg){
        char *cleanedreg = NULL, *curregval = NULL, *e = NULL;
        enum regtype curregtype = NONE;
        long val = regtol(focused, HEXADECIMAL, &curregtype,
                curreg, &cleanedreg, &curregval, &e);

        if(e)
            concat(outbuffer, "%10s %s\n", "error:", e);

        if(curregtype == LONG)
            concat(outbuffer, "%8s = 0x%16.16lx\n", cleanedreg, val);
        else if(curregtype == INTEGER)
            concat(outbuffer, "%8s = 0x%8.8x\n", cleanedreg, val);
        else if(curregtype == FLOAT)
            concat(outbuffer, "%8s = %g\n", cleanedreg, *(float *)&val);
        else if(curregtype == DOUBLE)
            concat(outbuffer, "%8s = %.15g\n", cleanedreg, *(double *)&val);
        else if(curregtype == QUADWORD)
            concat(outbuffer, "%8s = %s\n", cleanedreg, curregval);

        free(cleanedreg);
        free(curregval);
        free(e);
        free(curreg);

        curreg = argcopy(args, REGISTER_VIEW_COMMAND_REGEX_GROUPS[0]);
    }

    return CMD_SUCCESS;
}

enum cmd_error_t cmdfunc_register_write(struct cmd_args *args,
        int arg1, char **outbuffer, char **error){
    char *target_str = argcopy(args, REGISTER_WRITE_COMMAND_REGEX_GROUPS[0]);
    char *value_str = argcopy(args, REGISTER_WRITE_COMMAND_REGEX_GROUPS[1]);

    struct machthread *focused = get_focused_thread();

    setreg(focused, target_str, value_str, error);

    free(target_str);
    free(value_str);

    return *error ? CMD_FAILURE : CMD_SUCCESS;
}
