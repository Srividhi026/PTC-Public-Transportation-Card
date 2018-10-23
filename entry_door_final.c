#include <LPC214X.H>
#include <stdio.h>
#include <string.h>

void lcd_init(void);
void lcdcmd(char);
void lcddat(char);
void delay(int);
void delay1(int n);
void display(char *p);
void uart_display(unsigned char dat1);


void uart0_init(void);
void uart1_init(void);

void swipe_card(void);
int recharge(int);
int srividhya_amt,brindha_amt,vidhya_amt;
void get_gps_location(void);
void receive_stop_details(void);

void welcome_msg(void);
int welcome_flag = 0;
char read_rfid[15];

//-------------------------------------Delay loops---------------------------------------------------------------------
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

void delay1(int n)
{
	int i,j;
	for(i=0;i<n;i++)
	{
		for(j=0;j<1275;j++)
		{
		}
	}
}
//---------------------------------------------------------------------------------------------------------------------


//--------------------------------------Initilization functions--------------------------------------------------------
// LCD initilization
void lcd_init()
{
	IO0DIR |=1<<18;   		//rs
  IO0DIR |=1<<19;   		//rw
  IO0DIR |=1<<20;  		  //en
	IO1DIR |=0xff<<16;    //lcd data line
	
	lcdcmd(0x38); // to configure LCD for 2nd line
	lcdcmd(0x0E); // 
	lcdcmd(0x01); // to clear the display screen 
}

// UART0 intilization Used for Serial Communication(Zigbee) between exit & entry door
void uart0_init()    
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

// UART1 intilization Used for RFID Reader
void uart1_init()    
{
	/* Selecting P0.8 as TXD1 */
	PINSEL0 |= (1<<16);
	PINSEL0 &= ~(1<<17);
	/* Selecting P0.9 as RXD1 */
	PINSEL0 |= (1<<18);
	PINSEL0 &= ~(1<<19);
	
	U1LCR = 0x83;
	U1DLM |= 0x01;
	U1DLL |= 0x86;
	U1LCR = 0x03;     //to make DLAB =0 , that is unlocking
}
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
void welcome_msg(void)
{
	lcdcmd(0x01); //clear display screen
	lcdcmd(0x80);			//First line
	display("Welcome");
	lcdcmd(0xC0); //second line
	display("to");
	lcdcmd(0x94);	//third line
	display("Pondicherry");
	lcdcmd(0xD4);	//fourth line
	display("Bus");
	
}
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
int recharge(int amt)
{
	//lcdcmd(0x94);
	//display("Do You want to ");
	lcdcmd(0xD4);
	display("Recharge?1->Y/2->N");
	
	while(1)
	{
		if((IO0PIN & (1<<16)) ==0)		    //P0.16 as YES key button
		{
			amt= amt+100;
			lcdcmd(0x01);
			display("Recharge Successful!");
			return amt;
		}
		else if((IO0PIN & (1<<17)) ==0)		// P0.17 as NO key button
		{
			if(amt<=50)		// Rs 50 and less is low balance
			{
				lcdcmd(0x01);
				display("Low Balance");
				lcdcmd(0xC0);
				display("       Please");
				lcdcmd(0x94);
				display("Recharge:1->Y/2->N");
				delay(500);
			}	
			else
				return 0;
		}
	}
}
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
void get_gps_location(void)
{
	int dt;
	//IO0DIR |=1<<13;		// Make P0.13 as output
	IO0CLR |= 1<<13;		// Send signal to exit door to send bus stand details
	
	while((U0LSR & (1<<0))==0);	//until valid data is received,stay here
	dt=U0RBR;
	
	IO0SET |= 1<<13;		// After data is received, send high signal to exit_module
	
	lcdcmd(0x01);
	if(dt==1)
	{
		display("Beach Road");
	}
	else if(dt==2)
	{
		display("Mount Road");
	}
	else if(dt==3)
	{
		display("Park Road");
	}
	else if(dt==4)
	{
		display("TR Road");
	}
}
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
void swipe_card(void)
{
	int rfid_index=0,dt;
	char buf[20];
	int recharge_return=0;
	
	lcdcmd(0x01);
	delay(20);
	
	if((U1LSR & (1<<0))!=0)
	{
		dt=U1RBR;  //clearing buffer
	}
	IO0SET = 1<<21;
	display("Swipe card!");
	while(1)
	{
		while((U1LSR & (1<<0))==0);	//until valid data is received,stay here
		dt=U1RBR;		//data in the Received Buffer reg is copied to dt
		
		
		read_rfid[rfid_index++]=dt;
		
		if(rfid_index==12)  // length of rfid number is 12
		{
			
			read_rfid[rfid_index]='\0';
			//IO0CLR = 1<<21;
			break;
		}
	}
		lcdcmd(0x01);
		delay(20);
		//display(read_rfid);
	
		//n=strcmp(read_rfid,"0900966E0CFD"); 
		
		//if(n==0)
		if((strcmp(read_rfid,"11007943072C"))==0)
		{
			lcdcmd(0x80);
			display("SREE VIDHYA !");
			delay(50);
			lcdcmd(0xC0);
			//srividhya_amt = srividhya_amt+100;
			sprintf(buf,"Balance : Rs %d",srividhya_amt);
			display(buf);
			recharge_return=recharge(srividhya_amt);
			if(recharge_return !=0)
			{
				srividhya_amt = recharge_return;
				sprintf(buf,"Balance : Rs %d",srividhya_amt);
			  display(buf);
			}
			
			get_gps_location();
			
			delay(50);
			
		}
		else if((strcmp(read_rfid,"0E006169BEB8"))==0)
		{
			lcdcmd(0x80);
			display("BRINDHA !");
			delay(50);
			lcdcmd(0xC0);
			//brindha_amt = brindha_amt+100;
			sprintf(buf,"Balance : Rs %d",brindha_amt);
			display(buf);
			recharge_return=recharge(brindha_amt);
			if(recharge_return !=0)
			{
				brindha_amt = recharge_return;
				sprintf(buf,"Balance : Rs %d",brindha_amt);
			  display(buf);
			}
			get_gps_location();
			delay(50);
			
		}
		else if((strcmp(read_rfid,"0E00618538D2"))==0)
		{
			lcdcmd(0x80);
			display("Vidhya MONI !");
			delay(50);
			lcdcmd(0xC0);
			//vidhya_amt = vidhya_amt+100;
			sprintf(buf,"Balance : Rs %d",vidhya_amt);
			display(buf);
			recharge_return=recharge(vidhya_amt);
			if(recharge_return !=0)
			{
				vidhya_amt = recharge_return;
				sprintf(buf,"Balance : Rs %d",vidhya_amt);
			  display(buf);
			}
			get_gps_location();
			delay(50);
			
		}
		else
		{
			display("Invalid card!!");
		}
		delay(5000);
		lcdcmd(0x01);
	
	
	  IO0CLR = 1<<21;
}
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
void receive_stop_details(void)
{
	char dt[2];
	IO0SET = 1<<21;
	while((U0LSR & (1<<0))==0);	//until valid data is received,stay here
	dt[0]=U0RBR;
	
	while((U0LSR & (1<<0))==0);	//until valid data is received,stay here
	dt[1]=U0RBR;
	
	if(dt[0]=='R')
	{
		if(dt[1]=='1')
		{
			srividhya_amt = srividhya_amt -10;
		}
		else if(dt[1]=='2')
		{
			srividhya_amt = srividhya_amt -20;
		}
		else if(dt[1]=='3')
		{
			srividhya_amt = srividhya_amt -30;
		}
		else if(dt[1]=='4')
		{
			srividhya_amt = srividhya_amt -40;
		}
	}
	
	else if(dt[0]=='G')
	{
		if(dt[1]=='1')
		{
			brindha_amt = brindha_amt -10;
		}
		else if(dt[1]=='2')
		{
			brindha_amt = brindha_amt -20;
		}
		else if(dt[1]=='3')
		{
			brindha_amt = brindha_amt -30;
		}
		else if(dt[1]=='4')
		{
			brindha_amt = brindha_amt -40;
		}
	}
	
	else if(dt[0]=='M')
	{
		if(dt[1]=='1')
		{
			vidhya_amt = vidhya_amt -10;
		}
		else if(dt[1]=='2')
		{
			vidhya_amt = vidhya_amt -20;
		}
		else if(dt[1]=='3')
		{
			vidhya_amt = vidhya_amt -30;
		}
		else if(dt[1]=='4')
		{
			vidhya_amt = vidhya_amt -40;
		}
	}
	delay(2000);
	IO0CLR = 1<<21;
}
//---------------------------------------------------------------------------------------------------------------------
//------------------------------Main Fuction --------------------------------------------------------------------------
int main()
{
	IO0DIR |=1<<13;		// Make P0.13 as output and it is connected to P0.14 of exit_module
	IO0SET = 1<<13;        //to make the reg high
	VPBDIV = 0x01;
		
	lcd_init();
	uart0_init();
	uart1_init();
	IO0DIR |=1<<21;
	
	welcome_msg();		// Starting message
	srividhya_amt = 50;
	brindha_amt = 100;
	vidhya_amt = 100;
	
	while(1)
	{
		if((IO0PIN & (1<<14))==0)
		{
			swipe_card();
			welcome_flag = 0;
		}
		else if((IO0PIN & (1<<15))==0)		//to receive signal to deduct money
		{
			IO0CLR = 1<<21; 
			receive_stop_details();
		}
		
		if(welcome_flag==0)
		{
			welcome_msg();		// Starting message
			welcome_flag = 1;
		}
		/*IO0SET =1<<21;   //set en=1;
	  delay(1000);
	  IO0CLR =1<<21;   //clear en=0;
		lcdcmd(0x01);
	  delay(1000);*/
	}
}
//---------------------------------------------------------------------------------------------------------------------


//---------------------------All LCD Stuffs here-----------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
//.....................................................................................................................
void display(char *p)
{
	int i;
	for(i=0;p[i]!='\0';i++)
	{
		lcddat(p[i]);
		uart_display(p[i]);
		delay1(5);
		
	}	
}
//.....................................................................................................................
void uart_display(unsigned char dat)
{
    while((U0LSR & (1<<5))==0);
		U0THR=dat;       // send data
}
//.....................................................................................................................
void lcdcmd(char cmd)
{
	IO1CLR =0xff<<16;  //clear 8 bits
	IO1SET =cmd<<16;  //0x38 will come in cmd and will start from 6th bit
	IO0CLR =1<<18;   //clear rs=0;
	IO0CLR =1<<19;   //clear rw=0;
	IO0SET =1<<20;   //set en=1;
	delay1(10);
	IO0CLR =1<<20;   //clear en=0;
	delay1(50);
}
//.....................................................................................................................
void lcddat(char dat)
{
	IO1CLR =0xff<<16;   //clear 6 bits
	IO1SET =dat<<16;  //data vidhya will come to this function
	IO0SET =1<<18;   //clear rs=1;
	IO0CLR =1<<19;   //clear rw=0;
	IO0SET =1<<20;   //set en=1;
	delay1(10);
	IO0CLR =1<<20;   //clear en=0;
	delay1(50);
}
//.....................................................................................................................
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
