#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <readline/readline.h>
#include <stdbool.h>
#include "hashmap.h"
#include "oom.h"
#include "die.h"

static char *program_name = NULL;
int version_major = 0;
int version_minor = 1;
int version_patch = 0;

typedef enum {
	TOKEN_TYPE_NONE = 0,
	TOKEN_TYPE_KEYWORD,
	TOKEN_TYPE_IDENTIFIER,
	TOKEN_TYPE_STRING,
	TOKEN_TYPE_CHAR,
	TOKEN_TYPE_NUMBER,
	TOKEN_TYPE_PLUS,
	TOKEN_TYPE_MINUS,
	TOKEN_TYPE_MULTIPLY,
	TOKEN_TYPE_DIVIDE,
	TOKEN_TYPE_MOD,
	TOKEN_TYPE_EQUAL,
	TOKEN_TYPE_SEMICOLON,
} token_type;

typedef struct {
	token_type type;
	void *value;
} token;

typedef struct {
	char *str;
	size_t size;
} string;

typedef struct {
	const char *name;
	enum {
		KEYWORD_NONE = 0,
		KEYWORD_INT,
		KEYWORD_TRUE,
		KEYWORD_FALSE,
	} value;
} keyword;

Hashmap *keywords = NULL;

void ctc_run(char *str, size_t len) {
	if (!keywords) {
		keyword words[] = {
			{ .name = "int"   , .value = KEYWORD_INT  },
			{ .name = "true"  , .value = KEYWORD_TRUE },
			{ .name = "false" , .value = KEYWORD_TRUE },
			{ 0 }
		};

		keywords = hashmap_create(0, 0);
		keyword *iter = words;
		while (iter->name) {
			hashmap_put(keywords, iter->name, strlen(iter->name), (void*)iter->value);
			iter++;

		}
	}
	char *end = str + len;

	size_t tok_cap = 4096;
	token *tokens = malloc(sizeof(token) * tok_cap);
	if (!tokens) oom();
	size_t tok_size = 0;

	while (str < end) {
		switch (*str) {
			case ' ':
			case '\n':
			case '\t':
				break;

			case 'a':
			case 'b':
			case 'c':
			case 'd':
			case 'e':
			case 'f':
			case 'g':
			case 'h':
			case 'i':
			case 'j':
			case 'k':
			case 'l':
			case 'm':
			case 'n':
			case 'o':
			case 'p':
			case 'q':
			case 'r':
			case 's':
			case 't':
			case 'u':
			case 'v':
			case 'w':
			case 'x':
			case 'y':
			case 'z':
			case 'A':
			case 'B':
			case 'C':
			case 'D':
			case 'E':
			case 'F':
			case 'G':
			case 'H':
			case 'I':
			case 'J':
			case 'K':
			case 'L':
			case 'M':
			case 'N':
			case 'O':
			case 'P':
			case 'Q':
			case 'R':
			case 'S':
			case 'T':
			case 'U':
			case 'W':
			case 'X':
			case 'Y':
			case 'Z':
			case '_': {
				char *iter = str;
				while (isalnum(*iter) || *iter == '_') iter++;

				char *iden = strndup(str, iter - str);
				token_type type = TOKEN_TYPE_NONE;

				token *tok = &tokens[tok_size];
				tok->type = TOKEN_TYPE_IDENTIFIER;
				tok->value = iden;
				tok_size++;
				str = iter;

				break;
			}

			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9': {
				char *endptr = NULL;
				uint64_t val = strtoll(str, &endptr, 10);
				str = endptr - 1;

				token *tok = &tokens[tok_size];
				tok->type = TOKEN_TYPE_NUMBER;
				tok->value = (void*)val;
				tok_size++;

				break;
			}

			case '"': {
				char *iter = str + 1;
				bool in_escape = false;
				bool found_end = false;

				while (*iter) {
					char c = *iter;
					iter++;

					if (c == '"' && !in_escape) {
						found_end = true;
						break;
					}

					if (in_escape) {
						in_escape = false;
						break;
					}

					if (c == '\\') in_escape = true;
				}

				if (!found_end) {
					die("Unterminated string literal!\n");
				}

				char *buf = malloc(iter - str - 1);
				if (!buf) oom();
				size_t buf_size = 0;

				char *cur = str + 1;
				char *end = iter - 1;
				in_escape = false;
				while (cur < end) {
					if (!in_escape) {
						if (*cur == '\\') {
							in_escape = true;
						}
						else {
							buf[buf_size] = *cur;
							buf_size++;
						}
					}
					else {
						switch (*cur) {
							case 'n':
								buf[buf_size] = '\n';
								break;

							case 'r':
								buf[buf_size] = '\r';
								break;

							case 't':
								buf[buf_size] = '\t';
								break;

							default:
								buf[buf_size] = '\\';
								buf[buf_size + 1] = *cur;
								buf_size++;
								break;
						}

						buf_size++;
					}

					cur++;
				}

				buf[buf_size] = '\0';

				string *s = malloc(sizeof(*str));
				if (!s) oom();
				s->str = buf;
				s->size = buf_size;

				token *tok = &tokens[tok_size];
				tok->type = TOKEN_TYPE_STRING;
				tok->value = s;
				tok_size++;

				str = iter;
				break;
			}

			case '\'': {
				char *iter = str;
				if (!iter[1] || !iter[2]) {
					die("Unterminated character sequence!\n");
				}
				else if (iter[1] == '\\') {
					if (!iter[3]) {
						die("Unterminated character sequence!\n");
					}
					else if (iter[3] != '\'') {
						die("Character sequence too long!\n");
					}
					else {
						char c;
						switch (iter[2]) {
							case 'n': c = '\n'; break;
							case 'r': c = '\r'; break;
							case 't': c = '\t'; break;
							case '\\': c = '\\'; break;
							case '\'': c = '\''; break;
							case '0': c = '\0'; break;
							default: die("Unrecognized escape sequence '\\%c'\n", iter[2]);
						}
						token *tok = &tokens[tok_size];
						tok->type = TOKEN_TYPE_CHAR;
						tok->value = (void*)(uintptr_t)c;
						tok_size++;

						str = &iter[3];
					}
				}
				else {
					if (iter[2] != '\'') {
						die("Character sequence too long!\n");
					}
					else if (iter[1] == '\'') {
						die("Empty character sequence!\n");
					}
					else {
						token *tok = &tokens[tok_size];
						tok->type = TOKEN_TYPE_CHAR;
						tok->value = (void*)(uintptr_t)iter[1];
						tok_size++;
						str = &iter[2];
					}
				}

				break;
			}

			case '+': {
				token *tok = &tokens[tok_size];
				tok->type = TOKEN_TYPE_PLUS;
				tok->value = NULL;
				tok_size++;
				break;
			}

			case '-': {
				token *tok = &tokens[tok_size];
				tok->type = TOKEN_TYPE_MINUS;
				tok->value = NULL;
				tok_size++;
				break;
			}

			case '/': {
				token *tok = &tokens[tok_size];
				tok->type = TOKEN_TYPE_DIVIDE;
				tok->value = NULL;
				tok_size++;
				break;
			}

			case '*': {
				token *tok = &tokens[tok_size];
				tok->type = TOKEN_TYPE_MULTIPLY;
				tok->value = NULL;
				tok_size++;
				break;
			}

			case '%': {
				token *tok = &tokens[tok_size];
				tok->type = TOKEN_TYPE_MOD;
				tok->value = NULL;
				tok_size++;
				break;
			}

			case '=': {
				token *tok = &tokens[tok_size];
				tok->type = TOKEN_TYPE_EQUAL;
				tok->value = NULL;
				tok_size++;
				break;
			}

			case ';':  {
				token *tok = &tokens[tok_size];
				tok->type = TOKEN_TYPE_SEMICOLON;
				tok->value = NULL;
				tok_size++;
				break;
			}

			default:
				die("Unrecognized token: '%c'\n", *str);
				break;

		}

		str++;
	}

	token *iter_end = &tokens[tok_size];
	token *iter_cur = tokens;

	while (iter_cur < iter_end) {
		token tok = *iter_cur;
		iter_cur++;
		switch (tok.type) {
			case TOKEN_TYPE_NONE:
				printf("TOKEN_TYPE_NONE\n");
				break;

			case TOKEN_TYPE_KEYWORD: {
				string *s = tok.value;
				printf("TOKEN_TYPE_KEYWORD: (%s, %lu)\n", s->str, s->size);
				break;
			}

			case TOKEN_TYPE_PLUS:
				printf("TOKEN_TYPE_PLUS: +\n");
				break;

			case TOKEN_TYPE_MINUS:
				printf("TOKEN_TYPE_MINUS: -\n");
				break;

			case TOKEN_TYPE_MULTIPLY:
				printf("TOKEN_TYPE_MULTIPLY: *\n");
				break;

			case TOKEN_TYPE_DIVIDE:
				printf("TOKEN_TYPE_DIVIDE: /\n");
				break;

			case TOKEN_TYPE_MOD:
				printf("TOKEN_TYPE_MOD: %%\n");
				break;

			case TOKEN_TYPE_EQUAL:
				printf("TOKEN_TYPE_EQUAL: =\n");
				break;

			case TOKEN_TYPE_SEMICOLON:
				printf("TOKEN_TYPE_SEMICOLON: ;\n");
				break;

			case TOKEN_TYPE_CHAR:
				printf("TOKEN_TYPE_CHAR: '%c'\n", (char)(uint64_t)tok.value);
				break;

			case TOKEN_TYPE_STRING: {
				string *str = tok.value;
				printf("TOKEN_TYPE_STRING: (\"%s\", %lu)\n", str->str, str->size);
				break;

			}

			case TOKEN_TYPE_NUMBER:
				printf("TOKEN_TYPE_NUMBER: %lu\n", (uint64_t)tok.value);
				break;

			case TOKEN_TYPE_IDENTIFIER:
				printf("TOKEN_TYPE_IDENTIFIER: %s\n", (char *)tok.value);
				break;

		}
	}
}

void show_usage(int exit_code) {
	printf("Usage: %s [filename]\n", program_name);
	exit(exit_code);
	_exit(exit_code);
}

void show_version(void) {
	printf("%s %d.%d.%d\n", program_name, version_major, version_minor, version_patch);
	exit(0);
	_exit(0);
}

int main(int argc, char **argv) {
	program_name = basename(argv[0]);

	char **files = NULL;
	size_t files_size = 0;

	if (files_size == 0) {
		while (1) {
			char *line = readline(">> ");
			if (line == NULL) break;

			ctc_run(line, strlen(line));
			free(line);
		}
	}

	return 0;
}
