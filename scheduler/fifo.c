/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fifo.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: azgor <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/30 15:06:02 by azgor             #+#    #+#             */
/*   Updated: 2026/08/30 15:08:03 by azgor            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	is_dongles_free(t_codexion *codex, int coder_id)
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

int	is_fifo_coder_turn(t_codexion *codex, int coder_id)
{
	if (is_dongles_free(codex, coder_id) && codex->queue[0] == coder_id)
		return (1);
	return (0);
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
			break ;
		}
		i++;
	}
}
