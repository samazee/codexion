/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulation.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: azgor <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/30 16:12:56 by azgor             #+#    #+#             */
/*   Updated: 2026/08/30 16:12:57 by azgor            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	launch_coder_threads(t_codexion *codex, pthread_t *init_threads)
{
	int			i;
	t_workload	*workload;

	i = 0;
	while (i < codex->ncoders)
	{
		workload = malloc(sizeof(t_workload));
		if (!workload)
			break ;
		workload->codex = codex;
		workload->coder_id = i;
		if (pthread_create(&(codex->coders[i]->thread),
				NULL, coder_thread, workload) == 0)
			init_threads[i] = codex->coders[i]->thread;
		else
			free(workload);
		i++;
	}
}

static void	join_coder_threads(t_codexion *codex, pthread_t *init_threads)
{
	int	i;

	i = 0;
	while (i < codex->ncoders)
	{
		if (init_threads[i])
			pthread_join(init_threads[i], NULL);
		i++;
	}
}

static void	join_dongle_threads(t_codexion *codex)
{
	int	i;

	i = 0;
	while (i < codex->ndongles)
	{
		if (codex->dongles[i]->thread)
			pthread_join(codex->dongles[i]->thread, NULL);
		codex->dongles[i]->thread = 0;
		i++;
	}
}

void	start_simulation(t_codexion *codex)
{
	pthread_t	*init_threads;

	init_threads = malloc(codex->ncoders * sizeof(pthread_t));
	if (!init_threads)
		return ;
	memset(init_threads, 0, codex->ncoders * sizeof(pthread_t));
	pthread_create(&(codex->moniter), NULL, start_moniter, codex);
	launch_coder_threads(codex, init_threads);
	join_coder_threads(codex, init_threads);
	pthread_join(codex->moniter, NULL);
	join_dongle_threads(codex);
	free(init_threads);
}
