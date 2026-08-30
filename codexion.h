#ifndef CODEXION_H
# define CODEXION_H

# define COOLDOWN 0
# define FREE 1
# define ALIVE 2
# define DEAD 3
# define TAKEN 4

# define _DSTATE(state) ( \
    ((state) >= 0 && (state) <= 4) ? \
    ((const char *[]){ \
        "COOLDOWN", \
        "FREE", "ALIVE", "DEAD", "TAKEN" \
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
# include <strings.h>

typedef struct {
	int				state;
	pthread_mutex_t lock;
	pthread_t		thread;
} t_dongle;

typedef struct {
	int				ncompiles;
	int 			last_compile;
	long			deadline;
	int				state;
	t_dongle		*left;
	t_dongle		*right;
	pthread_cond_t	*cond;
	pthread_mutex_t cond_lock;
	pthread_t		thread;
} t_coder;

typedef struct {
	pthread_t		moniter;
	int 			ncoders;
	int 			ncompiles;
	char			*type;
	int				ndongles;
	int 			cooldown;
	int 			burnout;
	int 			tcompile;
	int 			tdebug;
	int 			trefactor;
	int				qsize;
	int				*queue;
	pthread_mutex_t	queue_lock;
	pthread_mutex_t output_lock;
	t_dongle		**dongles;
	t_coder			**coders;
} t_codexion;

typedef struct {
	int			coder_id;
	t_codexion	*codex;
} t_workload;


t_codexion	*init_codexion(int argc, char **argv);
int			workloads_done(t_codexion *codex);
void		*coder_thread(void *arg);
void		start_simulation(t_codexion *codex);
int			request_dongles(t_codexion *codex, int coder_id);
void		release_dongles(t_codexion *codex, int coder_id);
long		get_elapsed_time(struct timeval start, struct timeval end);
void		ts_printf(const char *format, struct timeval *start, struct timeval *end, int coder_id);
void		*start_moniter(void *arg);
void		destroy_codexion(t_codexion *codex);
void		set_codex_config(t_codexion *codex, int argc, char **argv);
t_dongle	*init_dongle();
t_coder		*init_coder();

#endif
