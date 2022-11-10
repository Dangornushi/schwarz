#include <iostream>

int sub() {
	return 0;
}

int main() {
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 32; j++) {
			printf("\033[48;2;%d;%d;255m(%d:%d)", i<<4, j<<3, i, j);
		}
		printf("\033[0m\n");
	}
	while (1);
	return 0;
}
