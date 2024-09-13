NAME      = nginx++
CC        = c++
FLAGS     = -Wall -Wextra -Werror
RM        = rm -fr
OBJDIR    = .obj
FILES     = Configuration/HTTP Configuration/LocationConf Configuration/MainConf Configuration/ServerConf Core/Client Core/CoreServer Core/Server Http/CGI Http/DefaultPages Http/DeleteRequest Http/GetRequest Http/MethodsUtils Http/PostRequest Http/ProcessRequest Http/PutRequest Http/Response Http/ResponseUtils main Parser/Lexer Parser/Parser Parser/Token utils/utils
SRC       = $(FILES:=.cpp)
OBJ       = $(addprefix $(OBJDIR)/, $(FILES:=.o))
INCLUEDES = Configuration/HTTP.hpp Configuration/LocationConf.hpp Configuration/MainConf.hpp Configuration/ServerConf.hpp Core/Client.hpp Core/CoreServer.hpp Core/Selector.hpp Core/Server.hpp Http/DefaultPages.hpp Http/DeleteRequest.hpp Http/GetRequest.hpp Http/IRequest.hpp Http/PostRequest.hpp Http/ProcessRequest.hpp Http/PutRequest.hpp Http/Response.hpp includes/Constants.hpp Parser/Lexer.hpp Parser/Parser.hpp Parser/Token.hpp utils/utils.hpp

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(OBJ) -o $(NAME)

$(OBJDIR)/%.o: %.cpp $(INCLUEDES)
	mkdir -p $(dir $@)
	$(CC) $(FLAGS) -c $< -o $@

up:
	@if [ -z "`docker ps -q -f name=nginxpp_container`" ]; then \
        echo "\033[0;32mInitiating startup process for Nginx++ ...\033[0m"; \
        docker build -t nginxpp:1.0.0 . | true && echo "\033[0;34mNginx++ image built successfully.\033[0m"; \
        docker run -d --name nginxpp_container -p 8080:8080 -p 3333:3333 -p 5500:5500 nginxpp:1.0.0 | true && echo "\033[0;34mNginx++ container started successfully.\033[0m"; \
        echo "\033[0;32mStartup process for Nginx++ completed.\033[0m"; \
        echo "\033[0;32mNginx++ is listening on 127.0.0.1:8080, 127.0.0.1:3333 and 127.0.0.1:5500.\033[0m"; \
		echo "\033[0;37mFor custom configurations, modify the file at './Configuration/servIO.conf'. If ports are changed, remember to update the Dockerfile and Makefile accordingly, then restart the server.\033[0m"; \
    else \
        echo "\033[0;33mNginx++ is already running. No action required.\033[0m"; \
    fi

down:
	@if [ -n "`docker ps -q -f name=nginxpp_container`" ]; then \
        echo "\033[0;31mInitiating shutdown process for Nginx++ ...\033[0m"; \
        docker stop nginxpp_container | true && echo "\033[0;34mNginx++ container stopped successfully.\033[0m"; \
        docker rm nginxpp_container | true && echo "\033[0;34mNginx++ container removed successfully.\033[0m"; \
        docker rmi nginxpp:1.0.0 | true && echo "\033[0;34mNginx++ image removed successfully.\033[0m"; \
        echo "\033[0;31mShutdown process for Nginx++ completed.\033[0m"; \
    else \
        echo "\033[0;33mNginx++ is not running. No action required.\033[0m"; \
    fi

dc_re: down up

clean:
	$(RM) $(OBJDIR) $(OBJ)

fclean: clean
	$(RM)  $(NAME)

re: fclean all

.PHONY: all clean fclean re up down dc_re
