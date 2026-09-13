/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   moniter.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: azgor <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/30 14:54:59 by azgor             #+#    #+#             */
/*   Updated: 2026/09/13 20:16:53 by azgor            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	is_burnout(t_codexion *codex)
{
	int				i;
	struct timeval	tv;
	long			now;

	i = 0;
	if (codex->burned_out >= 0)
		return ;
	gettimeofday(&tv, NULL);
	now = (tv.tv_sec * 1000L) + (tv.tv_usec / 1000);
	while (i < codex->ncoders)
	{
		if (codex->coders[i]->state == BURNED_OUT)
			codex->burned_out = i;
		if (codex->coders[i]->deadline <= now
			&& codex->coders[i]->state == WORKING)
		{
			codex->coders[i]->state = BURNED_OUT;
			codex->burned_out = i;
		}
		i++;
	}
}

int	workloads_done(t_codexion *codex)
{
	int	tncompiles;
	int	i;

	tncompiles = 0;
	i = 0;
	while (i < codex->ncoders)
	{
		if (codex->coders[i]->state == WORKING)
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
	while (!workloads_done(codex) && codex->burned_out < 0)
	{
		if (strcmp(codex->type, "fifo") == 0)
			fifo_scheduler(codex);
		if (strcmp(codex->type, "edf") == 0)
			edf_scheduler(codex);
		usleep(1000);
		is_burnout(codex);
	}
	return (NULL);
}
