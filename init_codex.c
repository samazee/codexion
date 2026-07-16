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

int validate_argv(int argc, char **argv)
{
	//printf("Number of args: %d\n", argc);
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

int init_scheduler(t_codexion *codex, char *type, int cooldown, int burnout, int tcompile, int tdebug, int trefactor, int ncompiles)
{
	int	i;

	codex->scheduler = malloc(sizeof(t_scheduler));
	codex->scheduler->dongles = malloc(codex->ncoders * sizeof(t_dongle));
	codex->scheduler->conds = malloc(codex->ncoders * sizeof(pthread_cond_t));
	codex->scheduler->thread = malloc(sizeof(pthread_t));
	codex->scheduler->queue = malloc(sizeof(int) * codex->ncoders + 1);
	codex->scheduler->ndongles = codex->ncoders;
	if (!codex->scheduler || !codex->scheduler->dongles || !codex->scheduler->conds || !codex->scheduler->thread)
		return (0);
	codex->scheduler->type = type;
	codex->scheduler->cooldown = cooldown;
	codex->scheduler->burnout = burnout;
	codex->scheduler->tcompile = tcompile;
	codex->scheduler->tdebug = tdebug;
	codex->scheduler->trefactor = trefactor;
	codex->scheduler->ncompiles = ncompiles;
	codex->scheduler->queue[0] = -1;
	pthread_mutex_init(&codex->scheduler->queue_lock, NULL);
	i = 0;
	while (i < codex->ncoders)
	{
		(codex->scheduler->dongles + i)->state = FREE;
		pthread_cond_init((codex->scheduler->conds + i), NULL);
		(codex->scheduler->dongles + i)->lock = malloc(sizeof(pthread_mutex_t));
		pthread_mutex_init((codex->scheduler->dongles + i)->lock, NULL);
		(codex->scheduler->dongles + i++)->thread = malloc(sizeof(pthread_t));
	}
	return (1);
}

int init_coders(t_codexion *codex)
{
	codex->coders = malloc(codex->ncoders * sizeof(t_coder));
	if (!codex->coders)
		return (0);
	return (1);
}

t_codexion	*init_codexion(int argc, char **argv)
{
	t_codexion	*codex;

	if (!validate_argv(argc, argv))
		return (NULL);
	codex = malloc(sizeof(t_codexion));
	codex->ncoders = atoi(argv[0]);
	if (!init_coders(codex) || !init_scheduler(codex, argv[7], atoi(argv[6]), atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5])))
		return (free(codex), NULL);
	printf("Codexion initialized\n");
	return (codex);
}
