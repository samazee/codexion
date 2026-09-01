/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   destroy_codex.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: azgor <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/29 17:49:04 by azgor             #+#    #+#             */
/*   Updated: 2026/08/30 13:54:33 by azgor            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	destroy_coders(t_codexion *codex)
{
	int	i;

	i = 0;
	while (i < codex->ncoders)
	{
		if (codex->coders[i])
		{
			pthread_mutex_destroy(&(codex->coders[i]->cond_lock));
			if (codex->coders[i]->cond)
			{
				pthread_cond_destroy(codex->coders[i]->cond);
				free(codex->coders[i]->cond);
			}
			free(codex->coders[i]);
		}
		i++;
	}
}

static void	destroy_dongles(t_codexion *codex)
{
	int	i;

	i = 0;
	while (i < codex->ncoders)
	{
		if (codex->dongles[i])
		{
			pthread_mutex_destroy(&(codex->dongles[i]->lock));
			free(codex->dongles[i]);
		}
		i++;
	}
}

void	destroy_codexion(t_codexion *codex)
{
	if (!codex)
		return ;
	destroy_coders(codex);
	destroy_dongles(codex);
	pthread_mutex_destroy(&(codex->queue_lock));
	pthread_mutex_destroy(&(codex->output_lock));
	free(codex->queue);
	free(codex->coders);
	free(codex->dongles);
	free(codex);
}
