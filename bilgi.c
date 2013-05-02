#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX 1000

void die(const char* e);
void usecard(int startline);

/* Global variables */
char delim = '\t';
FILE *fp;
int continueflag = 0;
char *actions = NULL;

void die(const char* e) {
    fprintf(stdout,"%s\n",e);
    exit(1);
}

int main(int argc, char *argv[]) {
	char c;
	char *d;
	int i;
	int startline = 0;
	int endline = 0;

	for (i=1; i<argc; i++) {
		c = *argv[i];
		if (i == (argc-1)) {
			if ((fp = fopen(argv[argc-1],"r")) == NULL) die("File could not be opened.");
		} else if (c == '-') {
			switch (argv[i][1]) {
			case 'a':
				i++;
				for (d = argv[i]; *d; d++)
					if (!isdigit(*d) && !isspace(*d) && *d != 'w' && *d != '-') die("Invalid actions listing.");
				actions = malloc(sizeof(char)*strlen(argv[i]));
				strcpy(actions,argv[i]);
				break;
			case 'c':
				continueflag = 1;
				break;
			case 'n':
				if (isdigit(argv[i][2])) d = argv[i]+2;
				else {
					i++;
					if (i == (argc-1)) die("-n requires an argument.");
					d = argv[i];
				}
				while (*d >= '0' && *d <= '9') {
					startline = startline*10 + *d-'0';
					d++;
				}
				if (startline == 0) die("Invalid argument to -n.");
				if (*d == '-') {
					d++;
					while (*d >= '0' && *d <= '9') {
						endline = endline*10 + *d-'0';
						d++;
					}
				}
				break;
			default:
				break;
			}
		}
	}
	if (actions == NULL) die("Invalid command.");
	if (startline == 0) startline++; /* Default is first line */
	int lines = 0;
	while ((c = fgetc(fp)) != EOF)
		if (c == '\n') lines++;  /* count the lines in the file */
	if (startline > lines) die("Start line exceeds file length.");
	
	usecard(startline);
	if (endline == 0) endline = lines;
	if (continueflag) {
		while (startline < endline) {
			startline++;
			usecard(startline);
		}
	}
	free(actions);
	exit(0);
}

void usecard (int startline) {
	char *a = actions;
	int num; int num2;
	int fieldsprinted = 0;
	int seekchars = 0;
	char word[MAX];
	int fields = 1;
	int i = 1;
	int j = 0;
	char c;
	
	rewind(fp);
	while ((c = fgetc(fp)) != EOF) {
		if (c == '\n') i++;
		seekchars++;
		if (i == startline) break;
	}
	
	while ((c = fgetc(fp)) != '\n' && c != EOF)
		if (c == delim) fields++; /* count the fields in the operative line */
	
	while (*a != '\0') {
		if (isdigit(*a)) {
			num = 0; num2 = 0;
			/* get the starting field */
			while (*a >= '0' && *a <= '9') {
				num = num*10 + *a-'0';
				a++;
			}
			/* get the end field, if any */
			if (*a == '-') {
				a++;
				while (*a >= '0' && *a <= '9') {
					num2 = num2*10 + *a-'0';
					a++;
				}
				if (num2 == 0) num2 = fields; /* a trailing - means we go to the end of the line */
			}
		//	printf("%d %d\n",num,num2);
			fseek(fp, seekchars, SEEK_SET);
			for (i=1; i<=fields; i++) {
				for (j=0;j < MAX;j++) /* (re-)initialise the string */
					word[j] = '\0';
				j = 0;
				while ((c = fgetc(fp)) != '\n' && c != delim && c != EOF) {
					word[j] = c;
					j++;
				}
				if (i == num || (num2 && i > num && i <= num2)) {
					if (fieldsprinted) printf("%c",delim);
					fieldsprinted++;
					printf("%s",word);
				}
			}
		}
		if (*a == 'w') {
			while (getchar() != '\n'); /* Wait for ENTER to be pressed */
			fieldsprinted = 0; /* this is only for 'formatting' purposes */
		}
		a++;
	}
	printf("\n");
}
