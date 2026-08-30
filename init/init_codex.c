/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_codex.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: azgor <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/29 17:48:25 by azgor             #+#    #+#             */
/*   Updated: 2026/08/29 18:09:05 by azgor            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	alloc_codex_arrays(t_codexion *codex)
{
	int	i;

	codex->dongles = malloc(sizeof(t_dongle*) * codex->ncoders);
	codex->coders = malloc(sizeof(t_coder*) * codex->ncoders);
	codex->queue = malloc(sizeof(int) * codex->qsize);
	if (!codex->dongles || !codex->coders || !codex->queue)
		return (0);
	i = 0;
	while (i < codex->qsize)
	{
		codex->queue[i] = -1;
		i++;
	}
	return (1);
}

static void	init_codex_mutexes(t_codexion *codex)
{
	pthread_mutex_init(&(codex->queue_lock), NULL);
	pthread_mutex_init(&(codex->output_lock), NULL);
}

static void	assign_coder_dongles(t_codexion *codex, int coder_id)
{
	t_coder			*coder;

	coder = codex->coders[coder_id];
	coder->left = codex->dongles[coder_id];
	if (coder_id == codex->ndongles - 1)
		coder->right = codex->dongles[0];
	else
		coder->right = codex->dongles[coder_id + 1];
}

static void	init_codex_resources(t_codexion *codex)
{
	int	i;

	i = 0;
	while (i < codex->ncoders)
	{
		codex->dongles[i] = init_dongle();
		codex->coders[i] = init_coder();
		i++;
	}
	i = 0;
	while (i < codex->ncoders)
	{
		assign_coder_dongles(codex, i);
		i++;
	}
}

t_codexion	*init_codexion(int argc, char **argv)
{
	t_codexion			*codex;

	if (!validate_argv(argc, argv))
		return (NULL);
	codex = malloc(sizeof(t_codexion));
	if (!codex)
		return (NULL);
	set_codex_config(codex, argc, argv);
	if (!alloc_codex_arrays(codex))
	{
		free(codex);
		return (NULL);
	}
	init_codex_mutexes(codex);
	init_codex_resources(codex);
	return (codex);
}
