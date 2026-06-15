#include "codexion.h"

int is_number(char *num)
{
	size_t i;

	i = 0;
	while (i < strlen(num))
	{
		if (num[i] != ' ' && num[i] != '+' && num[i] != '-' && (num[i] < '0' || num[i] > '9'))
			return (0);
	}
	return (1);
}

int	validate_codex(t_codexion *codex)
{
	if (codex->tburnout < codex->tdebug + codex->trefactor)
		return (printf("Error: the sum of time_to_debug and time_to_refactor cannot be greater than time_to_burnout\n"), 0);
	if (codex->tburnout < codex->cooldown + codex->tcompile)
		return (printf("Error: the sum of dongle_cooldown and time_to_compile cannot be greater than time_to_burnout\n"), 0);
	return (1);
}

t_codexion	*init_codex(int argc, char **argv)
{
	t_codexion	*codex;

	if (argc != 8)
		return (printf("Insufficient arguments\n"),NULL);
	while (argc > 0)
	{
		argc--;
		if (argc!=7 && atoi(argv[argc]) == 0 && !is_number(argv[argc]))
			return (printf("Invalid number argument\n"), NULL);
		if (argc==7 && (strcmp(argv[argc], "edf") != 0 && strcmp(argv[argc], "fifo") != 0))
			return (printf("Only strategies allowed are 'edf' or 'fifo'\n"), NULL);
	}
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
	if (!validate_codex(codex))
		return (free(codex), NULL);
	return (codex);
}

int main(int argc, char **argv)
{
	t_codexion	*codex;

	codex = init_codex(--argc, ++argv);
	if (!codex)
		return (printf("Usage: ./codexion <number_of_coders> <time_to_burnout> <time_to_compile> <time_to_debug> <time_to_refactor> <number_of_compiles_required> <dongle_cooldown> <scheduler>\n"), 1);
}
