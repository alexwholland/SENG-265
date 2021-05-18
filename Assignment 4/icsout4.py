'''
Seng 265 - Assignment 4
'''

import re
import datetime


class ICSout:
    '''
    Purpose:    This class reads and stores the contents 
                of the ics file.

    Attributes: __filename     - The name of the file to be read. 
                __event_by_day - A dictionary to map the events.
    ''' 
    def __init__(self, filename):
        '''
        ~Constructor~

        Parameters: filename the file to be read
        '''
        self.__filename = filename
        self.__events_by_day = {}
        self.__read()

    
    def get_events_for_day(self, dt):
        '''
        Purpose:    Returns the event that happens on
                    a given date. 

        Parameters: dt - The date of the event.
    
        Returns:    An event object.
        '''
        try:
            return self.__events_by_day[dt]
        except KeyError:
            return None

    
    def __read(self):
        '''
        Purpose: Open and read the file. Removes any unnecessary lines 
                 and puts each event into it's own group.

        Returns: Calls __get_events() passing in a list of lists.
        '''
        try:          
            # Open the file and strip the newline character from eachline. 
            # Also remove any empty lines.
            with open(self.__filename) as infile: 
                lines = list(line for line in (l.rstrip() for l in infile) if line)    
            # Remove lines not needed in the formatted output.    
            lines = [line for line in lines if line not in ('BEGIN:VCALENDAR', 'END:VCALENDAR') 
                if not re.compile('VERSION:').match(line)]
            # Seperate the ics file into groups.
            groups = ('\n'.join(lines)).split('END:VEVENT')
            # Split any groups that are empty. 
            groups.remove('')
            # Split and store the lines in each group into a list.                                     
            groups = [group.split('\n') for group in groups]
            #Remove unwanted lines.  
            groups = [[line for line in group if line not in ('BEGIN:VEVENT')] for group in groups]
            infile.close()
            self.__get_events(groups)
        except PermissionError:
            print("Permission not given")
        except FileNotFoundError:
            print("Cannot open file")
 

    def __get_events(self, groups):
        '''
        Purpose:    Extract the event information using regex and
                    store it into __events_by_day.

        Parameters: groups - a list of list that stores the contents 
                             of the information needed for the formatted
                             output.
        '''
        # Extracts the time info from the ics file. Ex: 20210102T111500 .
        timeobj = re.compile(r'(\d{4})(\d{2})(\d{2})T(\d{2})(\d{2})')

        for pos in groups:
            # Searches and stores the start date and time.
            yr_st, mo_st, d_st, h_st, mi_st = timeobj.search(pos[0]).groups()
            # Searches and stores the end time.
            h_end, mi_end = timeobj.search(pos[1]).group(4), timeobj.search(pos[1]).group(5)
            # Seaches and stores the location information.          
            location = re.compile(r'LOCATION:(.*)').search(pos[-2]).group(1)
            # Searches and stores the summary information.
            summary = re.compile(r'SUMMARY:(.*)').search(pos[-1]).group(1)
            # If the event is repeating create a datetime object of the until date, else placehold 
            # the datetime object with the start date.
            ser = timeobj.search(pos[2])
            if ser:
                dt_utilobj = datetime.datetime(int(ser.group(1)), int(ser.group(2)), int(ser.group(3)))
            else:
                dt_utilobj = datetime.datetime(int(yr_st), int(mo_st), int(d_st))

            # Creates an Event object.
            event = Event(int(yr_st), int(mo_st), int(d_st), int(h_st),
                int(mi_st), int(h_end), int(mi_end), location, summary)

            # Calls __determine_events() passing in the newly 
            # created Event object and a datetime object.
            event = self.__determine_event(event, dt_utilobj)


    def __determine_event(self, event, dt_util):
        '''
        Purpose:    Store all of the repetive events into a dictionary within 
                    the start and end date with date as the key.

        Parameters: event   - the event object.
                    dt_util - the date to repeat untill. 
        '''
        while (event.cur_date() <= dt_util):       
            try:
                self.__events_by_day[event.cur_date()].add_event(event)
            except KeyError:
                self.__events_by_day[event.cur_date()] = Events(event)
            event = event.dt_increment()
        return event



class Events:
    '''
    A class to represent events that happen on the same date.

    Attributes: __date   - the date of the event.
                __events - a list of the events on the same date.
    '''
    def __init__(self, event):
        '''
        ~Constructor~        

        Parameters: event - an event to be stored into a list      
        '''
        self.__date = event.cur_date()
        self.__events = [event]

    def __repr__(self):
        '''
        Purpose:    Create a string representation of the date, 
                    dashes and events in the following format:
    
                    Formatted date
                    --------------
                    First formatted event
                    Second formatted event
                    ...
                    n formatted event

        Returns:    output - a string of formatted date input^.
        '''
        output = self.__f_date() + '\n'         
        output += len(self.__f_date()) * '-'   
        for event in sorted(self.__events):     
            output += '\n' + str(event)         
        return output                           

   
    def add_event(self, event):
        '''
        Purpose:    Adds a new event to the end of the list.
        
        Parameters: event - the event to be added.
        '''
        self.__events.append(event)


    def __f_day(self):
        '''
        Purpose: Converts date into a string of an abbreviated
                 weekday name. Ex. Sun, Mon,...

        Returns: A string.
        '''
        return self.__date.strftime("%a")


    def __f_month(self):
        '''
        Purpose: Converts date into a string of the full month name.
                 Ex. January, February,...
    
        Returns: A string.
        '''
        return self.__date.strftime("%B")
 

    def __f_date(self):
        '''
        Purpose: Creates a string representation of the date
                 in the format:
            
                 <month text> <day>, <year> (<day of week>)

        Returns: A string representation of self.__date.    
        '''
        return '{} {:02}, {} ({})'.format(self.__f_month(), self.__date.day, self.__date.year, self.__f_day())



class Event:
    '''
    Purpose:    A class to represent a single event.
        
    Attributes: __t_st     - time object of starting hour and min. 
                __t_end    - time object of the end hour and min.
                __location - location of the event.
                __summary  - summary of the event.
                __dt       - date object of year, month, and day.

    Note:       The dunder '__' before the variable names are used
                for name mangling. Used to ensure the name will not
                overlap with a simlar name in another class.                
    '''
    def __init__(self, yr, mo, d, h_st, mi_st, h_end, mi_end, location, summary):
        '''
        ~Constructor~
       
        parameters: yr       - int year
                    mo       - int month
                    d        - int day
                    h_st     - int starting hour
                    mi_st    - int starting minute
                    h_end    - int end hour
                    mi_end   - int end minute
                    location - str
                    summary  - str
        '''
        self.__t_st = datetime.time(h_st, mi_st)
        self.__t_end = datetime.time(h_end, mi_end)
        self.__location = location
        self.__summary = summary
        self.__dt = datetime.datetime(yr, mo, d)


    def __repr__(self):
        '''
        Purpose: Creates a string representation of events time, location, 
                 and summary in the format:
    
        <start hr>:<start min> <AM/PM> to <end hr>:<end min> <AM/PM>: <Summary> {{<location>}}

        Returns: A string.
        '''
        time = self.__time(self.__t_st) + ' to ' + self.__time(self.__t_end) + ': '
        return time + self.__summary + ' {{' + self.__location + '}}'


    def __lt__(self, other):
        '''
        ~Magic Method~
        
        Purpose: Compare the start time between two events.

        Returns: boolean - True  = the start time of the first event
                                   is less then the second.
                           False = otherwise.
        '''
        return self.__t_st < other.__t_st


    def cur_date(self):
        '''
        Purpose: Gets the date object of year, month, and day.

        Returns: A datetime.datetime object.
        '''
        return self.__dt


    def __time(self, t):
        '''
        Purpose:    Convert from 24hr to 12hr time and determine
                    if the time should be displayed with AM or PM.
                    Create a string representation of this time.
    
                    <hour>:<minute> <AM/PM>    

        Parameters: t - the unformatted time object.

        Returns:    A string
        '''
        return t.strftime("%_I:%M %p")


    def dt_increment(self):
        '''
        Purpose: Creates a new event and increments the date 
                 by one week (7 days).
        
        Returns: Event - an event with only date changes made.
        '''
        return self.__create_event(self.__dt + datetime.timedelta(weeks = 1))

    
    def __create_event(self, dt):
        '''
        Purpose:    Creates a new event.
        
        Parameters: dt - the date of an event.

        Returns:    Event - a newly created event without any modifications. 
        '''
        return Event(dt.year, dt.month, dt.day, self.__t_st.hour, self.__t_st.minute,
            self.__t_end.hour, self.__t_end.minute, self.__location, self.__summary)

