#include "codexion.h"

# define LOG(codex, format, ...) \
	scheduler_log(__func__, __LINE__, codex, format, ##__VA_ARGS__);

t_dongle	**get_coder_dongles(t_codexion *codex, int coder_id)
{
	t_dongle	**dongles;

	dongles = malloc(sizeof(t_dongle*) * 2);
	if (!dongles)
		return (NULL);

	dongles[0] = codex->dongles[coder_id];
	dongles[1] = codex->dongles[coder_id + 1];
	if (coder_id == codex->ndongles - 1)
		dongles[1] = codex->dongles[0];
	return (dongles);
}

void scheduler_log(const char *func, int line, t_codexion *codex, const char *format, ...)
{
	va_list args;

	va_start(args, format);
	pthread_mutex_lock(codex->output_lock);
	printf("[%s:%d] ", func, line);
	vprintf(format, args);
	pthread_mutex_unlock(codex->output_lock);
	va_end(args);
}

void print_dongles(t_codexion *codex)
{
	int i;

	i = 0;
	pthread_mutex_lock(codex->output_lock);
	while (i < codex->ndongles) {
		printf("%s %d:%s", i == 0 ? "": ",", i, _DSTATE(codex->dongles[i]->state));
		i++;
	}
	printf("\n");
	pthread_mutex_unlock(codex->output_lock);
}

void print_queue(t_codexion *codex)
{
	int i;

	i = 0;
	pthread_mutex_lock(codex->output_lock);
	if (codex->queue[i] < 0)
		printf("<empty>");
	while (codex->queue[i] >= 0) {
		printf("%s %d", i == 0 ? "": ",", codex->queue[i] + 1);
		i++;
	}
	printf("\n");
	pthread_mutex_unlock(codex->output_lock);
}

int is_dongles_free(t_codexion *codex, int coder_id) {
	t_dongle	**dongles;

	dongles = get_coder_dongles(codex, coder_id);
	if (dongles[0]->state == FREE && dongles[1]->state == FREE)
		return (free(dongles), 1);
	return (free(dongles), 0);
}

int get_coder_queue_index(t_codexion *codex, int coder_id)
{
	int	i;
	int qi;

	i = 0;
	qi = -1;
	while (i < codex->ndongles) {
		if (codex->queue[i] == coder_id)
		{
			qi = i;
			break;
		}
		i++;
	}
	return (qi);
}


int	is_coder_turn(t_codexion *codex, int coder_id)
{
	if (!is_dongles_free(codex, coder_id))
		return (0);
	if (*(codex->queue) == coder_id)
		return (1);
	return (0);
}

void	queue_request(t_codexion *codex, int coder_id)
{
	int i;

	i = 0;
	while (codex->queue[i] >= 0)
		i++;
	pthread_mutex_lock(codex->queue_lock);
	codex->queue[i] = coder_id;
	codex->queue[i + 1] = -1;
	pthread_mutex_unlock(codex->queue_lock);
	LOG(codex, "queue:", coder_id + 1);
	print_queue(codex);
}

void	pop_queue(t_codexion *codex, int coder_id)
{
	int i;
	int iq;

	i = 0;
	pthread_mutex_lock(codex->queue_lock);
	iq = get_coder_queue_index(codex, coder_id);
	LOG(codex, "coder_index %d: %d\n", coder_id + 1, iq);
	while (codex->queue[iq + i] >= 0)
	{
		codex->queue[iq + i] = codex->queue[iq + i + 1];
		i++;
	}
	pthread_mutex_unlock(codex->queue_lock);
	LOG(codex, "pop:");
	print_queue(codex);
}

void	fifo_scheduler(t_codexion *codex)
{
	int	i;

	i = 0;
	while (codex->queue[i] >= 0)
	{
		if (is_coder_turn(codex, i))
		{
			LOG(codex, "signaling cond for %d\n", i + 1);
			pthread_cond_signal(codex->conds + i);
			break;
		}
		i++;
	}
}

void	edf_scheduler(t_codexion *codex)
{
	int	i;

	i = 0;
	while (codex->queue[i] >= 0)
	{
		if (is_coder_turn(codex, i))
		{
			pthread_cond_signal(codex->conds + i);
			break;
		}
		i++;
	}
}

void	*free_dongle(void *arg)
{
	t_dongle	**dongles;
	t_workload	*workload;
	int			*exit_status;

	workload = (t_workload *)(arg);
	exit_status = malloc(sizeof(int));
	if (!exit_status)
		return (NULL);

	dongles = get_coder_dongles( workload->codex, workload->coder_id);
	usleep(workload->codex->cooldown * 1000);
	LOG( workload->codex, "free dongles for %d\n", workload->coder_id + 1);
	pthread_mutex_lock(dongles[0]->lock);
	pthread_mutex_lock(dongles[1]->lock);
	dongles[0]->state = FREE;
	dongles[1]->state = FREE;
	pthread_mutex_unlock(dongles[0]->lock);
	pthread_mutex_unlock(dongles[1]->lock);
	LOG( workload->codex, "dongles for coder %d are free\n", workload->coder_id);
	LOG( workload->codex, "dongles at free_dongles:");
	print_dongles( workload->codex);
	free(dongles); 
	*exit_status = 1;
	pthread_exit(exit_status);
}

void	take_dongles(t_codexion *codex, int coder_id)
{
	t_dongle		**dongles;

	dongles = get_coder_dongles(codex, coder_id);
	pthread_mutex_lock(dongles[0]->lock);
	pthread_mutex_lock(dongles[1]->lock);
	dongles[0]->state = TAKEN;
	dongles[1]->state = TAKEN;
	pthread_mutex_unlock(dongles[0]->lock);
	pthread_mutex_unlock(dongles[1]->lock);
	LOG(codex, "dongles for coder %d taken\n", coder_id + 1);
	free(dongles);
}

int	wait_for_dongles(t_codexion *codex, int coder_id)
{
	t_dongle		**dongles;
	pthread_cond_t	*cond;
	int				rc;
	struct timeval	tv;
	struct timespec	ts;

	dongles = get_coder_dongles(codex, coder_id);
	cond = codex->conds + coder_id;
	gettimeofday(&tv, NULL);
	ts.tv_sec = tv.tv_sec;
	ts.tv_nsec = tv.tv_usec * 1000 + codex->burnout * 1000000;
	LOG(codex, "locking cond mutex for %d\n", coder_id + 1);
	pthread_mutex_lock(dongles[0]->lock);
	while (!is_coder_turn(codex, coder_id) && rc != ETIMEDOUT)
		rc = pthread_cond_timedwait(cond, dongles[0]->lock, &ts);
	if (rc == ETIMEDOUT)
		return (free(dongles), 0);
	return (free(dongles), 1);
}

t_dongle	*request_dongles(t_codexion *codex, int coder_id)
{
	t_dongle	**dongles;

	dongles = get_coder_dongles(codex, coder_id);
	LOG(codex, "request dongles for %d\n", coder_id + 1);
	queue_request(codex, coder_id);
	if (!wait_for_dongles(codex, coder_id))
		return (pop_queue(codex, coder_id), pthread_mutex_unlock(dongles[0]->lock), NULL);
	take_dongles(codex, coder_id);
	pop_queue(codex, coder_id);
	LOG(codex, "dongles at request_dongles:");
	print_dongles(codex);
	return (*dongles);
}

void	cooldown_dongles(t_codexion *codex, int coder_id)
{
	t_dongle	**dongles;

	dongles = get_coder_dongles(codex, coder_id);
	pthread_mutex_lock(dongles[0]->lock);
	pthread_mutex_lock(dongles[1]->lock);
	dongles[0]->state = COOLDOWN;
	dongles[1]->state = COOLDOWN;
	pthread_mutex_unlock(dongles[0]->lock);
	pthread_mutex_unlock(dongles[1]->lock);
	LOG(codex, "dongles for coder %d are cooling down\n", coder_id);
	free(dongles);
}


void	release_dongles(t_codexion *codex, int coder_id)
{
	t_workload		*workload;

	workload = malloc(sizeof(t_workload));
	if (!workload)
		return;
	LOG(codex, "release dongles for %d\n", coder_id + 1);
	cooldown_dongles(codex, coder_id);
	workload->codex = codex;
	workload->coder_id = coder_id;
	LOG(codex, "dongles at release_dongles:");
	print_dongles(codex);
	pthread_create(codex->dongles[coder_id]->thread, NULL, free_dongle, workload);
}

void	*start_moniter(void *arg)
{
	t_codexion *codex;

	codex = (t_codexion*)(arg);
	while (1)
	{
		if (strcmp(codex->type, "fifo") == 0)
			fifo_scheduler(codex);
		if (strcmp(codex->type, "edf") == 0)
			edf_scheduler(codex);
		usleep(1000);
	}
}
