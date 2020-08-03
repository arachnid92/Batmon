/*
 * =====================================================================================
 *
 *       Filename:  battmon_cmake/battmon.cpp
 *
 *    Description:  Core functionality of battmon
 *
 *        Created:  11/06/17 22:00:00
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

#define VERSION     "0.5beta3"


void checkBattery (); // poll function

namespace  pOpt = boost::program_options;

int delay;  // polling interval
int crit;   // critical battery level
int low;    // low battery level
int delta;  // notify delta
int ccap;   // current batt capacity percent
int pcap;   // previous batt capacity percent
std::string cstat; // current batt status
std::string pstat; // previous batt status
std::string capacityfilepath; // path of the file that has capacity 
std::string statusfilepath; // path of the file that has statistics

int battery; // flag to identify battery
bool debug;
bool bat;
bool cont; // continuous mode or poll once mode

// icon constants
const char *BATT_FULL;
const char *BATT_HIGH;
const char *BATT_LOW;
const char *BATT_CRIT;
const char *BATT_CHAR;


int main ( const int argc, const char *argv[] )
{

    ccap = 0;
    pcap = 0;
    cstat = "";
    pstat = "";

    std::string odes ( "battmon, a lightweight battery monitor in C++. Version " );
    odes.append ( VERSION );

    // parse options from command line
    pOpt::options_description desc ( odes + "\n\nOptions" );
    desc.add_options ()
            ( "help,h", "Show this help message." )
            (
                    "interval,i", pOpt::value < int > ( &delay )->default_value ( 5 ),
                    "Polling interval in s (default = 5s)."
            )
            ( "delta,d", pOpt::value < int > ( &delta )->default_value ( 10 ),
              "Minimum change in battery percent before notifying (Default = 10)." )
            ( "low,l", pOpt::value < int > ( &low )->default_value ( 15 ),
              "Low battery threshold in percent (default = 15%)." )
            ( "critical,c", pOpt::value < int > ( &crit )->default_value ( 5 ),
              "Critical battery threshold in percent (default = 5%)." )
            ( "run-once,r", "Only poll battery state once and exit." )
            ( "dark,k", "Use dark icons instead of the default light ones." )
            ( "debug", "Print debug comments to STDERR." );

    pOpt::variables_map vm;
    pOpt::store ( pOpt::parse_command_line ( argc, argv, desc ), vm );
    pOpt::notify ( vm );

    if ( vm.count ( "help" ) )
    {
        std::cout << desc << "\n";
        return 1;
    }

    if ( vm.count ( "dark" ) )
    {
        BATT_FULL = "/usr/share/battmon/icons/dark/full.png";
        BATT_HIGH = "/usr/share/battmon/icons/dark/high.png";
        BATT_LOW  = "/usr/share/battmon/icons/dark/low.png";
        BATT_CRIT = "/usr/share/battmon/icons/dark/empty.png";
        BATT_CHAR = "/usr/share/battmon/icons/dark/charging.png";
    }
    else
    {
        BATT_FULL = "/usr/share/battmon/icons/light/full.png";
        BATT_HIGH = "/usr/share/battmon/icons/light/high.png";
        BATT_LOW  = "/usr/share/battmon/icons/light/low.png";
        BATT_CRIT = "/usr/share/battmon/icons/light/empty.png";
        BATT_CHAR = "/usr/share/battmon/icons/light/charging.png";
    }

    if ( vm.count ( "debug" ) )
    {
        debug = true;
        std::cerr << "\x1b[01mDebug mode.\nPolling interval:\t" + std::to_string ( delay ) + "\x1b[00m\n";
    }
    else
        debug = false;


    bat = true;

    cont = !vm.count ( "run-once" );

    do
    {
        checkBattery ();
        sleep ( ( unsigned int ) delay );
    } while ( cont );

    return 0;
}

void checkBattery ()
{
    if (!boost::filesystem::exists ( "/sys/class/power_supply/BAT0" ) && !boost::filesystem::exists ( "/sys/class/power_supply/BAT1" ) )
    {
        if ( debug )
            std::cerr << "\x1b[01mNo BAT0 and No BAT1.\x1b[00m\n";
        if ( bat )
            // if there previously was a battery, notify that it has been removed
        {
            notify_init ( "battmon" );
            NotifyNotification *Batt;
            Batt = notify_notification_new ( "No Battery!", "No battery present!", BATT_CRIT );
            notify_notification_show ( Batt, NULL );
            g_object_unref ( G_OBJECT( Batt ) );
            notify_uninit ();
        }
        bat = false;
        return;
    }

    // This basically sets battery variable to a particular value depending upon which one is used.
    // It can be called as a bug/feature but if you have two batteries only battery 1 will be selected
    // next commit will probably fix this!
    
    boost::filesystem::exists ( "/sys/class/power_supply/BAT0" ) ? battery = 0 : battery = -1;
    boost::filesystem::exists ( "/sys/class/power_supply/BAT1" ) ? battery = 1 : battery = -1;
    
    if ( battery == 0 ) 
    {
        capacityfilepath = "/sys/class/power_supply/BAT0/capacity";
        statusfilepath = "/sys/class/power_supply/BAT0/status";
    }
    else if ( battery == 1 ) 
    {
        capacityfilepath = "/sys/class/power_supply/BAT1/capacity";
        statusfilepath = "/sys/class/power_supply/BAT1/status";
    }
    else 
    {
        std::cerr << "\x1b[01mNo BAT0 or BAT1.\x1b[00m\n";
    }
    bat = true;

    if ( debug )
        std::cerr << "Polling.\n";
    
    // read information from battery capacity and status files
    std::ifstream capfile ( capacityfilepath );
    std::ifstream statfile ( statusfilepath );
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
    if ( ccap == low && cstat != "Charging" )
    {

        if ( debug )
            std::cerr << "\x1b[01mLow battery threshold, notifying.\x1b[00m\n";

        std::string battext ( cstat + " " + std::to_string ( ccap ) + "%" );

        // Notification time
        notify_init ( "battmon" );
        NotifyNotification *Batt;
        Batt = notify_notification_new ( "Low Battery", battext.c_str (), BATT_LOW );
        notify_notification_show ( Batt, NULL );
        g_object_unref ( G_OBJECT( Batt ) );
        notify_uninit ();

    }
        // notify critical
    else if ( ccap == crit && cstat != "Charging" )
    {

        if ( debug )
            std::cerr << "\x1b[01mCritical battery threshold, notifying.\x1b[00m\n";

        std::string battext ( cstat + " " + std::to_string ( ccap ) + "%" );

        // Notification time
        notify_init ( "battmon" );
        NotifyNotification *Batt;
        Batt = notify_notification_new ( "Critical Battery", battext.c_str (), BATT_CRIT );
        notify_notification_show ( Batt, NULL );
        g_object_unref ( G_OBJECT( Batt ) );
        notify_uninit ();

    }
    else if ( cstat != pstat )
        // notify charging/discharging state
    {
        if ( debug )
            std::cerr << "\x1b[01mChange in charging state: \x1b[00m" << cstat << "\n";

        std::string battext ( cstat + " " + std::to_string ( ccap ) + "%" );

        // Notification time
        notify_init ( "battmon" );
        NotifyNotification *Batt;
        Batt = notify_notification_new ( cstat.c_str (), battext.c_str (), BATT_CHAR );
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
        const char *icon;

        //change icon according to charge level or if charging
        if ( cstat == "Charging" || cstat == "Full" )
            icon = BATT_CHAR;
        else if ( ccap >= 75 )
            icon = BATT_FULL;
        else
            icon = BATT_HIGH;

        // Notification time
        notify_init ( "battmon" );
        NotifyNotification *Batt;
        Batt = notify_notification_new ( "Battery", battext.c_str (), icon );
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
