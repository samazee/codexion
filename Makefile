NAME = codexion
SRCS = $(shell find . -name '*.c')
OBJS = $(SRCS:.c=.o)
CFLAGS = -Wall -Wextra -Werror -I. -pthread

all: $(NAME)

%.o: %.c 
	@cc $(CFLAGS) -c $< -o $@

$(NAME): $(OBJS)
	@cc $(CFLAGS) $(OBJS) -o $(NAME)

clean: 
	@rm -rf $(OBJS)

fclean: clean
	@rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re
