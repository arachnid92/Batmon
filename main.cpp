/*
 * =====================================================================================
 *
 *       Filename:  batmon_cmake/main.cpp
 *
 *    Description:  Core functionality of Batmon
 *
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
#include <boost/program_options/option.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/filesystem.hpp>

#define BATT_FULL   "/usr/share/batmon/icons/light/full.png"
#define BATT_HIGH   "/usr/share/batmon/icons/light/high.png"
#define BATT_LOW    "/usr/share/batmon/icons/light/low.png"
#define BATT_CRIT   "/usr/share/batmon/icons/light/empty.png"
#define BATT_CHAR   "/usr/share/batmon/icons/light/charging.png"

#define VERSION     "0.1.3alpha"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

void checkBattery ();

namespace  pOpt = boost::program_options;

int delay;  // polling interval
int crit;   // critical battery level
int low;    // low battery level
int delta;  // notify delta
int ccap;   // current batt capacity percent
int pcap;   // previous batt capacity percent
std::string cstat; // current batt status
std::string pstat; // previous batt status

bool debug;

int main ( const int argc, const char *argv[] )
{

    ccap = 0;
    pcap = 0;
    cstat = "";
    pstat = "";

    std::string odes ( "Batmon, a lightweight battery monitor in C++. Version " );
    odes.append ( VERSION );

    // parse options from command line
    pOpt::options_description desc ( odes + "\n\nOptions" );
    desc.add_options ()
            ( "help,h", "Show this help message." )
            ( "interval,i", pOpt::value < int > ( &delay )->default_value ( 1 ),
              "Polling interval in s (default = 1s)" )
            ( "delta,d", pOpt::value < int > ( &delta )->default_value ( 10 ),
              "Minimum change in battery percent before notifying (Default = 10)" )
            ( "low,l", pOpt::value < int > ( &low )->default_value ( 15 ),
              "Low battery threshold in percent (default = 15%)" )
            ( "critical,c", pOpt::value < int > ( &crit )->default_value ( 5 ),
              "Critical battery thresholf in percent (default = 5%)" )
            ( "debug", "Debug mode." );

    pOpt::variables_map vm;
    pOpt::store ( pOpt::parse_command_line ( argc, argv, desc ), vm );
    pOpt::notify ( vm );

    if ( vm.count ( "help" ) )
    {
        std::cout << desc << "\n";
        return 1;
    }

    if ( vm.count ( "debug" ) )
    {
        debug = true;
        std::cerr << "\x1b[01mDebug mode.\nPolling interval:\t" + std::to_string ( delay ) + "\x1b[00m\n";
    }
    else
        debug = false;


    while ( true )
    {
        checkBattery ();
        sleep ( delay );
    }
}

void checkBattery ()
{

    // check if battery is present
    if ( !boost::filesystem::exists ( "/sys/class/power_supply/BAT0" ) )
    {

        if ( debug )
            std::cerr << "\x1b[01mNo battery.\x1b[00m\n";

        return;
    }

    if ( debug )
        std::cerr << "Polling.\n";

    // read information from battery capacity and status files
    std::ifstream capfile ( "/sys/class/power_supply/BAT0/capacity" );
    std::ifstream statfile ( "/sys/class/power_supply/BAT0/status" );
    capfile >> ccap;
    statfile >> cstat;

    if ( debug )
        std::cerr << "Battery: " + std::to_string ( ccap ) + "%\n";

    // if no change, sleep and then poll again
    if ( ccap == pcap && cstat == pstat )
    {
        if ( debug )
            std::cerr << "No change.\n";

        statfile.close ();
        capfile.close ();
        return;
    }

    // notify low
    if ( ccap == low )
    {

        if ( debug )
            std::cerr << "\x1b[01mLow battery threshold, notifying.\x1b[00m\n";

        std::string battext ( cstat + " " + std::to_string ( ccap ) + "%" );

        // Notification time
        notify_init ( "batmon" );
        NotifyNotification *Batt;
        Batt = notify_notification_new ( "Low Battery", battext.c_str (), BATT_LOW );
        notify_notification_show ( Batt, NULL );
        g_object_unref ( G_OBJECT( Batt ) );
        notify_uninit ();

    }
        // notify critical
    else if ( ccap == crit )
    {

        if ( debug )
            std::cerr << "\x1b[01mCritical battery threshold, notifying.\x1b[00m\n";

        std::string battext ( cstat + " " + std::to_string ( ccap ) + "%" );

        // Notification time
        notify_init ( "batmon" );
        NotifyNotification *Batt;
        Batt = notify_notification_new ( "Critical Battery", battext.c_str (), BATT_CRIT );
        notify_notification_show ( Batt, NULL );
        g_object_unref ( G_OBJECT( Batt ) );
        notify_uninit ();

    }
        // else, notify every delta%
    else if ( ccap % delta == 0 )
    {

        if ( debug )
            std::cerr << "\x1b[01mChange, notifying.\x1b[00m\n";

        std::string battext ( cstat + " " + std::to_string ( ccap ) + "%" );

        // Notification time
        notify_init ( "batmon" );
        NotifyNotification *Batt;
        Batt = notify_notification_new ( "Battery", battext.c_str (), BATT_FULL );
        notify_notification_show ( Batt, NULL );
        g_object_unref ( G_OBJECT( Batt ) );
        notify_uninit ();

    }

    statfile.close ();
    capfile.close ();

    // store values for comparing next time we poll
    pcap = ccap;
    pstat = cstat;
    return;

}

#pragma clang diagnostic pop