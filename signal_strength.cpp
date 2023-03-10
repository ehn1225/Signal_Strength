//20230206 Best of the Best 11th 이예찬
#include <pcap.h>
#include <stdbool.h>
#include <stdio.h>
#include "mac.h"
#include <map>

using namespace std;

struct ieee80211_radiotap_header {
    u_char it_version;
    u_char it_pad;
    u_int16_t it_len;
	u_int32_t it_present_flags;
};

struct ieee80211_radiotap_channel {
	u_int16_t ch_frequency;
	u_int16_t ch_flags;
	int8_t antenna_signal;	
};

struct ieee80211_beacon_frame{
	u_int16_t frameCtl;
	u_int16_t duration;
	Mac dstAddr;
	Mac srcAddr;
	Mac bssId;
};

struct ieee80211_wireless_management{
	u_int64_t timestamp;
	u_int16_t beaconInterval;
	u_int16_t capablityInfo;
	u_int8_t tagNumber;
	u_int8_t tagLength;
	unsigned char SSID[33]; //MAX length 32
};

void usage() {
	printf("syntax : signal-strength [interface] [ap mac]\n");
	printf("sample : signal-strength mon0 00:11:22:33:44:55\n");
}

int Calc_ch(int frequency){
	if(frequency >= 2412 && frequency <= 2484){	
		if (frequency == 2484)
			return (frequency-2412) /5;
		return (frequency-2412) /5 + 1;
	}
	else if( frequency >= 5170 && frequency <= 5825)
		return (frequency-5170) /5 + 34;
	else 
		return -1;
}

int main(int argc, char* argv[]) {
	if(argc != 3){
		usage();
		return 1;
	}

	Mac ap(argv[2]);
	char errbuf[PCAP_ERRBUF_SIZE];
	pcap_t* pcap = pcap_open_live(argv[1], BUFSIZ, 1, 1000, errbuf);
	if (pcap == NULL) {
		fprintf(stderr, "pcap_open_live(%s) return null - %s\n", argv[1], errbuf);
		return -1;
	}

	bool once = true;
	unsigned short ch_offset = 4;
	char SSID[33] = {0, };
	while (true) {
		struct pcap_pkthdr* header;
		const unsigned char* packet;
		int res = pcap_next_ex(pcap, &header, &packet);
		if (res == 0) continue;
		if (res == PCAP_ERROR || res == PCAP_ERROR_BREAK) {
			printf("pcap_next_ex return %d(%s)\n", res, pcap_geterr(pcap));
			break;
		}
		struct ieee80211_radiotap_header *radiotap = (struct ieee80211_radiotap_header *)packet;
		struct ieee80211_beacon_frame *beaconframe = (struct ieee80211_beacon_frame *)(packet + radiotap->it_len);
		struct ieee80211_radiotap_channel *radiotap_ch = NULL;

		//Beacon 프레임인지 Type 확인
		if(ntohs(beaconframe->frameCtl) != 0x8000){
			continue; 
		}

		//BSSID가 내가 선택한 것인지
		if(beaconframe->srcAddr != ap){
			continue; 
		}
		if(once){
			//802.11 Radiotap에서 Radiotap 해더의 길이를 가져오고, 이를 통해 다음 레이어로 넘어감
			struct ieee80211_wireless_management *wireless_mgr = (struct ieee80211_wireless_management *)(packet + radiotap->it_len + 24);

			//Channel frequency Offset from packet[0]
			u_int32_t present_flags = radiotap->it_present_flags;

			bool TSFT = false;
			//MAC timestamp flag
			if(TSFT = (present_flags & 0x1))
				ch_offset += 8;

			//Flags flag
			if(present_flags & 0x2)
				ch_offset += 1;

			//Data Rate flag
			if(present_flags & 0x4)
				ch_offset += 1;

			//Channel flag
			if(!(present_flags & 0x8)){
				continue;
			}
			
			//Handling Extended Presence masks
			int count = 1;
			while(present_flags & 0x80000000){
				memcpy((char*)&present_flags, packet + 4 * (count + 1), 4);
				count++;
			}
			ch_offset += (4 * count);

			//Alignment in Radiotap
			if(count == 2 && TSFT)
				ch_offset += 4;
			radiotap_ch = (struct ieee80211_radiotap_channel *)(packet + ch_offset);
			//Channel 정보 파싱
			wireless_mgr->SSID[wireless_mgr->tagLength] = '\0';
			memcpy(SSID, wireless_mgr->SSID, 33);
			once = false;
			printf("SSID : %s (%s), ch : %3d(%dMHz)\n", SSID, string(beaconframe->srcAddr).c_str(), Calc_ch(radiotap_ch->ch_frequency), radiotap_ch->ch_frequency);
		}
		radiotap_ch = (struct ieee80211_radiotap_channel *)(packet + ch_offset);

		//화면 출력
		printf("PWR : %d\r", radiotap_ch->antenna_signal);
		fflush(stdout);
	}

	pcap_close(pcap);
}