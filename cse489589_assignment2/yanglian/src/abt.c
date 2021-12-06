#include "../include/simulator.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional data transfer 
   protocols (from A to B). Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

/********* STUDENTS WRITE THE NEXT SIX ROUTINES *********/
struct pkt buffer[1000]; //use to storage packet on wait. read <= write
int read; //index of buffer that will read next
int write;//index of buffer that will write next
int seqnum; //seq of the packet
int lock;// 0 ready to send, 1 wait on ack

struct pkt create_packet(char data[20], int seqnum ,int acknum, int checksum){
  struct pkt packet;
  strcpy(packet.payload,data);
  packet.seqnum = seqnum;
  packet.acknum = acknum;
  packet.checksum = checksum;
  return packet;
}

int calculate_checksum(char data[20], int seqnum ,int acknum){
  int checksum = 0;
  for (int i = 0; i < 20; i++){
    checksum += data[i];
  }
  checksum = checksum + seqnum + acknum;
  return checksum;
}

int check_packet(struct pkt packet, int checksum, int seqnum, int acknum){
  if(packet.checksum != checksum){
    printf("checksum error: packet_checksum %d and calculated checksum %d\n",packet.checksum,checksum);
    return 0;
  }
    if(packet.seqnum != seqnum){
    printf("seqnum error: packet_seqnum %d and calculated seqnum %d\n",packet.seqnum,seqnum);
    return 0;
  }
    if(packet.acknum != acknum){
    printf("acknum error: packet_acknum %d and calculated acknum %d\n",packet.acknum,acknum);
    return 0;
  }
  return 1;  
}

/* called from layer 5, passed the data to be sent to other side */
void A_output(message)
  struct msg message;
{
  int checksum = calculate_checksum(message.data,seqnum,0);
  struct pkt packet = create_packet(message.data,seqnum,0,checksum);
  buffer[write] = packet;
  write++;
  seqnum++;
  if(!lock)
  {
    tolayer3(0,buffer[read]);
    starttimer(0,20.0f);
    lock = 1;
  }
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{
  int checksum = calculate_checksum(packet.payload,packet.seqnum,packet.acknum);
  int valid = check_packet(packet,checksum,seqnum,0);
  if(valid){
    stoptimer(0);
    if(read < write){
      read++;
      tolayer3(0,buffer[read]);
      starttimer(0,20.0f);
    }
    else if(read == write)
      lock = 0;
    else{
      printf("read > write error");
      read = write;
    }
  }
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
  tolayer3(0,buffer[read]);
  starttimer(0,20.0f);
  printf("timeout, resending %d seqnum packet\n",buffer[read].seqnum);
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
  read = 0;
  write = 0;
  lock = 0;
  seqnum = 0;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{
  int checksum = calculate_checksum(packet.payload,packet.seqnum,packet.acknum);
  int valid = check_packet(packet,checksum,seqnum,0);
  if(valid){
    tolayer3(1,packet);
    tolayer5(1,packet.payload);
  }
}

/* the following routine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
  read = 0;
  write = 0;
  lock = 0;
  seqnum = 0;
}
