/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   actions.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: azgor <azgor@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/30 15:18:28 by azgor             #+#    #+#             */
/*   Updated: 2026/09/05 13:28:29 by azgor            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

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

int	request_dongles(t_codexion *codex, int coder_id,
					struct timeval *start, struct timeval *end)
{
	t_coder			*coder;

	coder = codex->coders[coder_id];
	queue_request(codex, coder_id);
	if (!wait_for_dongles(codex, coder_id))
		return (pop_queue(codex, coder_id), 0);
	pthread_mutex_lock(&(coder->left->lock));
	pthread_mutex_lock(&(coder->right->lock));
	coder->left->state = TAKEN;
	coder->right->state = TAKEN;
	pthread_mutex_unlock(&(coder->left->lock));
	pthread_mutex_unlock(&(coder->right->lock));
	pop_queue(codex, coder_id);
	codex_log(codex, "%ld %d has taken dongle\n",
		get_elapsed_time(start, end), coder_id + 1);
	codex_log(codex, "%ld %d has taken dongle\n",
		get_elapsed_time(start, end), coder_id + 1);
	codex_log(codex, "%ld %d is compiling\n",
		get_elapsed_time(start, end), coder_id + 1);
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
		return ;
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
