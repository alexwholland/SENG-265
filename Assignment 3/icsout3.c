/*
Alex Holland - V00928553
Seng 265 - Assignment 3
*/

/*
Provides access to nonstandard GNU/Linux extension functions.
Allows for the use of strdup() and getline() without warning.
*/
#define _GNU_SOURCE

#include <time.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "emalloc.h"
#include "ics.h"
#include "listy.h"


/*
~Function Prototypes~
Used inorder to put functions below main. Only functions
called in main are required here.
*/
void convert();
node_t *read_file(const char *, node_t *, const int, const int);
void print_date(node_t *);
void free_list(node_t *);
void print_details(node_t *, void *);


int main(int argc, char *argv[])
{
    int from_y = 0, from_m = 0, from_d = 0;
    int to_y = 0, to_m = 0, to_d = 0;
    char *filename = NULL;
    int cmd_start, cmd_end;
    node_t *head = NULL;

    for (int i = 0; i < argc; i++) {
        if (strncmp(argv[i], "--start=", 7) == 0) {
            sscanf(argv[i], "--start=%d/%d/%d", &from_y, &from_m, &from_d);
        } else if (strncmp(argv[i], "--end=", 5) == 0) {
            sscanf(argv[i], "--end=%d/%d/%d", &to_y, &to_m, &to_d);
        } else if (strncmp(argv[i], "--file=", 7) == 0) {
            filename = argv[i]+7;
        }
    }

    if (from_y == 0 || to_y == 0 || filename == NULL) {
        fprintf(stderr,
            "usage: %s --start=yyyy/mm/dd --end=yyyy/mm/dd --file=icsfile\n",
            argv[0]);
        exit(1);
    }

    convert(from_y, from_m, from_d, to_y, to_m, to_d, &cmd_start, &cmd_end);
    head = read_file(filename, head, cmd_start, cmd_end);
    print_date(head);
    apply(head, print_details, NULL);
    free_list(head);
    
    exit(0);
}


/*
Purpose:    Given a date-time, creates a more readable version of the 
            calendar date by using some C-library routines. For example,
            if the string in  "dt_time" corresponds to:

            20190520T111500
            
            then the string stores at "formatted _time" is:

            May 20, 2019 (Mon)

Source:     timeplay.c (from A1).
*/
void dt_format(char *formatted_time, const char *dt_time, const int len)
{
    struct tm temp_time;
    time_t    full_time;

    memset(&temp_time, 0, sizeof(struct tm));
    sscanf(dt_time, "%4d%2d%2d", &temp_time.tm_year, &temp_time.tm_mon, &temp_time.tm_mday);
    temp_time.tm_year -= 1900;
    temp_time.tm_mon -= 1;
    full_time = mktime(&temp_time);
    strftime(formatted_time, len, "%B %d, %Y (%a)",
    localtime(&full_time));
}


/*
Purpose:    Given a date-time, it adds the number of days in a way that
            results in the correct year, month, and day. For example, 
            if the string in "before" corresponds to:

            20190520T111500

            then the datetime string stored in "after", assuming that 
            "num_days" is 100, will be:

            20190828T111500

            which is 100 days after May 20, 2019 (i.e., Augus 28, 2019).

Source:     timeplay.c (from A1).
*/
void dt_increment(char *after, const char *before, int const num_days)
{
    struct tm temp_time;
    time_t    full_time;

    memset(&temp_time, 0, sizeof(struct tm));
    sscanf(before, "%4d%2d%2d", &temp_time.tm_year, &temp_time.tm_mon, &temp_time.tm_mday);
    temp_time.tm_year -= 1900;
    temp_time.tm_mon -= 1;
    temp_time.tm_mday += num_days;

    full_time = mktime(&temp_time);
    after[0] = '\0';
    strftime(after, 9, "%Y%m%d", localtime(&full_time));
    strncpy(after + 8, before + 8, 7);
    after[15] = '\0';
}


/*
Purpose:    Converts from 24hr to 12hr time and then prints
            the result. If the input is:

            1830

            then the ouput will be:

            6:30 PM

Parameters: const int time - the unformatted time.           
*/
void print_time(const int time) {
    int h = time / 100; 
    int m = time % 100;
    printf("%2d:%02d %s", (h > 12) ? (h - 12) : (h == 0) ? 12 : h, 
                        m, h >= 12 ? "PM" : "AM");
}


/*
Purpose:    Prints the date and dashes '-' to stdout in
            the following format:

            <month text> <day>, <year> (<day of week>)
            ------------------------------------------

Parameters: node_t* n - node containing datetime information.
*/
void print_date(node_t* n)
{
    char* time = NULL;
    time = (char *)emalloc(sizeof(char) * 25);
        dt_format(time, n->val->dtstart, 25);
        printf("%s\n", time);
        for (int i = 0; i < strlen(time); i++) {
            printf("-%s", (i == strlen(time) - 1) ? "\n":"");
        }
    free(time); 
}


/*
Purpose:    Responsible for printing out the formatted time,
            summary, and location (stuff below the dashes). 

Parameters: node_t *n - the node containing the event.  
            void *arg - function argument (not used).
*/
void print_details(node_t *n, void *arg) {
    int start_time, end_time;

    /*Read and store the 24hr time.*/
    sscanf(n->val->dtstart + 9, "%4d", &start_time);
    sscanf(n->val->dtend + 9, "%4d", &end_time);

    print_time(start_time);
    printf(" to ");
    print_time(end_time);
    printf(": %s {{%s}}\n", n->val->summary, n->val->location);

    /*If the next node is not Null and the 1st and 2nd nodes start dates are not the same.*/
    if (n->next != NULL && strncmp(n->val->dtstart, n->next->val->dtstart, 8) != 0) {
        printf("\n");
        print_date(n->next);
    } 
}


/*
Purpose:    Create an event by intilizing the contents of
            the event_t stucture in ics.h.

Parameters: event_t *event - the event to be created.
            char *dtstart, *dtend, *summary, *location,
            *rrule - the contents of the event.    
*/
void create_event(event_t *event, char *dtstart, char *dtend, 
        char *summary, char *location, char *rrule) {
    strncpy(event->dtstart, dtstart, strlen(dtstart));
    strncpy(event->dtend, dtend, strlen(dtend));
    strncpy(event->summary, summary, strlen(summary));
    strncpy(event->location, location, strlen(location));
    strncpy(event->rrule, rrule, strlen(rrule));
}


/*
Purpose:    Gets a new line and then removes the new 
            line character '\n' from the end of the 
            line. Replace with a null character '\0'.

Parameters: char* line - address of the first character position.
            size_t len - specifies the size in bytes of the block
                         of memory pointed to by line.
            FILE *cal  - the stream from which to read the line.
*/
void remove_line(char* line, size_t len, FILE *infile) 
{
    getline(&line, &len, infile);
    if (line[strlen(line) - 1] == '\n') {
        line[strlen(line) - 1] = '\0';
    }
}


/*
Purpose:    Determines if an event should be created. If so the function
            creates a new event.

Parameters: const int cmd_start - start date from command line.
            const int cmd_end   - end date from command line.
            char *dtstart       - char array that holds the start datetime.
            char *dtend         - char array that holds the end datetime.
            char *summary       - char array that hold the summary info.
            char *location      - char array that holds the location info.
            char *rrule         - char array that holds the repeat until info.
            node_t *head        - the head of the linked list.

Returns:    head - the new head of the linked list.
*/
void *determine_event(const int cmd_start, const int cmd_end, char *dtstart,
         char *dtend, char *summary, char *location, char *rrule, node_t *head)
{
    event_t *curr_ev = NULL;
    node_t *curr_node = NULL;
    if ((atoi(dtstart) >= cmd_start) && (atoi(dtstart) <= cmd_end)) {
        curr_ev = (event_t *)emalloc(sizeof(event_t));
        create_event(curr_ev, dtstart, dtend, summary, location, rrule);
        curr_node = new_node(curr_ev);
        head = sorted_insert(head, curr_node);
    }
    return head;                    
}


/*
Purpose:    The function opens and reads the file line by line and stores the 
            contents of the ics file into a linked list. The function also
            determines if there are any repeating events, and if so how many
            times to repeat the event.

Parameters: const char *filename - the file to be read.
            const node_t *head   - the head of the linked list.
            const int cmd_start  - the start date from the command line.
            const int cmd_end    - the end date from the command line.

Returns:    The new head of the linked list after insertion.

Note:       The use of while loops around each line of important information
            is used to avoid any misplaced lines in the ics files.
*/
node_t *read_file(const char *filename, node_t *head, const int cmd_start, const int cmd_end)
{
    char *start, *end;  //Stores the start/end date and time.
    char *rrule;        //Stores the repeating events date and time.
    char *summary;      //Stores the summary information. 
    char *location;     //Stores the location information.         
    char *line = NULL;  //Stores the address of the first character positon.
    size_t len = 0;     //Specifies the size in bytes of a block of memory.
    char *after = (char *)emalloc(sizeof(char) * 16); //Stores the date after incrementing.
   
    /*Open and read the file in read mode only.*/
    FILE* infile = fopen(filename, "r");
    if (infile == NULL) {
        exit(1);
    }
    /*Read Through each line of the file and stop at the end.*/
    while (getline(&line, &len, infile) >= 0) {
        /*Determine where to start gathering information of the event.*/
        if (strncmp(line, "BEGIN:VEVENT", 12) == 0) {
            /*Find and store the start and end datetime information.*/
            while (strncmp(line, "DTSTART:", 8) != 0) { 
                remove_line(line, len, infile); 
            }
            start = strdup(line + 8);
            
            while (strncmp(line, "DTEND:", 6) != 0) {
                remove_line(line, len, infile);
            }   
            end = strdup(line + 6);
            remove_line(line, len, infile);
            /*Determine if the event is repeating, and if so
              where to read the datetime information.*/
            if (strncmp(line, "RRULE:FREQ=WEEKLY;UNTIL", 23) == 0) {
                line[39] = '\0';
                rrule = strdup(line + 24);
                remove_line(line, len, infile);
            } else if (strncmp(line, "RRULE:", 6) == 0) {
                line[47] = '\0';
                rrule = strdup(line + 32);
                remove_line(line, len, infile);
            }
            else {
                rrule = strdup("");
            }
            /*Find and store the location and summary information.*/
            while (strncmp(line, "LOCATION:", 9) != 0) {
                 remove_line(line, len, infile);
            }
            location = strdup(line + 9);

            while (strncmp(line, "SUMMARY:", 8) != 0) {
                remove_line(line, len, infile);
            }
            summary = strdup(line + 8);

            head = determine_event(cmd_start, cmd_end, start, end, summary, location, rrule, head);
            /*Deteremine how many times to repeat the event*/          
            int count = 0;
            while (strcmp(start, rrule) <= 0 || (strcmp(rrule, "") != 0 && count == 0)) {
                if (count > 0){
                head = determine_event(cmd_start, cmd_end, start, end, summary, location, rrule, head); 
                }
                /*increment and store the date by 7 days (1 week)*/
                dt_increment(after, start, 7);
                strncpy(start, after, strlen(after));
                count++;
            } 
            free(start), free(end), free(location), free(summary), free(rrule);
        }
    }
    
    free(line);
    fclose(infile);
    free(after);

    return head;
}


/*
Purpose:    Given the start and end date from the command line the 
            function converts the date into a start date integer and 
            a end date integer that represents the same date
            information format as in the ics files.
            If from_y = 2021, from_m = 2, and from_d = 14 then:

            2021 * 10000 + 2 * 100 + 14 = 20210214

Parameters: int *start - stores the calculated start date.
            int *end   - stores the calculated end date.
            other      - command line arguments take from main.

*/
void convert(int from_y, int from_m, int from_d, int to_y, int to_m, int to_d, int *start, int *end) 
{
    *start = from_y * 10000 + from_m * 100 + from_d;
    *end = to_y * 10000 + to_m * 100 + to_d;
}


/*
Purpose:    Recursively free the memory in the linked list.

Parameters: node_t *head - the head of the list.
*/
void free_list(node_t *head) {
    if (head->next) {
        free_list(head->next);
    }
    free(head->val);
    free(head);
}  

