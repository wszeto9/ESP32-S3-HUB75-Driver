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

String apiEndpoint = "https://api-v3.mbta.com/predictions?page%5Boffset%5D=0&page%5Blimit%5D=100&sort=arrival_time&filter%5Blatitude%5D=" + String(lat) + "&filter%5Blongitude%5D=" + String(longitude) + "&filter%5Bradius%5D=" + String(radius) + "&filter%5Bstop%5D=95&filter%5Broute%5D=1";


class mclass(
  public:
  mclass();
  void SETUP();
  void GetArrivalTImes();
);

extern mclass MBTA_API;
#endif