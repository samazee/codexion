#include "codexion.h"

int is_dongles_free(t_scheduler *scheduler, int coder_id) {
	t_dongle	*dongles[2];

	dongles[0] = scheduler->dongles + coder_id;
	dongles[1] = scheduler->dongles + coder_id + 1;

	if (coder_id == scheduler->ndongles - 1)
		dongles[1] = scheduler->dongles;
	if (dongles[0]->state != FREE && dongles[1]->state != FREE)
		return (0);

	return (1);
}

int get_coder_queue_index(t_scheduler *scheduler, int coder_id)
{
	int	i;
	int qi;

	i = 0;
	qi = -1;
	while (i < scheduler->ndongles) {
		if (scheduler->queue[i] == coder_id)
		{
			qi = i;
			break;
		}
		i++;
	}
	return (qi);
}

int	is_coder_turn(t_scheduler *scheduler, int coder_id)
{
	int	i;
	int qi;

	i = 0;
	if (!is_dongles_free(scheduler, coder_id))
		return (0);
	qi = get_coder_queue_index(scheduler, coder_id);
	if (qi == 0)
		return (1);
	while (i < qi)
	{
		if (is_coder_turn(scheduler, scheduler->queue[i]))
			return (0);
		i++;
	}
	return (1);
}

void	queue_request(t_scheduler *scheduler, int coder_id)
{
	int i;

	i = 0;
	while (scheduler->queue[i] >= 0)
		i++;
	pthread_mutex_lock(&scheduler->queue_lock);
	scheduler->queue[i] = coder_id;
	scheduler->queue[i + 1] = -1;
	pthread_mutex_unlock(&scheduler->queue_lock);
}

void	pop_queue(t_scheduler *scheduler, int coder_id)
{
	int i;
	int iq;

	i = 0;
	pthread_mutex_lock(&scheduler->queue_lock);
	iq = get_coder_queue_index(scheduler, coder_id);
	while (scheduler->queue[iq + i] >= 0)
	{
		scheduler->queue[iq + i] = scheduler->queue[iq + i + 1];
		i++;
	}
	pthread_mutex_unlock(&scheduler->queue_lock);
}

void	fifo_scheduler(t_scheduler *scheduler)
{
	int	i;

	i = 0;
	while (scheduler->queue[i] >= 0)
	{
		if (is_coder_turn(scheduler, i))
		{
			pthread_cond_signal(scheduler->conds + i);
			break;
		}
		i++;
	}
}

void	edf_scheduler(t_scheduler *scheduler)
{
	int	i;

	i = 0;
	while (scheduler->queue[i] >= 0)
	{
		if (is_coder_turn(scheduler, i))
		{
			pthread_cond_signal(scheduler->conds + i);
			break;
		}
		i++;
	}
}

void	*free_dongle(void *arg)
{
	t_dongle	*dongles[2];
	t_workload	*workload;
	int			*exit_status;

	workload = (t_workload *)(arg);
	exit_status = malloc(sizeof(int));
	if (!exit_status)
		return (NULL);

	dongles[0] = workload->scheduler->dongles + workload->coder_id;
	dongles[1] = workload->scheduler->dongles + workload->coder_id + 1;

	if (workload->coder_id == workload->scheduler->ndongles - 1)
		dongles[1] = workload->scheduler->dongles;
	printf("dongle %d is cooldown\n", workload->coder_id);
	//usleep(workload->scheduler->cooldown * 1000);
	dongles[0]->state = FREE;
	dongles[1]->state = FREE;
	pthread_mutex_unlock(dongles[0]->lock);
	pthread_mutex_unlock(dongles[1]->lock);
	*exit_status = 1;
	pthread_exit(exit_status);
}

t_dongle	*request_dongles(t_scheduler *scheduler, int coder_id)
{
	t_dongle		*dongles[2];
	pthread_cond_t	*conds[2];
	int				rc;
	struct timeval	tv;
	struct timespec	ts;

	dongles[0] = scheduler->dongles + coder_id;
	conds[0] = scheduler->conds + coder_id;
	dongles[1] = scheduler->dongles + coder_id + 1;
	conds[1] = scheduler->conds + coder_id + 1;

	if (coder_id == scheduler->ndongles - 1)
	{
		dongles[1] = scheduler->dongles;
		conds[1] = scheduler->conds;
	}
	queue_request(scheduler, coder_id);
	pthread_mutex_lock(dongles[0]->lock);
	gettimeofday(&tv, NULL);
	ts.tv_sec = tv.tv_sec;
	ts.tv_nsec = tv.tv_usec * 1000 + scheduler->burnout * 1000000;
	while (!is_coder_turn(scheduler, coder_id) && rc != ETIMEDOUT)
		rc = pthread_cond_timedwait(conds[0], dongles[0]->lock, &ts);
	if (rc == ETIMEDOUT)
		return (NULL);
	pop_queue(scheduler, coder_id);
	pthread_mutex_lock(dongles[1]->lock);
	dongles[0]->state = TAKEN;
	dongles[1]->state = TAKEN;
	return (*dongles);
}

void	release_dongles(t_scheduler *scheduler, int coder_id)
{
	t_dongle		*dongles[2];
	t_workload		*workload;

	workload = malloc(sizeof(t_workload));
	if (!workload)
		return;
	workload->scheduler = scheduler;
	workload->coder_id = coder_id;
	dongles[0] = scheduler->dongles + coder_id;
	dongles[1] = scheduler->dongles + coder_id + 1;
	if (coder_id == scheduler->ndongles - 1)
		dongles[1] = scheduler->dongles;
	dongles[0]->state = COOLDOWN;
	dongles[1]->state = COOLDOWN;
	pthread_create(dongles[0]->thread, NULL, free_dongle, workload);
}

void	*start_scheduler(void *arg)
{
	t_scheduler *scheduler;

	scheduler = (t_scheduler*)(arg);
	while (1)
	{
		if (strcmp(scheduler->type, "fifo") == 0)
			fifo_scheduler(scheduler);
		if (strcmp(scheduler->type, "edf") == 0)
			edf_scheduler(scheduler);
	}
}
