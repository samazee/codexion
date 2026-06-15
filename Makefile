NAME = codexion
SRCS = $(shell find . -name '*.c')
OBJS = $(SRCS:.c=.o)
CFLAGS = -Wall -Wextra -Werror -I.

ifeq (run,$(firstword $(MAKECMDGOALS)))
  RUN_ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
  $(eval $(RUN_ARGS):;@:)
endif

all: $(NAME)

%.o: %.c 
	@cc $(CFLAGS) -c $< -o $@

$(NAME): $(OBJS)
	@cc $(CFLAGS) $(OBJS) -o $(NAME)

clean: 
	@rm -rf $(OBJS)

fclean: clean
	@rm -rf $(NAME)

run: fclean all
	./$(NAME) $(RUN_ARGS)

re: fclean all

.PHONY: all clean fclean re run
