#ifndef MBTA_API_Config_h
#define MBTA_API_Config_h

const char *ssid = "MIT";



float longMin = -71.083712;
float latMin = 42.328519;

float longMax = -71.089429;
float latMax = 42.350860;

float longitude = (longMin + longMax) / 2 ;
float lat = (latMin + latMin) / 2;
float radius = 1.5 * sqrt((longMin - longMax)* (longMin - longMax) +(latMin - latMax) * (latMin - latMax));

String apiEndpointMBTA = "https://api-v3.mbta.com/predictions?page%5Blimit%5D=100&sort=status&filter%5Bstop%5D=95";
String apiEndpointMIT = "https://passiogo.com/mapGetData.php?eta=3&deviceId=34241398&stopIds=992&userId=94";
String apiEndpointMITMorning = "https://passiogo.com/mapGetData.php?eta=3&deviceId=34241398&stopIds=992&routeId=332&position=10&userId=94";
#endif