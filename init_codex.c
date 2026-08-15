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

t_coder	*init_coder()
{
	t_coder	*coder;

	coder = malloc(sizeof(t_coder));
	if (!coder)
		return (NULL);
	coder->thread = malloc(sizeof(pthread_t));
	coder->last_compile = 0;
	coder->ncompiles = 0;
	return (coder);
}

t_dongle	*init_dongle()
{
	t_dongle			*dongle;
    pthread_mutexattr_t attr;

	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	dongle = malloc(sizeof(t_dongle));
	if (!dongle)
		return (NULL);
	dongle->state = FREE;
	dongle->lock = malloc(sizeof(pthread_mutex_t));
	dongle->thread = malloc(sizeof(pthread_t));
	pthread_mutex_init(dongle->lock, &attr);
	pthread_mutexattr_destroy(&attr);
	return (dongle);
}

t_codexion	*init_codexion(int argc, char **argv)
{
	t_codexion	*codex;
    pthread_mutexattr_t attr;
	int i;

	if (!validate_argv(argc, argv))
		return (NULL);
	codex = malloc(sizeof(t_codexion));
	codex->type = argv[7];
	codex->cooldown = atoi(argv[6]);
	codex->burnout = atoi(argv[1]);
	codex->tcompile = atoi(argv[2]);
	codex->tdebug = atoi(argv[3]);
	codex->trefactor = atoi(argv[4]);
	codex->ncompiles = atoi(argv[5]);
	codex->ncoders = atoi(argv[0]);
	codex->dongles = malloc(sizeof(t_dongle*));
	codex->coders = malloc(sizeof(t_coder*));
	codex->conds = malloc(codex->ncoders * sizeof(pthread_cond_t));
	codex->moniter = malloc(sizeof(pthread_t));
	codex->queue = malloc(sizeof(int) * codex->ncoders + 1);
	codex->ndongles = codex->ncoders;
	codex->queue_lock = malloc(sizeof(pthread_mutex_t));
	codex->output_lock = malloc(sizeof(pthread_mutex_t));
	codex->queue[0] = -1;

	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(codex->queue_lock, &attr);
	pthread_mutex_init(codex->output_lock, &attr);
	i = 0;
	while (i < codex->ncoders)
	{
		pthread_cond_init((codex->conds + i), NULL);
		codex->dongles[i] = init_dongle();
		codex->coders[i++] = init_coder();
	}
	pthread_mutexattr_destroy(&attr);
	printf("Codexion initialized\n");
	return (codex);
}
