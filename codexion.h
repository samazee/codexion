#ifndef CODEXION_H
# define CODEXION_H

# define COMPILE 0
# define DEBUG 1
# define REFACTOR 2
# define COOLDOWN 3
# define FREE 4
# define IDLE 5
# define BURNOUT 6

# include <stdlib.h>
# include <pthread.h>
# include <stdio.h>
# include <string.h>

typedef struct {
	int	id;
	int	state;
} t_coder;

typedef struct {
	int state;
} t_dongle;

typedef struct {
	int 		ncoders;
	int			ndongles;
	int 		tburnout;
	int 		tcompile;
	int 		tdebug;
	int 		trefactor;
	int 		ncompiles;
	int 		cooldown;
	char		*scheduler;
	t_coder		*coders;
	t_dongle	*dongles;
} t_codexion;

t_codexion *init_codexion(int argc, char **argv);

#endif
