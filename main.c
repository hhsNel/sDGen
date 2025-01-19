#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// $b
// $B (3) (sdgen main file)

/*
 $M (expectCAsNextChar) (sets local $(previously defined$) `char c;` as the next character of the second argument, compares it against the first, then exits if no match) 
*/
#define expectCAsNextChar(C, fin) \
	c = (char)fgetc(fin);\
	if(c != C) return;

/*
 $c
 $f (void) (write)
 $a (FILE*) (fout) (output file)
 $a (char*) (str) (string to be written to output)
 $m (Writes the desired string to output)
*/
void write(FILE* fout, char* str) {
	fwrite(str, sizeof(char), strlen(str), fout);
}
/*
 $c 
 $f (void) (writeTillParenthasisEnd)
 $a (FILE*) (fin) (read until closeParen character is read)
 $a (FILE*) (fout) (everything read from fin is forwarded to fout)
 $m (Copies input to output, until EOF or closeParen is encountered)
*/
void writeTillParenthasisEnd(FILE* fin, FILE* fout) {
	char c;
	while((c = (char)fgetc(fin)) != EOF && c != ')') {
		if(c == '$') {
			char escaped = (char)fgetc(fin);
			if(escaped != EOF) {
				fputc(escaped, fout);
			}
		}
		else fputc(c, fout);
	}
}
/*
 $c
 $f (void) (waitTillNoWhitespace)
 $a (FILE*) (fin) (is read until a non-whitespace character - space, horizontal tab or line feed, - is encountered)
 $m (Skips until the next meaningfull character - not a whitespace. CR not included on windows)
*/
void waitTillNoWhitespace(FILE* fin) {
	char c;
	do {
		c = (char)fgetc(fin);
	} while (c != EOF && (c == ' ' || c == '\t' || c == '\n'));
	fseek(fin, -1L, SEEK_CUR);	
}

/*
 $c
 $f (void) (handleInline)
 $a (FILE*) (fin) (file to be read from)
 $a (FILE*) (fout) (file to be written to)
 $m (Parses inline markdown. Usage: $I $(\[inline markdown\]$) -> \[inline markdown\])
*/
void handleInline(FILE* fin, FILE* fout) {
	waitTillNoWhitespace(fin);
	char c;
	expectCAsNextChar('(', fin);
	writeTillParenthasisEnd(fin, fout);
}
unsigned short int argNr;
/*
 $p
 $f (void) (handleFunc)
 $a (FILE*) (fin) (file to be read from)
 $a (FILE*) (fout) (file to be written to)
 $m (Handles documenting functions. Usage: $f $(\[return type\]$) $(\[function name\]$) -> \{reset internal argument count\} $(Function$) `[return type]` `[function name]`)
*/
void handleFunc(FILE* fin, FILE* fout) {
	argNr = 0;
	waitTillNoWhitespace(fin);
	char c;
	expectCAsNextChar('(', fin);
	write(fout, "(Function) `");
	writeTillParenthasisEnd(fin, fout);
	waitTillNoWhitespace(fin);
	expectCAsNextChar('(', fin);
	write(fout, "` `");
	writeTillParenthasisEnd(fin, fout);
	write(fout, "`  \n");
}
/*
 $p
 $f (void) (handleArg)
 $a (FILE*) (fin) (file to be read from)
 $a (FILE*) (fout) (file to be written to)
 $m (Handles arguments. Usage: $a $(\[type\]$) $(\[name\]$) $(\[description\]$) -> $(Argument \{internal argument count\}$) \n\t `[type]` `[name]` - \[description\] \{increment internal argument count\})
*/
void handleArg(FILE* fin, FILE* fout) {
	char argTxt[] = "(Argument \0\0\0\0\0\0";
	sprintf(argTxt+10, "%d", argNr);
	strcat(argTxt+11, ")\n");
	write(fout, argTxt);
	waitTillNoWhitespace(fin);
	char c;
	expectCAsNextChar('(', fin);
	write(fout, "\t\t`");
	writeTillParenthasisEnd(fin, fout);
	waitTillNoWhitespace(fin);
	expectCAsNextChar('(', fin);
	write(fout, "` `");
	writeTillParenthasisEnd(fin, fout);
	waitTillNoWhitespace(fin);
	expectCAsNextChar('(', fin);
	write(fout, "` - ");
	writeTillParenthasisEnd(fin, fout);
	write(fout, "  \n");
	++argNr;
}
/*
 $p
 $f (void) (handleMacro)
 $a (FILE*) (fin) (file to be read from)
 $a (FILE*) (fout) (file to be written to)
 $m (Handles documenting macros. Usage: $M $(\[macro name\]$) $(\[description\]$) -> $(Macro$) `[macro name]` - \[description\])
*/
void handleMacro(FILE* fin, FILE* fout) {
	waitTillNoWhitespace(fin);
	char c;
	expectCAsNextChar('(', fin);
	write(fout, "(Macro) `");
	writeTillParenthasisEnd(fin, fout);
	waitTillNoWhitespace(fin);
	expectCAsNextChar('(', fin);
	write(fout, "` - ");
	writeTillParenthasisEnd(fin, fout);
	write(fout, "  \n");	
}
/*
 $p
 $f (void) (handleBeginning)
 $a (FILE*) (fin) (file to be read from)
 $a (FILE*) (fout) (file to be written to)
 $m (Creates a header text. Usage: $b -> # Documentation  \n## Generated by sdgen\n)
*/ 
void handleBeginning(FILE* fin, FILE* fout) {
	write(fout, "# Documentation  \n## Generated by sdgen\n");
}
/*
 $p
 $f (void) (handleCustomBeginning)
 $a (FILE*) (fin) (file to be read from)
 $a (FILE*) (fout) (file to be written to)
 $m (Creates a customizable header text. Usage: $B $(\[size\]$) $(\[text\]$) -> \{'#' repeated \[size\] times\} \[text\])
*/
void handleCustomBeginning(FILE* fin, FILE* fout) {
	waitTillNoWhitespace(fin);
	char c;
	expectCAsNextChar('(', fin);
	waitTillNoWhitespace(fin);
	c = (char)fgetc(fin);
	unsigned short int count = c - '0';
	waitTillNoWhitespace(fin);
	c = (char)fgetc(fin);
	if(c != ')') return;
	unsigned short int i;
	for(i = 0; i < count; ++i) {
		fputc('#', fout);
	}
	fputc(' ', fout);
	waitTillNoWhitespace(fin);
	expectCAsNextChar('(', fin);
	writeTillParenthasisEnd(fin, fout);
	write(fout, "\n");
}
/*
 $p
 $f (void) (handleMention)
 $a (FILE*) (fin) (file to be read from)
 $a (FILE*) (fout) (file to be written to)
 $m (Creates text. Usage: $m $(\[text\]$) -> \[text\]. No functional difference from $i, used when not inserting inline markdown, but plaintext) 
*/
void handleMention(FILE* fin, FILE* fout) {
	waitTillNoWhitespace(fin);
	char c;
	expectCAsNextChar('(', fin);
	writeTillParenthasisEnd(fin, fout);
	write(fout, "\n");
}
/*
 $p
 $f (void) (handleImportant)
 $a (FILE*) (fin) (file to be read from)
 $a (FILE*) (fout) (file to be written to)
 $m (Creates bolded text. Usage: $s $(\[text\]$) -> \*\*\[text\]\*\*)
*/
void handleImportant(FILE* fin, FILE* fout) {
	waitTillNoWhitespace(fin);
	char c;
	expectCAsNextChar('(', fin);
	write(fout, "**");
	writeTillParenthasisEnd(fin, fout);
	write(fout, "**\n");
}
/*
 $p
 $f (void) (handleTable)
 $a (FILE*) (fin) (file to be read from)
 $a (FILE*) (fout) (file to be written to)
 $m (Arranges tables. Usage: $T $(\[columns\]$) -> \{Set internal column number to \[columns\]. Set internal current column to 0\})
*/
int columnNr, currentColumn;
void handleTable(FILE* fin, FILE* fout) {
	waitTillNoWhitespace(fin);
	char c;
	expectCAsNextChar('(', fin);;
	int columns = 0;
	while(isdigit(c = fgetc(fin))) {
		columns *= 10;
		columns += c-'0';	
	}
	fseek(fin, -1L, SEEK_CUR);
	waitTillNoWhitespace(fin);
	expectCAsNextChar(')', fin);
	columnNr = columns;
	currentColumn = 0;
}
/*
 $p
 $f (void) (handleTableHeader)
 $a (FILE*) (fin) (file to be read from)
 $a (FILE*) (fout) (file to be written to)
 $m (Creates a table header. Usage (After declaring a table with $T): $h $(\[header\]$) -> \{Constructs a table header\})
*/
void handleTableHeader(FILE* fin, FILE* fout) {
	waitTillNoWhitespace(fin);
	char c;
	expectCAsNextChar('(', fin);
	if(currentColumn == 0) {
		write(fout, "| ");
	}
	writeTillParenthasisEnd(fin, fout);
	write(fout, " | ");
	++currentColumn;
	if(currentColumn == columnNr) {
		write(fout, "\n| ");
		int i;
		for(i=0; i<columnNr; ++i) {
			write(fout, "--- | ");
		}
		write(fout, "\n");
		currentColumn = 0;
	}
}
/*
 $p
 $f (void) (handleTableCell)
 $a (FILE*) (fin) (file to be read from)
 $a (FILE*) (fout) (file to be written to)
 $m (Creates a table cell. Usage (After declaring a table with $T): $t $(\[text\]$) -> \{Constructs a table cell\})
*/
void handleTableCell(FILE* fin, FILE* fout) {
	waitTillNoWhitespace(fin);
	char c;
	expectCAsNextChar('(', fin);
	if(currentColumn == 0) {
		write(fout, "| ");
	}
	writeTillParenthasisEnd(fin, fout);
	write(fout, " | ");
	++currentColumn;
	if(currentColumn == columnNr) {
		write(fout, "\n");
		currentColumn = 0;
	}
}
/*
 $p
 $f (void) (handleDefine)
 $a (FILE*) (fin) (file to be read from)
 $a (FILE*) (fout) (file to be written to)
 $m (Handles documenting #defined constants. Usage: $d $(\[defined name\]$) $(\[description\]$) -> $(#defined constant$) `[defined name]` - \[description\])
*/
void handleDefine(FILE* fin, FILE* fout) {
	waitTillNoWhitespace(fin);
	char c;
	expectCAsNextChar('(', fin);
	write(fout, "(#defined constant) `");
	writeTillParenthasisEnd(fin, fout);
	waitTillNoWhitespace(fin);
	expectCAsNextChar('(', fin);
	write(fout, "` - ");
	writeTillParenthasisEnd(fin, fout);
	write(fout, "  \n");	
}

/*
 $c
 $f (void) (handleDocs)
 $a (FILE*) (fin) (passed to all invoked functions, read from)
 $a (FILE*) (fout) (passed to all invoked functions, written to)
 $s (The main function parsing files.)
 $m (Reads and parses through the input file and writes to the output file, until EOF or $q is encountered)
 $p
*/
void handleDocs(FILE* fin, FILE* fout) {
	char c;
	int ignore = 0;
	while((c = (char)fgetc(fin)) != EOF) {
		if(c == '$') {
			if(ignore) {
				ignore = 0;
				continue;
			}
			c = (char)fgetc(fin);
			switch(c) {
				case 'i':	// $m ($$i - ignore the next $ sign outside of parsed space) $p
					ignore = 1;
					break;
				case 'f':
					handleFunc(fin, fout);
					break;
				case 'a':
					handleArg(fin, fout);
					break;
				case 'm':
					handleMention(fin, fout);
					break;
				case 's':
					handleImportant(fin, fout);
					break;
				case 'M':
					handleMacro(fin, fout);
					break;
				case 'q':	// $m ($$q - quit the parser and don't parse past this point) $p
					return;
				case 'b':
					handleBeginning(fin, fout);
					break;
				case 'B':
					handleCustomBeginning(fin, fout);
					break;
				case 'I':
					handleInline(fin, fout);
					break;
				case 'c':	// $m ($$c - write a horizontal rule/horizontal rine) $p
					write(fout, "\n---\n\n");
					break;
				case 'p':	// $m ($$p - separate a paragraph) $p
					write(fout, "\n");
					break;
				case 'F':	// $m ($$F - make the last markdown end with a line feed) $p
					fseek(fout, -1L, SEEK_CUR);
					write(fout, "  \n");
					break;
				case 'T':
					handleTable(fin, fout);
					break;
				case 'h':
					handleTableHeader(fin, fout);
					break;
				case 't':
					handleTableCell(fin, fout);
					break;
				case 'd':
					handleDefine(fin, fout);
					break;
				//default:
					
			}
		}
	}
}


/*
 $c
 $s (Escape code cheatsheet)
 $p
 $T (5)
 $h (Escape code)	$h (Meaning)			$h (Argument #0)	$h (Argument #1)	$h (Argument #2)
 $t ($$I)			$t (Inline Markdown)	$t (Markdown)		$t ()				$t ()
 $t ($$f)			$t (Function)			$t (Return value)	$t (Function name)	$t ()
 $t ($$a)			$t (Argument)			$t (Data type)		$t (Name)			$t (Description)
 $t ($$M)			$t (Macro)				$t (Name)			$t (Description)	$t ()
 $t ($$b)			$t (Beginning/Header)	$t ()				$t ()				$t ()
 $t ($$B)			$t (Custom Header)		$t (Size, in #s)	$t (Text)			$t ()
 $t ($$m)			$t (Mention)			$t (Text)			$t ()				$t ()
 $t ($$s)			$t (Important)			$t (Text)			$t ()				$t ()
 $t ($$T)			$t (Table Declaration)	$t (Columns)		$t ()				$t ()
 $t ($$h)			$t (Table Header)		$t (Text)			$t ()				$t ()
 $t ($$t)			$t (Table Cell)			$t (Text)			$t ()				$t ()
 $t ($$d)			$t (A #defined constant)	$t (Name)			$t(Description)			$t ()
 $t ($$i)			$t (Ignore Flag)		$t ()				$t ()				$t ()
 $t ($$q)			$t (Quit Parsing)		$t ()				$t ()				$t ()
 $t ($$c)			$t (Horizontal Rule)	$t ()				$t ()				$t ()
 $t ($$p)			$t (Paragraph)			$t ()				$t ()				$t ()
 $t ($$F)			$t (Line Feed)			$t ()				$t ()				$t ()
 $p
*/

/*
 $c
 $f (void) (handleFile)
 $a (char*) (finName) (file name to be read from)
 $a (char*) (foutName) (file name to be written to)
 $p
*/
void handleFile(char* finName, char* foutName) {
	FILE* fin = fopen(finName, "rb");
	if(fin==NULL) {
		printf("Error opening file %s\n", finName);
		exit(1);
	}
	int outNameAllocated = 0;
	if(foutName == NULL) {
		const char* defaultName = ".readme.md";
		foutName = malloc(sizeof(char)*(strlen(defaultName)+strlen(finName)+1));
		if(foutName==NULL) {
			printf("Error allocating memory\n");
			exit(1);
		}
		strcpy(foutName, finName);
		strcpy(foutName + strlen(finName), defaultName);
		outNameAllocated = 1;
	}
	FILE* fout = fopen(foutName, "wb");
	if(fout==NULL) {
		printf("Error opening file %s\n", foutName);
		exit(1);
	}

	handleDocs(fin, fout);

	if(outNameAllocated) {
		free(foutName);
	}
	fclose(fin);
	fclose(fout);
}

int main(int argc, char** argv) {
	if(argc == 1) {
		printf("Usage:\n\
%s [[-o outputFile] inputFile]...\n\
outputFile default = (inputFile).readme.md\n", argv[0]);
		exit(0);
	}
	
	char* foutName = NULL;
	for(unsigned int arg = 1; arg < argc; ++arg) {
		if(argv[arg][0] == '-') {
			switch(argv[arg][1]) {
				case 'o':
					if(arg+1 >= argc) {
						printf("Expected another argument after -o\n");
						exit(1);
					}
					foutName = argv[++arg];
					break;
				case 'h':
					printf("\
| Escape code | Meaning             | Argument #0  | Argument #1   | Argument #2 | \n\
| ----------- | ------------------- | ------------ | ------------- | ----------- | \n\
| $I          | Inline Markdown     | Markdown     |               |             | \n\
| $f          | Function            | Return value | Function name |             | \n\
| $a          | Argument            | Data type    | Name          | Description | \n\
| $M          | Macro               | Name         | Description   |             | \n\
| $b          | Beginning/Header    |              |               |             | \n\
| $B          | Custom Header       | Size, in #s  | Text          |             | \n\
| $m          | Mention             | Text         |               |             | \n\
| $s          | Important           | Text         |               |             | \n\
| $T          | Table Declaration   | Columns      |               |             | \n\
| $h          | Table Header        | Text         |               |             | \n\
| $t          | Table Cell          | Text         |               |             | \n\
| $d          | A #defined constant | Name         | Description   |             | \n\
| $i          | Ignore Flag         |              |               |             | \n\
| $q          | Quit Parsing        |              |               |             | \n\
| $c          | Horizontal Rule     |              |               |             | \n\
| $p          | Paragraph           |              |               |             | \n\
| $F          | Line Feed           |              |               |             | \n");
					exit(0);
				default:
					printf("Unrecognized argument: %s\n", argv[arg]);
					exit(1);
			}
		} else {
			char* finName = argv[arg];
			handleFile(finName, foutName);
			foutName = NULL;
		}
	}
	return 0;
}
