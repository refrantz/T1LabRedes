#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>

#include <net/if.h>  //estrutura ifr
#include <netinet/ether.h> //header ethernet
#include <netinet/in.h> //definicao de protocolos
#include <arpa/inet.h> //funcoes para manipulacao de enderecos IP

#include <netinet/in_systm.h> //tipos de dados

#define BUFFSIZE 1518
#define NUM_PORTS 65536


  unsigned char buff1[BUFFSIZE]; // buffer de recepcao

  int sockd;
  int on;
  struct ifreq ifr;
  int packetType;
  int ipProtocol;
  int arpOpCode;
  int icmpType;
  int destinationport;
  int sourceport;
  int possibleDNS;

  int maxsize = -1; //
  int minsize = -1; //
  int sumSizes = 0; //

  int totalcount = 0; //

  int arprequestcount = 0; // 
  int arpreplycount = 0; //

  int ipv4count = 0; //
  int icmpcount = 0; //
  int icmprequestcount = 0; //
  int icmpreplycount = 0; //
  int ipv6count = 0; //
  int icmpv6count = 0; //
  int icmpv6requestcount = 0; //
  int icmpv6replycount = 0; //

  int udpcount = 0; //
  int tcpcount = 0; //

  int httpcount = 0;
  int dnscount = 0;
  int dhcpcount = 0;

  int sourceportUDP[NUM_PORTS] = {0}; //
  int destinationportUDP[NUM_PORTS] = {0}; //
  int sourceportTCP[NUM_PORTS] = {0}; //
  int destinationportTCP[NUM_PORTS] = {0}; //


void updateMax(int max[5][2], int i, int port[]){

	if (max[0][0] < port[i]){

		max[4][0] = max[3][0];
		max[3][0] = max[2][0];
		max[2][0] = max[1][0];
		max[1][0] = max[0][0];
		max[0][0] = port[i];

		max[4][1] = max[3][1];
		max[3][1] = max[2][1];
		max[2][1] = max[1][1];
		max[1][1] = max[0][1];
		max[0][1] = i;

	}else if (max[1][0] < port[i]){

		max[4][0] = max[3][0];
		max[3][0] = max[2][0];
		max[2][0] = max[1][0];
		max[1][0] = port[i];

		max[4][1] = max[3][1];
		max[3][1] = max[2][1];
		max[2][1] = max[1][1];
		max[1][1] = i;

	}else if (max[2][0] < port[i]){

		max[4][0] = max[3][0];
		max[3][0] = max[2][0];
		max[2][0] = port[i];

		max[4][1] = max[3][1];
		max[3][1] = max[2][1];
		max[2][1] = i;

	}else if (max[3][0] < port[i]){

		max[4][0] = max[3][0];
		max[3][0] = port[i];

		max[4][1] = max[3][1];
		max[3][1] = i;

	}else if (max[4][0] < port[i]){

		max[4][0] = port[i];

		max[4][1] = i;

	}

}


int main(int argc,char *argv[])
{
    /* Criacao do socket. Todos os pacotes devem ser construidos a partir do protocolo Ethernet. */
    /* De um "man" para ver os parametros.*/
    /* htons: converte um short (2-byte) integer para standard network byte order. */
    if((sockd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) {
       printf("Erro na criacao do socket.\n");
       exit(1);
    }

	// O procedimento abaixo eh utilizado para "setar" a interface em modo promiscuo
	strcpy(ifr.ifr_name, "eth0");
	if(ioctl(sockd, SIOCGIFINDEX, &ifr) < 0)
		printf("erro no ioctl!\n");
		//exit(1);
	ioctl(sockd, SIOCGIFFLAGS, &ifr);
	ifr.ifr_flags |= IFF_PROMISC;
	ioctl(sockd, SIOCSIFFLAGS, &ifr);

	int loops;
	printf("Insira o numero de loops: ");
	scanf("%d", &loops);

	// recepcao de pacotes
	for(int i = 0; i < loops; i++) {
   		int size = recv(sockd,(char *) &buff1, sizeof(buff1), 0x0);

		sumSizes += size;

		if(size > maxsize){
			maxsize = size;
		}
		if(size < minsize || minsize == -1){
			minsize = size;
		}

		totalcount++;

		packetType = (buff1[12] << 8) | buff1[13];

		//ipv4
		if(packetType == 0x0800){
			ipProtocol = buff1[23];
			ipv4count++;

			sourceport = (buff1[34]<<8) | buff1[35];
			destinationport = (buff1[36]<<8) | buff1[37];

			//dns
			if(sourceport == 53 || destinationport == 53){
				dnscount++;

			//dhcp
			}else if((sourceport == 68 && destinationport == 67) || (sourceport == 67 && destinationport == 68)){
				dhcpcount++;
			
			//http
			}else if(sourceport == 80 || destinationport == 80){
				httpcount++;

			}

			//TCP
			if(ipProtocol == 0x06){
				sourceportTCP[sourceport]++;
  				destinationportTCP[destinationport]++;
				
				tcpcount++;
			}else

			//UDP
			if(ipProtocol == 0x11){
				sourceportUDP[sourceport]++;
  				destinationportUDP[destinationport]++;
				
				udpcount++;

			}else

			//ICMP
			if(ipProtocol == 0x01){

				icmpType = buff1[34];
				icmpcount++;

				//request
				if(icmpType == 0x08){
					icmprequestcount++;
				}

				//reply
				else if(icmpType == 0x00){
					icmpreplycount++;
				}
			}

		}else

		//ipv6
		if(packetType == 0x86DD){
			ipProtocol = buff1[20];
			ipv6count++;

			sourceport = (buff1[54]<<8) | buff1[55];
			destinationport = (buff1[56]<<8) | buff1[57];

			//dns
			if(sourceport == 53 || destinationport == 53){
				dnscount++;

			//dhcp
			}else if((sourceport == 68 && destinationport == 67) || (sourceport == 67 && destinationport == 68)){
				dhcpcount++;
			
			//http
			}else if(sourceport == 80 || destinationport == 80){
				httpcount++;

			}

			//TCP
			if(ipProtocol == 0x06){
				
				sourceportTCP[sourceport]++;
  				destinationportTCP[destinationport]++;
				
				tcpcount++;
			}else

			//UDP
			if(ipProtocol == 0x11){
				
				sourceportUDP[sourceport]++;
  				destinationportUDP[destinationport]++;
				
				udpcount++;
			}else

			//ICMPv6
			if(ipProtocol == 0x3a){

				icmpType = buff1[54];
				icmpv6count++;

				//request
				if(icmpType == 0x80){
					icmpv6requestcount++;
				}

				//reply
				else if(icmpType == 0x81){
					icmpv6replycount++;
				}
			}
		}

		//arp
		if(packetType == 0x0806){
			arpOpCode = (buff1[20]<<8) | buff1[21];

			//request
			if(arpOpCode == 0x0001){
				arprequestcount++;
			}

			//reply
			if(arpOpCode == 0x0002){
				arpreplycount++;
			}
		}
		
	}

	int maxSourceTCP[5][2] = {0};
	int maxSourceUDP[5][2] = {0};
	int maxDestinationTCP[5][2] = {0};
	int maxDestinationUDP[5][2] = {0};

	for(int i = 0; i<NUM_PORTS; i++){

		updateMax(maxSourceTCP, i, sourceportTCP);

		updateMax(maxSourceUDP, i, sourceportUDP);

		updateMax(maxDestinationTCP, i, destinationportTCP);

		updateMax(maxDestinationUDP, i, destinationportUDP);
	}

	 printf("======================================\n");
	 printf("max packet size: %d \n", maxsize);
	 printf("min packet size: %d \n",minsize);
	 printf("sum of packet sizes: %d \n",sumSizes);
	 printf("--------------------------------------\n");
	 printf("total count of packets: %d \n",totalcount);
	 printf("--------------------------------------\n");
	 printf("count of arp requests: %d | percentage of arp requests: %.2f \n",arprequestcount, (float)arprequestcount/totalcount*100); 
	 printf("count of arp replies: %d | percentage of arp replies: %.2f \n",arpreplycount, (float)arpreplycount/totalcount*100); 
	 printf("--------------------------------------\n");
	 printf("count of ipv4 packets: %d | percentage of ipv4: %.2f \n",ipv4count, (float)ipv4count/totalcount*100); 
	 printf("count of icmp packets: %d | percentage of icmp: %.2f \n",icmpcount, (float)icmpcount/totalcount*100); 
	 printf("count of icmp requests: %d | percentage of icmp requests: %.2f \n",icmprequestcount, (float)icmprequestcount/totalcount*100); 
	 printf("count of icmp replies: %d | percentage of icmp replies: %.2f \n",icmpreplycount, (float)icmpreplycount/totalcount*100);
	 printf("--------------------------------------\n");
	 printf("count of ipv6 packets: %d | percentage of ipv6: %.2f \n",ipv6count, (float)ipv6count/totalcount*100); 
	 printf("count of icmpv6 packets: %d | percentage of icmpv6: %.2f \n",icmpv6count, (float)icmpv6count/totalcount*100); 
	 printf("count of icmpv6 requests: %d | percentage of icmp requests: %.2f \n",icmpv6requestcount, (float)icmpv6requestcount/totalcount*100); 
	 printf("count of icmpv6 replies: %d | percentage of icmp replies: %.2f \n",icmpv6replycount, (float)icmpv6replycount/totalcount*100); 
	 printf("--------------------------------------\n");
	 printf("count of udp packets: %d | percentage of udp: %.2f \n",udpcount, (float)udpcount/totalcount*100); 
	 printf("count of tcp packets: %d | percentage of tcp: %.2f \n",tcpcount, (float)tcpcount/totalcount*100);
	 printf("--------------------------------------\n");
	 printf("count of http packets: %d | percentage of http: %.2f \n",httpcount, (float)httpcount/totalcount*100); 
	 printf("count of dns packets: %d | percentage of dns: %.2f \n",dnscount, (float)dnscount/totalcount*100);
	 printf("count of dhcp packets: %d | percentage of dhcp: %.2f \n",dhcpcount, (float)dhcpcount/totalcount*100);
	 printf("======================================\n"); 
	 for(int i = 0; i < 5; i++){
		printf("%d most used udp source port: %d \n",i,maxSourceUDP[i][1]); 
		printf("%d most used tcp source port: %d \n",i,maxSourceTCP[i][1]); 
		printf("---------------------------------------------------------------------\n");
		printf("%d most used udp destination port: %d \n",i,maxDestinationUDP[i][1]); 
		printf("%d most used tcp destination port: %d \n",i,maxDestinationTCP[i][1]); 
		printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	 }

}