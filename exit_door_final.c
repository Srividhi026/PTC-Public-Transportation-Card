#include <LPC214X.H>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


void lcd_init(void);
void lcdcmd(char);
void lcddat(char);
void delay(int);
void display(char *p);

void uart0_init(void);
void uart1_init(void);

char test[6]={"$GPGGA"};
char read_gps[70];
char comma_position[15];
void display_gps(void);
void lcd_longitude(void);
void lcd_latitude(void);	
void find_comma(void);
void find_gps_location(void);
void find_bus_stop(void);

void exit_door_msg(void);
int exit_door_flag=0;
int entry_door_GPS_flag=0;

char lat[10];
char lon[11];

void swipe_card(void);
int key_press_exit = 0;
//int srividhya_amt,brindha_amt,vidhya_amt;

char read_rfid[15];


void lcd_init()
{
	IO0DIR |=1<<18;   		//rs
  IO0DIR |=1<<19;   		//rw
  IO0DIR |=1<<20;  		  //en
	IO1DIR |=0xff<<16;    //lcd data line
	
	lcdcmd(0x38);
	lcdcmd(0x0E);
	lcdcmd(0x01);
}

void delay(int n)
{
	int i,j;
	for(i=0;i<n;i++)
	{
		for(j=0;j<1275;j++)
		{
		}
	}
}

void uart0_init()   // Used for RFID
{
	/* Selecting P0.0 as TXD0 */
	PINSEL0 |= (1<<0);
	PINSEL0 &= ~(1<<1);
	/* Selecting P0.1 as RXD0 */
	PINSEL0 |= (1<<2);
	PINSEL0 &= ~(1<<3);
	
	U0LCR = 0x83;
	U0DLM |= 0x01;
	U0DLL |= 0x86;
	U0LCR = 0x03;     //to make DLAB =0 , that is unlocking
}

void uart1_init()   // Used for GPS
{
	/* Selecting P0.8 as TXD1 */
	PINSEL0 |= (1<<16);
	PINSEL0 &= ~(1<<17);
	/* Selecting P0.9 as RXD1 */
	PINSEL0 |= (1<<18);
	PINSEL0 &= ~(1<<19);
	
	U1LCR = 0x83;  // Enable DLAB Divisor LAtch to write the baud rate generators
	U1DLM |= 0x01;
	U1DLL |= 0x86;
	U1LCR = 0x03;     //to make DLAB =0 , that is unlocking
}

//---------------------------------------------------------------------------------------------------------------------
void exit_door_msg()
{
	lcdcmd(0x01);
	display("Exit Door");
}

 //---------------------------------------------------------------------------------------------------------------------
void swipe_card()
{
	int rfid_index=0,dt;
	//char buf[20];
	//int recharge_return=0;
	//IO0DIR |=1<<15; // Making PO.15 as output now to send signal to entry door PO.15 for payment deduction
		
	IO0CLR = 1<<15;	// Signal to PO.15 entry door to receive data
	
	lcdcmd(0x01);
	delay(20);
	if((U0LSR & (1<<0))!=0)
	{
		dt=U0RBR;  //clearing buffer
	}
	IO0SET = 1<<21;
	
	
	delay(500);
	display("Swipe card!");
	while(1)
	{
		while((U0LSR & (1<<0))==0);	//until valid data is received,stay here
		dt=U0RBR;		//data in the Received Buffer reg is copied to dt
				
		read_rfid[rfid_index++]=dt;
		
		if(rfid_index==12)  // length of rfid number is 12
		{
			
			read_rfid[rfid_index]='\0';
			//IO0CLR = 1<<21;
			break;
		}
	}
	lcdcmd(0x01);
	delay(1000);
	
	IO0SET = 1<<15;			// High signal to entry side as it is already connected
		
		if((strcmp(read_rfid,"11007943072C"))==0)
		{
			lcdcmd(0x80);
			display("SREE VIDHYA !");
			
			while((U0LSR & (1<<5))==0);
			U0THR='R';        // send data 1 for Beach Road
			
			if(key_press_exit == 1)
			{
				return;
			}
			else
				find_bus_stop();
			
			//IO0CLR = 1<<15;
			
			//lcdcmd(0x94);
			//delay(50);
			
		}
		else if((strcmp(read_rfid,"0E006169BEB8"))==0)
		{
			lcdcmd(0x80);
			display("BRINDHA !");
			
			while((U0LSR & (1<<5))==0);
			U0THR='G';        // send data 1 for Beach Road
			
			
			
			if(key_press_exit == 1)
			{
				return;
			}
			else
				find_bus_stop();
			
			//delay(50);
		}
		else if((strcmp(read_rfid,"0E00618538D2"))==0)
		{
			lcdcmd(0x80);
			display("VIDHYA MONI !");
			
			while((U0LSR & (1<<5))==0);
			U0THR='M';        // send data 1 for Beach Road
			
			if(key_press_exit == 1)
			{
				return;
			}
			else
				find_bus_stop();
		
			//delay(50);
		}
		else
		{
			display("Invalid card!!");
		}
		delay(5000);
		lcdcmd(0x01);
	
	  IO0CLR = 1<<21;
		IO0SET = 1<<15;			// High signal to entry side
}

//-------------------------- Main Fuction -----------------------------------------------------------------------------
int main()
{
	IO0DIR |=1<<15; // Making PO.15 as output to send signal to entry door PO.15 for payment deduction
	IO0SET = 1<<15;
	
	IO0DIR |= 1<<21;	//LED
	IO0DIR |= 1<<22;	//LED
	
	IO1DIR &= (~(1<<28));	//SW0 i/p to P1.28
	IO1DIR &= (~(1<<29));	//SW1 i/p	to P1.29
	IO1DIR &= (~(1<<30));	//SW2 i/p	to P1.30
	IO1DIR &= (~(1<<31));	//SW3 i/p to P1.31
	
	VPBDIV = 0x01;
	uart0_init();
	uart1_init();
	lcd_init();
	while(1)
	{
		if(exit_door_flag==0)
		{
			exit_door_msg();
			exit_door_flag=1;
		}
		
		if((IO0PIN & (1<<14))==0)			// P0.14 acts as input and receives signal from entry_door module (P0.13) to find bus stand name
		{
			entry_door_GPS_flag = 1;
			IO0SET = 1<<22;
			find_bus_stop();
			delay(1000);
			IO0CLR = 1<<22;
			
			exit_door_flag=0;
			
		}
		
		
			//swipe_card();
			if((IO0PIN & (1<<17))==0)		// For exiting using GPS reading first press switch connected to P0.17 to enter this mode
		  {
		  	//find_gps_location();
		  	swipe_card();
				exit_door_flag=0;
			}
			/*else if((IO1PIN & (1<<28))==0)	// For exiting using key press switch connected to P0.17 to enter this mode
			{
				key_press_exit = 1;
				swipe_card();
				
				lcdcmd(0xC0);
				display("Beach Road");
				while((U0LSR & (1<<5))==0);
				U0THR='1';        // send data 1 for Beach Road to deduct money
				
				IO0CLR = 1<<21;
				IO0SET = 1<<15;
				key_press_exit = 0;
				lcdcmd(0xD4);
				display("Thank You !");
				delay(5000);
				lcdcmd(0x01);
				exit_door_flag=0;
			}*/
			else if((IO1PIN & (1<<29))==0)
			{
				key_press_exit = 1;
				swipe_card();
				lcdcmd(0x01);
				lcdcmd(0xC0);
				display("Mount Road");
				while((U0LSR & (1<<5))==0);
				U0THR='2';        // send data '2' for Mount Road
				
				IO0CLR = 1<<21;
				IO0SET = 1<<15;
				key_press_exit = 0;
				lcdcmd(0xD4);
				display("Thank You !");
				delay(5000);
				lcdcmd(0x01);
				exit_door_flag=0;
			}
			else if((IO1PIN & (1<<30))==0)
			{
				key_press_exit = 1;
				swipe_card();
				lcdcmd(0x01);
				lcdcmd(0xC0);
				display("Park Road");
				while((U0LSR & (1<<5))==0);
				U0THR='3'; 				// send data '3' for Park Road
				
			  IO0CLR = 1<<21;	
				IO0SET = 1<<15;
				key_press_exit = 0;
				lcdcmd(0xD4);
				display("Thank You !");
				delay(5000);
				lcdcmd(0x01);
				exit_door_flag=0;
			}
			else if((IO1PIN & (1<<31))==0)
			{
				key_press_exit = 1;
				swipe_card();
				lcdcmd(0x01);
				lcdcmd(0xC0);
				display("TR Road");
				while((U0LSR & (1<<5))==0);
				U0THR='4';				// send data '4' for TR Road 
				
				IO0CLR = 1<<21;
				IO0SET = 1<<15;
				key_press_exit = 0;
				lcdcmd(0xD4);
				display("Thank You !");
				delay(5000);
				lcdcmd(0x01);
				exit_door_flag=0;
			}			
	}
}
//---------------------------------------------------------------------------------------------------------------------
void find_bus_stop()
{
	float lat_val,lon_val;
	find_gps_location();
	lat_val = atof(lat);
	lon_val = atof(lon);
			
	if(lat_val >= 1301.0000 && lat_val <= 1302.9999)			// Change these value after finding location
	{
		if(lon_val >= 07733.0000 && lon_val <=07734.9999)		// Change these value after finding location
		{
			if(entry_door_GPS_flag == 1)
			{
				while((U0LSR & (1<<5))==0);
				U0THR=1;        // send data 1 for Beach Road to find which stop while entering
				
				entry_door_GPS_flag = 0;
				return ;
			}
			lcdcmd(0xC0);
			display("Beach Road");
			while((U0LSR & (1<<5))==0);
			U0THR='1';        // send data 1 for Beach Road to deduct money
		}
	}
	else if(lat_val >= 1301.9000 && lat_val <= 1301.9900)		// Change these value after finding location
	{
		if(lon_val >= 07734.0000 && lon_val <=07734.0500)			// Change these value after finding location
		{
			if(entry_door_GPS_flag == 1)
			{
				while((U0LSR & (1<<5))==0);
				U0THR=2;        // send data 2 for Mount Road
				
				entry_door_GPS_flag = 0;
				return ;
			}
			lcdcmd(0xC0);
			display("Mount Road");
			while((U0LSR & (1<<5))==0);
			U0THR='2';        // send data '2' for Mount Road
		}
	}
	else if(lat_val >= 1301.9000 && lat_val <= 1301.9900)		// Change these value after finding location
	{
		if(lon_val >= 07734.0000 && lon_val <=07734.0500)			// Change these value after finding location
		{
			if(entry_door_GPS_flag == 1)
			{
				while((U0LSR & (1<<5))==0);
				U0THR=3;        // send data 3 for Park Road
				
				entry_door_GPS_flag = 0;
				return ;
			}
			lcdcmd(0xC0);
			display("Park Road");
			while((U0LSR & (1<<5))==0);
			U0THR='3'; 				// send data '3' for Park Road
		}
	}
	else if(lat_val >= 1301.9000 && lat_val <= 1301.9900)		// Change these value after finding location
	{
		if(lon_val >= 07734.0000 && lon_val <=07734.0500)			// Change these value after finding location
		{
			if(entry_door_GPS_flag == 1)
			{
				while((U0LSR & (1<<5))==0);
				U0THR=4;        // send data 4 for TR Road
				
				entry_door_GPS_flag = 0;
				return ;
			}
			lcdcmd(0xC0);
			display("TR Road");
			while((U0LSR & (1<<5))==0);
			U0THR='4';				// send data '4' for TR Road 
		}
	}
	delay(50);
}

//---------------------------------------------------------------------------------------------------------------------
//-------------------------------- GPS Functions ----------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
void find_gps_location()
{
	int gps_index=0,dt,flag=0;
	IO0SET = 1<<21;
  flag=0;
	while(flag==0)
	{
		while((U1LSR & (1<<0))==0);	//until valid data is received,stay here
		dt=U1RBR;		//data in the Received Buffer reg is copied to dt
			
		read_gps[gps_index++]=dt;
			
		if(gps_index<7)	         //Condition to check the required data
		{
			if(read_gps[gps_index-1]!=test[gps_index-1])
				gps_index=0;
		}
			
		if(gps_index==69)  // length of GPS data is 69
		{
			IO0SET = 1<<21;
			display_gps();
			gps_index=0;
			IO0CLR = 1<<21;
			flag=1;
		}
	}
	delay(1000);
	IO0CLR = 1<<21;
}

void display_gps()
{
	find_comma();	   //Function to detect position of comma in the string
	lcd_latitude();    //Function to show Latitude
	//delay(10000);
	lcd_longitude();   //Function to show Longitude
	delay(10000);
}

void find_comma()
{
	unsigned int i,count=0;
	for(i=0;i<70;i++)
	{	
		if(read_gps[i]==',')
		{
			comma_position[count++]=i;
		}
  }
}

// Function to extract and display Latitude
void lcd_latitude()		 			
{
	unsigned int c2=comma_position[1]; //Position of second comma
	lcdcmd(0x01);	         		// Clear LCD display
	lcdcmd(0x84);	         		//Move cursor to position 6 of line 1
	display("LATITUDE");	 		
	lcdcmd(0xC0);					 		//Beginning of second line  
	lat[0] = read_gps[c2+1];
	lat[1] = read_gps[c2+2];	
	lat[2] = read_gps[c2+3];
	lat[3] = read_gps[c2+4];
	lat[4] = read_gps[c2+5];
	lat[5] = read_gps[c2+6];
	lat[6] = read_gps[c2+7];
	lat[7] = read_gps[c2+8];
	lat[8] = read_gps[c2+9];
	lat[9] ='\0';
	display(lat);
	delay(1000);
	//lcddat(read_gps[c2+1]);
  //lcddat(read_gps[c2+2]);
	//lcddat(0xDF);  //degree
	//lcddat(read_gps[c2+3]);	
	//lcddat(read_gps[c2+4]);
	//lcddat(read_gps[c2+5]);
	//lcddat(read_gps[c2+6]);
	//lcddat(read_gps[c2+7]);
	//lcddat(read_gps[c2+8]);
	//lcddat(read_gps[c2+9]);
	//lcddat(read_gps[c2+10]);
	//lcddat(0x27);          //ASCII of minute sign(')
	//lcddat(read_gps[c2+11]);
	//lcddat(read_gps[c2+12]);
	//lcdcmd(0x94);
} 
 
// Function to extract and display Longitude
void lcd_longitude()
{
	unsigned int c4=comma_position[3];
	lcdcmd(0x98);	        	   //Move cursor to position 4 of line 3
	display("LONGITUDE");		   
	lcdcmd(0xD4);			         //Begining of 4th line  
	lon[0] = read_gps[c4+1];
	lon[1] = read_gps[c4+2];
	lon[2] = read_gps[c4+3];
	lon[3] = read_gps[c4+4];
	lon[4] = read_gps[c4+5];
	lon[5] = read_gps[c4+6];
	lon[6] = read_gps[c4+7];
	lon[7] = read_gps[c4+8];
	lon[8] = read_gps[c4+9];
	lon[9] = read_gps[c4+10];
	lon[10] = '\0';
	display(lon);
	delay(1000);	
	//lcddat(read_gps[c4+1]);
	//lcddat(read_gps[c4+2]);
	//lcddat(read_gps[c4+3]);
	//lcddat(0xDF);  //degree
	//lcddat(read_gps[c4+4]);
	//lcddat(read_gps[c4+5]);
	//lcddat(read_gps[c4+6]);
	//lcddat(read_gps[c4+7]);
	//lcddat(read_gps[c4+8]);
	//lcddat(read_gps[c4+9]);
	//lcddat(read_gps[c4+10]);
	//lcddat(read_gps[c4+11]);
	//lcddat(0x27);               //ASCII of minute sign(')
	//lcddat(read_gps[c4+12]);
	//uart_send_gsm(read_gps[c4+12]);
	//lcddat(read_gps[c4+13]);
	//lcdcmd(0x94);
}
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------

//--------------------------------------- LCD display stuffs-----------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
void display(char *p)
{
	int i;
	for(i=0;p[i]!='\0';i++)
	{
		lcddat(p[i]);
		delay(10);
		
	}	
}

void lcdcmd(char cmd)
{
	IO1CLR =0xff<<16;  //clear 8 bits
	IO1SET =cmd<<16;  //0x38 will come in cmd and will start from 6th bit
	IO0CLR =1<<18;   //clear rs=0;
	IO0CLR =1<<19;   //clear rw=0;
	IO0SET =1<<20;   //set en=1;
	delay(10);
	IO0CLR =1<<20;   //clear en=0;
	delay(100);
}

void lcddat(char dat)
{
	IO1CLR =0xff<<16;   //clear 6 bits
	IO1SET =dat<<16;  //data manoj will come to this function
	IO0SET =1<<18;   //clear rs=1;
	IO0CLR =1<<19;   //clear rw=0;
	IO0SET =1<<20;   //set en=1;
	delay(10);
	IO0CLR =1<<20;   //clear en=0;
	delay(100);
}
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
