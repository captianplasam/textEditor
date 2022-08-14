# Simple Text Editor
A Simple VIM like textEditor created for practice and personal use

| Contents     |
|--------------|
|[Description](#description)|
|[How to use](#how-to-use)|
|[Custom Structs, Defines, and Enums](#custom-defines-enums-and-structs)|
|[Functions](#functions)|

## Description

This is a custom text editor that uses the terminal on linux to make the editor is inspired by programs like VIM. This is a very basic version and only includes the most basic of features currently.

## How to use

It will put the program into a usr/bin folder on Linux so if there isn't one on your machine either change the Makefile to create it somewhere else or create the file, recommended to do the later. The run the word make in the terminal to create the program.  
When wanting to run the program, use the command word robedit (filename), the filename will open the editor with that file or filename set, if left blank then it will create a blank editor.

## Custom Defines, Enums, and Structs

### Defines

- **CTRL_KEY** - This is to see if the user is holding the control key when clicking another button.
- **BUF_INIT** - This is to initalise the buffer.
- **VERSION** - This is the current version number of the program.
- **TAB_SPACE** - This is a configurable define that states how many spaces a tab click should create.
- **QUIT_TIMES** - States how many extra times the user has to click quit for the program to quit with unsaved chnages.

### Enums

- **keys** - This assigns a special number value to each special key that can be pressed to make it easier for the program to deal with and make it much more human readable.

### Structs

- **erow** - This holds the current row the cursor is on and what it has to render out to the editor, this has two strings one being the full line and the other being only what is rendered out to the editor currently and holds the size of both strings.
- **editorConfig** - This holds the cursor position (cursorX and cursorY), the render position (renderX), the offset for the row and coloumns to display correctly (rowOffset and colOffset), the amount of row and coloumns (rows and cols), the number of the current row (numrows), how many changes have been made since last save (dirty), the name of the file currently open (filename), the status message to display to the user (statusMsg), the status message time for how long it should be displayed (statusMsgTime), the current row of an erow struct (currentRow), The original terminal and parameters for it (origTermios).
- **buf** - this holds the length of the string in the buffer (len), and the string in the buffer (str).

## Functions

- **enableRawMode** - This enables raw mode which is needed for turning the terminal into a an editor it will disable terminal commands and everything to turn it into a blank canvas.
- **disableRawMode** - This will disable raw mode when exiting the program to return the terminal back to it's original condition.
- **initEditor** - This is used to initialise the editor and set all the values to defualt
- **processKeyPress** - This will process a key press that has been read from the user and will do the necessary action required with the key press.
- **readKeyPress** - This will read a key press from the user and make it ready to be processed by a different function. It returns an int which will detail which button was pressed
- **moveCursor** - This will move the cursor in a given direction which is given by the arguments, then will the call other functions to update everything.
- **prompt** - This is the overall function to tie in all the status message functions and it is what is called when a status needs to be displayed. This will take in a string and a function called findCallback. This returns a string and will return a buffer that will continue the answer from the user from the prompt.
- **openFile** - This will take in a file name as a string and attempt to open a file of the same name if no file exist it will create a temporary file and open it ready to be edited and saved.
- **rowsToString** - This will convert all rows in the editor into a string ready to be saved by the save function. This will take in the length of the buffer as an argument
- **save** - This will save the current file and if it does not have a file name it will prompt the user for one.
- **find** - This is a function which will prompt the user to enter a search phrase and it will search the document for all instances of said phrase and display them all.
- **findCallback** - This is used for the find function that helps navigate through all the results from the search from the user. This takes in the query from the user and the key pressed by the user and will do what is needed with the key press.
- **getWindowSize** - This will get the window size of the current terminal the user is running the program from and set the rows and cols correctly to fit it.
- **getCursorPosition** - This will get the cursorX and cursorY position of the cursor. This will return -1 on an error. It takes in rows and column number to know how far to search through.
- **bufAppend** - This will append a string onto the end of the buf struct, it takes in the buffer to append to, the string to append, and the length of the string that it is appending.
- **bufFree** - This will free the entire buffer for when the program is closing.
- **insertRow** - This will insert a row into a certian point, it will take in a point where to insert it, the string of the row to insert and the length of the row.
- **updateRow** - This will update the row if a character is changed or any part of the row is changed, and update it to the editor. It takes in all rows in the ediitor to edit the row correctly.
- **cursorXToRenderX** - This will convert the cursorX position to the renderX position which will turn where the cursor actually is to where it should be rendered on screen and. It takes in the row and the cursorX to convert it.
- **renderXToCursorX** - This will turn the renderX which is x position of where the cursor is actually rendered and turn it into the cursorX value. This takes in the row and the renderX value.
- **rowInsertChar** - This will insert a character into a row whether that be the start, middle, or end. it will take in the row, where to insert it and an int of the character to insert.
- **rowDeleteChar** - This will delete a character from a row when the user presses on of the many delete keys and changes the row around it. This takes in the row and where to delete the character.
- **freeRow** - This will free all memory associated with a row when the user deletes it from the editor meaning the row will no longer exist and it will adjust th other rows. This will take in the row to free
- **deleteRow** - This will delete a row from existence and help adjust the other rows, it will call freeRow to help do it. This takes in the position of the row that needs to be deleted.
- **rowAppendString** - This will append a string to the end of the row for use of copying and pasting purposes, this will take in the row, the string to append, and the length of the string.
- **insertChar** - This will insert a character into the editor when the user presses a key that can be inserted. This will take in an int of the character that needs to be inserted.
- **deleteChar** - This will delete a character from the editor when the user presses a delete key. This will take in the position for the character to be deleted.
- **insertNewLine** - This will insert a new line into the document and create a brand new line that either contains any text after the cursor when enter is pressed or it will be completely empty.
- **rewriteScreen** - This will rewrite the entire screen when updates happen so that any updates to the editor actually show on the screen.
- **writeRow** - This will write a row from the buffer onto the screen and is used in rewriteScreen, it will also be used to create the effects of each line and write the lines of a new document. This will take in a buf struct.
- **scroll** - This will change whats on screen when the user is scrolling through the file, this includes all directions for long files and long lines.
- **writeStatus** - This will write a status message to the buffer struct for when the user wants to close the program and will write a correct messaged based on everythin that needs to be done. It takes in the buf struct.
- **setStatusMessage** - This will set the status message inside the config struct and sets the time in the config. This will take in a string and any argument after that will be used to help write the statuse message.
- **writeMessageBar** - This will write the status message to the message bar at the bottom of the screen and display it for a set amount of time. This will take in a struct buf which will have the status written to it.
- **die** - This will be used to error out of the program if an error occurs with one of the functions. This will take in a string for an error message.