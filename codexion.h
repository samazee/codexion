#ifndef CODEXION_H
# define CODEXION_H

# define COMPILE 0
# define DEBUG 1
# define REFACTOR 2
# define COOLDOWN 3
# define FREE 4
# define IDLE 5
# define BURNOUT 6
# define TAKEN 7

# define _DSTATE(state) ( \
    ((state) >= 0 && (state) <= 7) ? \
    ((const char *[]){ \
        "COMPILE", "DEBUG", "REFACTOR", "COOLDOWN", \
        "FREE", "IDLE", "BURNOUT", "TAKEN" \
    })[state] : "UNKNOWN" \
)

# include <stdlib.h>
# include <pthread.h>
# include <sys/time.h>
# include <errno.h>
# include <unistd.h>
# include <stdio.h>
# include <stdarg.h>
# include <string.h>

typedef struct {
	pthread_t	*thread;
	int			ncompiles;
	int 		last_compile;
} t_coder;

typedef struct {
	pthread_mutex_t *lock;
	pthread_t		*thread;
	int				state;
} t_dongle;

typedef struct {
} t_scheduler;

typedef struct {
	pthread_t		*moniter;
	int 			ncoders;
	int 			ncompiles;
	t_coder			**coders;
	char			*type;
	pthread_cond_t	*conds;
	t_dongle		**dongles;
	int				ndongles;
	int 			cooldown;
	int 			burnout;
	int 			tcompile;
	int 			tdebug;
	int 			trefactor;
	int				*queue;
	pthread_mutex_t	*queue_lock;
	pthread_mutex_t *output_lock;
} t_codexion;

typedef struct {
	int			coder_id;
	t_codexion	*codex;
} t_workload;


t_codexion	*init_codexion(int argc, char **argv);
void		*coder_workload(void *arg);
void		start_simulation(t_codexion *codex);
t_dongle	*request_dongles(t_codexion *codex, int coder_id);
void		release_dongles(t_codexion *codex, int coder_id);
long		get_elapsed_time(struct timeval start, struct timeval end);
void		ts_printf(const char *format, struct timeval *start, struct timeval *end, int coder_id);
void		*start_moniter(void *arg);

#endif
