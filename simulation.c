#include "codexion.h"

long	get_elapsed_time(struct timeval start, struct timeval end) {
	if (end.tv_sec != start.tv_sec)
		return ((((end.tv_sec - start.tv_sec) * 1000000) + end.tv_usec - start.tv_usec) / 1000);
	return ((end.tv_usec - start.tv_usec) / 1000);
}

void codex_log(pthread_mutex_t *lock, const char *format, struct timeval *start, struct timeval *end, int coder_id)
{
	pthread_mutex_lock(lock);
	gettimeofday(end, NULL);
	printf(format, get_elapsed_time(*start, *end), coder_id);
	pthread_mutex_unlock(lock);
}

int	workloads_done(t_codexion *codex)
{
	int	tncompiles;
	int	i;

	tncompiles = 0;
	i = 0;
	while (i < codex->ncoders) {
		if (codex->coders[i]->state == ALIVE){
			tncompiles += codex->coders[i]->ncompiles;
		}
		i++;
	}
	return (tncompiles == codex->ncoders * codex->ncompiles);
}

int		coder_cycle(t_codexion *codex, int coder_id)
{
	t_coder	*coder;

	coder = codex->coders[coder_id];
	if (!request_dongles(codex, coder_id)) {
		codex_log(&(codex->output_lock), "%ld %d burned out\n", &start, &end, coder_id + 1);
		coder->state = DEAD;
		return (0);
	}
	codex_log(&(codex->output_lock), "%ld %d is compiling\n", &start, &end, coder_id + 1);
	usleep(codex->tcompile * 1000);
	release_dongles(codex, coder_id);
	coder->ncompiles++;
	codex_log(&(codex->output_lock), "%ld %d is debugging\n", &start, &end, coder_id + 1);
	usleep(codex->tdebug * 1000);
	codex_log(&(codex->output_lock), "%ld %d is refactoring\n", &start, &end,coder_id + 1);
	usleep(codex->trefactor * 1000);
	return (1);
}

int	is_burnout(t_codexion *codex)
{
	int	i;

	i = 0;
	while (i < codex->ncoders)
	{
		if (codex->coders[i]->state == DEAD)
			return (1);
		i++;
	}
	return (0);
}

void	*coder_thread(void *arg)
{
	t_workload		*workload;
	t_coder			*coder;
	struct timeval	start, end;
	int				i;

	workload = (t_workload *)arg;
	coder = workload->codex->coders[workload->coder_id];
	gettimeofday(&start, NULL);
	i = 0;
	while (i < workload->codex->ncompiles && !is_burnout(codex)) {
		if (!coder_cycle(workload->codex, workload->coder_id)) {
			return (free(workload), NULL);
		}
		i++;
	}
	return (free(workload), NULL);
}

static void	launch_coder_threads(t_codexion *codex, pthread_t *init_threads)
{
	int			i;
	t_workload	*workload;

	i = 0;
	while (i < codex->ncoders)
	{
		workload = malloc(sizeof(t_workload));
		if (!workload)
			break;
		workload->codex = codex;
		workload->coder_id = i;
		if (pthread_create(&(codex->coders[i]->thread), NULL, coder_thread, workload) == 0)
			init_threads[i] = codex->coders[i]->thread;
		else
			free(workload);
		i++;
	}
}

static void	join_coder_threads(t_codexion *codex, pthread_t *init_threads)
{
	int	i;

	i = 0;
	while (i < codex->ncoders)
	{
		if (init_threads[i])
			pthread_join(init_threads[i], NULL);
		i++;
	}
}

static void	join_dongle_threads(t_codexion *codex)
{
	int	i;

	i = 0;
	while (i < codex->ndongles)
	{
		if (codex->dongles[i]->thread)
			pthread_join(codex->dongles[i]->thread, NULL);
		codex->dongles[i]->thread = 0;
		i++;
	}
}

void	start_simulation(t_codexion *codex)
{
	pthread_t	*init_threads;

	init_threads = malloc(codex->ncoders * sizeof(pthread_t));
	if (!init_threads)
		return;
	memset(init_threads, 0, codex->ncoders * sizeof(pthread_t));
	pthread_create(&(codex->moniter), NULL, start_moniter, codex);
	launch_coder_threads(codex, init_threads);
	join_coder_threads(codex, init_threads);
	pthread_join(codex->moniter, NULL);
	join_dongle_threads(codex);
	free(init_threads);
}
