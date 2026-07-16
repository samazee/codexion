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

# include <stdlib.h>
# include <pthread.h>
# include <sys/time.h>
# include <errno.h>
# include <unistd.h>
# include <stdio.h>
# include <string.h>

typedef pthread_t t_coder;

typedef struct {
	pthread_mutex_t *lock;
	pthread_t		*thread;
	int				state;
} t_dongle;

typedef struct {
	pthread_t		*thread;
	char			*type;
	pthread_cond_t	*conds;
	t_dongle		*dongles;
	int				ndongles;
	int				ncompiles;
	int 			cooldown;
	int 			burnout;
	int 			tcompile;
	int 			tdebug;
	int 			trefactor;
	int				*queue;
	pthread_mutex_t	queue_lock;
} t_scheduler;

typedef struct {
	int 		ncoders;
	int 		ncompiles;
	t_scheduler	*scheduler;
	t_coder		*coders;
} t_codexion;

typedef struct {
	int			coder_id;
	t_scheduler *scheduler;
} t_workload;


t_codexion	*init_codexion(int argc, char **argv);
void		*coder_workload(void *arg);
void		start_simulation(t_codexion *codex);
t_dongle	*request_dongles(t_scheduler *scheduler, int coder_id);
void		release_dongles(t_scheduler *scheduler, int coder_id);
long		get_elapsed_time(struct timeval start, struct timeval end);
void		ts_printf(const char *format, struct timeval *start, struct timeval *end, int coder_id);
void		*start_scheduler(void *arg);

#endif
