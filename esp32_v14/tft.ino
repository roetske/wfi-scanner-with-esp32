

//symbool lock en unlock graphic
static const uint8_t lock_bitmap[] PROGMEM  = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x02, 0x40, 0x04, 0x20, 0x04, 0x20, 0x04, 0x20, 0x04, 0x20, 0x0f, 0xf0, 0x1f, 0xf8, 0x1e, 0x78, 0x1e, 0x78, 0x1f, 0xf8, 0x0f, 0xf0, 0x00, 0x00 };
static const uint8_t unlock_bitmap[] PROGMEM  = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x48, 0x00, 0x84, 0x00, 0x84, 0x00, 0x84, 0x00, 0x04, 0x00, 0x0f, 0xf0, 0x1f, 0xf8, 0x1e, 0x78, 0x1e, 0x78, 0x1f, 0xf8, 0x0f, 0xf0, 0x00, 0x00 };


void tft_espi_scanning()
{
  tft.fillScreen(TFT_BLACK);
  tft.fillRect(0, 0, 240, 16, TFT_LIGHTGREY);
  tft.fillRect(0, 304, 240, 16, TFT_LIGHTGREY);
  tft.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
  tft.drawString("ESP32 WiFi Analyzer", 4, 4, TEXT_FONT);

  tft.fillRect(0, 16, 240, 288, TFT_BLACK);
  tft.setTextDatum(BC_DATUM);
  tft.setTextPadding(240);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.drawString("Scanning...", 120, 172, 4);
  tft.setTextDatum(L_BASELINE);

}

//*************************************************
void bigSignalIndicator(int rssi)
 {
  uint16_t color = TFT_GREEN;
  int rssi2 = rssi;

  if (rssi <= -66) color = TFT_YELLOW;
  if (rssi < -88) color = TFT_RED;
  if (rssi <= -95) color = TFT_DARKGREY;

  rssi2 = abs(rssi);
  rssi2 = (rssi2 - 35) * 2;
  rssi2 = 144 - rssi2;
  if (rssi2 < 24) rssi2 = 24;
  if (rssi2 > 144) rssi2 = 144;

  for (int i = 0; i < 120; i += 2) {
    if ((i < rssi2) && (i % 10 != 0)) tft.fillRect(120 - i, 144 - i, i * 2 , 2, color);
    else tft.fillRect(120 - i, 144 - i, i * 2 , 2, TFT_DARKGREY);
  }
  String srssi = String(rssi);
  if (rssi == -100) srssi = "  --";

  tft.setTextDatum(L_BASELINE);
  tft.setTextPadding(50);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(srssi, 4, 136, 4);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString("dBm", 44, 129, TEXT_FONT);

}

//******************************************
void displaySelectionMarker(int pos)
{
  pos = pos % 9;
  int prevPos = pos - 1;
  if (prevPos == -1) prevPos = 8;

  int yPos = 16 + (32 * pos);
  int yPrevPos = 16 + (32 * prevPos);

  int xPos = 227;

  // clear previous selection marker
  tft.fillTriangle(xPos, yPrevPos + 10, xPos, yPrevPos + 22, xPos + 12, yPrevPos + 16, TFT_BLACK);

  // set current selection marker
  tft.fillTriangle(xPos, yPos + 10, xPos, yPos + 22, xPos + 12, yPos + 16, TFT_LIGHTGREY);
}
//***** Signal indicator on network list screen *****/
void signalIndicator(int32_t s, int xpos, int ypos, uint16_t color) {
  tft.fillRect(xpos, ypos + 12, 2, 3, color);

  if (s < -88) tft.fillRect(xpos + 3, ypos + 9, 2, 6, TFT_DARKGREY);
  else tft.fillRect(xpos + 3, ypos + 9, 2, 6, color);

  if (s < -78) tft.fillRect(xpos + 6, ypos + 6, 2, 9, TFT_DARKGREY);
  else tft.fillRect(xpos + 6, ypos + 6, 2, 9, color);

  if (s <= -66) tft.fillRect(xpos + 9, ypos + 3, 2, 12, TFT_DARKGREY);
  else tft.fillRect(xpos + 9, ypos + 3, 2, 12, color);

  if (s < -55) tft.fillRect(xpos + 12, ypos, 2, 15, TFT_DARKGREY);
  else tft.fillRect(xpos + 12, ypos, 2, 15, color);
}

//*************************************************
void selectNextNetwork() 

{
  bool loadMoreAPs = false; // true when reached the end of current list and new APs should be plotted

  selectedAP++;
  
  
  if (selectedAP >= APfound) {
    selectedAP = 0;
    loadMoreAPs = true;
  }
  
  if (selectedAP % 9 == 0) loadMoreAPs = true;
  if (loadMoreAPs) networksListScreen();

  displaySelectionMarker(selectedAP);
  
  //load data here
  selectedBSSID   = idx_rssi[selectedAP].bssidstr;
  selectedChannel = idx_rssi[selectedAP].channel;
  selectedSSID = idx_rssi[selectedAP].ssid;
  selectedRSSI = idx_rssi[selectedAP].rssi;
  selectedAUTH = encriptionTostring(selectedAP);

}

//******************************************
void displayNetwork(int i, int pos)
 {
  int ypos = 20 + (pos * 32);
  const int xoff = 4;
  uint16_t color;

  String buf;

  // ROW 1
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  buf = idx_rssi[i].ssid + " (" + idx_rssi[i].bssidstr + ")";
  tft.drawString(buf.c_str(), xoff, ypos, TEXT_FONT);

  // ROW 2
  int32_t rssi = idx_rssi[i].rssi;
  if (rssi < -88) color = TFT_RED;
  else if (rssi > -66) color = TFT_GREEN;
  else color = TFT_YELLOW;

  signalIndicator(rssi, xoff, ypos + 12, color);
  tft.setTextColor(color, TFT_BLACK);
  buf = String(rssi) + "dBm";
  tft.drawString(buf.c_str(), xoff + 16, ypos + 16, TEXT_FONT);

  // ROW 2, channel
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString("CH", xoff + 64, ypos + 16, TEXT_FONT);

  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  buf = String(idx_rssi[i].channel );
  tft.drawString(buf, xoff + 80, ypos + 16, TEXT_FONT);
  byte enc = idx_rssi[i].encrypt;
  // ROW 2, encryption
  switch (enc) 
   {
      case WIFI_AUTH_OPEN:  
        buf = "WIFI_AUTH_OPEN";
        break;
      case  WIFI_AUTH_WPA_PSK:  
       buf = "WIFI_AUTH_WPA_PSK";
        break;
      case  WIFI_AUTH_WPA2_PSK:  
        buf = "WIFI_AUTH_WPA2_PSK";
        break;
      case WIFI_AUTH_WPA_WPA2_PSK:
       buf = "WIFI_AUTH_WPA_WPA2_PSK";
        break;
      case WIFI_AUTH_WPA2_ENTERPRISE:  
       buf = "WIFI_AUTH_WPA2_ENTERPRISE";
        break;
      case WIFI_AUTH_WPA3_PSK:  
        buf = "WIFI_AUTH_WPA3_PSK";
        break;
      case WIFI_AUTH_WPA2_WPA3_PSK:  
        buf ="WIFI_AUTH_WPA2_WPA3_PSK";
        break;
      case WIFI_AUTH_WAPI_PSK:  
        buf = "WIFI_AUTH_WAPI_PSK";
        break;
      default:
          buf = "Unknown enc";
          break;
   }
  
  // Serial.print("buf: ");
  // Serial.println(buf);
  
  if (buf == "WIFI_AUTH_OPEN")
    {tft.drawBitmap(xoff + 96, ypos + 10, unlock_bitmap, 16, 16, TFT_OLIVE);}
  else 
    {tft.drawBitmap(xoff + 96, ypos + 10, lock_bitmap, 16, 16, TFT_MAROON);}

  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString(buf, xoff + 112, ypos + 16, TEXT_FONT);
}

//*************************************************
void networksListScreen()
 {
  int displayedAPs = 0;
  String msg;
  
  tft.setTextSize(1);
  tft.setTextDatum(L_BASELINE);
  tft.setTextPadding(0);

  displayedAPs = (selectedAP / 9) * 9;

  int maxCnt = min(displayedAPs + 10, APfound);
  if (displayedAPs + 1 == maxCnt)
    msg = "Showing network " + String(maxCnt) + " of " + String(APfound) + ".";
  else
    msg = "Showing networks " + String(displayedAPs + 1) + '-' + String(maxCnt) + " of " + String(APfound) + ".";
  tft.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
  tft.setTextPadding(220);
  tft.drawString(msg.c_str(), 4, 308, TEXT_FONT);

  tft.fillRect(0, 16, 240, 288, TFT_BLACK);

  for (int i = 0; i < 9; i++) {
    int remaining = APfound - displayedAPs;
    if (i >= remaining) break;
    displayNetwork(idx_rssi[i + displayedAPs].idx, i);
  }
}

//single ap show
void SelectedAPshow()
  {  //blank screen 
      tft.fillScreen(TFT_BLACK);
     
      String msg ="";
      //RSSI
      tft.fillRect(224, 304, 16, 16, TFT_LIGHTGREY);
      bigSignalIndicator(selectedRSSI);
		  tft.setTextDatum(BC_DATUM);
      tft.setTextPadding(240);
      
      //ssid
      msg = selectedSSID;
      tft.setTextDatum(L_BASELINE);
      tft.setTextSize(2);
      tft.setTextPadding(40);//erase old data
     
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.drawString(msg, 4, 184, TEXT_FONT);
      //turnback to textsize 1
      tft.setTextSize(1);
      //channel
      int frq = selectedChannel * 5 + 2407;
      if (frq == 2477) frq = 2484;
      msg = "Channel " + String(selectedChannel) + " (" + String(frq) + "MHz)";
      tft.setTextDatum(L_BASELINE);
      tft.setTextPadding(40);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.drawString(msg, 4, 216, TEXT_FONT);

      //auth
      msg = selectedAUTH;
      tft.setTextDatum(L_BASELINE);
      tft.setTextPadding(40);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.drawString(msg,4, 232, TEXT_FONT);

      //BSSID
      msg = selectedBSSID;
      tft.setTextDatum(L_BASELINE);
      tft.setTextPadding(40);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.drawString(msg,4, 248, TEXT_FONT); 
      
    }

void refreshtft()
{
  /***** LIST DISPLAY *****/
      if (!singleNetworkDisplay && dataloaded  )
        {   Serial.println("Refresh List Full APscan");
            networksListScreen();
            displaySelectionMarker(selectedAP);
          
        }
      /***** SINGLE AP SCAN *****/
       
      if (dataloaded && singleNetworkDisplay )
        { Serial.println("Refresh List Single APscan");
          SelectedAPshow();  
        }

}