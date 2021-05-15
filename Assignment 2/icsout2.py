#!/usr/bin/env python3

'''
Seng 265 - icsout2.py
Alex Holland - V00928553
'''

import argparse
import datetime


'''
Helper Functions:
'''

# Sorts events by date where time[0] is the start time.
# Source: https://tinyurl.com/vnvfndv4
def sorting(events):
    for date in events:
        events[date].sort(key = lambda time: [time[0]])
    return events
 

# Get's the string representation of the month of a date object.
# %B returns the full name of the month, eg. September.
def month(date):
    return date.strftime("%B")


# Get's the string representation of the weekday of a date object.
# %a returns the first three characters of the weekday, eg. Wed.
def weekday(date):
    return date.strftime("%a")


# Get's the string representation of the time of a time object.
# Converts the 24hr time to 12hr time with 'AM' or 'PM'.
# eg. 18:30:00 becomes 6:30 PM
def time(time):
    return time.strftime("%_I:%M %p")


# Increments the the date by one week.
def increment_week(date):
    return date + datetime.timedelta(weeks = 1)


# Create a datetime object from the icalendar format. 
# eg. 20210214T180000 -> 2021-02-14 18:00:00 
def ical_obj(ical_date):
    # Parameters are passed as year, month, day, hour, minute, second.
    return datetime.datetime(int(ical_date[0:4]), int(ical_date[4:6]), int(ical_date[6:8]),
                             int(ical_date[9:11]), int(ical_date[11:13]), int(ical_date[13:]))   


# Create a date object from the command argument.
# eg. 2021/2/14 -> 2021-02-14
def cmd_obj(cmd_date):
    # Line source: https://tinyurl.com/yph7e2a8
    i = [pos for (pos, word) in enumerate(cmd_date) if word == '/']
    # Parameters are passes as day, month, year.
    return datetime.date(int(cmd_date[:i[0]]), int(cmd_date[(i[0] + 1):i[1]]), int(cmd_date[(i[1] + 1):]))


'''
Primary Functions:
'''

'''
Purpose: Open and read the file. Put each event into it's own group 
Parameters: filename - the ics file to be read
Returns: Events - A collection of events
'''
def read(filename):
    try:
        # Open the file and strip the newline character from each line.
        # Takes approx 0.03 of a second with a file.
        # that has 487k single words, each on a new line. 
        # Source: https://tinyurl.com/ent2e2s8 
        with open(filename) as infile:
            lines = [ line.strip() for line in infile] 
        # Remove lines not needed in the formatted output.
        lines = [line for line in lines if line not in
            ('BEGIN:VCALENDAR', 'END:VCALENDAR') if (line[0:8] != 'VERSION:')]
        # Seperate the ics file into groups.    
        groups  = ('\n'.join(lines)).split('END:VEVENT')
        # Remove any groups that are empty.
        groups.remove('')
        # Split and store the lines in each group into a list.
        groups = [group.split('\n') for group in groups]
        # Remove unwanted lines.
        groups = [[line for line in group if line not in ('', 'BEGIN:VEVENT')] for group in groups]
        infile.close()
        return groups
    except FileNotFoundError:
        print("Cannot open file")
    except PermissionError:
        print("Permission not given")


'''
Purpse: Stores all the events into a dictionary.
'''
def store_events(groups):  
    # Create an empty dictionary to hold all the events. 
    events = {}
    for group in groups:
        # Detemine if the event is repeating.   
        if (group[2][18:23] == 'UNTIL'):
            dt_until = group[2][24:39]
        elif (group[2][0] == 'R'):
           dt_until = group[2][32:47]
        else:
           dt_until = group[1][6:]
        # Calls the function store with the following parameters:
        # events, start time object, end time object, repeat until date, summary, location.
        events = store(events, ical_obj(group[0][8:]), ical_obj(group[1][6:]), ical_obj(dt_until), group[-1][8:], group[-2][9:])
    
    #Sort the dates.
    events = sorting(events)
    
    return events


'''
Purpose: Store repeating events into a dictionary by their date
'''
def store(events, start, end, until, summary, location):
    # Repeat while the end date is still less then the repeat until date
    while end <= until:
        date = start.date()
        if date not in events:
            events[date] = []
        # Add event information to the specific date
        events[date].append((start.time(), end.time(), summary, location))
        start = increment_week(start)
        end = increment_week(end)
    return events


'''
Purpose: Print the events stored in the dictionary 
'''
def print_events(events, start, end):
    # Sort the dates that are within the range specified 
    # by the command line argument.
    keys = list(events.keys())
    keys.sort() 
    keys = [key for key in keys if (cmd_obj(start) <= key <= cmd_obj(end))]
    event_count = 0
    for key in keys[0:]:
        if (event_count > 0):
            print()
        # Print the formatted dates above the dashes.
        date = '{0} {1:0>2}, {2} ({3})'.format(month(key),
            key.day, key.year, weekday(key))
        print(date)
        # Print the dashes '-' such that they are flush with the above date.
        print('-' * len(date))
        # Print the formatted time, summary, and location below the dashes.
        # Double curly braces are needed to print 1 curly brace.
        for event in events[key]:
            print('{0} to {1}: {2} {{{{{3}}}}}'.format(time(event[0]),
                time(event[1]), event[2], event[3]))
        event_count += 1



# The code below configures the argparse module for use with
# assignment #2.
# 

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--file', type=str, help='file to be processed')
    parser.add_argument('--start', type=str, help='start of date range')
    parser.add_argument('--end', type=str, help='end of data range')

    args = parser.parse_args()

   # print ('file: {}; start: {}; end: {}'.format( args.file, args.start,
   #    args.end))

    if not args.file:
        print("Need --file=<ics filename>")

    if not args.start:
        print("Need --start=dd/mm/yyyy")

    if not args.end:
        print("Need --end=dd/mm/yyyy")

    entries = read(args.file)
    events = store_events(entries)
    print_events(events, args.start, args.end)

if __name__ == "__main__":
    main()

