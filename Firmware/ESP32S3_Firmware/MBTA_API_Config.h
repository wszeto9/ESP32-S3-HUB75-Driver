#ifndef MBTA_API_Config_h
#define MBTA_API_Config_h

const char *ssid = "TheySeeMeScrollin";

// MBTA Route 66 at N Harvard St @ Oxford St.
// Stop ID 2559: N Harvard St @ Oxford St.
// Route 66: Harvard Square - Nubian Station.
const char *MBTA_ROUTE_ID = "66";
const char *MBTA_STOP_ID = "2559";
String apiEndpointMBTA = "https://api-v3.mbta.com/predictions?filter%5Broute%5D=66&filter%5Bstop%5D=2559&page%5Blimit%5D=10";

#endif
