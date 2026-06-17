#include "codexion.h"

int is_number(char *num)
{
	size_t i;

	i = 0;
	while (i < strlen(num))
	{
		if (num[i] != ' ' && num[i] != '+' && num[i] != '-' && (num[i] < '0' || num[i] > '9'))
			return (0);
		i++;
	}
	return (1);
}


int	validate_codex(t_codexion *codex)
{
	printf("validating codex...\n");
	if (codex->tburnout < codex->tdebug + codex->trefactor)
		return (printf("Error: the sum of time_to_debug and time_to_refactor cannot be greater than time_to_burnout\n"), 0);
	if (codex->tburnout < codex->cooldown + codex->tcompile)
		return (printf("Error: the sum of dongle_cooldown and time_to_compile cannot be greater than time_to_burnout\n"), 0);
	return (1);
}

int validate_argv(int argc, char **argv)
{
	printf("validating args...\n");
	if (argc != 8)
		return (printf("Insufficient arguments\n"), 0);
	while (argc > 0)
	{
		argc--;
		if (argc!=7 && (atoi(argv[argc]) <= 0 || !is_number(argv[argc])))
			return (printf("Invalid number argument\n"), 0);
		if (argc==7 && (strcmp(argv[argc], "edf") != 0 && strcmp(argv[argc], "fifo") != 0))
			return (printf("Only strategies allowed are 'edf' or 'fifo'\n"), 0);
	}
	return (1);
}

int init_coders_dongles(t_codexion *codex)
{
	int	i;

	printf("initializing coders&dongles\n");
	codex->coders = malloc(codex->ncoders * sizeof(t_coder));
	if (!codex->coders)
		return (0);
	codex->dongles = malloc(codex->ndongles * sizeof(t_dongle));
	if (!codex->dongles)
		return (0);
	i = 0;
	while (i < codex->ncoders)
	{
		(codex->coders + i)->id = i + 1;
		(codex->coders + i)->state = IDLE;
		(codex->coders + i++)->ncompiles = 0; 
	}
	i = 0;
	while (i < codex->ndongles)
	{
		(codex->dongles + i)->id = i + 1;
		(codex->dongles + i)->state = FREE;
		pthread_mutex_init(&((codex->dongles + i++)->lock), NULL);
	}
	return (1);
}

t_codexion	*init_codexion(int argc, char **argv)
{
	t_codexion	*codex;

	if (!validate_argv(argc, argv))
		return (NULL);
	codex = malloc(sizeof(t_codexion));
	codex->ncoders = atoi(argv[0]);
	codex->ndongles = atoi(argv[0]);
	codex->tburnout = atoi(argv[1]);
	codex->tcompile = atoi(argv[2]);
	codex->tdebug = atoi(argv[3]);
	codex->trefactor = atoi(argv[4]);
	codex->ncompiles = atoi(argv[5]);
	codex->cooldown = atoi(argv[6]);
	codex->scheduler = argv[7];
	if (!validate_codex(codex) || !init_coders_dongles(codex))
		return (free(codex), NULL);
	printf("Codexion initialized\n");
	return (codex);
}
