NAME	=	webserv

INC_D	=	inc
OBJ_D	=	obj
SRC_D	=	src

INCS	=	\
			$(INC_D)/BlockConfigRoute.hpp \
			$(INC_D)/BlockConfigServer.hpp \
			$(INC_D)/CgiExecutor.hpp \
			$(INC_D)/CgiHandler.hpp \
			$(INC_D)/Client.hpp \
			$(INC_D)/ConfigListener.hpp \
			$(INC_D)/ConfigParser.hpp \
			$(INC_D)/ErrorPage.hpp \
			$(INC_D)/FlagHandler.hpp \
			$(INC_D)/Logger.hpp \
			$(INC_D)/main.hpp \
			$(INC_D)/Request.hpp \
			$(INC_D)/RequestBody.hpp \
			$(INC_D)/RequestCgi.hpp \
			$(INC_D)/Response.hpp \
			$(INC_D)/Server.hpp \
			$(INC_D)/Socket.hpp \
			$(INC_D)/Utils.hpp \

SRC		=	\
			$(SRC_D)/BlockConfigRoute.cpp \
			$(SRC_D)/BlockConfigServer.cpp \
			$(SRC_D)/CgiExecutor.cpp \
			$(SRC_D)/CgiHandler.cpp \
			$(SRC_D)/Client.cpp \
			$(SRC_D)/ConfigListener.cpp \
			$(SRC_D)/ConfigParser.cpp \
			$(SRC_D)/ErrorPage.cpp \
			$(SRC_D)/FlagHandler.cpp \
			$(SRC_D)/Logger.cpp \
			$(SRC_D)/main.cpp \
			$(SRC_D)/Request.cpp \
			$(SRC_D)/RequestBody.cpp \
			$(SRC_D)/RequestCgi.cpp \
			$(SRC_D)/Response.cpp \
			$(SRC_D)/Server.cpp \
			$(SRC_D)/Socket.cpp \
			$(SRC_D)/Utils.cpp \


OBJ		=	$(SRC:$(SRC_D)/%.cpp=$(OBJ_D)/%.o)
CC		=	c++
CFLAGS	=	-Wall -Werror -Wextra -std=c++98 -I$(INC_D)

all: $(NAME)

$(OBJ_D):
	@mkdir -p $(OBJ_D)

$(OBJ_D)/%.o: $(SRC_D)/%.cpp Makefile $(INCS) | $(OBJ_D)
	$(CC) $(CFLAGS) -c $< -o $@

$(NAME): $(OBJ)
	$(CC) $(OBJ) -o $(NAME)

clean:
	rm -rdf $(OBJ_D)

fclean:	clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
