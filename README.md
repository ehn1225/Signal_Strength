<h1>Signal Strength</h1>

<h3>Usage</h3>
syntax : signal-strength [interface] [ap mac]<br>
sample : signal-strength mon0 00:11:22:33:44:55<br>

<h3>Before Execute</h3>
sudo ifconfig wlan0 down<br>
sudo iwconfig wlan0 mode monitor<br>
sudo ifconfig wlan0 up<br>

<h3>새로고침 주기를 빠르게 하는 법</h3>
sudo airmon-ng check kill<br>
sudo airodump-ng wlan0              //타겟의 채널 정보 및 BSSID 확인<br>
sudo airmon-ng start wlan0 6        //타겟의 채널 입력<br>
//아래 항목은 위의 항목이 안될 때 시도<br>
//sudo iwconfig wlan0 channel 6 #<br>
//sudo ifconfig wlan0 up<br>

<p>https://gitlab.com/gilgil/sns/-/wikis/deauth-attack/report-deauth-attack</p>
