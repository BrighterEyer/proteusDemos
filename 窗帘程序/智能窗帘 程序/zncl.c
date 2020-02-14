/*********51单片机智能窗帘设计*********/
/*************JJ电子直营店*************/
/***************原创设计***************/

#include <REGX52.H>	  			//头文件
#define uchar unsigned char
#define uint unsigned int

#define CS	P3_0		//ADC0804 CS端口
#define RD	P3_1		//ADC0804 RD端口
#define WR	P3_2		//ADC0804 WR端口
#define g_kz	P2_5 	//数码管个位控制
#define s_kz	P2_4	//数码管十位控制
#define b_kz	P2_3	//数码管百位控制
#define q_kz	P2_2	//码管千位控制
#define SMG_XS	P0		//数码管显示端口
#define AD_data	P1		//ADC0804输出端口
#define Up		P2_1	//上限限位开关端口
#define Down 	P2_0	//下限限位开关端口
#define Key1	P3_3	//手动/自动切换 按键
#define Key2	P3_4	//定时/光控功能切换 按键
#define Key3	P3_5	//设置 按键
#define Key4	P3_6	//加 按键 (手动模式:打开窗帘 定时模式：定时打开窗帘)
#define Key5	P3_7	//减 按键 (手动模式:关闭窗帘 定时模式：定时关闭窗帘)

#define IA		P2_7	//直流电机控制端口
#define IB		P2_6	//直流电机控制端口

int adval; 		//ADC 输出变量
int j;			//定义循环变量 i j
uchar flag=0;	//显示标志位(0:正常显示 1:上限光照度设置(调时) 2:下限光照度设置(调分)				 
bit flag_gd=0;	//光控/定时标志位(0:当前光照度 1:定时时间 )	
bit ms=0;		//模式(0:手动模式 1:自动模式)
bit move=0;		//定时时间到标志位(0:时间未到 1:时间到)
bit OFF_ON=0;	//电机正反转标志位,表示窗帘打开或关闭(0:窗帘关闭 1:窗帘打开)
bit Time_OFF_ON=0;//定时时间到执行窗帘打开或关闭(0:定时关闭窗帘 1:定时打开窗帘)

char hour=12,min=0;sec=0; //定义时间 “时 分 秒”变量 上电默认12.00.00
char num=0;	//时间基数
uint H_GM=240;	//定义上限光敏度设置变量，上电默认200
uint L_GM=100;	//定义下限光敏度设置变量，上电默认100
uchar t=1; 		//数码管动态扫描延时参数
                
uchar code table[]={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90};	
//数码管显示数组      0    1    2    3    4    5    6    7    8    9

void delay(uint time)	   	//延时函数
{
	uint x,y; 			//定义临时变量 x y
	for(x=time;x>0;x--)
	for(y=110;y>0;y--);	//空操作
}

void  Time_init()		//定时器初始化
{
	EA=1;
	TMOD=0x11;

	/*定时器0初始化(产生PWM控制电机转速)*/	
	TH0=0xf8;	//定时2ms
	TL0=0xcc;
	ET0=1;		//使能T0中断
	TR0=0;		//先不启动定时器0
	/*定时器1初始化(产生50ms定时时间)*/
	TH1=0x4c; 		//50ms
	TL1=0x00;
	ET1=1;
	TR1=0;		//先不启动定时器1	
}

void Key_cl()
{
	static bit keybuf1=1;  	//Key1值暂存,临时保存按键扫描值
	static bit backup1=1;  	//Key1值备份,保存前一次的扫描值
	static bit keybuf2=1; 	//Key2值暂存,临时保存按键扫描值	
	static bit backup2=1;	//Key2值备份,保存前一次的扫描值
	static bit keybuf3=1;	//Key3值暂存,临时保存按键扫描值
	static bit backup3=1; 	//Key3值备份,保存前一次的扫描值
	static bit keybuf4=1;	//Key4值暂存,临时保存按键扫描值
	static bit backup4=1; 	//Key4值备份,保存前一次的扫描值
	static bit keybuf5=1;	//Key5值暂存,临时保存按键扫描值
	static bit backup5=1; 	//Key5值备份,保存前一次的扫描值
	keybuf1=Key1;  			//把Key1当前值扫描值暂存
	if(keybuf1!=backup1)	//Key1当前值与前次值不相等,说明Key1按键有动作
	{
		delay(1);			//延时去抖
		if(keybuf1==Key1)	//判断Key1扫描值有没有发生变化,即按键抖动
		{
			if(backup1==1)	//Key1按键按下有效
			{
				move=0;	   	//重置时间标志位为0(时间未到)
				TR0=0;		//关闭定时器0
				ms=~ms;		// 模式取反
				q_kz=1;b_kz=1;s_kz=1;g_kz=1;//关闭数码管显示
				IA=0;IB=0;	//窗帘停止	
			}
			backup1=keybuf1; //更新备份Key1当前值,以备进行下次比较
		}
	}
	if(ms==0)  			//手动模式下
	{
		if(Key4==0)		//当打开窗帘按键 按下
		{
			delay(10);	//延时去抖
			if(Key4==0)	//再判断按键是否按下
			{
				move=0;				//重置时间标志位为0(时间未到)
				OFF_ON=1;	 		//窗帘打开
				TR0=1;				//开启定时器0
				SMG_XS=table[1]; 	//数码管显示"1"
				q_kz=1;b_kz=1;s_kz=1;g_kz=0;	//个位显示
				while(Key4==0);	//等待按键松开
				TR0=0;			//关闭定时器0
				IA=0;IB=0;		//窗帘停止
				q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//关闭数码管	
			}
		}
		if(Key5==0)		//当关闭窗帘按键 按下
		{
			delay(10); 	//延时去抖	
			if(Key5==0)	//再判断按键是否按下
			{
				move=0;				//重置时间标志位为0(时间未到)
				OFF_ON=0;	 		//窗帘关闭
				TR0=1;				//开启定时器0
				SMG_XS=table[0]; 	//数码管显示"0"
				q_kz=1;b_kz=1;s_kz=1;g_kz=0;	//个位显示
				while(Key5==0);	//等待按键松开
				TR0=0;			//关闭定时器0
				IA=0;IB=0;	//窗帘停止	
				q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//关闭数码管
			}
		}		
	}
	else 		//自动模式下
	{
		keybuf2=Key2;  			//把Key2当前值扫描值暂存
		if(keybuf2!=backup2)	//Key2当前值与前次值不相等,说明Key2按键有动作
		{
			delay(1);			//延时去抖
			if(keybuf2==Key2)	//判断Key2扫描值有没有发生变化,即按键抖动
			{
				if(backup2==1)	//Key2按键按下有效
				{	
					move=0;		//重置时间标志位为0(时间未到)
					flag_gd=~flag_gd; //光控/定时标志位取反(0:当前光照度 1:定时时间 )								
				}
				backup2=keybuf2; //更新备份Key2当前值,以备进行下次比较
			}
		}
		keybuf3=Key3; 		//把Key3当前值扫描值暂存			
		if(keybuf3!=backup3)//Key3当前值与前次值不相等,说明Key3按键有动作
		{
			delay(1);		//延时去抖
			if(keybuf3==Key3)	//判断Key3扫描值有没有发生变化,即按键抖动	
			{
				if(backup3==1)	//Key3按键按下有效
				{
					move=0;		//重置时间标志位为0(时间未到)
					TR0=0;		//关闭定时器0
					flag++;
					if(flag==3) {flag=0;}	//让flag保持在0-2之间
				}
				backup3=keybuf3;//更新备份Key3当前值,以备进行下次比较
			}
		}
		if(flag==1)				//允许上限光照度设置(调时)			
		{
			keybuf4=Key4;  		//把Key4当前值扫描值暂存
			if(keybuf4!=backup4)//Key4当前值与前次值不相等,说明Key4按键有动作
			{
				delay(1);  		//延时去抖
				if(keybuf4==Key4)	//判断Key4扫描值有没有发生变化,即按键抖动	
				{
					if(backup4==1)	//Key4按键按下有效
					{
						if(flag_gd==0)	//光照度设置允许	
						{
							H_GM++;	 	//设置上限光敏度加1
							if(H_GM==256) {H_GM=0;}	//让光敏度在0-255之间	
						}
						else   			//否则 调时允许
						{
							hour++;		//时加1
							if(hour==24) {hour=0;}	//hour保持在0-23之间		
						}		
					}
					backup4=keybuf4;//更新备份Key4当前值,以备进行下次比较
				}
			}
			keybuf5=Key5;  		//把Key5当前值扫描值暂存
			if(keybuf5!=backup5)//Key5当前值与前次值不相等,说明Key5按键有动作
			{
				delay(1);  		//延时去抖
				if(keybuf5==Key5)	//判断Key5扫描值有没有发生变化,即按键抖动	
				{
					if(backup5==1)	//Key5按键按下有效
					{
						if(flag_gd==0)	//光照度设置允许	
						{
							H_GM--;	 	//设置上限光敏度减1
							if(H_GM==-1) {H_GM=255;}	//让光敏度在0-255之间	
						}
						else   			//否则 调时允许
						{
							hour--;		//时减1
							if(hour==-1) {hour=23;}	//hour保持在0-23之间		
						}		
					}
					backup5=keybuf5;//更新备份Key5当前值,以备进行下次比较
				}
			}
		}
		if(flag==2)	   			//下限光照度设置(调分)
		{
			keybuf4=Key4;  		//把Key4当前值扫描值暂存
			if(keybuf4!=backup4)//Key4当前值与前次值不相等,说明Key4按键有动作
			{
				delay(1);  		//延时去抖
				if(keybuf4==Key4)	//判断Key4扫描值有没有发生变化,即按键抖动	
				{
					if(backup4==1)	//Key4按键按下有效
					{
						if(flag_gd==0)	//光照度设置允许	
						{
							L_GM++;	 	//设置下限光敏度加1
							if(L_GM==256) {L_GM=0;}	//让光敏度在0-255之间	
						}
						else   			//否则 调分允许
						{
							min++;		//分加1
							if(min==60) {min=0;}	//min保持在0-59之间		
						}		
					}
					backup4=keybuf4;//更新备份Key4当前值,以备进行下次比较
				}
			}
			keybuf5=Key5;  		//把Key5当前值扫描值暂存
			if(keybuf5!=backup5)//Key5当前值与前次值不相等,说明Key5按键有动作
			{
				delay(1);  		//延时去抖
				if(keybuf5==Key5)	//判断Key5扫描值有没有发生变化,即按键抖动	
				{
					if(backup5==1)	//Key5按键按下有效
					{
						if(flag_gd==0)	//光照度设置允许	
						{
							L_GM--;	 	//设置下限光敏度减1
							if(L_GM==-1) {L_GM=255;}	//让光敏度在0-255之间	
						}
						else   			//否则 调分允许
						{
							min--;		//分减1
							if(min==-1) {min=59;}	//min保持在0-59之间			
						}		
					}
					backup5=keybuf5;//更新备份Key5当前值,以备进行下次比较
				}
			}
		}
		if(flag_gd==1&&flag==0)		//在正常显示倒计时的情况下
		{
			keybuf4=Key4;  		//把Key4当前值扫描值暂存
			if(keybuf4!=backup4)//Key4当前值与前次值不相等,说明Key4按键有动作
			{
				delay(1);  		//延时去抖
				if(keybuf4==Key4)	//判断Key4扫描值有没有发生变化,即按键抖动	
				{
					if(backup4==1)	//Key4按键按下有效
					{
						Time_OFF_ON=1;	//定时开启窗帘	
					}
					backup4=keybuf4;//更新备份Key4当前值,以备进行下次比较
				}
			}
			keybuf5=Key5;  		//把Key5当前值扫描值暂存
			if(keybuf5!=backup5)//Key5当前值与前次值不相等,说明Key5按键有动作
			{
				delay(1);  		//延时去抖
				if(keybuf5==Key5)	//判断Key5扫描值有没有发生变化,即按键抖动	
				{
					if(backup5==1)	//Key5按键按下有效
					{
						Time_OFF_ON=0;	//定时关闭窗帘	
					}
					backup5=keybuf5;//更新备份Key5当前值,以备进行下次比较
				}
			}											
		}
	}
}

void disp1()		//光照强度显示
{
	SMG_XS=table[adval/100];   		//ADC输出百位显示		
	q_kz=1;b_kz=0;s_kz=1;g_kz=1;	//百位允许显示
	delay(t);						//延时
	q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//消隐

	SMG_XS=table[adval%100/10];		//ADC输出十位显示
	q_kz=1;b_kz=1;s_kz=0;g_kz=1;	//十位允许显示
	delay(t);						//延时
	q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//消隐

	SMG_XS=table[adval%10];			//ADC输出个位显示
	q_kz=1;b_kz=1;s_kz=1;g_kz=0;	//个位允许显示
	delay(t);	   					//延时
	q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//消隐
}

void disp2()		//上限光照度设置状态
{
	SMG_XS=0x89; 					//显示"H"
 	q_kz=0;b_kz=1;s_kz=1;g_kz=1;	//千位允许显示
	delay(t);						//延时
	q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//消隐

	SMG_XS=table[H_GM/100]; 		//显示设置光照度百位
 	q_kz=1;b_kz=0;s_kz=1;g_kz=1;	//百位允许显示
	delay(t);						//延时
	q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//消隐

	SMG_XS=table[H_GM%100/10]; 		//显示设置光照度十位
	q_kz=1;b_kz=1;s_kz=0;g_kz=1;	//十位允许显示
	delay(t);	  					//延时
	q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//消隐

	SMG_XS=table[H_GM%10];			//显示设置光照度个位
	q_kz=1;b_kz=1;s_kz=1;g_kz=0;	//个位允许显示
	delay(t);			   			//延时
	q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//消隐	
}

void disp3()		//下限光照度设置状态
{
	SMG_XS=0xc7; 					//显示"L"
 	q_kz=0;b_kz=1;s_kz=1;g_kz=1;	//千位允许显示
	delay(t);						//延时
	q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//消隐

	SMG_XS=table[L_GM/100]; 		//显示设置光照度百位
 	q_kz=1;b_kz=0;s_kz=1;g_kz=1;	//百位允许显示
	delay(t);						//延时
	q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//消隐

	SMG_XS=table[L_GM%100/10]; 		//显示设置光照度十位
	q_kz=1;b_kz=1;s_kz=0;g_kz=1;	//十位允许显示
	delay(t);	  					//延时
	q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//消隐

	SMG_XS=table[L_GM%10];			//显示设置光照度个位
	q_kz=1;b_kz=1;s_kz=1;g_kz=0;	//个位允许显示
	delay(t);			   			//延时
	q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//消隐	
}

void disp4()	//调时显示
{
	j++;						//j加1		
	while(j==150) {j=0;}		//j为40时,j清0
	if(j<75)					//j小于20  时,分 都显示			
	{
		SMG_XS=table[hour/10]; 			//显示小时十位
	 	q_kz=0;b_kz=1;s_kz=1;g_kz=1;	//千位允许显示
		delay(t);						//延时
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//消隐
	
		SMG_XS=table[hour%10]&0x7f; 	//显示小时个位,带小数点
	 	q_kz=1;b_kz=0;s_kz=1;g_kz=1;	//百位允许显示
		delay(t);						//延时
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//消隐
	
		SMG_XS=table[min/10]; 			//显示分十位
		q_kz=1;b_kz=1;s_kz=0;g_kz=1;	//十位允许显示
		delay(t);	  					//延时
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//消隐
	
		SMG_XS=table[min%10]&0x7f;		//显示分个位,带小数点
		q_kz=1;b_kz=1;s_kz=1;g_kz=0;	//个位允许显示
		delay(t);			   			//延时
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//消隐
	}
	else		//否则20<j<40	时不显示,分显示	
	{
		SMG_XS=table[hour/10]; 			//显示小时十位
	 	q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//不显示
		delay(t);						//延时
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//消隐
	
		SMG_XS=table[hour%10]&0x7f; 	//显示小时个位,带小数点
	 	q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//不显示
		delay(t);						//延时
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//消隐
	
		SMG_XS=table[min/10]; 			//显示分十位
		q_kz=1;b_kz=1;s_kz=0;g_kz=1;	//十位允许显示
		delay(t);	  					//延时
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//消隐
	
		SMG_XS=table[min%10]&0x7f;		//显示分个位,带小数点
		q_kz=1;b_kz=1;s_kz=1;g_kz=0;	//个位允许显示
		delay(t);			   			//延时
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//消隐		
	}		
}

void disp5()	//调分显示
{
	j++;						//j加1		
	while(j==150) {j=0;}		//j为40时,j清0
	if(j<75)					//j小于20  时,分 都显示			
	{
		SMG_XS=table[hour/10]; 			//显示小时十位
	 	q_kz=0;b_kz=1;s_kz=1;g_kz=1;	//千位允许显示
		delay(t);						//延时
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//消隐
	
		SMG_XS=table[hour%10]&0x7f; 	//显示小时个位,带小数点
	 	q_kz=1;b_kz=0;s_kz=1;g_kz=1;	//百位允许显示
		delay(t);						//延时
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//消隐
	
		SMG_XS=table[min/10]; 			//显示分十位
		q_kz=1;b_kz=1;s_kz=0;g_kz=1;	//十位允许显示
		delay(t);	  					//延时
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//消隐
	
		SMG_XS=table[min%10]&0x7f;		//显示分个位,带小数点
		q_kz=1;b_kz=1;s_kz=1;g_kz=0;	//个位允许显示
		delay(t);			   			//延时
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//消隐
	}
	else		//否则20<j<40	时显示,分不显示	
	{
		SMG_XS=table[hour/10]; 			//显示小时十位
	 	q_kz=0;b_kz=1;s_kz=1;g_kz=1;	//不显示
		delay(t);						//延时
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//消隐
	
		SMG_XS=table[hour%10]&0x7f; 	//显示小时个位,带小数点
	 	q_kz=1;b_kz=0;s_kz=1;g_kz=1;	//不显示
		delay(t);						//延时
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//消隐
	
		SMG_XS=table[min/10]; 			//显示分十位
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//不显示
		delay(t);	  					//延时
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//消隐
	
		SMG_XS=table[min%10]&0x7f;		//显示分个位,带小数点
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//不显示
		delay(t);			   			//延时
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//消隐		
	}		
}

void disp6()		//时间显示
{
	if(num>=10)		//0.5秒时间 显示带小时点
	{
		SMG_XS=table[hour/10]; 			//显示小时十位
		q_kz=0;b_kz=1;s_kz=1;g_kz=1;	//千位允许显示
		delay(t);						//延时
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//消隐
		
		SMG_XS=table[hour%10]&0x7f; 	//显示小时个位,带小数点
		q_kz=1;b_kz=0;s_kz=1;g_kz=1;	//百位允许显示
		delay(t);						//延时
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//消隐
		
		SMG_XS=table[min/10]; 			//显示分十位
		q_kz=1;b_kz=1;s_kz=0;g_kz=1;	//十位允许显示
		delay(t);	  					//延时
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//消隐

		if(Time_OFF_ON==0)	   			//如果定时窗帘关闭
		{		
			SMG_XS=table[min%10];			//显示分个位
		}
		else   							//否则定时窗帘打开
		{
			SMG_XS=table[min%10]&0x7f;;		//显示分个位,带小数点
		}
		q_kz=1;b_kz=1;s_kz=1;g_kz=0;	//个位允许显示
		delay(t);			   			//延时
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//消隐
	}
	else  	
	{
		SMG_XS=table[hour/10]; 			//显示小时十位
		q_kz=0;b_kz=1;s_kz=1;g_kz=1;	//千位允许显示
		delay(t);						//延时
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//消隐
		
		SMG_XS=table[hour%10]; 			//显示小时个位
		q_kz=1;b_kz=0;s_kz=1;g_kz=1;	//百位允许显示
		delay(t);						//延时
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//消隐
		
		SMG_XS=table[min/10]; 			//显示分十位
		q_kz=1;b_kz=1;s_kz=0;g_kz=1;	//十位允许显示
		delay(t);	  					//延时
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//消隐
		
		SMG_XS=table[min%10];			//显示分个位
		q_kz=1;b_kz=1;s_kz=1;g_kz=0;	//个位允许显示
		delay(t);			   			//延时
		q_kz=1;b_kz=1;s_kz=1;g_kz=1;	//消隐		
	}	
}

void disp_cl()		//显示处理
{
	if(move==0)			//定时时间未到，处理显示函数
	{
		if(flag==0)		//正常显示
		{
			if(flag_gd==0)	//光控模式下
			{
				disp1();		//光照强度显示		
			}
			else 			//否则定时模式下
			{
				disp6();	//时间显示
				TR0=0;		//关闭定时器0	
			}			
		}
		else if(flag==1)	//上限光照度设置显示(调时)
		{
			if(flag_gd==0)	//光控模式下
			{
				disp2();	//上限光照度设置状态		
			}
			else 			//否则定时模式下
			{
				disp4();	//调时显示	
			}			
		}
		else if(flag==2)	//下限光照度设置显示(调分)
		{
			if(flag_gd==0)	//光控模式下
			{
				disp3();	//下限光照度设置状态		
			}
			else 			//否则定时模式下
			{
				disp5();	//调分显示	
			}
		}
	}
	else 		//定时器时间到了		
	{
		if(Time_OFF_ON==1)		//如果定时打开窗帘
		{
			OFF_ON=1;			//窗帘打开
			TR0=1;	  			//开启定时器0
			SMG_XS=table[1]; 	//数码管显示"1"
			q_kz=1;b_kz=1;s_kz=1;g_kz=0;	//个位显示
		}	
		else    				//否则定时窗帘	   
		{
			OFF_ON=0;		  	//窗帘关闭
			TR0=1;				//开启定时器0
			SMG_XS=table[0]; 	//数码管显示"0"
			q_kz=1;b_kz=1;s_kz=1;g_kz=0;	//个位显示
		}			
	}
}

void GZQD()		//光照强度(根据ADC0804转换器时序编写)
{
	static uint i=0;	//定义临时变量i
	CS=0;			  	
	WR=0;	 	//启动AD转换
	delay(1);
	WR=1;
	CS=1;
	for(i=0;i<130;i++)	//AD转换需要一点时间,利用for循环处理,来延时一会，不影响显示和按键处理
	{
		disp_cl();	//显示处理
		Key_cl();  	//按键处理
	} 
	P1=0xff;		//读取AD转换值前,先把P1口置1		
	CS=0;
	RD=0;	 		//AD读使能
	delay(1);	  	
	adval=P1;  		//把AD转换好的值,赋给 adval
	RD=1;
	CS=1;		 	//置1后,为下次AD转换做准备
	delay(1);
}		

void GM_cl()		//光照度处理
{
	if(ms==1)		//自动模式下
	{										 
		if(adval<L_GM) 		{OFF_ON=1;TR0=1;}	//实际光照强度小于设定下限光照度值时,打开窗帘(启动定时器0)
		else if(adval>H_GM)	{OFF_ON=0;TR0=1;} 	//反之光照强度大于设定上限光照度值,关闭窗帘(启动定时器0)
		else 				{TR0=0;IA=0;IB=0;}	//光照强度在正常范围内,窗帘不动(关闭定时器0)
	}
}


void main()				//主函数入口
{
	Time_init();		//定时器初始化
	Time_OFF_ON=0;		//默认定时关闭窗帘
	while(1)	 		//主循环
	{ 	
		Key_cl();		//按键处理
		if(ms==1)		//自动模式下
		{
			if(flag_gd==0)	//光控模式
			{	
				TR1=0;		//关闭定时器1
				GZQD();		//光照强度
				GM_cl();	//光照度处理
			}
			else 			//否则定时器模式
			{
				TR1=1;		//开启定时器1	
			}
			disp_cl();		//显示处理	
		}
		else				//否则手动模式下 
		{
			TR1=0;			//关闭定时器1	
		}				
	}
}

void Time0() interrupt 1  	//定时器0中断服务函数
{							//利用定时器0产生PWM占空比,占空比小,可以降低直流电机转速
	static uchar a=1;		//定义临时变量a=1
	TH0=0xf8;				//定时2ms
	TL0=0xcc;
	if(OFF_ON==1)	//如果窗帘需要打开
	{
		if(Up==1) 	//上限限位开关为1(表示窗帘没有到上限位置)
		{
			if(a<=1) {IA=1; IB=0;} 		//电机正转
			else	 {IA=0; IB=0;}
		}
		else		//否则到上限位置
		{
			IA=0; IB=0;					//电机停止		
		} 
	}
	else 			//否则窗帘需要关闭
	{
		if(Down==1)	//下限限位开关为1(表示窗帘没有到下限位置)
		{
			if(a<=1) {IA=0; IB=1;} 		//电机反转
			else	 {IA=0; IB=0;} 		
		}
		else		//否则到下限位置
		{
			IA=0; IB=0;					//电机停止	
		}
	}
	a++;  	//a加1
	while(a==18) {a=1;}	//a保持在1到27之间(a=1,电机转，1<a<18电机不转,这样的PWM比较小,达到转速慢)
}

void Time1() interrupt 3 	//定时器1中断服务函数(产生定时时间)
{
	TH1=0x4c; 		//50ms
	TL1=0x00;
	num++;			//num加1		
	while(num==20)	//1秒时间到
	{
		num=0;	  	//秒清0
		sec--;		//秒减1
		while(sec==-1)	//当秒等于-1
		{
			sec=59;		//秒置59
			min--;	 	//分减1
			while(min==-1)	//当分等于-1
			{
				min=59;		//分置59
				hour--;	  	//小时减1
			}
		}
		while(hour==0&&min==0&&sec==0) 	//定时时间到
		{
			hour=12;min=0;sec=0;	//时间初始化恢复 12.00.00
			move=1;		//定时时间到	
		}
	}
}