/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   edf.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: azgor <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/30 15:00:48 by azgor             #+#    #+#             */
/*   Updated: 2026/08/30 15:15:49 by azgor            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	get_edf_coder(t_codexion *codex)
{
	int		i;
	int		best;
	int		coder_id;
	t_coder	*coder;

	i = 0;
	best = -1;
	pthread_mutex_lock(&(codex->queue_lock));
	while (i < codex->qsize && codex->queue[i] >= 0)
	{
		coder_id = codex->queue[i];
		coder = codex->coders[coder_id];
		if (coder_id < codex->ncoders
			&& is_dongles_free(codex, coder_id))
		{
			if (best < 0
				|| coder->deadline < codex->coders[best]->deadline)
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

void	edf_scheduler(t_codexion *codex)
{
	int	best;

	best = get_edf_coder(codex);
	if (best >= 0 && best < codex->ncoders)
	{
		pthread_cond_signal(codex->coders[best]->cond);
	}
}
