textEditor: src/textEditor.c
	$(CC) src/textEditor.c -o textEditor -Wall -Wextra -pedantic -std=c99 && \
	$(CC) src/textEditor.c -o ~/bin/robedit -Wall -Wextra -pedantic -std=c99 && \
	chmod +x ~/bin/robedit