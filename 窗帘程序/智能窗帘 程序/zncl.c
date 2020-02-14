/*********51��Ƭ�����ܴ������*********/
/*************JJ����ֱӪ��*************/
/***************ԭ�����***************/

#include <REGX52.H>	  			//ͷ�ļ�
#define uchar unsigned char
#define uint unsigned int

#define CS	P3_0		//ADC0804 CS�˿�
#define RD	P3_1		//ADC0804 RD�˿�
#define WR	P3_2		//ADC0804 WR�˿�
#define g_kz	P2_5 	//����ܸ�λ����
#define s_kz	P2_4	//�����ʮλ����
#define b_kz	P2_3	//����ܰ�λ����
#define q_kz	P2_2	//���ǧλ����
#define SMG_XS	P0		//�������ʾ�˿�
#define AD_data	P1		//ADC0804����˿�
#define Up		P2_1	//������λ���ض˿�
#define Down 	P2_0	//������λ���ض˿�
#define Key1	P3_3	//�ֶ�/�Զ��л� ����
#define Key2	P3_4	//��ʱ/��ع����л� ����
#define Key3	P3_5	//���� ����
#define Key4	P3_6	//�� ���� (�ֶ�ģʽ:�򿪴��� ��ʱģʽ����ʱ�򿪴���)
#define Key5	P3_7	//�� ���� (�ֶ�ģʽ:�رմ��� ��ʱģʽ����ʱ�رմ���)

#define IA		P2_7	//ֱ��������ƶ˿�
#define IB		P2_6	//ֱ��������ƶ˿�

int adval; 		//ADC �������
int j;			//����ѭ������ i j
uchar flag=0;	//��ʾ��־λ(0:������ʾ 1:���޹��ն�����(��ʱ) 2:���޹��ն�����(����)				 
bit flag_gd=0;	//���/��ʱ��־λ(0:��ǰ���ն� 1:��ʱʱ�� )	
bit ms=0;		//ģʽ(0:�ֶ�ģʽ 1:�Զ�ģʽ)
bit move=0;		//��ʱʱ�䵽��־λ(0:ʱ��δ�� 1:ʱ�䵽)
bit OFF_ON=0;	//�������ת��־λ,��ʾ�����򿪻�ر�(0:�����ر� 1:������)
bit Time_OFF_ON=0;//��ʱʱ�䵽ִ�д����򿪻�ر�(0:��ʱ�رմ��� 1:��ʱ�򿪴���)

char hour=12,min=0;sec=0; //����ʱ�� ��ʱ �� �롱���� �ϵ�Ĭ��12.00.00
char num=0;	//ʱ�����
uint H_GM=240;	//�������޹��������ñ������ϵ�Ĭ��200
uint L_GM=100;	//�������޹��������ñ������ϵ�Ĭ��100
uchar t=1; 		//����ܶ�̬ɨ����ʱ����
                
uchar code table[]={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90};	
//�������ʾ����      0    1    2    3    4    5    6    7    8    9

void delay(uint time)	   	//��ʱ����
{
	uint x,y; 			//������ʱ���� x y
	for(x=time;x>0;x--)
	for(y=110;y>0;y--);	//�ղ���
}

void  Time_init()		//��ʱ����ʼ��
{
	EA=1;
	TMOD=0x11;

	/*��ʱ��0��ʼ��(����PWM���Ƶ��ת��)*/	
	TH0=0xf8;	//��ʱ2ms
	TL0=0xcc;
	ET0=1;		//ʹ��T0�ж�
	TR0=0;		//�Ȳ�������ʱ��0
	/*��ʱ��1��ʼ��(����50ms��ʱʱ��)*/
	TH1=0x4c; 		//50ms
	TL1=0x00;
	ET1=1;
	TR1=0;		//�Ȳ�������ʱ��1	
}

void Key_cl()
{
	static bit keybuf1=1;  	//Key1ֵ�ݴ�,��ʱ���水��ɨ��ֵ
	static bit backup1=1;  	//Key1ֵ����,����ǰһ�ε�ɨ��ֵ
	static bit keybuf2=1; 	//Key2ֵ�ݴ�,��ʱ���水��ɨ��ֵ	
	static bit backup2=1;	//Key2ֵ����,����ǰһ�ε�ɨ��ֵ
	static bit keybuf3=1;	//Key3ֵ�ݴ�,��ʱ���水��ɨ��ֵ
	static bit backup3=1; 	//Key3ֵ����,����ǰһ�ε�ɨ��ֵ
	static bit keybuf4=1;	//Key4ֵ�ݴ�,��ʱ���水��ɨ��ֵ
	static bit backup4=1; 	//Key4ֵ����,����ǰһ�ε�ɨ��ֵ
	static bit keybuf5=1;	//Key5ֵ�ݴ�,��ʱ���水��ɨ��ֵ
	static bit backup5=1; 	//Key5ֵ����,����ǰһ�ε�ɨ��ֵ
	keybuf1=Key1;  			//��Key1��ǰֵɨ��ֵ�ݴ�
	if(keybuf1!=backup1)	//Key1��ǰֵ��ǰ��ֵ�����,˵��Key1�����ж���
	{
		delay(1);			//��ʱȥ��
		if(keybuf1==Key1)	//�ж�Key1ɨ��ֵ��û�з����仯,����������
		{
			if(backup1==1)	//Key1����������Ч
			{
				move=0;	   	//����ʱ���־λΪ0(ʱ��δ��)
				TR0=0;		//�رն�ʱ��0
				ms=~ms;		// ģʽȡ��
				q_kz=1;b_kz=1;s_kz=1;g_kz=1;//�ر��������ʾ
				IA=0;IB=0;	//����ֹͣ	
			}
			backup1=keybuf1; //���±���Key1��ǰֵ,�Ա������´αȽ�
		}
	}
	if(ms==0)  			//�ֶ�ģʽ��
	{
		if(Key4==0)		//���򿪴������� ����
		{
			delay(10);	//��ʱȥ��
			if(Key4==0)	//���жϰ����Ƿ���
			{
				move=0;				//����ʱ���־λΪ0(ʱ��δ��)
				OFF_ON=1;	 		//������
				TR0=1;				//������ʱ��0
				SMG_XS=table[1]; 	//�������ʾ"1"
				q_kz=1;b_kz=1;s_kz=1;g_kz=0;	//��λ��ʾ
				while(Key4==0);	//�ȴ������ɿ�
				TR0=0;			//�رն�ʱ��0
				IA=0;IB=0;		//����ֹͣ
				q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//�ر������	
			}
		}
		if(Key5==0)		//���رմ������� ����
		{
			delay(10); 	//��ʱȥ��	
			if(Key5==0)	//���жϰ����Ƿ���
			{
				move=0;				//����ʱ���־λΪ0(ʱ��δ��)
				OFF_ON=0;	 		//�����ر�
				TR0=1;				//������ʱ��0
				SMG_XS=table[0]; 	//�������ʾ"0"
				q_kz=1;b_kz=1;s_kz=1;g_kz=0;	//��λ��ʾ
				while(Key5==0);	//�ȴ������ɿ�
				TR0=0;			//�رն�ʱ��0
				IA=0;IB=0;	//����ֹͣ	
				q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//�ر������
			}
		}		
	}
	else 		//�Զ�ģʽ��
	{
		keybuf2=Key2;  			//��Key2��ǰֵɨ��ֵ�ݴ�
		if(keybuf2!=backup2)	//Key2��ǰֵ��ǰ��ֵ�����,˵��Key2�����ж���
		{
			delay(1);			//��ʱȥ��
			if(keybuf2==Key2)	//�ж�Key2ɨ��ֵ��û�з����仯,����������
			{
				if(backup2==1)	//Key2����������Ч
				{	
					move=0;		//����ʱ���־λΪ0(ʱ��δ��)
					flag_gd=~flag_gd; //���/��ʱ��־λȡ��(0:��ǰ���ն� 1:��ʱʱ�� )								
				}
				backup2=keybuf2; //���±���Key2��ǰֵ,�Ա������´αȽ�
			}
		}
		keybuf3=Key3; 		//��Key3��ǰֵɨ��ֵ�ݴ�			
		if(keybuf3!=backup3)//Key3��ǰֵ��ǰ��ֵ�����,˵��Key3�����ж���
		{
			delay(1);		//��ʱȥ��
			if(keybuf3==Key3)	//�ж�Key3ɨ��ֵ��û�з����仯,����������	
			{
				if(backup3==1)	//Key3����������Ч
				{
					move=0;		//����ʱ���־λΪ0(ʱ��δ��)
					TR0=0;		//�رն�ʱ��0
					flag++;
					if(flag==3) {flag=0;}	//��flag������0-2֮��
				}
				backup3=keybuf3;//���±���Key3��ǰֵ,�Ա������´αȽ�
			}
		}
		if(flag==1)				//�������޹��ն�����(��ʱ)			
		{
			keybuf4=Key4;  		//��Key4��ǰֵɨ��ֵ�ݴ�
			if(keybuf4!=backup4)//Key4��ǰֵ��ǰ��ֵ�����,˵��Key4�����ж���
			{
				delay(1);  		//��ʱȥ��
				if(keybuf4==Key4)	//�ж�Key4ɨ��ֵ��û�з����仯,����������	
				{
					if(backup4==1)	//Key4����������Ч
					{
						if(flag_gd==0)	//���ն���������	
						{
							H_GM++;	 	//�������޹����ȼ�1
							if(H_GM==256) {H_GM=0;}	//�ù�������0-255֮��	
						}
						else   			//���� ��ʱ����
						{
							hour++;		//ʱ��1
							if(hour==24) {hour=0;}	//hour������0-23֮��		
						}		
					}
					backup4=keybuf4;//���±���Key4��ǰֵ,�Ա������´αȽ�
				}
			}
			keybuf5=Key5;  		//��Key5��ǰֵɨ��ֵ�ݴ�
			if(keybuf5!=backup5)//Key5��ǰֵ��ǰ��ֵ�����,˵��Key5�����ж���
			{
				delay(1);  		//��ʱȥ��
				if(keybuf5==Key5)	//�ж�Key5ɨ��ֵ��û�з����仯,����������	
				{
					if(backup5==1)	//Key5����������Ч
					{
						if(flag_gd==0)	//���ն���������	
						{
							H_GM--;	 	//�������޹����ȼ�1
							if(H_GM==-1) {H_GM=255;}	//�ù�������0-255֮��	
						}
						else   			//���� ��ʱ����
						{
							hour--;		//ʱ��1
							if(hour==-1) {hour=23;}	//hour������0-23֮��		
						}		
					}
					backup5=keybuf5;//���±���Key5��ǰֵ,�Ա������´αȽ�
				}
			}
		}
		if(flag==2)	   			//���޹��ն�����(����)
		{
			keybuf4=Key4;  		//��Key4��ǰֵɨ��ֵ�ݴ�
			if(keybuf4!=backup4)//Key4��ǰֵ��ǰ��ֵ�����,˵��Key4�����ж���
			{
				delay(1);  		//��ʱȥ��
				if(keybuf4==Key4)	//�ж�Key4ɨ��ֵ��û�з����仯,����������	
				{
					if(backup4==1)	//Key4����������Ч
					{
						if(flag_gd==0)	//���ն���������	
						{
							L_GM++;	 	//�������޹����ȼ�1
							if(L_GM==256) {L_GM=0;}	//�ù�������0-255֮��	
						}
						else   			//���� ��������
						{
							min++;		//�ּ�1
							if(min==60) {min=0;}	//min������0-59֮��		
						}		
					}
					backup4=keybuf4;//���±���Key4��ǰֵ,�Ա������´αȽ�
				}
			}
			keybuf5=Key5;  		//��Key5��ǰֵɨ��ֵ�ݴ�
			if(keybuf5!=backup5)//Key5��ǰֵ��ǰ��ֵ�����,˵��Key5�����ж���
			{
				delay(1);  		//��ʱȥ��
				if(keybuf5==Key5)	//�ж�Key5ɨ��ֵ��û�з����仯,����������	
				{
					if(backup5==1)	//Key5����������Ч
					{
						if(flag_gd==0)	//���ն���������	
						{
							L_GM--;	 	//�������޹����ȼ�1
							if(L_GM==-1) {L_GM=255;}	//�ù�������0-255֮��	
						}
						else   			//���� ��������
						{
							min--;		//�ּ�1
							if(min==-1) {min=59;}	//min������0-59֮��			
						}		
					}
					backup5=keybuf5;//���±���Key5��ǰֵ,�Ա������´αȽ�
				}
			}
		}
		if(flag_gd==1&&flag==0)		//��������ʾ����ʱ�������
		{
			keybuf4=Key4;  		//��Key4��ǰֵɨ��ֵ�ݴ�
			if(keybuf4!=backup4)//Key4��ǰֵ��ǰ��ֵ�����,˵��Key4�����ж���
			{
				delay(1);  		//��ʱȥ��
				if(keybuf4==Key4)	//�ж�Key4ɨ��ֵ��û�з����仯,����������	
				{
					if(backup4==1)	//Key4����������Ч
					{
						Time_OFF_ON=1;	//��ʱ��������	
					}
					backup4=keybuf4;//���±���Key4��ǰֵ,�Ա������´αȽ�
				}
			}
			keybuf5=Key5;  		//��Key5��ǰֵɨ��ֵ�ݴ�
			if(keybuf5!=backup5)//Key5��ǰֵ��ǰ��ֵ�����,˵��Key5�����ж���
			{
				delay(1);  		//��ʱȥ��
				if(keybuf5==Key5)	//�ж�Key5ɨ��ֵ��û�з����仯,����������	
				{
					if(backup5==1)	//Key5����������Ч
					{
						Time_OFF_ON=0;	//��ʱ�رմ���	
					}
					backup5=keybuf5;//���±���Key5��ǰֵ,�Ա������´αȽ�
				}
			}											
		}
	}
}

void disp1()		//����ǿ����ʾ
{
	SMG_XS=table[adval/100];   		//ADC�����λ��ʾ		
	q_kz=1;b_kz=0;s_kz=1;g_kz=1;	//��λ������ʾ
	delay(t);						//��ʱ
	q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//����

	SMG_XS=table[adval%100/10];		//ADC���ʮλ��ʾ
	q_kz=1;b_kz=1;s_kz=0;g_kz=1;	//ʮλ������ʾ
	delay(t);						//��ʱ
	q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//����

	SMG_XS=table[adval%10];			//ADC�����λ��ʾ
	q_kz=1;b_kz=1;s_kz=1;g_kz=0;	//��λ������ʾ
	delay(t);	   					//��ʱ
	q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//����
}

void disp2()		//���޹��ն�����״̬
{
	SMG_XS=0x89; 					//��ʾ"H"
 	q_kz=0;b_kz=1;s_kz=1;g_kz=1;	//ǧλ������ʾ
	delay(t);						//��ʱ
	q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//����

	SMG_XS=table[H_GM/100]; 		//��ʾ���ù��նȰ�λ
 	q_kz=1;b_kz=0;s_kz=1;g_kz=1;	//��λ������ʾ
	delay(t);						//��ʱ
	q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//����

	SMG_XS=table[H_GM%100/10]; 		//��ʾ���ù��ն�ʮλ
	q_kz=1;b_kz=1;s_kz=0;g_kz=1;	//ʮλ������ʾ
	delay(t);	  					//��ʱ
	q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//����

	SMG_XS=table[H_GM%10];			//��ʾ���ù��նȸ�λ
	q_kz=1;b_kz=1;s_kz=1;g_kz=0;	//��λ������ʾ
	delay(t);			   			//��ʱ
	q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//����	
}

void disp3()		//���޹��ն�����״̬
{
	SMG_XS=0xc7; 					//��ʾ"L"
 	q_kz=0;b_kz=1;s_kz=1;g_kz=1;	//ǧλ������ʾ
	delay(t);						//��ʱ
	q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//����

	SMG_XS=table[L_GM/100]; 		//��ʾ���ù��նȰ�λ
 	q_kz=1;b_kz=0;s_kz=1;g_kz=1;	//��λ������ʾ
	delay(t);						//��ʱ
	q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//����

	SMG_XS=table[L_GM%100/10]; 		//��ʾ���ù��ն�ʮλ
	q_kz=1;b_kz=1;s_kz=0;g_kz=1;	//ʮλ������ʾ
	delay(t);	  					//��ʱ
	q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//����

	SMG_XS=table[L_GM%10];			//��ʾ���ù��նȸ�λ
	q_kz=1;b_kz=1;s_kz=1;g_kz=0;	//��λ������ʾ
	delay(t);			   			//��ʱ
	q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//����	
}

void disp4()	//��ʱ��ʾ
{
	j++;						//j��1		
	while(j==150) {j=0;}		//jΪ40ʱ,j��0
	if(j<75)					//jС��20  ʱ,�� ����ʾ			
	{
		SMG_XS=table[hour/10]; 			//��ʾСʱʮλ
	 	q_kz=0;b_kz=1;s_kz=1;g_kz=1;	//ǧλ������ʾ
		delay(t);						//��ʱ
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//����
	
		SMG_XS=table[hour%10]&0x7f; 	//��ʾСʱ��λ,��С����
	 	q_kz=1;b_kz=0;s_kz=1;g_kz=1;	//��λ������ʾ
		delay(t);						//��ʱ
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//����
	
		SMG_XS=table[min/10]; 			//��ʾ��ʮλ
		q_kz=1;b_kz=1;s_kz=0;g_kz=1;	//ʮλ������ʾ
		delay(t);	  					//��ʱ
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//����
	
		SMG_XS=table[min%10]&0x7f;		//��ʾ�ָ�λ,��С����
		q_kz=1;b_kz=1;s_kz=1;g_kz=0;	//��λ������ʾ
		delay(t);			   			//��ʱ
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//����
	}
	else		//����20<j<40	ʱ����ʾ,����ʾ	
	{
		SMG_XS=table[hour/10]; 			//��ʾСʱʮλ
	 	q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//����ʾ
		delay(t);						//��ʱ
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//����
	
		SMG_XS=table[hour%10]&0x7f; 	//��ʾСʱ��λ,��С����
	 	q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//����ʾ
		delay(t);						//��ʱ
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//����
	
		SMG_XS=table[min/10]; 			//��ʾ��ʮλ
		q_kz=1;b_kz=1;s_kz=0;g_kz=1;	//ʮλ������ʾ
		delay(t);	  					//��ʱ
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//����
	
		SMG_XS=table[min%10]&0x7f;		//��ʾ�ָ�λ,��С����
		q_kz=1;b_kz=1;s_kz=1;g_kz=0;	//��λ������ʾ
		delay(t);			   			//��ʱ
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//����		
	}		
}

void disp5()	//������ʾ
{
	j++;						//j��1		
	while(j==150) {j=0;}		//jΪ40ʱ,j��0
	if(j<75)					//jС��20  ʱ,�� ����ʾ			
	{
		SMG_XS=table[hour/10]; 			//��ʾСʱʮλ
	 	q_kz=0;b_kz=1;s_kz=1;g_kz=1;	//ǧλ������ʾ
		delay(t);						//��ʱ
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//����
	
		SMG_XS=table[hour%10]&0x7f; 	//��ʾСʱ��λ,��С����
	 	q_kz=1;b_kz=0;s_kz=1;g_kz=1;	//��λ������ʾ
		delay(t);						//��ʱ
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//����
	
		SMG_XS=table[min/10]; 			//��ʾ��ʮλ
		q_kz=1;b_kz=1;s_kz=0;g_kz=1;	//ʮλ������ʾ
		delay(t);	  					//��ʱ
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//����
	
		SMG_XS=table[min%10]&0x7f;		//��ʾ�ָ�λ,��С����
		q_kz=1;b_kz=1;s_kz=1;g_kz=0;	//��λ������ʾ
		delay(t);			   			//��ʱ
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//����
	}
	else		//����20<j<40	ʱ��ʾ,�ֲ���ʾ	
	{
		SMG_XS=table[hour/10]; 			//��ʾСʱʮλ
	 	q_kz=0;b_kz=1;s_kz=1;g_kz=1;	//����ʾ
		delay(t);						//��ʱ
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//����
	
		SMG_XS=table[hour%10]&0x7f; 	//��ʾСʱ��λ,��С����
	 	q_kz=1;b_kz=0;s_kz=1;g_kz=1;	//����ʾ
		delay(t);						//��ʱ
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//����
	
		SMG_XS=table[min/10]; 			//��ʾ��ʮλ
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//����ʾ
		delay(t);	  					//��ʱ
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//����
	
		SMG_XS=table[min%10]&0x7f;		//��ʾ�ָ�λ,��С����
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//����ʾ
		delay(t);			   			//��ʱ
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//����		
	}		
}

void disp6()		//ʱ����ʾ
{
	if(num>=10)		//0.5��ʱ�� ��ʾ��Сʱ��
	{
		SMG_XS=table[hour/10]; 			//��ʾСʱʮλ
		q_kz=0;b_kz=1;s_kz=1;g_kz=1;	//ǧλ������ʾ
		delay(t);						//��ʱ
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//����
		
		SMG_XS=table[hour%10]&0x7f; 	//��ʾСʱ��λ,��С����
		q_kz=1;b_kz=0;s_kz=1;g_kz=1;	//��λ������ʾ
		delay(t);						//��ʱ
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//����
		
		SMG_XS=table[min/10]; 			//��ʾ��ʮλ
		q_kz=1;b_kz=1;s_kz=0;g_kz=1;	//ʮλ������ʾ
		delay(t);	  					//��ʱ
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//����

		if(Time_OFF_ON==0)	   			//�����ʱ�����ر�
		{		
			SMG_XS=table[min%10];			//��ʾ�ָ�λ
		}
		else   							//����ʱ������
		{
			SMG_XS=table[min%10]&0x7f;;		//��ʾ�ָ�λ,��С����
		}
		q_kz=1;b_kz=1;s_kz=1;g_kz=0;	//��λ������ʾ
		delay(t);			   			//��ʱ
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//����
	}
	else  	
	{
		SMG_XS=table[hour/10]; 			//��ʾСʱʮλ
		q_kz=0;b_kz=1;s_kz=1;g_kz=1;	//ǧλ������ʾ
		delay(t);						//��ʱ
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//����
		
		SMG_XS=table[hour%10]; 			//��ʾСʱ��λ
		q_kz=1;b_kz=0;s_kz=1;g_kz=1;	//��λ������ʾ
		delay(t);						//��ʱ
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//����
		
		SMG_XS=table[min/10]; 			//��ʾ��ʮλ
		q_kz=1;b_kz=1;s_kz=0;g_kz=1;	//ʮλ������ʾ
		delay(t);	  					//��ʱ
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//����
		
		SMG_XS=table[min%10];			//��ʾ�ָ�λ
		q_kz=1;b_kz=1;s_kz=1;g_kz=0;	//��λ������ʾ
		delay(t);			   			//��ʱ
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//����		
	}	
}

void disp_cl()		//��ʾ����
{
	if(move==0)			//��ʱʱ��δ����������ʾ����
	{
		if(flag==0)		//������ʾ
		{
			if(flag_gd==0)	//���ģʽ��
			{
				disp1();		//����ǿ����ʾ		
			}
			else 			//����ʱģʽ��
			{
				disp6();	//ʱ����ʾ
				TR0=0;		//�رն�ʱ��0	
			}			
		}
		else if(flag==1)	//���޹��ն�������ʾ(��ʱ)
		{
			if(flag_gd==0)	//���ģʽ��
			{
				disp2();	//���޹��ն�����״̬		
			}
			else 			//����ʱģʽ��
			{
				disp4();	//��ʱ��ʾ	
			}			
		}
		else if(flag==2)	//���޹��ն�������ʾ(����)
		{
			if(flag_gd==0)	//���ģʽ��
			{
				disp3();	//���޹��ն�����״̬		
			}
			else 			//����ʱģʽ��
			{
				disp5();	//������ʾ	
			}
		}
	}
	else 		//��ʱ��ʱ�䵽��		
	{
		if(Time_OFF_ON==1)		//�����ʱ�򿪴���
		{
			OFF_ON=1;			//������
			TR0=1;	  			//������ʱ��0
			SMG_XS=table[1]; 	//�������ʾ"1"
			q_kz=1;b_kz=1;s_kz=1;g_kz=0;	//��λ��ʾ
		}	
		else    				//����ʱ����	   
		{
			OFF_ON=0;		  	//�����ر�
			TR0=1;				//������ʱ��0
			SMG_XS=table[0]; 	//�������ʾ"0"
			q_kz=1;b_kz=1;s_kz=1;g_kz=0;	//��λ��ʾ
		}			
	}
}

void GZQD()		//����ǿ��(����ADC0804ת����ʱ���д)
{
	static uint i=0;	//������ʱ����i
	CS=0;			  	
	WR=0;	 	//����ADת��
	delay(1);
	WR=1;
	CS=1;
	for(i=0;i<130;i++)	//ADת����Ҫһ��ʱ��,����forѭ������,����ʱһ�ᣬ��Ӱ����ʾ�Ͱ�������
	{
		disp_cl();	//��ʾ����
		Key_cl();  	//��������
	} 
	P1=0xff;		//��ȡADת��ֵǰ,�Ȱ�P1����1		
	CS=0;
	RD=0;	 		//AD��ʹ��
	delay(1);	  	
	adval=P1;  		//��ADת���õ�ֵ,���� adval
	RD=1;
	CS=1;		 	//��1��,Ϊ�´�ADת����׼��
	delay(1);
}		

void GM_cl()		//���նȴ���
{
	if(ms==1)		//�Զ�ģʽ��
	{										 
		if(adval<L_GM) 		{OFF_ON=1;TR0=1;}	//ʵ�ʹ���ǿ��С���趨���޹��ն�ֵʱ,�򿪴���(������ʱ��0)
		else if(adval>H_GM)	{OFF_ON=0;TR0=1;} 	//��֮����ǿ�ȴ����趨���޹��ն�ֵ,�رմ���(������ʱ��0)
		else 				{TR0=0;IA=0;IB=0;}	//����ǿ����������Χ��,��������(�رն�ʱ��0)
	}
}


void main()				//���������
{
	Time_init();		//��ʱ����ʼ��
	Time_OFF_ON=0;		//Ĭ�϶�ʱ�رմ���
	while(1)	 		//��ѭ��
	{ 	
		Key_cl();		//��������
		if(ms==1)		//�Զ�ģʽ��
		{
			if(flag_gd==0)	//���ģʽ
			{	
				TR1=0;		//�رն�ʱ��1
				GZQD();		//����ǿ��
				GM_cl();	//���նȴ���
			}
			else 			//����ʱ��ģʽ
			{
				TR1=1;		//������ʱ��1	
			}
			disp_cl();		//��ʾ����	
		}
		else				//�����ֶ�ģʽ�� 
		{
			TR1=0;			//�رն�ʱ��1	
		}				
	}
}

void Time0() interrupt 1  	//��ʱ��0�жϷ�����
{							//���ö�ʱ��0����PWMռ�ձ�,ռ�ձ�С,���Խ���ֱ�����ת��
	static uchar a=1;		//������ʱ����a=1
	TH0=0xf8;				//��ʱ2ms
	TL0=0xcc;
	if(OFF_ON==1)	//���������Ҫ��
	{
		if(Up==1) 	//������λ����Ϊ1(��ʾ����û�е�����λ��)
		{
			if(a<=1) {IA=1; IB=0;} 		//�����ת
			else	 {IA=0; IB=0;}
		}
		else		//��������λ��
		{
			IA=0; IB=0;					//���ֹͣ		
		} 
	}
	else 			//��������Ҫ�ر�
	{
		if(Down==1)	//������λ����Ϊ1(��ʾ����û�е�����λ��)
		{
			if(a<=1) {IA=0; IB=1;} 		//�����ת
			else	 {IA=0; IB=0;} 		
		}
		else		//��������λ��
		{
			IA=0; IB=0;					//���ֹͣ	
		}
	}
	a++;  	//a��1
	while(a==18) {a=1;}	//a������1��27֮��(a=1,���ת��1<a<18�����ת,������PWM�Ƚ�С,�ﵽת����)
}

void Time1() interrupt 3 	//��ʱ��1�жϷ�����(������ʱʱ��)
{
	TH1=0x4c; 		//50ms
	TL1=0x00;
	num++;			//num��1		
	while(num==20)	//1��ʱ�䵽
	{
		num=0;	  	//����0
		sec--;		//���1
		while(sec==-1)	//�������-1
		{
			sec=59;		//����59
			min--;	 	//�ּ�1
			while(min==-1)	//���ֵ���-1
			{
				min=59;		//����59
				hour--;	  	//Сʱ��1
			}
		}
		while(hour==0&&min==0&&sec==0) 	//��ʱʱ�䵽
		{
			hour=12;min=0;sec=0;	//ʱ���ʼ���ָ� 12.00.00
			move=1;		//��ʱʱ�䵽	
		}
	}
}