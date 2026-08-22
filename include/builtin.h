#ifndef BUILTIN_H
#define BUILTIN_H

#include "parser.h"

/*
 * Result returned by the builtin dispatcher.
 *
 * BUILTIN_NOT_FOUND:
 *     The command is not a shell builtin.
 *
 * BUILTIN_DONE:
 *     Builtin executed successfully.
 *
 * BUILTIN_FAILED:
 *     Builtin was recognized but failed.
 *
 * BUILTIN_EXIT:
 *     Shell should terminate.
 */
typedef enum
{
    BUILTIN_NOT_FOUND = 0,
    BUILTIN_DONE,
    BUILTIN_FAILED,
    BUILTIN_EXIT
} builtin_result_t;

/*
 * Execute a builtin command.
 *
 * Returns one of builtin_result_t values.
 */
builtin_result_t builtin_execute(command_t *command);

/*
 * Returns the name of the directory that was active
 * before the most recent successful cd operation.
 */
const char *builtin_previous_directory(void);

#endif
