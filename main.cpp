/*
 * =====================================================================================
 *
 *       Filename:  batmon_cmake/main.cpp
 *
 *    Description:  Core functionality of Batmon
 *
 *        Version:  0.1alpha
 *        Created:  11/06/17 22:00:00
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  Manuel Olguin, molguin@dcc.uchile.cl
 *   Organization:  Universidad de Chile
 *         GitHub:  arachnid92
 *
 * =====================================================================================
 */

#include <iostream>
#include <libnotify/notify.h>
#include <fstream>

using namespace std;

static const string helptext =
        "Batmon -- a simple command line battery monitor written in C++.\n\n"
                "Usage: batmon [options]\n"
                "Options:\n"
                "\t-h, --help\tPrint this help text.\n"
                "\t--version\tPrint program version and exit.\n"
                "\t-p\t\tOnly print current battery percent.\n"
                "\t\t\tIf there's no battery, prints -1\n"
                "\t-t\t\tText mode, prints battery status (Charging, Discharging,\n"
                "\t\t\tFull and Unavailable) followed by current battery percent.\n"
                "\t\t\tThis is the default mode.\n"
                "\t-i INTERVAL\tUpdate interval in seconds. Set to 0 to run once and exit.\n";

int main ()
{

    int cap;

    cout << helptext;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    while ( false )
    {
        ifstream sfile ( "/sys/class/power_supply/BAT0/capacity" );
        sfile >> cap;
        string s_cap ( "Battery: " + to_string ( cap ) + "%" );

        notify_init ( "Hello world!" );
        NotifyNotification *Hello;
        Hello = notify_notification_new ( "Hello world", s_cap.c_str (), "face-kiss" );
        notify_notification_show ( Hello, NULL );
        g_object_unref (G_OBJECT( Hello ));
        notify_uninit ();

        sfile.close ();
        sleep ( 1 );
    }
#pragma clang diagnostic pop
}