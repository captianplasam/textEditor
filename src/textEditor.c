#include "textEditor.h"

// setup
int main(int argc, char *argv[]) {
    // enabling raw mode and initialising editor
    enableRawMode();
    initEditor();
    if (argc >= 2) {
        openFile(argv[1]); // opening the file
    }
    setStatusMessage("HELP: CTRL-S to save, CTRL-F to search, CTRL-Q to quit"); // sets a status message at the bottom of the screen
    while (1) { // constantly updates the screen with user inputs
        rewriteScreen();
        processKeyPress();
    }

    return 0;
}

void initEditor() {
    // initialises the editor with all base values
    config.cursorX = 0;
    config.cursorY = 0;
    config.renderX = 0;
    config.rowOffset = 0;
    config.colOffset = 0;
    config.numrows = 0;
    config.dirty = 0;
    config.filename = NULL;
    config.currentRow = NULL;
    config.statusMsg[0] = '\0';
    config.statusMsgTime = 0;
    if (getWindowSize(&config.rows, &config.cols) == -1) {
        die("getWindowSize");
    }
    config.rows -= 2;
}

// Raw Mode Disable And Enable
void enableRawMode() {
    if (tcgetattr(STDIN_FILENO, &config.origTermios) == -1) { // checks it can get the parameter of the terminal
        die("tcgetattr");
    }
    atexit(disableRawMode); // runs disableRawMode when exiting program

    struct termios raw = config.origTermios; // setting the properties of the terminal to origTermios in config

    // sets the flages for the terminal to turn it into an editor
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag &= ~(CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) { // sets the attributes 
        die("tcsetattr");
    }
}

void disableRawMode() { // disables raw mode on exit
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &config.origTermios) == -1) {
        die("tcsetattr");
    }
}

//Keyboard Input
void processKeyPress() {
    int c = readKeyPress(), times; // reads the key press from the user
    static int quitTimes = QUIT_TIMES; // sets a quit confirm for control q click

    switch (c) { // takes in a key press from the user 
        case '\r': // detects enter key
            insertNewLine();
            break;
        case CTRL_KEY('q'): // detects control q for quiting 
            if (config.dirty && quitTimes > 0) {
                setStatusMessage("File has unsaved changes press CTRL-Q %d times to quit without saving", quitTimes);
                quitTimes--;
                return;
            }
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);
            exit(0);
            break;
        case CTRL_KEY('s'): // saves the program on control s click
            save();
            break;
        case HOME_KEY: // returns to start of file on home click
            config.cursorX = 0;
            break;
        case END_KEY: // goes to the end of the file on end press click
            if (config.cursorY < config.numrows) {
                config.cursorX = config.currentRow[config.cursorY].size;
            }
            break;
        case CTRL_KEY('f'): // brings up a search box on control f click
            find();
            break;
        case BACKSPACE:
        case CTRL_KEY('h'):
        case DEL_KEY: // deletes a character on backspace, control h or the del key
            if (c == DEL_KEY) {
                moveCursor(ARROW_RIGHT);
            }
            deleteChar();
            break;
        case PAGE_UP:
        case PAGE_DOWN: // either goes page up or page down whether page up or page down clicked respectively
            if (c == PAGE_UP) {
                config.cursorY = config.rowOffset;
            } else if (c == PAGE_DOWN) {
                config.cursorY = config.rowOffset + config.rows - 1;
                if (config.cursorY > config.numrows) {
                    config.cursorY = config.numrows;
                }
            }
            times = config.rows;
            while (times--) {
                moveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
            }
            break;
        case ARROW_UP:
        case ARROW_DOWN:
        case ARROW_LEFT:
        case ARROW_RIGHT: // setting the cursor move key presses for the arrow keys
            moveCursor(c);
            break;
        case CTRL_KEY('l'):
        case '\x1b': // detects contrl l or escape seqeunce and does nothing
            break;
        default: // defaults to insert character in file
            insertChar(c);
            break;
    }
    quitTimes = QUIT_TIMES;
}

int readKeyPress() { // reads a key from the user
    int nread;
    char c, seq[3];

    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) { // tries to read a key from the user
        if (nread == -1 && errno != EAGAIN) {
            die("read"); // dies if fails
        }
    }

    if (c == '\x1b') { // checks if the keypress is there
        if (read(STDIN_FILENO, &seq[0], 1) != 1) {
            return '\x1b';
        }
        if (read(STDIN_FILENO, &seq[1], 1 ) != 1) {
            return '\x1b';
        } 
        if (seq[0] == '[') { // reads the keypress from the seq
            if (seq[1] >= '0' && seq[1] <= '9') {
                if (read(STDIN_FILENO, &seq[2], 1) != 1) {
                    return '\x1b';
                }
                if (seq[2] == '~') {
                    switch (seq[1]) { // assigns each case to a custom enum number
                        case '1':
                            return HOME_KEY;
                        case '3':
                            return DEL_KEY;
                        case '4':
                            return END_KEY;
                        case '5':
                            return PAGE_UP;
                        case '6':
                            return PAGE_DOWN;
                        case '7':
                            return HOME_KEY;
                        case '8':
                            return END_KEY;
                    }
                }
            } else {
                switch (seq[1]) { // checks for arrow presses or home and end key press
                    case 'A':
                        return ARROW_UP;
                    case 'B':
                        return ARROW_DOWN;
                    case 'C':
                        return ARROW_RIGHT;
                    case 'D':
                        return ARROW_LEFT;
                    case 'H':
                        return HOME_KEY;
                    case 'F':
                        return END_KEY;
                }
            }
        } else if (seq[0] == 'O') { // checks home key and end key presses
            switch (seq[1]) {
                case 'H':
                    return HOME_KEY;
                case 'F':
                    return END_KEY;
            }
        }
        return '\x1b';
    } else {
        return c;
    }
}

void moveCursor(int key) { // moves the cursor in a direction
    int rowLen;
    erow *row = (config.cursorY >= config.numrows) ? NULL : &config.currentRow[config.cursorY];

    switch (key) { // checks which arrow is pressed
        case ARROW_LEFT: //moves left
            if (config.cursorX != 0) {
                config.cursorX--;
            } else if (config.cursorY > 0) {
                config.cursorY--;
                config.cursorX = config.currentRow[config.cursorY].size;
            }
            break;
        case ARROW_RIGHT: // moves right
            if (row && config.cursorX < row->size) {
                config.cursorX++;
            } else if (row && config.cursorX == row->size) {
                config.cursorY++;
                config.cursorX = 0;
            }
            break;
        case ARROW_UP: // moves right
            if (config.cursorY != 0) {
                config.cursorY--;
            }
            break;
        case ARROW_DOWN: // moves down
            if (config.cursorY < config.numrows) {
                config.cursorY++;
            }
            break;
    }
    // sets the value to correct number
    row = (config.cursorY >= config.numrows) ? NULL : &config.currentRow[config.cursorY];
    rowLen = row ? row->size : 0;
    if (config.cursorX > rowLen) {
        config.cursorX = rowLen;
    }
}

char *prompt(char *prompt, void (callback)(char *, int)) { // creates a prompt for the user
    size_t bufferSize = 128, bufferLen = 0;
    char *buffer = malloc(bufferSize);
    int c;

    buffer[0] = '\0';
    while (1) {
        setStatusMessage(prompt, buffer); // sets a status message and displays it
        rewriteScreen();
        c = readKeyPress();
        if (c == DEL_KEY || c == CTRL_KEY('h') || c == BACKSPACE) { // delete a charcater
            if (bufferLen != 0) {
                buffer[--bufferLen] = '\0';
            }
        } else if (c == '\x1b') { // grabs sequnce for key press
            setStatusMessage("");
            if (callback) {
                callback(buffer, c);
            }
            free(buffer);
            return NULL;
        } else if (c == '\r') { // detects enter key
            if (bufferLen != 0) {
                setStatusMessage("");
                if (callback) {
                    callback(buffer, c);
                }
                return buffer;
            }
        } else if (!iscntrl(c) && c < 128) { // doubles the buffer size used
            if (bufferLen == bufferSize - 1) {
                bufferSize *= 2;
                buffer = realloc(buffer, bufferSize);
            }
            buffer[bufferLen++] = c;
            buffer[bufferLen]= '\0';
        }
        if (callback) {
            callback(buffer, c);
        }
    }
}

// file i/o
void openFile(char *filename) { // opens an input file by name 
    ssize_t lineLen;
    size_t lineCap = 0;
    char *line = NULL;
    FILE *fp;

    free(config.filename);
    config.filename = strdup(filename);
    fp = fopen(filename, "r");
    if (!fp) {
        die("fopen"); // dies if it can open for some reason
    }

    while ((lineLen = getline(&line, &lineCap, fp)) != -1) { // writes the file to the editor
        while (lineLen > 0 && (line[lineLen - 1] == '\n' || line[lineLen - 1] == '\r'))
            lineLen--;
        insertRow(config.numrows, line, lineLen);
    }
    free(line);
    fclose(fp);
    config.dirty = 0;
}

char *rowsToString(int *bufferLen) { // converts an entire row in the editor to a string and returns it
    int i, totalLen = 0;
    char *buffer, *p;

    for (i = 0; i < config.numrows; i++) {
        totalLen += config.currentRow[i].size + 1;
    }
    *bufferLen = totalLen;
    buffer = malloc(totalLen);
    p = buffer;
    for (i = 0; i < config.numrows; i++) {
        memcpy(p, config.currentRow[i].str, config.currentRow[i].size);
        p += config.currentRow[i].size;
        *p = '\n';
        p++;
    }
    return buffer;
}

void save() { // saves the file with a file name if it doesn't already have one
    int len, fd;
    char *buffer;

    if (config.filename == NULL) { // sets file name
        config.filename = prompt("Save as: %s (ESC to cancel)", NULL);
        if (config.filename == NULL) {
            setStatusMessage("Save aborted");
            return;
        }
    }
    buffer = rowsToString(&len); // writes the strings to the disk
    fd = open(config.filename, O_RDWR | O_CREAT, 0644);
    if (fd != -1) {
        if (ftruncate(fd, len) != -1) {
            if (write(fd, buffer, len) == len) {
                close(fd);
                free(buffer);
                config.dirty = 0;
                setStatusMessage("%d bytes written to disk", len);
                return;
            }
        }
        close(fd);
    }
    free(buffer);
    setStatusMessage("Can't save!!! I/O error: %s", strerror(errno));
}

//Search
void find() { // find a bit of text the user enters in the file open
    int savedCursorX = config.cursorX;
    int savedCursorY = config.cursorY;
    int savedColOffset = config.colOffset;
    int savedRowOffset = config.rowOffset;
    char *query = prompt("Search: %s (ESC to cancel, ARROWS to move, ENTER to resume editing from there)", findCallback);

    if (query) {
        free(query);
    } else {
        config.cursorX = savedCursorX;
        config.cursorY = savedCursorY;
        config.colOffset = savedColOffset;
        config.rowOffset = savedRowOffset;
    }
}

void findCallback(char *query, int key) { // This helps navigate the find function through all the different results
    static int lastMatch = -1, direction = 1;
    int i, current;
    char *match;
    erow *row;

    if (key == '\r' || key == '\x1b') {
        lastMatch = -1;
        direction = 1;
        return;
    } else if (key == ARROW_RIGHT || key == ARROW_DOWN) {
        direction = 1;
    } else if (key == ARROW_LEFT || key == ARROW_UP) {
        direction = -1;
    } else {
        lastMatch = -1;
        direction = 1;
    }
    if (lastMatch == -1) {
        direction = 1;
    }
    current = lastMatch;
    for (i = 0; i < config.numrows; i++) {
        current += direction;
        if (current == -1) {
            current = config.numrows - 1;
        } else if (current == config.numrows) {
            current = 0;
        }
        row = &config.currentRow[current];
        match = strstr(row->render, query);
        if (match) {
            lastMatch = current;
            config.cursorY = current;
            config.cursorX = renderXToCursorX(row, match - row->render);
            config.rowOffset = config.numrows;
            break;
        }
    }
}

// Set The Number Of Cols And Rows In The Config Struct
int getWindowSize(int *rows, int *cols) { // gets the window size of the terminal
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) {
            return -1;
        }
        return getCursorPosition(rows, cols);
    } else {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}

// Get The Current Cursor Position
int getCursorPosition(int *rows, int *cols) {
    char buffer[32];
    unsigned int i = 0;
    
    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) { // writes the cursor
        return -1;
    }
    while (i < sizeof(buffer) - 1) {
        if (read(STDIN_FILENO, &buffer[i], 1) != 1) { //finds the cursor
            break;
        } 
        if (buffer[i] == 'R') {
            break;
        }
        i++;
    }
    buffer[i] ='\0';
    if (buffer[0] != '\x1b' || buffer[1] != '[') {
        return -1;
    }
    if (sscanf(&buffer[2], "%d;%d", rows, cols) != 2) {
        return -1;
    }
    return -1;
}

// Append Buffer
void bufAppend(struct buf *b, const char *s, int len) { // add a string to the buffer for writing to the editor
    char *new = realloc(b->str, b->len + len);

    if (new == NULL) {
        return;
    }
    memcpy(&new[b->len], s, len);
    b->str = new;
    b->len += len;
}

void bufFree(struct buf *b) { // frees the buffer
    free(b->str);
}

// Operations For The Rows
void insertRow(int at, char *str, size_t len) { // inserts a row into the editor
    if (at < 0 || at > config.numrows) { 
        return;
    }
    config.currentRow = realloc(config.currentRow, sizeof(erow) * (config.numrows + 1)); // reallocs memory for the row
    // moves the row in and moves the other rows to fit it in
    memmove(&config.currentRow[at + 1], &config.currentRow[at], sizeof(erow) * (config.numrows - at));
    config.currentRow[at].size = len;
    config.currentRow[at].str = malloc(len + 1);
    memcpy(config.currentRow[at].str, str, len);
    config.currentRow[at].str[len] = '\0';
    config.currentRow[at].renderSize = 0;
    config.currentRow[at].render = NULL;
    updateRow(&config.currentRow[at]);
    config.numrows++;
    config.dirty++;
}

void updateRow(erow *row) { // updates a row with any user inputs and key presses
    int i, ind = 0, tabs = 0;

    for (i = 0; i < row->size; i++) {
        if (row->str[i] == '\t') {
            tabs++;
        }
    }
    free(row->render);
    row->render = malloc(row->size + tabs*(TAB_SPACE - 1) + 1);
    for (i = 0; i < row->size; i++) {
        if (row->str[i] == '\t') {
            row->render[ind++] = ' ';
            while (ind % TAB_SPACE != 0) {
                row->render[ind++] = ' ';
            }
        } else {
            row->render[ind++] = row->str[i];
        }
    }
    row->render[ind] = '\0';
    row->renderSize = ind;
}

int cursorXToRenderX(erow *row, int cursorX) { // changes the cursor position to the render position 
    int i, renderX = 0;

    for (i = 0; i < cursorX; i++) {
        if (row->str[i] == '\t') {
            renderX += (TAB_SPACE - 1) - (renderX % TAB_SPACE);
        }
        renderX++;
    }
    return renderX;
}

int renderXToCursorX(erow *row, int renderX) { // changes the render position to the cursor position
    int i, cursorX = 0;

    for (i = 0; i < row->size; i++) {
        if (row->str[i] == '\t') {
            cursorX += (TAB_SPACE - 1) - (renderX % TAB_SPACE);
        }
        cursorX++;
        if (cursorX > renderX) {
            return i;
        }
    }
    return i;
}

void rowInsertChar(erow *row, int at, int c) { // insert a char into a row
    if (at < 0 || at > row->size) {
        at = row->size;
    }
    row->str = realloc(row->str, row->size + 2);
    memmove(&row->str[at + 1], &row->str[at], row->size - at + 1);
    row->size++;
    row->str[at] = c;
    updateRow(row);
    config.dirty++;
}

void rowDeleteChar(erow *row, int at) { // delete a char from a row when the user press backspace
    if (at < 0 || at >= row->size) {
        return;
    }
    memmove(&row->str[at], &row->str[at + 1], row->size - at);
    row->size--;
    updateRow(row);
    config.dirty++;
}

void freeRow(erow *row) { // free memory from an unused row
    free(row->render);
    free(row->str);
}

void deleteRow(int at) { // delete an entire row when the user deletes it
    if (at < 0 || at >= config.numrows) {
        return;
    }
    freeRow(&config.currentRow[at]);
    memmove(&config.currentRow[at], &config.currentRow[at + 1], sizeof(erow) * (config.numrows - at - 1));
    config.numrows--;
    config.dirty++;
}

void rowAppendString(erow *row, char *str, size_t len) { // adds a string onto the end of a row 
    row->str = realloc(row->str, row->size + len + 1);
    memcpy(&row->str[row->size], str, len);
    row->size += len;
    row->str[row->size] = '\0';
    updateRow(row);
    config.dirty++;
}

// Edit Methods
void insertChar(int c) { // insert a char into the editor
    if (config.cursorY == config.numrows) {
        insertRow(config.numrows, "", 0);
    }
    rowInsertChar(&config.currentRow[config.cursorY], config.cursorX, c);
    config.cursorX++;
}

void deleteChar() { // deletes a char from the editor
    erow *row;

    if (config.cursorY == config.numrows) {
        return;
    }
    if (config.cursorX == 0 && config.cursorY == 0) {
        return;
    }
    row = &config.currentRow[config.cursorY];
    if (config.cursorX > 0) {
        rowDeleteChar(row, config.cursorX - 1);
        config.cursorX--;
    } else {
        config.cursorX = config.currentRow[config.cursorY - 1].size;
        rowAppendString(&config.currentRow[config.cursorY - 1], row->str, row->size);
        deleteRow(config.cursorY);
        config.cursorY--;
    }
}

void insertNewLine() { // inserts a new line when the user presses enter
    erow *row;

    if (config.cursorX == 0) {
        insertRow(config.cursorY, "", 0);
    } else {
        row = &config.currentRow[config.cursorY];
        insertRow(config.cursorY + 1, &row->str[config.cursorX], row->size - config.cursorX);
        row = &config.currentRow[config.cursorY];
        row->size = config.cursorX;
        row->str[row->size] = '\0';
        updateRow(row);
    }
    config.cursorY++;
    config.cursorX = 0;
}

// Write The Output
void rewriteScreen() { // rewrites the screen with any updates the user does like pressing keys
    struct buf b = BUF_INIT;
    char buffer[32];

    scroll();
    bufAppend(&b, "\x1b[?25l", 6);
    bufAppend(&b, "\x1b[H", 3);
    writeRow(&b);
    writeStatus(&b);
    writeMessageBar(&b);
    snprintf(buffer, sizeof(buffer), "\x1b[%d;%dH", (config.cursorY - config.rowOffset) + 1, (config.renderX - config.colOffset) + 1);
    bufAppend(&b, buffer, strlen(buffer));
    bufAppend(&b, "\x1b[?25h", 6);
    write(STDIN_FILENO, b.str, b.len);
    bufFree(&b);
}

void writeRow(struct buf *b) { // writes rows to the buffer
    int i, welcomeLen, padding, len, fileRow;
    char welcome[80];
    
    for (i = 0; i < config.rows; i++) {
        fileRow = i + config.rowOffset;
        if (fileRow >= config.numrows) {
            if (config.numrows == 0 && i == config.rows / 3) {
                welcomeLen = snprintf(welcome, sizeof(welcome), "??? Editor - Version: %s", VERSION);
                if (welcomeLen > config.cols) {
                    welcomeLen = config.cols;
                }
                padding = (config.cols - welcomeLen)/2;
                if (padding) {
                    bufAppend(b, "~", 1);
                    padding--;
                }
                while (padding--) {
                    bufAppend(b, " ", 1);
                }
                bufAppend(b,  welcome, welcomeLen);
            } else {
                bufAppend(b, "~", 1);
            }
        } else {
            len = config.currentRow[fileRow].renderSize - config.colOffset;
            if (len < 0) {
                len = 0;
            }
            if (len > config.cols) {
                len = config.cols;
            }
            bufAppend(b, &config.currentRow[fileRow].render[config.colOffset], len);
        }
        bufAppend(b, "\x1b[K", 3);
        bufAppend(b, "\r\n", 2);
    }
}

void scroll() { // implements a scroll for longer files 
    config.renderX = 0;
    if (config.cursorY < config.numrows) {
        config.renderX = cursorXToRenderX(&config.currentRow[config.cursorY], config.cursorX);
    }
    if (config.cursorY < config.rowOffset) {
        config.rowOffset = config.cursorY;
    }
    if (config.cursorY >= config.rowOffset + config.rows) {
        config.rowOffset = config.cursorY - config.rows + 1;
    }
    if (config.renderX < config.colOffset) {
        config.colOffset = config.renderX;
    }
    if (config.renderX >= config.colOffset + config.cols) {
        config.colOffset = config.renderX - config.cols + 1;
    }
}

void writeStatus(struct buf *b) { // write a status for the file to tell the user what is happening and what needs to happen
    char status[80], renderStatus[80];
    int len = snprintf(status, sizeof(status), "%.20s - %d lines %s", 
        config.filename ? config.filename : "No File", config.numrows,
        config.dirty ? "(modified)" : "");
    int renderLen = snprintf(renderStatus, sizeof(renderStatus), "%d/%d", config.cursorY + 1, config.numrows);

    if (len > config.cols) {
        len = config.cols;
    }
    bufAppend(b, status, len);
    while (len < config.cols) {
        if (config.cols - len == renderLen) {
            bufAppend(b, renderStatus, renderLen);
            break;
        } else {
            bufAppend(b, " ", 1);
            len++;
        }
    }
    bufAppend(b, "\x1b[m", 3);
    bufAppend(b, "\r\n", 2);
}

void setStatusMessage(const char *str, ...) { // sets a status message for the user to read
    va_list args;

    va_start(args, str);
    vsnprintf(config.statusMsg, sizeof(config.statusMsg), str, args);
    va_end(args);
    config.statusMsgTime = time(NULL);
}

void writeMessageBar(struct buf *b) { // writes to the message bar for the user
    int msgLen;

    bufAppend(b, "\x1b[K", 3);
    msgLen = strlen(config.statusMsg);
    if (msgLen > config.cols) {
        msgLen = config.cols;
    }
    if (msgLen && time(NULL) - config.statusMsgTime < 5) {
        bufAppend(b, config.statusMsg, msgLen);
    }
}

// Print Out Error Message And Exits Program
void die(const char *str) {
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    perror(str);
    exit(1);
}