textEditor: textEditor.c
	$(CC) textEditor.c -o textEditor -Wall -Wextra -pedantic -std=c99 && \
	$(CC) textEditor.c -o ~/bin/robedit -Wall -Wextra -pedantic -std=c99 && \
	chmod +x ~/bin/robedit