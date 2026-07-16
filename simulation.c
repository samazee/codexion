#include "codexion.h"

long	get_elapsed_time(struct timeval start, struct timeval end) {
	if (end.tv_sec != start.tv_sec)
		return ((((end.tv_sec - start.tv_sec) * 1000000) + end.tv_usec - start.tv_usec) / 1000);
	return ((end.tv_usec - start.tv_usec) / 1000);
}

void ts_printf(const char *format, struct timeval *start, struct timeval *end, int coder_id)
{
	gettimeofday(end, NULL);
	printf(format, get_elapsed_time(*start, *end), coder_id);
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
	while (i < workload->scheduler->ncompiles) {
		dongles = request_dongles(workload->scheduler, workload->coder_id);
		if (!dongles) {
			ts_printf("%ld %d has burned out\n", &start, &end, workload->coder_id + 1);
			*exit_status = 0;
			pthread_exit(exit_status);
		}
		ts_printf("%ld %d has taken a dongle\n", &start, &end, workload->coder_id + 1);
		ts_printf("%ld %d has taken a dongle\n", &start, &end, workload->coder_id + 1);
		ts_printf("%ld %d is compiling\n", &start, &end, workload->coder_id + 1);
		usleep(workload->scheduler->tcompile * 1000);
		release_dongles(workload->scheduler, workload->coder_id);
		ts_printf("%ld %d is debugging\n", &start, &end, workload->coder_id + 1);
		usleep(workload->scheduler->tdebug * 1000);
		ts_printf("%ld %d is refactoring\n", &start, &end, workload->coder_id + 1);
		usleep(workload->scheduler->trefactor * 1000);
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
	
	pthread_create(codex->scheduler->thread, NULL, start_scheduler, codex->scheduler);
	while (i < codex->ncoders)
	{
		workload = malloc(sizeof(t_workload));
		if (!workload)
			continue;
		workload->scheduler = codex->scheduler;
		workload->coder_id = i;
		pthread_create(codex->coders + i++, NULL, coder_workload, workload);
	}
	i = 0;
	while (i < codex->ncoders)
	{
		pthread_join(codex->coders[i++], NULL);
	}
}
