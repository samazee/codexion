/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: azgor <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/30 15:47:19 by azgor             #+#    #+#             */
/*   Updated: 2026/09/01 19:05:13 by azgor            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# define COOLDOWN 0
# define FREE 1
# define ALIVE 2
# define DEAD 3
# define TAKEN 4

# include <stdlib.h>
# include <pthread.h>
# include <sys/time.h>
# include <errno.h>
# include <unistd.h>
# include <stdio.h>
# include <stdarg.h>
# include <string.h>
# include <strings.h>

typedef struct s_dongle
{
	int				state;
	pthread_mutex_t	lock;
	pthread_t		thread;
}	t_dongle;

typedef struct s_coder
{
	int				ncompiles;
	int				last_compile;
	long			deadline;
	int				state;
	t_dongle		*left;
	t_dongle		*right;
	pthread_cond_t	*cond;
	pthread_mutex_t	cond_lock;
	pthread_t		thread;
}	t_coder;

typedef struct s_codexion
{
	pthread_t		moniter;
	int				ncoders;
	int				ncompiles;
	char			*type;
	int				ndongles;
	int				cooldown;
	int				burnout;
	int				tcompile;
	int				tdebug;
	int				trefactor;
	int				qsize;
	int				*queue;
	pthread_mutex_t	queue_lock;
	pthread_mutex_t	output_lock;
	t_dongle		**dongles;
	t_coder			**coders;
}	t_codexion;

typedef struct s_workload
{
	int			coder_id;
	t_codexion	*codex;
}	t_workload;

t_codexion	*init_codexion(int argc, char **argv);
t_dongle	*init_dongle(void);
t_coder		*init_coder(void);
int			validate_argv(int argc, char **argv);
void		set_codex_config(t_codexion *codex, char **argv);
void		destroy_codexion(t_codexion *codex);
int			get_coder_queue_index(t_codexion *codex, int coder_id);
void		queue_request(t_codexion *codex, int coder_id);
void		pop_queue(t_codexion *codex, int coder_id);
void		start_simulation(t_codexion *codex);
int			request_dongles(t_codexion *codex, int coder_id);
void		release_dongles(t_codexion *codex, int coder_id);
void		cooldown_dongles(t_codexion *codex, int coder_id);
int			wait_for_dongles(t_codexion *codex, int coder_id);
void		*free_dongle(void *arg);
void		*start_moniter(void *arg);
int			is_burnout(t_codexion *codex);
int			workloads_done(t_codexion *codex);
int			is_dongles_free(t_codexion *codex, int coder_id);
int			is_coder_turn(t_codexion *codex, int coder_id);
int			is_fifo_coder_turn(t_codexion *codex, int coder_id);
void		fifo_scheduler(t_codexion *codex);
int			get_edf_coder(t_codexion *codex);
int			is_edf_coder_turn(t_codexion *codex, int coder_id);
void		edf_scheduler(t_codexion *codex);
long		get_elapsed_time(struct timeval *start, struct timeval *end);
void		codex_log(t_codexion *codex, const char *format,
				long dt, int coder_id);
int			coder_cycle(t_codexion *codex, int coder_id,
				struct timeval *start, struct timeval *end);
void		*coder_thread(void *arg);

#endif
