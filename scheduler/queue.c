/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   queue.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: azgor <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/30 14:51:45 by azgor             #+#    #+#             */
/*   Updated: 2026/08/30 15:17:45 by azgor            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	get_coder_queue_index(t_codexion *codex, int coder_id)
{
	int	i;
	int	qi;

	i = 0;
	qi = -1;
	while (i < codex->qsize)
	{
		if (codex->queue[i] == coder_id)
		{
			qi = i;
			break ;
		}
		i++;
	}
	return (qi);
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
	int	i;
	int	iq;

	i = 0;
	pthread_mutex_lock(&(codex->queue_lock));
	iq = get_coder_queue_index(codex, coder_id);
	if (iq >= 0)
	{
		while (iq + i + 1 < codex->qsize && codex->queue[iq + i] >= 0)
		{
			codex->queue[iq + i] = codex->queue[iq + i + 1];
			i++;
		}
	}
	pthread_mutex_unlock(&(codex->queue_lock));
}
