#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>

#define MAX 1000

/* Prototypes */
void die(const char* e);
void gotoline(int line);
void usecard(int startline);
void movecard(int startline,int moveto);
void shuffledeck();

/* Global variables */
char delim = '\t';
char *filename;
FILE *fp;
char *actions = NULL;
unsigned int snooze = 3;
unsigned int lines = 0;
int moveto = 0;
int rateflag = 0;

/* Utility functions */
void die(const char* e) {
    fprintf(stdout,"%s\n",e);
    exit(1);
}

void gotoline(int line) {
	char c;
	int i = 1;
	
	rewind(fp);
	while ((c = fgetc(fp)) != EOF && i != line)
		if (c == '\n') i++;
	ungetc(c,fp);
}

/* Main gubbins */
int main(int argc, char *argv[]) {
	char c;
	char *d;
	int i; int j = 0;
	int startline = 0;
	int endline = 0;
	int randomflag = 0;
	int continueflag = 0;
	
	if (argv[1][1] == 'h') {
		printf("Usage: bilgi [OPTIONS] [INPUT FILE]\n\t-a ACTIONS\tlist the actions to perform on the card\n\t-b\t\tput the card to the back after acting\n\t-c\t\tcontinue to the next card\n\t-d CHAR\t\tspecify the delimiter\n\t-h\t\tshow this help\n\t-i\t\tmove card interactively after actions are complete\n\t-m NUM[%%]\tmove card to position NUM after acting\n\t-n NUM-[NUM]\tuse cards in the given range\n\t-r\t\tuse a random card\n\t-s\t\tshuffle the deck prior to acting\n\t-t NUM\t\tspecify the sleep timeout in seconds\n");
		exit(0);
	}
	
	srand((unsigned int)time((time_t *)NULL)); /* Initialise the random seed */
	filename = argv[argc-1]; /* filename has to be final argument */
	if ((fp = fopen(filename,"r")) == NULL) die("File could not be opened.");
	while ((c = fgetc(fp)) != EOF)
		if (c == '\n') lines++;  /* count the lines in the file */

	for (i=1; i<argc; i++) {
		c = *argv[i];
		if (c == '-') {
			switch (argv[i][1]) {
			case 'a':
				i++;
				for (d = argv[i]; *d; d++)
					if (isdigit(*d)) j++;
				if (j == 0) die("Invalid actions listing.");
				//	if (!isdigit(*d) && !isspace(*d) && *d != 'w' && *d != 's' && *d != '-') die("Invalid actions listing.");
				actions = malloc(sizeof(char)*strlen(argv[i]));
				strcpy(actions,argv[i]);
				break;
			case 'b':
				moveto = lines;
				break;
			case 'c':
				continueflag = 1;
				break;
			case 'd':
				if (!argv[i][2]) {
					i++; j = 1;
				} else j = 3;
				if (argv[i][j]) die("Delimiter must be a single character.");
				delim = argv[i][j-1];
				break;
			case 'i':
				rateflag = 1;
				break;
			case 'm':
				if (isdigit(argv[i][2])) d = argv[i]+2;
				else {
					i++;
					if (i == (argc-1)) die("-s requires an argument.");
					d = argv[i];
				}
				while (*d >= '0' && *d <= '9') {
					moveto = moveto*10 + *d-'0';
					d++;
				}
				if (*d == '%') {
					moveto = (lines*moveto)/100;
					if (moveto == 0) moveto = 1;
				}
				break;
			case 'n':
				if (randomflag) die("-n and -r are exclusive.");
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
			case 'r':
				if (startline) die("-n and -r are exclusive.");
				randomflag = 1;
				break;
			case 's':
				shuffledeck();
				break;
			case 't':
				if (isdigit(argv[i][2])) d = argv[i]+2;
				else {
					i++;
					if (i == (argc-1)) die("-t requires an argument.");
					d = argv[i];
				}
				snooze = 0;
				while (*d >= '0' && *d <= '9') {
					snooze = snooze*10 + *d-'0';
					d++;
				}
				if (snooze == 0) die("Invalid argument to -t.");
				break;
			default:
				break;
			}
		}
	}
	if (actions == NULL) die("Invalid command.");

	if (randomflag) startline = rand()%lines + 1;
	if (startline == 0) startline++; /* Default is first line */
	if (startline > lines) die("Start line exceeds file length.");
	
	usecard(startline);
	if (endline == 0) endline = lines;
	if (continueflag) {
		while (startline < endline) {
			if (randomflag) startline = rand()%lines + 1;
			else if (moveto == 0) startline++;
			usecard(startline);
		}
	}
	free(actions);
	fclose(fp);
	exit(0);
}

void shuffledeck() {
	int j = 0;
	for (int i = lines; i; i--) {
		j = rand()%i + 1;
		movecard(i,j);
	}
}

void usecard (int startline) {
	char *a = actions;
	int num; int num2;
	int fieldsprinted = 0;
	char word[MAX];
	char input[MAX];
	int fields = 1;
	int i = 0;
	int j = 0;
	char c;
	
	/* Let's seek to the right position in the file stream */
	gotoline(startline);
	
	while ((c = fgetc(fp)) != '\n' && c != EOF)
		if (c == delim) fields++; /* count the fields in the operative line */
	
	while (*a) { /* main loop for parsing the actions list */
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
			gotoline(startline);
				
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
		} else if (*a == 't') {
			fflush(stdout);
			sleep(snooze);
		}
		a++;
	}
	printf("\n");
	if (rateflag) {
		moveto = 0; /* interactive rating overrides manual rating */
		printf("move to: ");
		fgets(input,MAX,stdin);
		if (sscanf(input,"+%d",&moveto)) moveto = moveto + startline;
		else if (sscanf(input,"-%d",&moveto)) moveto = startline - moveto;
		else if (input[0] == 'b' && input[1] == '\n') moveto = lines; /* shortcut for send to back */
		else if (sscanf(input,"%d",&moveto));
		else printf("Invalid move position.\n");
		if (input[(strlen(input))-2] == '%') moveto = (moveto*lines)/100;
		if (moveto <= 0) moveto = 1;
	}
	if (moveto) {
		if (moveto > lines) printf("Invalid move position.\n");
		else movecard(startline,moveto);
	}
}

void movecard(int startline,int moveto) {
	FILE *tempfp = tmpfile();
	int i = 1;
	char card[MAX];
	char c;
	unsigned int flag = 0;
	
	gotoline(startline);
	fgets(card,MAX,fp);
	rewind(fp);
	while ((c = fgetc(fp)) != EOF) {
		if (i == moveto && !flag) {
			fputs(card,tempfp); /* paste the card in its new position */
			flag = 1; /* This must only happen once */
		}
		if (i == startline) {
			while ((c = fgetc(fp)) != '\n'); /* skip out the line in its original position */
			moveto++; /* preserve the semantics of moveto */
		} else fputc(c,tempfp); /* write to the temporary file */
		if (c == '\n') i++;
	}
	if (i == moveto) fputs(card,tempfp); /* paste the card if it has to go at the end */
	fclose(fp);
	fp = fopen(filename,"w+");
	rewind(tempfp);
	while ((c = fgetc(tempfp)) != EOF) fputc(c,fp); /* copy the temp file to the original file */
	fclose(tempfp); /* temp file gets automatically deleted at this point */
	fclose(fp);
	fp = fopen(filename,"r");
}
