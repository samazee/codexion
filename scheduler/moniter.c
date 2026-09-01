/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   moniter.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: azgor <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/30 14:54:59 by azgor             #+#    #+#             */
/*   Updated: 2026/09/01 18:56:45 by azgor            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	is_burnout(t_codexion *codex)
{
	int	i;

	i = 0;
	while (i < codex->ncoders)
	{
		if (codex->coders[i]->state == DEAD)
			return (1);
		i++;
	}
	return (0);
}

int	workloads_done(t_codexion *codex)
{
	int	tncompiles;
	int	i;

	tncompiles = 0;
	i = 0;
	while (i < codex->ncoders)
	{
		if (codex->coders[i]->state == ALIVE)
		{
			tncompiles += codex->coders[i]->ncompiles;
		}
		i++;
	}
	return (tncompiles == codex->ncoders * codex->ncompiles);
}

int	is_coder_turn(t_codexion *codex, int coder_id)
{
	if (strcmp(codex->type, "fifo") == 0)
		return (is_fifo_coder_turn(codex, coder_id));
	else if (strcmp(codex->type, "edf") == 0)
		return (is_edf_coder_turn(codex, coder_id));
	else
		return (0);
}

void	*start_moniter(void *arg)
{
	t_codexion	*codex;

	codex = (t_codexion *)(arg);
	while (!workloads_done(codex) && !is_burnout(codex))
	{
		if (strcmp(codex->type, "fifo") == 0)
			fifo_scheduler(codex);
		if (strcmp(codex->type, "edf") == 0)
			edf_scheduler(codex);
		usleep(1000);
	}
	return (NULL);
}
