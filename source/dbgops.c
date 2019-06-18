#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "breakpoint.h"
#include "convvar.h"
#include "dbgops.h"
#include "debuggee.h"
#include "exception.h"
#include "linkedlist.h"
#include "memutils.h"
#include "printing.h"
#include "ptrace.h"
#include "queue.h"
#include "servers.h"
#include "sigsupport.h"
#include "strext.h"
#include "thread.h"
#include "trace.h"
#include "watchpoint.h"

void ops_printsiginfo(char **outbuffer){
    concat(outbuffer, "%-11s %-5s %-5s %-6s\n", "NAME", "PASS", "STOP", "NOTIFY");
    concat(outbuffer, "=========== ===== ===== ======\n");

    int signo = 0;

    while(signo++ < (NSIG - 1)){
        int notify, pass, stop;
        char *e = NULL;

        sigsettings(signo, &notify, &pass, &stop, 0, &e);

        if(e)
            free(e);

        char *sigstr = strdup(sys_signame[signo]);
        size_t sigstrlen = strlen(sigstr);

        for(int i=0; i<sigstrlen; i++)
            sigstr[i] = toupper(sigstr[i]);
        
        char *fullsig = NULL;
        concat(&fullsig, "SIG%s", sigstr);
        free(sigstr);

        const char *notify_str = notify ? "true" : "false";
        const char *pass_str = pass ? "true" : "false";
        const char *stop_str = stop ? "true" : "false";

        concat(outbuffer, "%-11s %-5s %-5s %-6s\n",
                fullsig, pass_str, stop_str, notify_str);

        free(fullsig);
    }
}

void ops_detach(int from_death, char **outbuffer){
    ops_suspend();

    breakpoint_delete_all();
    watchpoint_delete_all();

    /* Disable hardware single stepping on all threads. */
    for(struct node_t *current = debuggee->threads->front;
            current;
            current = current->next){
        struct machthread *t = current->data;

        get_debug_state(t);
        t->debug_state.__mdscr_el1 = 0;
        set_debug_state(t);
    }

    void *request = dequeue(debuggee->exc_requests);
    
    /* Reply to any exceptions. */
    while(request){
        reply_to_exception(request, KERN_SUCCESS);
        free(request);
        request = dequeue(debuggee->exc_requests);
    }

    debuggee->deallocate_ports(outbuffer);
    debuggee->restore_exception_ports();

    /* Send SIGSTOP to set debuggee's process status to
     * SSTOP so we can detach. Calling ptrace with PT_THUPDATE
     * to handle Unix signals sets this status to SRUN, and ptrace 
     * bails if this status is SRUN. See bsd/kern/mach_process.c
     */
    if(!from_death){
        kill(debuggee->pid, SIGSTOP);
        int ret = ptrace(PT_DETACH, debuggee->pid, (caddr_t)1, 0);

        /* In some cases, it takes a while for the debuggee to notice the
         * SIGSTOP.
         */
        while(ret == -1){
            ret = ptrace(PT_DETACH, debuggee->pid, 0, 0);
            usleep(500);
        }

        kill(debuggee->pid, SIGCONT);
    }

    queue_free(debuggee->exc_requests);
    debuggee->exc_requests = NULL;

    linkedlist_free(debuggee->breakpoints);
    linkedlist_free(debuggee->threads);
    linkedlist_free(debuggee->watchpoints);

    debuggee->breakpoints = NULL;
    debuggee->threads = NULL;
    debuggee->watchpoints = NULL;

    debuggee->num_breakpoints = 0;
    debuggee->num_watchpoints = 0;
    debuggee->pid = -1;

    free(debuggee->debuggee_name);
    debuggee->debuggee_name = NULL;

    void_convvar("$_");
    void_convvar("$__");
    void_convvar("$ASLR");

    ops_resume();
}

void ops_resume(void){
    debuggee->resume();
}

void ops_suspend(void){
    debuggee->suspend();
}

void ops_threadupdate(char **out){
    thread_act_port_array_t threads;
    debuggee->get_threads(&threads, out);

    machthread_updatethreads(threads);

    struct machthread *focused = machthread_getfocused();

    if(!focused){
        concat(out, "[Previously selected thread dead, selecting thread #1]\n\n");
        machthread_setfocused(threads[0]);
        focused = machthread_getfocused();
    }

    if(focused)
        machthread_updatestate(focused);

    /* Adjust thread specific breakpoints. */
    for(struct node_t *current = debuggee->breakpoints->front;
            current;
            current = current->next){
        struct breakpoint *bp = current->data;

        if(bp->threadinfo.all || !bp->hw)
            continue;

        struct machthread *thread = machthread_find(bp->threadinfo.iosdbg_tid);

        if(thread->tid != bp->threadinfo.real_tid){
            get_debug_state(thread);

            thread->debug_state.__bcr[bp->hw_bp_reg] = 0;
            thread->debug_state.__bvr[bp->hw_bp_reg] = 0;
            thread->debug_state.__mdscr_el1 = 0;

            set_debug_state(thread);
            
            struct machthread *correct = machthread_find_via_tid(bp->threadinfo.real_tid);

            if(!correct){
                concat(out, "[The thread assigned to breakpoint %d has gone"
                        " away, deleting it]\n", bp->id);
                breakpoint_delete(bp->id, NULL);
                continue;
            }

            get_debug_state(correct);

            correct->debug_state.__bcr[bp->hw_bp_reg] = bp->bcr;
            correct->debug_state.__bvr[bp->hw_bp_reg] = bp->bvr;

            set_debug_state(correct);

            bp->threadinfo.iosdbg_tid = correct->ID;
            bp->threadinfo.real_tid = correct->tid;

            concat(out, "[Corrected thread info for breakpoint %d]\n", bp->id);
        }
    }
}
