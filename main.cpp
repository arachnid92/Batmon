#include <iostream>
#include <libnotify/notify.h>

using namespace std;

int main ()
{
    cout << "Hello, World!" << endl;

    notify_init ( "Hello world!" );
    NotifyNotification *Hello;
    Hello = notify_notification_new ( "Hello world", "Battery: 100%", "face-kiss" );
    notify_notification_show ( Hello, NULL );
    g_object_unref (G_OBJECT( Hello ));
    notify_uninit ();

    return 0;
}