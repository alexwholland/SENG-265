/*
Alex Holland - V00928553
Seng 265 - Assignment 1
icsout.c
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_LINE_LEN 132
#define MAX_EVENTS 500

/*
typdef structure used to group together data in a event data type.
*/

typedef struct event {
    char dt_start[MAX_LINE_LEN];    //DTSTART
    char dt_end[MAX_LINE_LEN];      //DTEND
    char dt_repeat[MAX_LINE_LEN];   //RRULE
    char location[MAX_LINE_LEN];    //LOCATION
    char summary[MAX_LINE_LEN];     //SUMMARY
    char *ampm_start;               //start time using 'AM' or 'PM'
    char *ampm_end;                 //end time using 'AM' or 'PM'
} event;

/*
~Global variable~
Used to store the events from the read to the print_events function.
*/
event arr_events[MAX_EVENTS];

/*
~Function prototypes~
Used inorder to put functions below main. Functions not called in main
are not required in the function prototype but are here for completeness.
*/

void dt_format(char *formatted_time, const char *dt_time, const int len);
void dt_increment(char *after, const char *before, int const num_days);
int compare_event(const void *a, const void *b);
int read(const char *filename, int date_from, int date_to, int count);
void print_events(int count);
void convert(int from_y, int from_m, int from_d, int to_y, int to_m, int to_d, int *start, int *end); 

 
int main(int argc, char *argv[])
{
    int from_y = 0, from_m = 0, from_d = 0;
    int to_y = 0, to_m = 0, to_d = 0;
    char *filename = NULL;
    int i; 

    for (i = 0; i < argc; i++) {
        if (strncmp(argv[i], "--start=", 8) == 0) {
            sscanf(argv[i], "--start=%d/%d/%d", &from_y, &from_m, &from_d);
        } else if (strncmp(argv[i], "--end=", 6) == 0) {
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

    /*Variable count used to keep track of the number of events*/
    int count = 0;
    /*Variable starting_date and end_date used to hold the command line argument dates*/
    int starting_date, end_date;
    convert(from_y, from_m, from_d, to_y, to_m, to_d, &starting_date, &end_date);
    count = read(filename, starting_date, end_date, count);   
    print_events(count);
    exit(0);
}


/*
 * Function dt_format.
 *
 * Given a date-time, creates a more readable version of the
 * calendar date by using some C-library routines. For example,
 * if the string in "dt_time" corresponds to:
 *
 *   20190520T111500
 *
 * then the string stored at "formatted_time" is:
 *
 *   May 20, 2019 (Mon).
 *
 */
/*
Source: timeplay.c 
*/

void dt_format(char *formatted_time, const char *dt_time, const int len)
{
    struct tm temp_time;
    time_t    full_time;
    char      temp[5];

    /*  
     * Ignore for now everything other than the year, month and date.
     * For conversion to work, months must be numbered from 0, and the 
     * year from 1900.
     */  
    memset(&temp_time, 0, sizeof(struct tm));
    sscanf(dt_time, "%4d%2d%2d",
        &temp_time.tm_year, &temp_time.tm_mon, &temp_time.tm_mday);
    temp_time.tm_year -= 1900;
    temp_time.tm_mon -= 1;
    full_time = mktime(&temp_time);
    strftime(formatted_time, len, "%B %d, %Y (%a)", 
        localtime(&full_time));
}


/*
 * Function dt_increment:
 *
 * Given a date-time, it adds the number of days in a way that
 * results in the correct year, month, and day. For example,
 * if the string in "before" corresponds to:
 *
 *   20190520T111500
 *
 * then the datetime string stored in "after", assuming that
 * "num_days" is 100, will be:
 *
 *   20190828T111500
 *
 * which is 100 days after May 20, 2019 (i.e., August 28, 2019).
 *
 */
/*
Source: timeplay.c
Note: Changed strftime(after, 8... to strftime(after, 9....
      This was to account for the absence of day
*/

void dt_increment(char *after, const char *before, int const num_days)
{
    struct tm temp_time, *p_temp_time;
    time_t    full_time;
    char      temp[5];

    memset(&temp_time, 0, sizeof(struct tm));
    sscanf(before, "%4d%2d%2d", &temp_time.tm_year,
        &temp_time.tm_mon, &temp_time.tm_mday);
    temp_time.tm_year -= 1900;
    temp_time.tm_mon -= 1;
    temp_time.tm_mday += num_days;

    full_time = mktime(&temp_time);
    after[0] = '\0';
    strftime(after, 9, "%Y%m%d", localtime(&full_time));
    strncpy(after + 8, before + 8, MAX_LINE_LEN - 8); 
    after[MAX_LINE_LEN - 1] = '\0';
}

/*
Purpose: Called from qsort in the read function.
         Used to sort events by date by taking two void
         pointers a and b, as arguments and returns their 
         diference.

Parameters: const void *a
            const void *b
*/

int compare_event(const void *a, const void *b)
{

    /*
    Convert the pointers to pointer to int and dereference them to
    their actual integer values.
    */
    event *ea = (event *)a;
    event *eb = (event *)b;

    /*
    If a positive result is return the first argument 
    greater than the second. If a negative result 
    is returned, the second argument is greater.
    0 if the arguents are equivalent.
    */
    return strcmp(ea -> dt_start, eb -> dt_start);
      
}


/*
Purpose: The function opens and reads the file line by line and stores the
         values of the ics file into the structure. The function also determines
         if there are any repeating events then determine how many times to repeat
         the event. If there are multiple events use the C qsort() function to 
         sort the events.
    
Parameters: const char *filename. 
            int start_arg - holds the starting date from the command line.
            int end_arg   - holds the end date from the command line.
            int count     - tracks the number of event
 
Returns: int - count    
*/

int read(const char *filename, int start_arg, int end_arg, int count)
{
    char line[MAX_LINE_LEN];
   
    /*Open and read the file in read mode only*/
    FILE* infile = fopen(filename, "r");
    
    /*If the file cannot be opened exit by abnormal termination
      of the program*/
    if (infile == NULL){
        exit(1);
    }
    
    /*Read through each line of the file and stop at the end*/
    while (fgets(line, MAX_LINE_LEN, infile)) {
        /*Determine where to start gather information of the even*/
        if (strncmp(line, "BEGIN:VEVENT", 12) == 0) {
            /*Create an event to store the events information*/
            event cur_event;
            
            /*
            If the property is found on the line, scan and store the value.
            <property>:<value>

            fgets(line, MAX_LINE_LEN, infile) is used frequently in this
            function in order to read a limited number of characters from
            the given file stream.
            */
            /*Find the file's line with "DTSTART:"*/
            while (strncmp(line, "DTSTART:", 8) != 0){
                fgets(line, MAX_LINE_LEN, infile);
            }
            sscanf(line, "DTSTART:%s", cur_event.dt_start);

            /*Find the file's line with "DTEND:"*/
            while (strncmp(line, "DTEND:", 6) != 0) {         
                fgets(line, MAX_LINE_LEN, infile);
            }
            sscanf(line, "DTEND:%s", cur_event.dt_end);

            fgets(line, MAX_LINE_LEN, infile);
            /*Check if event is repeating by determing if "RRULE" is within the event*/
            if (strncmp(line, "RRULE", 5) == 0) {

                /*
                Check if the repeating instruction containts 'WKST=MO'.
                Then when determined if does or does not, read and store the
                formatted data of the 'UNTIL' date. This is to account for the 
                noticed varainces within the ics files.
                */
                if (strncmp(line, "RRULE:FREQ=WEEKLY;WKST=MO;UNTIL=", 32) == 0) {
                    sscanf(line, "RRULE:FREQ=WEEKLY;WKST=MO;UNTIL=%15s", cur_event.dt_repeat);
                } else if (strncmp(line, "RRULE:FREQ=WEEKLY;UNTIL=", 24) == 0) {
                    sscanf(line, "RRULE:FREQ=WEEKLY;UNTIL=%15s", cur_event.dt_repeat);
                }
                fgets(line, MAX_LINE_LEN, infile);
            }
            else {
                /*There is no repeating event so use the start date as a
                place holder within the repeating events variable.*/
                strncpy(cur_event.dt_repeat, cur_event.dt_start, MAX_LINE_LEN);
            }
            /*Find the line with the LOCATION property*/             
            while (strncmp(line, "LOCATION", 8) != 0) {
                fgets(line, MAX_LINE_LEN, infile);
            }
            
            /*Store all the location information after the property*/
            strncpy(cur_event.location, &line[9], strlen(line));
            /*Used to prevent unwanted spacing and newlines*/
            cur_event.location[strlen(cur_event.location) - 1] = '\0';
           
           /*Fine the line with the SUMMARY property*/
            while (strncmp(line, "SUMMARY", 7) != 0){
                fgets(line, MAX_LINE_LEN, infile);
            }
            /*store all the summary information after the property*/
            strncpy(cur_event.summary, &line[8], strlen(line));
            /*Used to prevent unwated spacing and newlines*/
            cur_event.summary[strlen(cur_event.summary) - 1] = '\0';

            /*
            Determine if the start and end date of the file fall within the command line 
            argument. If so, store the current event into global arr_events[] array and 
            increase the amount of events
              
            atoi() is used to convert a string into an integer
            */
            if ((atoi(cur_event.dt_start) >= start_arg) && (atoi(cur_event.dt_end) <= end_arg)) {
                arr_events[count] = cur_event;
                count++;
            }

            /*Determine how many times to repeat the event.*/
            while (strncmp(cur_event.dt_start, cur_event.dt_repeat, 15) < 0) {
                /*Used to hold the new string ouputted by dt_increment*/
                char temp[MAX_LINE_LEN];

                /*
                Increment the start and end date by 7 days (1 week).
                Then copy the newly incremented time from the temporary
                character array into the events start and end
                */
                dt_increment(temp, cur_event.dt_start, 7);
                strncpy(cur_event.dt_start, temp, 15);
                dt_increment(temp, cur_event.dt_end, 7);
                strncpy(cur_event.dt_end, temp, 15);

                /*
                The start and end date of the file must fall within the dates of 
                the command line argument. If the start date is still less or equal
                to the repeat untill date, then add the event into the arr_events 
                array and increase the count of the events.  
                */                
                if (((atoi(cur_event.dt_start) >= start_arg) && (atoi(cur_event.dt_end) <= end_arg)) && (strncmp(cur_event.dt_start, cur_event.dt_repeat, 15) <= 0)) {
                    arr_events[count] = cur_event;
                    count++;
                }
            }            
        }
    }

    fclose(infile);
    
    /*Sorts the array of events by calling upon the compare_event function*/     
    qsort(arr_events, count, sizeof(event), compare_event);
    
    return count;
}


/*
Purpose: Responsible for printing out all the events. Creates more 
         readable ouput of the dates by calling upon the dt_format
         function. In addition, the function converts from 24-hour
         to 12-hour clock time and determines if the time should be 
         in 'AM' or 'PM'.
          
Parameters: int count - the amount of events that

Return: void 
*/

void print_events(int count)
{
    /*varibles to hold the hours and minutes of the start and end date/time*/ 
    int hr_start, min_start;
    int hr_end, min_end;

    //Loop trhough the amount of events
    for (int i = 0; i < count; i++) {
        
        /*Extract and store the hours and minutes from the event*/
        sscanf(arr_events[i].dt_start, "%*8dT%2d%2d", &hr_start, &min_start);
        sscanf(arr_events[i].dt_end, "%*8dT%2d%2d", &hr_end, &min_end);

        /*
        Determine if the time should be displaye in AM or PM.
        Convert the time from 24-hour to 12-hour clock time.
        */
        /*representation of start time*/
        if (hr_start < 12){
            arr_events[i].ampm_start = "AM";
        }else if (hr_start == 12){
            arr_events[i].ampm_start = "PM";
        } else {
            arr_events[i].ampm_start = "PM";
            hr_start -= 12;
        }

        /*Representation of end time*/        
        if (hr_end < 12){
            arr_events[i].ampm_end = "AM";
        }else if (hr_end == 12){
            arr_events[i].ampm_end = "PM";
        } else {
            arr_events[i].ampm_end = "PM";
            hr_end -= 12;
        }
        /*
        Determine if a new day needs to be printed. We do not want to display a new day if
        events are on the same day
        */
        if ((i == 0) || ((i > 0) && (strncmp(arr_events[i].dt_start, arr_events[i - 1].dt_start, 8) != 0))) {
           
            /*Create a new line if there is multiple events*/
            if (i > 0){
               printf("\n");
            }

            /*
            Call the dt_format function provided from timeplay.c to convert the 
            ics file date into a more readable version, and then print it out.
            the formatted_date character array is used to hold the new formatted
            date.
            */
            char formatted_date[MAX_LINE_LEN];
            dt_format(formatted_date, arr_events[i].dt_start, MAX_LINE_LEN);          
            printf("%s\n", formatted_date);

            //print the dashes such that they are flush with the date above 
            for (int m = 0; m < strlen(formatted_date); m++) {
                printf("-");
            }
            printf("\n");
        }

        printf("%2d:%02d %s to %2d:%02d %s: %s {{%s}}\n",
            hr_start, min_start, arr_events[i].ampm_start,
            hr_end, min_end, arr_events[i].ampm_end, arr_events[i].summary, arr_events[i].location);
    }
}


/*
Purpose: Given the start and end date from the command line the
         function converts the data into a start date integer and 
         an end date integer that represents the same  date
         information format as in the ics files.
         If from_y = 2021, from_m = 2, and from_d = 14 then:
         2021 * 10000 + 2 * 100 + 14 = 20210214

Parameters: int *start - stores the calculated start date
            int *end   - stores the calculated end date
            other      - command line arguments taken from main
            
Return: void  
*/ 

void convert(int from_y, int from_m, int from_d, int to_y, int to_m, int to_d, int *start, int *end) 
{
    *start = from_y * 10000 + from_m * 100 + from_d;
    *end = to_y * 10000 + to_m * 100 + to_d;
}

