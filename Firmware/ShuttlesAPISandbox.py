import requests
import math
import json
import time

direction_Harvard = "0" # To Harvard
stop_id_Beacon = "95" # Mass Ave / Beacon St. 
route = "1" #1 Bus


def GetETAMIT():
    MITSaferideApi = "https://passiogo.com/mapGetData.php?eta=3&deviceId=34241398&stopIds=992&routeId=45003&position=8&userId=94&routeIds=332,45003"
    response = requests.get(MITSaferideApi)
    print(response.text + "\n")
    if(response.status_code != 200):
        return "Error " + str(response.status_code)
    response_data = json.loads(response.text)
    ETATimes = []
    for routes in response_data["ETAs"]:
        print() 
        ETATimes.append(response_data["ETAs"][routes][0].get("eta"))
    return ETATimes
         

def GetTripIDsMBTA(pageLimit = "100", route = "1", direction = direction_Harvard):
    apiEndpoint = "https://api-v3.mbta.com/trips?page%5Blimit%5D=" + pageLimit + "&fields%5Btrip%5D=predictions&filter%5Bdirection_id%5D=" + direction + "&filter%5Broute%5D=" + route + "&filter%5Bdate%5D=2024-02-05"
    response = requests.get(apiEndpoint)
    print(response.text)
    if(response.status_code != 200):
        return "Error " + str(response.status_code)
    response_data = json.loads(response.text)
    TripIDs = []
    for trips in response_data["data"]:
        if(trips.get("id", None) != "Hi"):
            TripIDs.append(trips.get("id", None))
    return TripIDs
    

def GetPredictionFromIDMBTA(tripID, stop_id = stop_id_Beacon, route = "1", direction = direction_Harvard):
    apiEndpoint = "https://api-v3.mbta.com/predictions?filter%5Bdirection_id%5D=" + direction + "&filter%5Bstop%5D=" + stop_id + "&filter%5Broute%5D=" + route + "&filter%5Btrip%5D=" + tripID
    print(apiEndpoint)
    response = requests.get(apiEndpoint)
    if(response.status_code != 200):
        return "Error " + str(response.status_code)
    response_data = json.loads(response.text)
    for prediction in response_data["data"]:
        attributes = prediction["attributes"]
        stop = prediction.get("relationships", None).get("stop", None).get("data", None)
        if stop.get("id", None) == "95" and attributes.get("arrival_time", None):
            arrival_time = attributes.get("arrival_time", None)
            return arrival_time[11:(11+5)]
    return "Error: No prediction found"

if __name__ == "__main__":
    TripIDs = GetTripIDsMBTA()
    print(TripIDs)
    ETAs = GetETAMIT()
    print(ETAs)
    
    # for tripID in TripIDs:
    #     print(f"Arrival time for trip {tripID}, stop_sequence 95: {GetPredictionFromIDMBTA(tripID)}")
    #     time.sleep(0.1)
    
    
    