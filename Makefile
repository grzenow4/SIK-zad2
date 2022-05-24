main:
	g++ -std=gnu++20 -Wall -Wextra -Wconversion -Werror -O2 -o robots-client robots-client.cpp

clean:
	rm robots-client
