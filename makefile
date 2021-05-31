# super simple makefile
# call it using 'make NAME=name_of_code_file_without_extension'
# (assumes a .cpp extension)
NAME = "a1_demo"

# Add $(MAC_OPT) to the compile line for macOS
# (should be ignored by Linux, set to nothing if causing problems)
MAC_OPT = -I/opt/X11/include

all:
	@echo "Compiling..."
	#g++ -o $(NAME) $(NAME).cpp -L/opt/X11/lib -lX11 -lstdc++ $(MAC_OPT)
	#g++-5 -std=c++14 program.cc -o program
	gcc -std=c++14 -o $(NAME) $(NAME).cpp -L/opt/X11/lib -lX11 -lstdc++ $(MAC_OPT)

run: all
	@echo "Running..."
	./$(NAME)

clean:
	-rm *o
