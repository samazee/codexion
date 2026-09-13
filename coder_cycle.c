/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_cycle.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: azgor <azgor@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/30 15:49:50 by azgor             #+#    #+#             */
/*   Updated: 2026/09/13 16:35:32 by azgor            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	coder_sleep(t_codexion *codex, int coder_id, long dt)
{
	long			wakeup;
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	wakeup = ((tv.tv_sec * 1000L) + (tv.tv_usec / 1000) + dt / 1000);
	if (is_burnout(codex) < 0 && wakeup < codex->coders[coder_id]->deadline)
	{
		usleep(dt);
		return (1);
	}
	return (0);
}

long	get_elapsed_time(struct timeval *start, struct timeval *end)
{
	gettimeofday(end, NULL);
	if (end->tv_sec != start->tv_sec)
		return (((end->tv_sec - start->tv_sec) * 1000L)
			+ (end->tv_usec - start->tv_usec) / 1000);
	return ((end->tv_usec - start->tv_usec) / 1000);
}

void	codex_log(t_codexion *codex, const char *format,
			long dt, int coder_id)
{
	if (is_burnout(codex) < 0
		|| (is_burnout(codex) >= 0 && strcmp(format, BURNOUT_LOG) == 0))
	{
		pthread_mutex_lock(&(codex->output_lock));
		printf(format, dt, coder_id);
		pthread_mutex_unlock(&(codex->output_lock));
	}
}

int	coder_cycle(t_codexion *codex, int coder_id,
				struct timeval *start, struct timeval *end)
{
	t_coder	*coder;

	coder = codex->coders[coder_id];
	if (!request_dongles(codex, coder_id, start, end))
		return (0);
	coder_sleep(codex, coder_id, codex->tcompile * 1000);
	release_dongles(codex, coder_id);
	coder->ncompiles++;
	codex_log(codex, DEBUG_LOG,
		get_elapsed_time(start, end), coder_id + 1);
	coder_sleep(codex, coder_id, codex->tdebug * 1000);
	codex_log(codex, REFACTOR_LOG,
		get_elapsed_time(start, end), coder_id + 1);
	coder_sleep(codex, coder_id, codex->trefactor * 1000);
	return (1);
}

void	*coder_thread(void *arg)
{
	t_workload		*workload;
	t_coder			*coder;
	struct timeval	start;
	struct timeval	end;
	int				i;

	workload = (t_workload *)arg;
	gettimeofday(&start, NULL);
	coder = workload->codex->coders[workload->coder_id];
	coder->last_compile = (start.tv_sec * 1000L) + (start.tv_usec / 1000);
	coder->deadline = coder->last_compile + workload->codex->burnout;
	coder->state = WORKING;
	i = 0;
	while (i < workload->codex->ncompiles && is_burnout(workload->codex) < 0)
	{
		if (!coder_cycle(workload->codex, workload->coder_id, &start, &end))
			break ;
		i++;
	}
	if (is_burnout(workload->codex) == workload->coder_id)
		codex_log(workload->codex, BURNOUT_LOG,
			get_elapsed_time(&start, &end), workload->coder_id + 1);
	return (free(workload), NULL);
}
