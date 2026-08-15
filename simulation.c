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

void	*coder_workload(void *arg)
{
	t_workload		*workload;
	int				*exit_status;
	t_dongle		*dongles;
	struct timeval	start, end;
	int				i;
	
	workload = (t_workload *)arg;
	exit_status = malloc(sizeof(int));
	if (!exit_status)
		return (NULL);
	gettimeofday(&start, NULL);
	i = 0;
	while (i < workload->codex->ncompiles) {
		dongles = request_dongles(workload->codex, workload->coder_id);
		if (!dongles) {
			codex_log(workload->codex->output_lock, "%ld %d burned out\n", &start, &end, workload->coder_id + 1);
			*exit_status = 0;
			pthread_exit(exit_status);
		}
		//codex_log(workload->codex->output_lock, "%ld %d has taken a dongle\n", &start, &end, workload->coder_id + 1);
		//codex_log(workload->codex->output_lock, "%ld %d has taken a dongle\n", &start, &end, workload->coder_id + 1);
		//codex_log(workload->codex->output_lock, "%ld %d is compiling\n", &start, &end, workload->coder_id + 1);
		usleep(workload->codex->tcompile * 1000);
		release_dongles(workload->codex, workload->coder_id);
		//codex_log(workload->codex->output_lock, "%ld %d is debugging\n", &start, &end, workload->coder_id + 1);
		usleep(workload->codex->tdebug * 1000);
		//codex_log(workload->codex->output_lock, "%ld %d is refactoring\n", &start, &end, workload->coder_id + 1);
		usleep(workload->codex->trefactor * 1000);
		i++;
	}
	*exit_status = 1;
	pthread_exit(exit_status);
}

void	start_simulation(t_codexion *codex)
{
	int i;
	t_workload *workload;

	i = 0;
	
	pthread_create(codex->moniter, NULL, start_moniter, codex);
	while (i < codex->ncoders)
	{
		workload = malloc(sizeof(t_workload));
		if (!workload)
			continue;
		workload->codex = codex;
		workload->coder_id = i;
		pthread_create(codex->coders[i++]->thread, NULL, coder_workload, workload);
	}
	i = 0;
	while (i < codex->ncoders)
	{
		pthread_join(*(codex->coders[i++]->thread), NULL);
	}
}
