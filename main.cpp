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
#include <boost/program_options/option.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>


namespace  pOpt = boost::program_options;

int delay;  // polling interval
int crit;   // critical battery level
int low;    // low battery level
int ccap;   // current batt capacity percent
int pcap;   // previous batt capacity

int main ( const int argc, const char *argv[] )
{

    ccap = 0;
    pcap = 0;

    // parse options from command line
    pOpt::options_description desc ( "Batmon, a lightweight battery monitor in C++. \n\nOptions" );
    desc.add_options ()
            ( "help,h", "Show this help message." )
            ( "interval,i", pOpt::value < int > ( &delay )->default_value ( 1000 ),
              "Polling interval in ms (default = 1000ms)" )
            ( "low,l", pOpt::value < int > ( &low )->default_value ( 15 ),
              "Low battery threshold in percent (default = 15%)" )
            ( "critical,c", pOpt::value < int > ( &crit )->default_value ( 5 ),
              "Critical battery thresholf in percent (default = 5%)" );

    pOpt::variables_map vm;
    pOpt::store ( pOpt::parse_command_line ( argc, argv, desc ), vm );
    pOpt::notify ( vm );

    if ( vm.count ( "help" ))
    {
        std::cout << desc << "\n";
        return 1;
    }

    std::ifstream capfile ( "/sys/class/power_supply/BAT0/capacity" );
    std::ifstream statfile ( "/sys/class/power_supply/BAT0/status" );


    capfile >> ccap;
    std::string s_cap ( "Battery: " + std::to_string ( ccap ) + "%" );

    notify_init ( "Hello world!" );
    NotifyNotification *Hello;
    Hello = notify_notification_new ( "Hello world", s_cap.c_str (), "face-kiss" );
    notify_notification_show ( Hello, NULL );
    g_object_unref (G_OBJECT( Hello ));
    notify_uninit ();

    capfile.close ();

    return 1;
}