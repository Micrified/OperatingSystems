#include "pm.h"
#include <minix/callnr.h>
#include <minix/com.h>
#include <signal.h>
#include "mproc.h"
#include "param.h"

/*
 ******************************************************************************
 *                           University of Groningen                          * 
 * AUTHORS: Barnabas Busa, Charles Randolph, Joe Jones.                       *
 ******************************************************************************
*/

#define EPOCH           1970
#define SEC_PER_DAY     (24 * 3600)
#define DAYS_PER_YEAR   365
#define MAX_LEAPS       49

int leaps[MAX_LEAPS][2] = {
    {+0, +0},           // 1970
    {+0, +0},           // 1971
    {-1, -1},           // 1972
    {+0, -1},           // 1973
    {+0, -1},           // 1974
    {+0, -1},           // 1975
    {+0, -1},           // 1976
    {+0, -1},           // 1977
    {+0, -1},           // 1978
    {+0, -1},           // 1979
    {+0, +0},           // 1980
    {-1, +0},           // 1981
    {-1, +0},           // 1982
    {-1, +0},           // 1983
    {+0, +0},           // 1984
    {-1, +0},           // 1985
    {+0, +0},           // 1986
    {+0, -1},           // 1987
    {+0, +0},           // 1988
    {+0, -1},           // 1989
    {+0, -1},           // 1990
    {+0, +0},           // 1991
    {-1, +0},           // 1992
    {-1, +0},           // 1993
    {-1, +0},           // 1994
    {+0, -1},           // 1995
    {+0, +0},           // 1996
    {-1, +0},           // 1997
    {+0, -1},           // 1998
    {+0, +0},           // 1999
    {+0, +0},           // 2000
    {+0, +0},           // 2001
    {+0, +0},           // 2002
    {+0, +0},           // 2003
    {+0, +0},           // 2004
    {+0, -1},           // 2005
    {+0, +0},           // 2006
    {+0, +0},           // 2007
    {+0, -1},           // 2008
    {+0, +0},           // 2009
    {+0, +0},           // 2010
    {+0, +0},           // 2011
    {-1, +0},           // 2012
    {+0, +0},           // 2013
    {+0, +0},           // 2014
    {-1, +0},           // 2015
    {+0, -1},           // 2016
    {+0, +0},           // 2017
    {+0, +0},           // 2018
};

/* Returns one if the year is a leap year. Otherwise zero. */
static int isLeap (int y) {
    if ((y % 400) == 0) return 1;
    if ((y % 100) == 0) return 0;
    return ((y % 4) == 0);
}

/* Returns the number of seconds in a given year. */
static int secondsInYear (int y) {
    return SEC_PER_DAY * (DAYS_PER_YEAR + isLeap(y));
}

/* Returns the number of leap seconds since the epoch, for given elapsed seconds. */
static int leapSeconds (time_t elapsed) {
   int y = EPOCH, n = 0;
   while (y - EPOCH < (2 * MAX_LEAPS) && (elapsed - (secondsInYear(y) / 2)) > 0) {
       n += leaps[(y - EPOCH) / 2][y % 2];
       y++;
       elapsed -= (secondsInYear(y) / 2);
   }
   return n;
}

/* Returns the utc time given the current number of seconds since 1/1/1970 */
static time_t toUTC (time_t elapsed) {
    return elapsed + leapSeconds(elapsed);
}

/* Returns UNIX time adjusted for leap seconds */
int do_utctime(void) {
    clock_t uptime, boottime;
    int s;

    // Attempt to extract time.
    if ((s = getuptime2(&uptime, &boottime)) != OK) {
        panic("do_utctime couldn't get uptime: %d", s);
    }

    // Extract seconds since 1/1/1970.
    time_t unix_time = (time_t)(boottime + (uptime/system_hz));

    // Save adjusted time in message reply. 
    mp->mp_reply.reply_time = toUTC(unix_time);

    return(OK);
}
