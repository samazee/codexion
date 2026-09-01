/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_cycle.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: azgor <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/30 15:49:50 by azgor             #+#    #+#             */
/*   Updated: 2026/09/01 19:04:30 by azgor            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long	get_elapsed_time(struct timeval *start, struct timeval *end)
{
	gettimeofday(end, NULL);
	if (end->tv_sec != start->tv_sec)
		return ((((end->tv_sec - start->tv_sec) * 1000000)
				+ end->tv_usec - start->tv_usec) / 1000);
	return ((end->tv_usec - start->tv_usec) / 1000);
}

void	codex_log(t_codexion *codex, const char *format,
			long dt, int coder_id)
{
	if (!is_burnout(codex))
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
	if (!request_dongles(codex, coder_id))
	{
		codex_log(codex, "%ld %d burned out\n",
			get_elapsed_time(start, end), coder_id + 1);
		coder->state = DEAD;
		return (0);
	}
	codex_log(codex, "%ld %d is compiling\n",
		get_elapsed_time(start, end), coder_id + 1);
	usleep(codex->tcompile * 1000);
	release_dongles(codex, coder_id);
	coder->ncompiles++;
	codex_log(codex, "%ld %d is debugging\n",
		get_elapsed_time(start, end), coder_id + 1);
	usleep(codex->tdebug * 1000);
	codex_log(codex, "%ld %d is refactoring\n",
		get_elapsed_time(start, end), coder_id + 1);
	usleep(codex->trefactor * 1000);
	return (1);
}

void	*coder_thread(void *arg)
{
	t_workload		*workload;
	struct timeval	start;
	struct timeval	end;
	int				i;

	workload = (t_workload *)arg;
	gettimeofday(&start, NULL);
	i = 0;
	while (i < workload->codex->ncompiles && !is_burnout(workload->codex))
	{
		if (!coder_cycle(workload->codex, workload->coder_id, &start, &end))
			return (free(workload), NULL);
		i++;
	}
	return (free(workload), NULL);
}
