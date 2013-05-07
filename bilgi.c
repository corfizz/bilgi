#define _XOPEN_SOURCE

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
void yankarg(char **store,char *arg);
void usecard(int startline);
void movecard(int startline,int moveto);
void shuffledeck();
int randommatching(char *search);
int nextmatching(int i,char *search);

/* Global variables */
char delim = '\t';
char *filename;
FILE *fp;
char *actions = NULL;
unsigned int snooze = 3;
unsigned int lines = 0;
int moveto = 0;

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

void yankarg(char **store,char *arg) {
	*store = malloc((strlen(arg)+1)*sizeof(char));
	if (*store == NULL) {
		printf("Error with argument %s.\n",arg);
		exit(1);
	}
	strcpy(*store,arg);
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
	int rateflag = 0;
	unsigned int endspecified = 0;
	unsigned int nolog = 0;
	char *search = NULL;
	char input[MAX];
	char *store = NULL;
	
	char fieldget[MAX];
	FILE *procbilgi = NULL;
	
	if (argv[1][1] == 'h') {
		printf("Usage: bilgi [OPTIONS] [INPUT FILE]\n\t-a ACTIONS\tlist the actions to perform on the card\n\t-b\t\tput the card to the back after acting\n\t-c\t\tcontinue to the next card\n\t-d CHAR\t\tspecify the delimiter\n\t-h\t\tshow this help\n\t-i\t\tmove card interactively after actions are complete\n\t-l\t\tdisable logging\n\t-m NUM[%%]\tmove card to position NUM after acting\n\t-n NUM-[NUM]\tuse cards in the given range\n\t-p\t\tuse the last card accessed, via the log file\n\t-r\t\tuse a random card\n\t-s\t\tshuffle the deck prior to acting\n\t-t NUM\t\tspecify the sleep timeout in seconds\n\t-x REGEXP\tonly operate on lines matching REGEXP\n");
		exit(0);
	}
	
	srand((unsigned int)time((time_t *)NULL)); /* Initialise the random seed */
	
	for (i=1; i<argc; i++) {
		if (argv[i][0] == '-' && argv[i][1] == 'p') {
			if ((procbilgi = popen("bilgi -l -n-1 -a \"2\" bilgi.log","r")) == NULL) die("Error calling bilgi.");
			filename = malloc(MAX*sizeof(char));
			fgets(filename,MAX,procbilgi);
			pclose(procbilgi);
			filename[(strlen(filename)-1)] = '\0'; /* remove newline character */
			if ((procbilgi = popen("bilgi -l -n-1 -a \"4\" bilgi.log","r")) == NULL) die("Error calling bilgi.");
			fgets(fieldget,MAX,procbilgi);
			pclose(procbilgi);
			startline = atoi(fieldget);
		}
	}
	if (filename == NULL) filename = argv[argc-1]; /* filename has to be final argument */
	
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
				actions = malloc(sizeof(char)*(strlen(argv[i])+1));
				if (actions == NULL) die("Invalid actions listing.");
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
			case 'l':
				nolog = 1;
				break;
			case 'm':
				if (isdigit(argv[i][2])) d = argv[i]+2;
				else {
					i++;
					if (i == (argc-1)) die("-m requires an argument.");
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
				if (isdigit(argv[i][2]) || argv[i][2] == '-') d = argv[i]+2;
				else {
					i++;
					if (i == (argc-1)) die("-n requires an argument.");
					d = argv[i];
				}
				
				yankarg(&store,d);
				if (*d == '-') {
					sscanf((store+1),"%d",&startline);
					startline = lines-(startline-1);
				} else if (sscanf(store,"%d-%d",&startline,&endline) == 2);
				else sscanf(store,"%d",&startline);
				if (startline <= 0) die("Invalid argument to -n.");
				free(store);
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
			case 'x':
				i++;
				yankarg(&search,argv[i]);
				break;
			default:
				break;
			}
		}
	}

	if (startline == 0) startline = 1; /* Default is first line */
	if (startline > lines) die("Start line exceeds file length.");
	if (endline) endspecified = 1;
	else endline = lines;
	if (!continueflag) endline = startline;
	
	char thetime[40];
	FILE *log = fopen("bilgi.log","a");
	if (!nolog && log == NULL) die("Cannot open log file for writing.");
	
	while (startline <= endline) {
		if (randomflag) {
			if (search) startline = randommatching(search);
			else startline = rand()%lines + 1;
		} else if (search) startline = nextmatching(startline,search);
		
		if (!startline || (endspecified && startline > endline)) die("No other matching line.");
		
		if (actions) usecard(startline);
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
			if (moveto > lines) {
				moveto = startline;
				printf("Invalid move position.\n");
			} else movecard(startline,moveto);
		}
		if (!nolog) {
			time_t tp = time(NULL);
			struct tm tim = *localtime(&tp);
			strftime(thetime,40,"%Y-%m-%d %H:%M:%S",&tim);
			fprintf(log,"%s\t%s\t%d\t%d\n",thetime,filename,startline,(moveto ? moveto : startline));
		}
		if (!moveto || !continueflag) startline++;
	}

	free(actions);
	free(search);
	fclose(fp);
	fclose(log);
	exit(0);
}

void shuffledeck() {
	int j = 0;
	for (int i = lines; i; i--) {
		j = rand()%i + 1;
		movecard(i,j);
	}
}

int randommatching(char *search) {
	int startline = rand()%lines + 1;
	startline = nextmatching(startline,search);
	if (startline == 0) {
		rewind(fp);
		startline = nextmatching(startline,search);
	}
	return startline;
}

int nextmatching(int i,char *search) {
	char *searchcmd = NULL;
	FILE *grepstream;
	char card[MAX];
	
	if (i == 0) i = 1;
	gotoline(i);
	while (i <= lines) {
		fgets(card,MAX,fp);
		searchcmd = malloc((strlen(search)+20)*sizeof(char));
		strcpy(searchcmd,"grep -Eq \"");
		strcat(searchcmd,search);
		strcat(searchcmd,"\"");
		grepstream = popen(searchcmd,"w");
		if (grepstream == NULL) die("Error calling grep.");
		fputs(card,grepstream);
		if (pclose(grepstream) == 0) return i;
		i++;
	}
	return 0;
}

void usecard (int startline) {
	char *a = actions;
	int num; int num2;
	int fieldsprinted = 0;
	char word[MAX];
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
		} else if (*a == '\0') break;
		a++;
	}
	printf("\n");
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
