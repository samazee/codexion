/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_resource.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: azgor <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/29 17:33:00 by azgor             #+#    #+#             */
/*   Updated: 2026/08/29 17:46:20 by azgor            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

t_dongle	*init_dongle()
{
	t_dongle			*dongle;

	dongle = malloc(sizeof(t_dongle));
	if (!dongle)
		return (NULL);
	dongle->state = FREE;
	dongle->thread = 0;
	pthread_mutex_init(&(dongle->lock), NULL);
	return (dongle);
}

t_coder	*init_coder()
{
	t_coder	*coder;

	coder = malloc(sizeof(t_coder));
	if (!coder)
		return (NULL);
	coder->last_compile = 0;
	coder->deadline = 0;
	coder->ncompiles = 0;
	coder->state = ALIVE;
	coder->thread = 0;
	coder->cond = malloc(sizeof(pthread_cond_t));
	pthread_cond_init(coder->cond, NULL);
	pthread_mutex_init(&(coder->cond_lock), NULL);
	return (coder);
}
