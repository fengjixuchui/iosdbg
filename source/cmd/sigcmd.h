#ifndef _SIGCMD_H_
#define _SIGCMD_H_

#include "argparse.h"

enum cmd_error_t cmdfunc_signal_deliver(struct cmd_args *, int, char **, char **);
enum cmd_error_t cmdfunc_signal_handle(struct cmd_args *, int, char **, char **);

static const char *SIGNAL_COMMAND_DOCUMENTATION =
    "'signal' describes the group of commands which deal with signals.\n";

static const char *SIGNAL_DELIVER_COMMAND_DOCUMENTATION =
    "Deliver an arbitrary signal to the debuggee.\n"
    "This command has one mandatory argument and no optional arguments.\n"
    "\nMandatory arguments:\n"
    "\tsignal\n"
    "\t\tThe signal to deliver.\n"
    "\nSyntax:\n"
    "\tsignal deliver signal\n"
    "\n";

static const char *SIGNAL_HANDLE_COMMAND_DOCUMENTATION =
    "Change how iosdbg will handle signals sent from the OS to the debuggee.\n"
    "This command has no mandatory arguments and four optional arguments.\n"
    "\nOptional arguments:\n"
    "\tsignals\n"
    "\t\tThe signal(s) you're updating.\n"
    "\tnotify\n"
    "\t\tWhether or not iosdbg notifies you if the signal has been received.\n"
    "\tpass\n"
    "\t\tWhether or not the signal will be passed to the debuggee.\n"
    "\tstop\n"
    "\t\tWhether or not the debuggee will be stopped upon receiving this signal.\n"
    "\nIf none of these arguments are given, the current policy is shown.\n"
    "\nSyntax:\n"
    "\tsignal handle signals --notify <boolean> --pass <boolean> --stop <boolean>\n"
    "\tsignal handle signals -n <boolean> -p <boolean> -s <boolean>\n"
    "\n";

/*
 * Regexes
 */
static const char *SIGNAL_DELIVER_COMMAND_REGEX =
    "^(?<signal>[^\\s]+)";

static const char *SIGNAL_HANDLE_COMMAND_REGEX =
    "^(?<signals>[\\w\\s]+)\\s+"
    "--?(n(otify)?)\\s+(?<notify>1|0|true|false)\\s+"
    "--?(p(ass)?)\\s+(?<pass>1|0|true|false)\\s+"
    "--?(s(top)?)\\s+(?<stop>1|0|true|false)";

/*
 * Regex groups
 */
static const char *SIGNAL_DELIVER_COMMAND_REGEX_GROUPS[MAX_GROUPS] =
    { "signal" };

static const char *SIGNAL_HANDLE_COMMAND_REGEX_GROUPS[MAX_GROUPS] =
    { "signals", "notify", "pass", "stop" };

#endif
