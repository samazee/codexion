#include "codexion.h"

int is_dongles_free(t_codexion *codex, int coder_id)
{
	t_coder	*coder;
	int		result;

	coder = codex->coders[coder_id];
	result = 0;
	pthread_mutex_lock(&(coder->left->lock));
	pthread_mutex_lock(&(coder->right->lock));
	if (coder->left->state == FREE && coder->right->state == FREE)
		result = 1;
	pthread_mutex_unlock(&(coder->left->lock));
	pthread_mutex_unlock(&(coder->right->lock));
	return (result);
}

int get_coder_queue_index(t_codexion *codex, int coder_id)
{
	int	i;
	int qi;

	i = 0;
	qi = -1;
	while (i < codex->qsize) {
		if (codex->queue[i] == coder_id)
		{
			qi = i;
			break;
		}
		i++;
	}
	return (qi);
}


int	is_fifo_coder_turn(t_codexion *codex, int coder_id)
{
	if (is_dongles_free(codex, coder_id) && codex->queue[0] == coder_id)
		return (1);
	return (0);
}

int	get_edf_coder(t_codexion *codex)
{
	int	i;
	int	best;
	int	coder_id;

	i = 0;
	best = -1;
	pthread_mutex_lock(&(codex->queue_lock));
	while (i < codex->qsize && codex->queue[i] >= 0)
	{
		coder_id = codex->queue[i];
		if (coder_id >= 0 && coder_id < codex->ncoders
			&& is_dongles_free(codex, coder_id))
		{
			if (best < 0
				|| codex->coders[coder_id]->deadline < codex->coders[best]->deadline)
				best = coder_id;
		}
		i++;
	}
	pthread_mutex_unlock(&(codex->queue_lock));
	return (best);
}

int	is_edf_coder_turn(t_codexion *codex, int coder_id)
{
	if (get_edf_coder(codex) == coder_id)
		return (1);
	return (0);
}

int is_coder_turn(t_codexion *codex, int coder_id)
{
	if (strcmp(codex->type, "fifo") == 0)
		return (is_fifo_coder_turn(codex, coder_id));
	else if (strcmp(codex->type, "edf") == 0)
	 	return (is_edf_coder_turn(codex, coder_id));
	else
		return (0);
}

void	queue_request(t_codexion *codex, int coder_id)
{
	int				i;
	struct timeval	tv;

	i = 0;
	pthread_mutex_lock(&(codex->queue_lock));
	while (i + 1 < codex->qsize && codex->queue[i] >= 0)
		i++;
	gettimeofday(&tv, NULL);
	codex->coders[coder_id]->deadline = (tv.tv_sec * 1000L)
		+ (tv.tv_usec / 1000) + codex->burnout;
	if (i + 1 < codex->qsize)
	{
		codex->queue[i] = coder_id;
		codex->queue[i + 1] = -1;
	}
	pthread_mutex_unlock(&(codex->queue_lock));
}

void	pop_queue(t_codexion *codex, int coder_id)
{
	int i;
	int iq;

	i = 0;
	pthread_mutex_lock(&(codex->queue_lock));
	iq = get_coder_queue_index(codex, coder_id);
	if (iq >= 0) {
		while (iq + i + 1 < codex->qsize && codex->queue[iq + i] >= 0)
		{
			codex->queue[iq + i] = codex->queue[iq + i + 1];
			i++;
		}
	}
	pthread_mutex_unlock(&(codex->queue_lock));
}

void	fifo_scheduler(t_codexion *codex)
{
	int	i;
	int	coder_id;

	i = 0;
	while (i < codex->qsize && codex->queue[i] >= 0)
	{
		coder_id = codex->queue[i];
		if (coder_id >= 0 && coder_id < codex->ncoders
			&& is_fifo_coder_turn(codex, coder_id))
		{
			pthread_cond_signal(codex->coders[coder_id]->cond);
			break;
		}
		i++;
	}
}

void	edf_scheduler(t_codexion *codex)
{
	int	best;

	best = get_edf_coder(codex);
	if (best >= 0 && best < codex->ncoders)
	{
		pthread_cond_signal(codex->coders[best]->cond);
	}
}

void	*free_dongle(void *arg)
{
	t_workload	*workload;
	t_coder		*coder;

	workload = (t_workload *)(arg);
	coder = workload->codex->coders[workload->coder_id];
	usleep(workload->codex->cooldown * 1000);
	pthread_mutex_lock(&(coder->left->lock));
	pthread_mutex_lock(&(coder->right->lock));
	coder->left->state = FREE;
	coder->right->state = FREE;
	pthread_mutex_unlock(&(coder->left->lock));
	pthread_mutex_unlock(&(coder->right->lock));
	free(workload);
	return (NULL);
}

void	take_dongles(t_codexion *codex, int coder_id)
{
	t_coder	*coder;

	coder = codex->coders[coder_id];
	pthread_mutex_lock(&(coder->left->lock));
	pthread_mutex_lock(&(coder->right->lock));
	coder->left->state = TAKEN;
	coder->right->state = TAKEN;
	pthread_mutex_unlock(&(coder->left->lock));
	pthread_mutex_unlock(&(coder->right->lock));
}

int	wait_for_dongles(t_codexion *codex, int coder_id)
{
	int				rc;
	struct timespec	ts;
	t_coder			*coder;

	coder = codex->coders[coder_id];
	ts.tv_sec = coder->deadline / 1000;
	ts.tv_nsec = (coder->deadline % 1000) * 1000000L;
	rc = 0;
	pthread_mutex_lock(&(coder->cond_lock));
	while (!is_coder_turn(codex, coder_id) && rc != ETIMEDOUT)
		rc = pthread_cond_timedwait(coder->cond, &(coder->cond_lock), &ts);
	if (rc == ETIMEDOUT)
		return (pthread_mutex_unlock(&(coder->cond_lock)), 0);
	return (pthread_mutex_unlock(&(coder->cond_lock)), 1);
}

int	request_dongles(t_codexion *codex, int coder_id)
{
	queue_request(codex, coder_id);
	if (!wait_for_dongles(codex, coder_id))
		return (pop_queue(codex, coder_id), 0);
	take_dongles(codex, coder_id);
	pop_queue(codex, coder_id);
	return (1);
}

void	cooldown_dongles(t_codexion *codex, int coder_id)
{
	t_coder	*coder;

	coder = codex->coders[coder_id];
	pthread_mutex_lock(&(coder->left->lock));
	pthread_mutex_lock(&(coder->right->lock));
	coder->left->state = COOLDOWN;
	coder->right->state = COOLDOWN;
	pthread_mutex_unlock(&(coder->left->lock));
	pthread_mutex_unlock(&(coder->right->lock));
}


void	release_dongles(t_codexion *codex, int coder_id)
{
	t_workload	*workload;
	t_dongle	*dongle;

	workload = malloc(sizeof(t_workload));
	if (!workload)
		return;
	cooldown_dongles(codex, coder_id);
	workload->codex = codex;
	workload->coder_id = coder_id;
	dongle = codex->dongles[coder_id];
	if (dongle->thread)
		pthread_join(dongle->thread, NULL);
	if (pthread_create(&(dongle->thread), NULL, free_dongle, workload) != 0)
	{
		dongle->thread = 0;
		free(workload);
	}
}

void	*start_moniter(void *arg)
{
	t_codexion *codex;

	codex = (t_codexion*)(arg);
	while (!workloads_done(codex))
	{
		if (strcmp(codex->type, "fifo") == 0)
			fifo_scheduler(codex);
		if (strcmp(codex->type, "edf") == 0)
			edf_scheduler(codex);
		usleep(1000);
	}
	return (NULL);
}
