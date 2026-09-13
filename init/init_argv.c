/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_argv.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: azgor <azgor@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/29 17:47:19 by azgor             #+#    #+#             */
/*   Updated: 2026/09/13 20:17:49 by azgor            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	is_number(char *num)
{
	size_t	i;

	if (strlen(num) == 0)
		return (0);
	i = 0;
	while (i < strlen(num))
	{
		if (num[i] < '0' || num[i] > '9')
			return (0);
		i++;
	}
	return (1);
}

int	validate_argv(int argc, char **argv)
{
	if (argc != 8)
		return (printf("Insufficient arguments\n"), 0);
	while (argc > 0)
	{
		argc--;
		if (argc != 7 && !is_number(argv[argc]))
			return (printf("Invalid number argument\n"), 0);
		if (argc == 7 && (strcmp(argv[argc], "edf") != 0
				&& strcmp(argv[argc], "fifo") != 0))
			return (printf("Only strategies allowed are 'edf' or 'fifo'\n"), 0);
	}
	return (1);
}

void	set_codex_config(t_codexion *codex, char **argv)
{
	codex->type = argv[7];
	codex->cooldown = atoi(argv[6]);
	codex->burnout = atoi(argv[1]);
	codex->tcompile = atoi(argv[2]);
	codex->tdebug = atoi(argv[3]);
	codex->trefactor = atoi(argv[4]);
	codex->ncompiles = atoi(argv[5]);
	codex->ncoders = atoi(argv[0]);
	codex->ndongles = codex->ncoders;
	codex->qsize = codex->ncoders + 1;
	codex->moniter = 0;
	codex->burned_out = -1;
}
