target extended-remote localhost:3333
set remotetimeout 60000
set mem inaccessible-by-default off
mon ondisconnect cont
set arm force-mode thumb
