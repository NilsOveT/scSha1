#include <stdio.h>
#include <unistd.h>
#include <fcntl.h> 
#include <sys/mman.h> 
#include <stdint.h>
#include <string.h>
#include "hwlib.h" 
#include "soc_a10/socal/socal.h"
#include "soc_a10/socal/hps.h" 
#include "soc_a10/socal/alt_gpio.h"
#include "hps_0.h"


#define REG_BASE 0xFF200000
#define REG_SPAN 0x00200000
#define REG_MASK ( REG_SPAN - 1 )

void* virtual_base;
void* result_addr;
void* hash0_addr;
void* snddincount_addr;
void* snddoutcount_addr;
int fd;
int count;
int st;
uint32_t hash[5];
uint32_t hashRec[5];
int corr;
uint32_t output[16];

int main (){
	if( ( fd = open( "/dev/mem", ( O_RDWR | O_SYNC ) ) ) == -1 ) {
		printf( "ERROR: could not open \"/dev/mem\"...\n" );
		return( 1 );
	}
	virtual_base=mmap(NULL,REG_SPAN,(PROT_READ|PROT_WRITE),MAP_SHARED,fd,REG_BASE);
	if( virtual_base == MAP_FAILED ) {
			printf( "ERROR: mmap() failed...\n" );
			close( fd );
			return( 1 );
		}

result_addr=virtual_base + ( ( unsigned long  )( RESULT_BASE ) & ( unsigned long)( REG_MASK ) );
hash0_addr=virtual_base + ( ( unsigned long  )( HASH0_BASE ) & ( unsigned long)( REG_MASK ) );
snddincount_addr=virtual_base + ( ( unsigned long  )( SNDDINCOUNT_BASE ) & ( unsigned long)( REG_MASK ) );
snddoutcount_addr=virtual_base + ( ( unsigned long  )( SNDDOUTCOUNT_BASE ) & ( unsigned long)( REG_MASK ) );

while(1){
	if(st == 1){
		*(uint32_t *)snddoutcount_addr = count;
		if(count == *(uint32_t *)snddincount_addr){
			hashRec[count] = *(uint32_t *)result_addr;
			printf("Received hash part %i\r\n", count);
			count = count + 1;
		}
		if(count >= 5){
		st = 0;
		count = 0;
		printf("%x %x %x %x %x\n", hashRec[4], hashRec[3], hashRec[2], hashRec[1], hashRec[0]);
		
			if (hashRec[4] == hash[4] && hashRec[3] == hash[3] && hashRec[2] == hash[2] && hashRec[1] == hash[1] && hashRec[0] == hash[0]){
			printf("Recieved hash is correct, starting brute force cracker. \r\n");
			st = 2;
				count = 0;
				*(uint32_t *)snddoutcount_addr = 15;
			}
			else{
			
			printf("Hash from the FPGA is not the same as the hash from the input, restarting. \r\n");
			*(uint32_t *)snddoutcount_addr = 14;
			st = 0;
			}
		}
	
	}

	else if(st == 2){
		*(uint32_t *)snddoutcount_addr = count;
		if(count == *(uint32_t *)snddincount_addr){
			output[count] = *(uint32_t *)result_addr;
			printf("Received part %i\r\n", count);
			count = count + 1;
		}
		if(count >= 16){
		st = 0;
		count = 0;
		*(uint32_t *)snddoutcount_addr = 0;
		printf("%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\n", output[15], output[14], output[13], output[12], output[11], output[10], output[9], output[8], output[7], output[6], output[5], output[4], output[3], output[2], output[1], output[0]);		
		}
	}

	else {
		printf("From the left, enter the first 8 digits of the hash: \r\n");
		scanf("%x", &hash[4]);
		*(uint32_t *)hash0_addr = hash[4];
		*(uint32_t *)snddoutcount_addr = 4;
		printf("Enter next 8 digits of the hash: \r\n");
		scanf("%x", &hash[3]);
		*(uint32_t *)hash0_addr = hash[3];
		*(uint32_t *)snddoutcount_addr = 3;
		printf("Enter next 8 digits of the hash: \r\n");
		scanf("%x", &hash[2]);
		*(uint32_t *)hash0_addr = hash[2];
		*(uint32_t *)snddoutcount_addr = 2;
		printf("Enter next 8 digits of the hash: \r\n");
		scanf("%x", &hash[1]);
		*(uint32_t *)hash0_addr = hash[1];
		*(uint32_t *)snddoutcount_addr = 1;
		printf("Enter last 8 digits of the hash: \r\n");
		scanf("%x", &hash[0]);
		*(uint32_t *)hash0_addr = hash[0];
		*(uint32_t *)snddoutcount_addr = 0;

		printf("%x %x %x %x %x\n", hash[4], hash[3], hash[2], hash[1], hash[0]);
		
			st = 1;
			count = 0;
			*(uint32_t *)snddoutcount_addr = 10;
			corr = 0;
		
	}
}
return 0;




}