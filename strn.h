#ifndef _STRN_H
#define _STRN_H

inline int natoi(char* str, size_t len) {
	int final = 0;
	int i = 0;
	// ignore leading zeroes
	while(i < len && str[i] == '0') i++;
	for(;i < len; i++) {
		final *= 10;
		final += str[i] - '0';
	}
	return final;
}

#endif
