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

#define BATT_FULL   "battery"
#define BATT_LOW    "battery-caution"
#define BATT_CRIT   "battery-low"

#define VERSION     "0.1.1alpha"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

void checkBattery ();

namespace  pOpt = boost::program_options;

int delay;  // polling interval
int crit;   // critical battery level
int low;    // low battery level
int ccap;   // current batt capacity percent
int pcap;   // previous batt capacity percent
std::string cstat; // current batt status

bool debug;

int main ( const int argc, const char *argv[] )
{

    ccap = 0;
    pcap = 0;

    std::string odes ( "Batmon, a lightweight battery monitor in C++. Version " );
    odes.append ( VERSION );

    // parse options from command line
    pOpt::options_description desc ( odes + "\n\nOptions" );
    desc.add_options ()
            ( "help,h", "Show this help message." )
            ( "interval,i", pOpt::value < int > ( &delay )->default_value ( 1 ),
              "Polling interval in s (default = 1s)" )
            ( "low,l", pOpt::value < int > ( &low )->default_value ( 15 ),
              "Low battery threshold in percent (default = 15%)" )
            ( "critical,c", pOpt::value < int > ( &crit )->default_value ( 5 ),
              "Critical battery thresholf in percent (default = 5%)" )
            ( "debug,D", "Debug mode." );

    pOpt::variables_map vm;
    pOpt::store ( pOpt::parse_command_line ( argc, argv, desc ), vm );
    pOpt::notify ( vm );

    if ( vm.count ( "help" ) )
    {
        std::cout << desc << "\n";
        return 1;
    }

    if ( vm.count ( "debug" ) )
        debug = true;
    else
        debug = false;

    if ( debug )
        std::cerr << "Debug mode.\nPolling interval:\t" + std::to_string ( delay ) + "\n";


    while ( true )
    {
        checkBattery ();

    }
}

void checkBattery ()
{
    if ( !boost::filesystem::exists ( "/sys/class/power_supply/BAT0" ) )
    {

        if ( debug )
            std::cerr << "No battery.\n";

        sleep ( delay );
        return;
    }

    if ( debug )
        std::cerr << "Polling.\n";

    std::ifstream capfile ( "/sys/class/power_supply/BAT0/capacity" );
    std::ifstream statfile ( "/sys/class/power_supply/BAT0/status" );
    capfile >> ccap;
    statfile >> cstat;

    if ( debug )
        std::cerr << "Battery: " + std::to_string ( ccap ) + "%\n";

    if ( ccap % 10 == 0 )
    {
        // notify every 10%, but not if battery charge is not changing

        if ( ccap == pcap )
        {
            if ( debug )
                std::cerr << "No change.\n";

            statfile.close ();
            capfile.close ();
            sleep ( delay );
            return;
        }

        if ( debug )
            std::cerr << "Change, notifying.\n";

        std::string battext ( cstat + " " + std::to_string ( ccap ) + "%" );

        // Notification time
        notify_init ( "Battery" );
        NotifyNotification *Batt;
        Batt = notify_notification_new ( "Battery", battext.c_str (), BATT_FULL );
        notify_notification_show ( Batt, NULL );
        g_object_unref ( G_OBJECT( Batt ) );
        notify_uninit ();

    }

    statfile.close ();
    capfile.close ();
    sleep ( delay );
    pcap = ccap;
    return;
}

#pragma clang diagnostic pop