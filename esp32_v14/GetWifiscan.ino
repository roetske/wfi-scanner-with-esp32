
//subs
void Load_data_apscan()
{
     // refresh data na new scan
       for (int i = 0; i < nNet; i++)
       {
        idx_rssi[i].idx = i;
        idx_rssi[i].bssidstr = WiFi.BSSIDstr(i);
        idx_rssi[i].ssid = WiFi.SSID(i);
        idx_rssi[i].rssi = WiFi.RSSI(i);
        idx_rssi[i].channel = WiFi.channel(i);
        idx_rssi[i].encrypt = WiFi.encryptionType(i);
       } 
}
//get encryption to string
String  encriptionTostring(int index)
{ String result ="Unknown";
  //encryption byte to readable string
  byte encryptionTypeDescription = idx_rssi[index].encrypt;
    
    switch (encryptionTypeDescription) {
      case WIFI_AUTH_OPEN:  
       result = "WIFI_AUTH_OPEN";
        break;
      case  WIFI_AUTH_WPA_PSK:  
       result = "WIFI_AUTH_WPA_PSK";
        break;
      case  WIFI_AUTH_WPA2_PSK:  
        result = "WIFI_AUTH_WPA2_PSK";
        break;
      case WIFI_AUTH_WPA_WPA2_PSK:
        result = "WIFI_AUTH_WPA_WPA2_PSK";
        break;
      case WIFI_AUTH_WPA2_ENTERPRISE:  
       result = "WIFI_AUTH_WPA2_ENTERPRISE";
        break;
      case WIFI_AUTH_WPA3_PSK:  
        result = "WIFI_AUTH_WPA3_PSK";
        break;
      case WIFI_AUTH_WPA2_WPA3_PSK:  
        result = "WIFI_AUTH_WPA2_WPA3_PSK";
        break;
      case WIFI_AUTH_WAPI_PSK:  
       result = "WIFI_AUTH_WAPI_PSK";
        break;
      default:
          result ="Wifi encoding unknown.";
          break;
      }

  return result;
}
//***************************************
//main code for getting scanning results
//***************************************
//starnewscan is trigger
void v_scannetwork()
{ //startnewscan starts scanning
  //startscan trigger
  if (!scanbusy && startnewscan)
   { //wifi scan async trigger
     WiFi.scanDelete();
     delay(500);
     WiFi.scanNetworks(true,true); //async and hidden true
     //Serial.println("Scanning async started .....");
     startnewscan = false;
     scanbusy = true;
     scandone = false;
     newscandataready = false;
     //timer to check scan duration
     emWifiscan = 0;
     }
  //check results
  if (scanbusy )
   {
     nNet = WiFi.scanComplete();
     if (nNet > 0)
      {//scancomplete
      //  Serial.println("Scanning Complete");
      //  Serial.println(nNet);
      //  Serial.print("Scanduration in ms:=");
      //  Serial.println( emWifiscan);
       scanbusy= false;
       scandone = true;
      }
    if (nNet == 0)
     { APfound = 99; //special nothing found
       scanbusy= false; //allow rescan
       dataloaded = false; //data is irrelevant
       Serial.println("No networksfound");
       Serial.println("WiFi.scanComplete():=");
       Serial.println(nNet);
     }
   }
   //write in memory if scandone (trigger)
   if (scandone && !scanbusy && nNet > 0)
    { 
      if (!newscandataready && scandone)
         { dataloaded = false;
           Load_data_apscan();
           delay(100);//just to be sure :)
           newscandataready = true;
         }
    }
   //dataready set marker results loaded in mem
   if (newscandataready && !dataloaded)
    { //this bool as condition load data to tft
      dataloaded = true;
      APfound = nNet;
      //start timer for refresh
      emrefreshtft =0;
    } 
}