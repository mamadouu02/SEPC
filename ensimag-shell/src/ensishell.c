/*****************************************************
 * Copyright Grégory Mounié 2008-2015                *
 *           Simon Nieuviarts 2002-2009              *
 * This code is distributed under the GLPv3 licence. *
 * Ce code est distribué sous la licence GPLv3+.     *
 *****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "variante.h"
#include "readcmd.h"

#ifndef VARIANTE
#error "Variante non défini !!"
#endif

/* Guile (1.8 and 2.0) is auto-detected by cmake */
/* To disable Scheme interpreter (Guile support), comment the
 * following lines.  You may also have to comment related pkg-config
 * lines in CMakeLists.txt.
 */

#if USE_GUILE == 1
#include <libguile.h>

typedef struct process {
	pid_t pid;
	char *cmd;
	struct process *next;
} Process;

Process *process_list = NULL;

void insert_job(pid_t pid, char** argv)
{
	Process *process = malloc(sizeof(Process));

	if (process == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	process->pid = pid;
	char cmd[255];
	strcpy(cmd, argv[0]);

	for (int i = 1; argv[i] != 0; i++) {
		strcat(cmd, " ");
		strcat(cmd, argv[i]);
	}

	process->cmd = malloc(sizeof(char *));

	if (process->cmd == NULL) {
		perror("malloc");
		free(process);
		exit(EXIT_FAILURE);
	}

	strcpy(process->cmd, cmd);
	process->next = process_list;
	process_list = process;
}

void jobs(void)
{
	Process *prev = NULL;
	Process *process = process_list;

	while (process) {
		int status;

		if (waitpid(process->pid, &status, WNOHANG) == 0) {
			printf("[%d] %s\n", process->pid, process->cmd);
			prev = process;
			process = process->next;
		} else {
			if (prev) {
				prev->next = process->next;
				free(process);
				process = prev->next;
			} else {
				process_list = process->next;
				free(process);
				process = process_list;
			}
		}
	}
}

void exec_pipe(char **argv1, char**argv2)
{
	int fds[2];
	pipe(fds);
	pid_t pid = fork();

	if (pid == -1) {
		perror("fork");
		exit(EXIT_FAILURE);
	} else if (pid == 0) {
		dup2(fds[0], STDIN_FILENO);
		close(fds[1]);
		close(fds[0]);
		execvp(argv2[0], argv2);
	} else {
		dup2(fds[1], STDOUT_FILENO);
		close(fds[0]);
		close(fds[1]);
	}
}

void exec(struct cmdline *l)
{
	char **argv = l->seq[0];

	if (argv) {
		if (strncmp(argv[0], "jobs", 4) == 0) {
			jobs();
		} else {
			pid_t pid = fork();

			if (pid == -1) {
				perror("fork");
				exit(EXIT_FAILURE);
			} else if (pid == 0) {
				if (l->in) {
					int fd = open(l->in, O_RDONLY);

					if (fd == -1) {
						perror("open");
						exit(EXIT_FAILURE);
					}

					dup2(fd, STDIN_FILENO);
					close(fd);
				}

				if (l->out) {
					int fd = open(l->out, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
					ftruncate(fd, 0);

					if (fd == -1) {
						perror("open");
						exit(EXIT_FAILURE);
					}

					dup2(fd, STDOUT_FILENO);
					close(fd);
				}

				if (l->seq[1]) {
					exec_pipe(argv, l->seq[1]);
				}

				execvp(argv[0], argv);
			} else {
				if (l->bg) {
					insert_job(pid, argv);
				} else {
					int status;
					waitpid(pid, &status, 0);
				}
			}
		}
	}
}

int question6_executer(char *line)
{
	/* Question 6: Insert your code to execute the command line
	 * identically to the standard execution scheme:
	 * parsecmd, then fork+execvp, for a single command.
	 * pipe and i/o redirection are not required.
	 */
	printf("Not implemented yet: can not execute %s\n", line);

	/* Remove this line when using parsecmd as it will free it */
	free(line);
	
	return 0;
}

SCM executer_wrapper(SCM x)
{
        return scm_from_int(question6_executer(scm_to_locale_stringn(x, 0)));
}
#endif


void terminate(char *line)
{
#if USE_GNU_READLINE == 1
	/* rl_clear_history() does not exist yet in centOS 6 */
	clear_history();
#endif
	if (line)
		free(line);
	printf("exit\n");
	exit(0);
}


int main()
{
	printf("Variante %d: %s\n", VARIANTE, VARIANTE_STRING);

#if USE_GUILE == 1
	scm_init_guile();
	/* register "executer" function in scheme */
	scm_c_define_gsubr("executer", 1, 0, 0, executer_wrapper);
#endif

	while (1) {
		struct cmdline *l;
		char *line = 0;
		// int i, j;
		char *prompt = "ensishell>";

		/* Readline use some internal memory structure that
		   can not be cleaned at the end of the program. Thus
		   one memory leak per command seems unavoidable yet */
		line = readline(prompt);
		if (line == 0 || !strncmp(line, "exit", 4)) {
			terminate(line);
		}

#if USE_GNU_READLINE == 1
		add_history(line);
#endif


#if USE_GUILE == 1
		/* The line is a scheme command */
		if (line[0] == '(') {
			char catchligne[strlen(line) + 256];
			sprintf(catchligne, "(catch #t (lambda () %s) (lambda (key . parameters) (display \"mauvaise expression/bug en scheme\n\")))", line);
			scm_eval_string(scm_from_locale_string(catchligne));
			free(line);
            continue;
        }
#endif

		/* parsecmd free line and set it up to 0 */
		l = parsecmd(&line);

		/* If input stream closed, normal termination */
		if (!l) {
			terminate(0);
		}
		
		if (l->err) {
			/* Syntax error, read another command */
			printf("error: %s\n", l->err);
			continue;
		}

		// if (l->in) printf("in: %s\n", l->in);
		// if (l->out) printf("out: %s\n", l->out);
		// if (l->bg) printf("background (&)\n");

		/* Display each command of the pipe */
		// for (i=0; l->seq[i]!=0; i++) {
		// 	char **cmd = l->seq[i];
		// 	printf("seq[%d]: ", i);
		// 	for (j=0; cmd[j]!=0; j++) {
		// 		printf("'%s' ", cmd[j]);
		// 	}
		// 	printf("\n");
		// }
		
		exec(l);
	}
}
