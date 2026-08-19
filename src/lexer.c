#include <ctype.h>
#include "lexer.h"

static void add_word(token_list_t *list, char *word, int *j)
{
    word[*j] = '\0';

    if (*j > 0)
        token_add(list, TOKEN_WORD, word);

    *j = 0;
}

void lexer(const char *input, token_list_t *list)
{
    int i = 0;

    token_list_init(list);

    while (input[i] != '\0')
    {
        /* Skip whitespace */
        if (isspace((unsigned char)input[i]))
        {
            i++;
            continue;
        }

        /* Pipe */
        if (input[i] == '|')
        {
            token_add(list, TOKEN_PIPE, "|");
            i++;
            continue;
        }

        /* Input redirection */
        if (input[i] == '<')
        {
            token_add(list, TOKEN_INPUT, "<");
            i++;
            continue;
        }

        /* Output / append redirection */
        if (input[i] == '>')
        {
            if (input[i + 1] == '>')
            {
                token_add(list, TOKEN_APPEND, ">>");
                i += 2;
            }
            else
            {
                token_add(list, TOKEN_OUTPUT, ">");
                i++;
            }

            continue;
        }

        /* Background */
        if (input[i] == '&')
        {
            token_add(list, TOKEN_BACKGROUND, "&");
            i++;
            continue;
        }

        char word[MAX_TOKEN_LEN];
        int j = 0;

        while (input[i] != '\0')
        {
            char c = input[i];

            /* End of current word */
            if (isspace((unsigned char)c) ||
                c == '|' ||
                c == '<' ||
                c == '>' ||
                c == '&')
            {
                break;
            }

            /* Single quote */
            if (c == '\'')
            {
                i++;

                while (input[i] != '\0' && input[i] != '\'')
                {
                    if (j < MAX_TOKEN_LEN - 1)
                        word[j++] = input[i];

                    i++;
                }

                if (input[i] == '\'')
                    i++;

                continue;
            }

            /* Double quote */
            if (c == '"')
            {
                i++;

                while (input[i] != '\0' && input[i] != '"')
                {
                    if (input[i] == '\\' && input[i + 1] != '\0')
                        i++;

                    if (j < MAX_TOKEN_LEN - 1)
                        word[j++] = input[i];

                    i++;
                }

                if (input[i] == '"')
                    i++;

                continue;
            }

            /*
             * Backslash handling.
             *
             * This is written to reproduce the examples
             * shown in the Milestone-2 document.
             */
            if (c == '\\')
            {
                if (input[i + 1] == '\0')
                {
                    if (j < MAX_TOKEN_LEN - 1)
                        word[j++] = '\\';

                    i++;
                    continue;
                }

                /*
                 * In the document:
                 *
                 * ls \ hi
                 *
                 * becomes:
                 *
                 * ls
                 * hi
                 *
                 * Therefore a backslash before whitespace
                 * is skipped.
                 */
                if (isspace((unsigned char)input[i + 1]))
                {
                    i += 2;
                    continue;
                }

                /* \\ becomes one literal backslash */
                i++;

                if (j < MAX_TOKEN_LEN - 1)
                    word[j++] = input[i];

                i++;
                continue;
            }

            /* Normal character */
            if (j < MAX_TOKEN_LEN - 1)
                word[j++] = c;

            i++;
        }

        add_word(list, word, &j);
    }

    /* Always add END token */
    token_add(list, TOKEN_END, "END");
}
