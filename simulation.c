#include "codexion.h"

void	*coder_workload(void *arg)
{
	t_workload *workload;

	workload = (t_workload *)arg;
	printf("coder working %p\n", workload);
	return (NULL);
}

void	start_simulation(t_codexion *codex)
{
	int i;
	t_workload *workload;

	i = 0;
	
	printf("Started simulation\n");
	while (i < codex->ncoders)
	{
		workload = malloc(sizeof(t_workload));
		if (!workload)
			continue;
		workload->codex = codex;
		workload->coder_id = i + 1;
		pthread_create(&(codex->coders + i++)->thread, NULL, coder_workload, workload);
		printf("created tread %d\n", i);
	}
	i = 0;
	while (i < codex->ncoders)
	{
		pthread_join((codex->coders + i++)->thread, NULL);
		printf("joined tread %d and finished\n", i);
	}
}
