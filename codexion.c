#include "codexion.h"

int main(int argc, char **argv)
{
	t_codexion	*codex;

	codex = init_codexion(--argc, ++argv);
	if (!codex)
		return (printf("Usage: ./codexion <number_of_coders> <time_to_burnout> <time_to_compile> <time_to_debug> <time_to_refactor> <number_of_compiles_required> <dongle_cooldown> <scheduler>\n"), 1);
	start_simulation(codex);
}
