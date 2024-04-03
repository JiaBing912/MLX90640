#include <rtthread.h>
#include <rtdevice.h>
#include "MLX90640_DISPLAY.h"

#define  FPS2HZ   0x02
#define  FPS4HZ   0x03
#define  FPS8HZ   0x04
#define  FPS16HZ  0x05
#define  FPS32HZ  0x06

#define  MLX90640_ADDR 0x33
#define	 RefreshRate FPS8HZ
#define  TA_SHIFT 8 //Default shift for MLX90640 in open air

#define SRC_HEIGHT 24
#define SRC_WIDTH 32
#define DEST_HEIGHT 240
#define DEST_WIDTH 320

static uint16_t eeMLX90640[832];
static float mlx90640To[768];
static int frame[834];
float src_matrix[24][32];
float dest_matrix[240][320];
float emissivity = 0.95;
int status;
paramsMLX90640 mlx90640;

int MLX90640_INIT(void)
{
    MLX90640_SetRefreshRate(MLX90640_ADDR, RefreshRate);
    MLX90640_SetChessMode(MLX90640_ADDR);
    status = MLX90640_DumpEE(MLX90640_ADDR, eeMLX90640); //��ȡ���ؽ�������
    if(status != 0) rt_kprintf("\r\nload system parameters error with code:%d\r\n", status);
    status = MLX90640_ExtractParameters(eeMLX90640, &mlx90640); //������������
    if(status != 0) rt_kprintf("\r\nParameter extraction failed with error code:%d\r\n", status);
}
INIT_DEVICE_EXPORT(MLX90640_INIT);

void bilinear_interpolation(float src_matrix[SRC_HEIGHT][SRC_WIDTH], float dest_matrix[DEST_HEIGHT][DEST_WIDTH])
{
    float x_ratio = (float)(SRC_WIDTH - 1) / (float)(DEST_WIDTH - 1);
    float y_ratio = (float)(SRC_HEIGHT - 1) / (float)(DEST_HEIGHT - 1);

    for(int y = 0; y < DEST_HEIGHT; y++)
    {
        for(int x = 0; x < DEST_WIDTH; x++)
        {
            float x_src = x * x_ratio;
            float y_src = y * y_ratio;

            int x1 = (int)x_src;
            int y1 = (int)y_src;
            int x2 = x1 + 1;
            int y2 = y1 + 1;

            float x_diff = x_src - x1;
            float y_diff = y_src - y1;

            // Bilinear interpolation
            float top_left = src_matrix[y1][x1] * (1 - x_diff) * (1 - y_diff);
            float top_right = src_matrix[y1][x2] * x_diff * (1 - y_diff);
            float bottom_left = src_matrix[y2][x1] * (1 - x_diff) * y_diff;
            float bottom_right = src_matrix[y2][x2] * x_diff * y_diff;

            dest_matrix[y][x] = top_left + top_right + bottom_left + bottom_right;
        }
    }
}
float max_temperature;
void find_max_temperature(float src_matrix[SRC_HEIGHT][SRC_WIDTH], float dest_matrix[DEST_HEIGHT][DEST_WIDTH], int* max_dest_row, int* max_dest_col)
{
    // �ҵ�����¶ȼ���λ��
    max_temperature = src_matrix[0][0];
//	printf("max_temperature[0][0]: %f\r\n", src_matrix[0][0]);
    int max_row = 0;
    int max_col = 0;
    for(int row = 0; row < SRC_HEIGHT; row++)
    {
        for(int col = 0; col < SRC_WIDTH; col++)
        {
            if(src_matrix[row][col] > max_temperature)
            {
                max_temperature = src_matrix[row][col];
                max_row = row;
                max_col = col;
            }
        }
    }
//	printf("max_temperature[%d][%d]: %f\r\n", max_row,max_col,max_temperature);
    // ������¶�ֵ���Ƶ�Ŀ��������Ӧλ��
    *max_dest_row = max_row * (DEST_HEIGHT / SRC_HEIGHT);  // Ŀ������е�������
    *max_dest_col = max_col * (DEST_WIDTH / SRC_WIDTH);    // Ŀ������е�������
    dest_matrix[*max_dest_row][*max_dest_col] = max_temperature;
}
float min_temperature;
void find_min_temperature(float src_matrix[SRC_HEIGHT][SRC_WIDTH], float dest_matrix[DEST_HEIGHT][DEST_WIDTH], int* min_dest_row, int* min_dest_col)
{
    // �ҵ�����¶ȼ���λ��
    min_temperature = src_matrix[0][0];
//	printf("min_temperature[0][0]==: %f   ", src_matrix[0][0]);
    int min_row = 0;
    int min_col = 0;
    for(int row = 0; row < SRC_HEIGHT; row++)
    {
        for(int col = 0; col < SRC_WIDTH; col++)
        {
            if(src_matrix[row][col] < min_temperature)
            {
                min_temperature = src_matrix[row][col];
                min_row = row;
                min_col = col;
            }
        }
    }
//		printf("min_temperature[%d][%d]: %f\r\n", min_row,min_col,min_temperature);
    // ������¶�ֵ���Ƶ�Ŀ��������Ӧλ��
    *min_dest_row = min_row * (DEST_HEIGHT / SRC_HEIGHT);  // Ŀ������е�������
    *min_dest_col = min_col * (DEST_WIDTH / SRC_WIDTH);    // Ŀ������е�������
    dest_matrix[*min_dest_row][*min_dest_col] = min_temperature;
}

struct drv_lcd_device
{
    struct rt_device parent;
    struct rt_device_graphic_info lcd_info;
};

uint16_t calculate_color(float temperature)
{
    // ���¶�����ӳ�䵽 RGB565 ��ɫ��Χ
    int red = (int)((temperature / 7.0) * 31) << 11; // ��ɫռ�ø� 5 λ
    int green = 5; // �ɸ�����Ҫ����
    int blue = 31 - (int)((temperature / 90.0) * 30); // ��ɫռ�õ� 5 λ

    // �ϲ���ɫ���������� RGB565 ֵ
    return (red & 0xF800) | ((green & 0x07E0) << 5) | (blue & 0x001F);
}

uint16_t Temp2RGB(float temperature)
{
	double miniNum = 0.0002;
	float L = max_temperature;
	float PI = 3.14;

		/* ת�¶�Ϊ�Ҷ� */
		float grey = (temperature*255)/80;
		/* ����HSI */
		float I = grey,H = (2*PI*grey)/L;
		float S;
		/* grey < L/2 */
		if((grey-L/2) < miniNum){
		//if(grey<L/2){
			S = 1.5 * grey;
		}else{
			S = 1.5 * (L-grey);
		}
		/* ����RGB */
		float V1 = S* cos(H);
		float V2 = S* sin(H);
		float R = I - 0.204*V1 + 0.612*V2;
		float G = I - 0.204*V1 - 0.612*V2;
		float B = I + 0.408*V1; 
		/* תΪ16bits RGB[5-6-5]ɫ�� */
		/* 
			(2^5-1)/(2^8-1) = 0.12 
			(2^6-1)/(2^8-1) = 0.24
		*/		
		uint16_t rbits = (R*0.125);
		uint16_t gbits = (G*0.250);
		uint16_t bbits = (B*0.125);
		return (rbits<<11)|(gbits<<5)|bbits;
	
}


#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       16384
#define THREAD_TIMESLICE        5
static rt_thread_t tid1 = RT_NULL;
static void thread1_entry(void* parameter)
{
    int max_dest_row, max_dest_col;
    int min_dest_row, min_dest_col;
    struct drv_lcd_device* lcd;
    struct rt_device_rect_info rect_info;
    rect_info.x = 0;
    rect_info.y = 0;
    rect_info.width = LCD_WIDTH;
    rect_info.height = LCD_HEIGHT;

    lcd = (struct drv_lcd_device*)rt_device_find("lcd");

    /* red */
    for(int i = 0; i < LCD_BUF_SIZE / 2; i++)
    {
        lcd->lcd_info.framebuffer[2 * i] = 0xFF;
        lcd->lcd_info.framebuffer[2 * i + 1] = 0xFF;
    }

    lcd->parent.control(&lcd->parent, RTGRAPHIC_CTRL_RECT_UPDATE, &rect_info);



    while(1)
    {
        int status = MLX90640_GetFrameData(MLX90640_ADDR, frame);  //��ȡһ֡ԭʼ����
        if(status < 0)
        {
            rt_kprintf("GetFrame Error: %d\r\n", status);
        }
        float vdd = MLX90640_GetVdd(frame, &mlx90640);  //���� Vdd�������п��ޣ�
        float Ta = MLX90640_GetTa(frame, &mlx90640);    //����ʵʱ����¶�
        float tr = Ta - TA_SHIFT; //Reflected temperature based on the sensor ambient temperature

        MLX90640_CalculateTo(frame, &mlx90640, emissivity , tr, mlx90640To);            //�������ص��¶�
        MLX90640_BadPixelsCorrection(mlx90640.brokenPixels, mlx90640To, 1, &mlx90640);  //���㴦��
        MLX90640_BadPixelsCorrection(mlx90640.outlierPixels, mlx90640To, 1, &mlx90640); //���㴦��

//    rt_kprintf("\r\n==========================I==========================\r\n");
//    for(int i = 0; i < 768; i++)
//    {
//        if(i % 32 == 0 && i != 0)
//        {
//            rt_kprintf("\r\n");
//        }
//        printf("%2.0f   ", mlx90640To[i]);
//    }
//    rt_kprintf("\r\n==========================I==========================\r\n");
		
//һά����ת��Ϊ��ά����
        int index = 767;
        for(int i = 0; i < 24; i++)
        {
            for(int j = 31;j>=0; j--)
            {
                src_matrix[i][j] = mlx90640To[index--];
            }
        }
        bilinear_interpolation(src_matrix, dest_matrix);
        find_max_temperature(src_matrix, dest_matrix, &max_dest_row, &max_dest_col);
        find_min_temperature(src_matrix, dest_matrix, &min_dest_row, &min_dest_col);
		
        for(int row = 0; row < 240; row++)
        {
            for(int col = 0; col < 320; col++)
            {
                // �����¶�ֵ�� RGB565 ��ʽ����ɫ
                float temperature = dest_matrix[row][col];
//                uint16_t color = calculate_color(temperature);
					uint16_t color=Temp2RGB(temperature);
                // ����Ļ�ϻ�������
                lcd_draw_pixel(col + 80,  row + 60, color);
            }
        }

        if(max_dest_row > 5 && max_dest_row < 235 && max_dest_col > 5 && max_dest_col < 315)
        {
            for(int cross_row = max_dest_row - 5; cross_row < max_dest_row + 5; cross_row++)
            {
                lcd_draw_pixel(max_dest_col + 80, cross_row + 60, 0xFFFF);
            }
            for(int cross_col = max_dest_col - 5; cross_col < max_dest_col + 5; cross_col++)
            {
                lcd_draw_pixel(cross_col + 80, max_dest_row + 60, 0xFFFF);
            }
        }
        if(min_dest_row > 5 && min_dest_row < 235 && min_dest_col > 5 && min_dest_col < 315)
        {
            for(int cross_row = min_dest_row - 5; cross_row < min_dest_row + 5; cross_row++)
            {
                lcd_draw_pixel(min_dest_col + 80, cross_row + 60, 0xFFFF);
            }
            for(int cross_col = min_dest_col - 5; cross_col < min_dest_col + 5; cross_col++)
            {
                lcd_draw_pixel(cross_col + 80, min_dest_row + 60, 0xFFFF);
            }
        }
        LCD_ShowString(30, 315, "MAX_temperature:", BLACK, BLACK, 32, 1);
        LCD_ShowFloatNum1(325, 320, max_temperature, 2, BLACK, BLACK, 24);
        LCD_ShowIntNum(300, 320, max_temperature, 2, BLACK, BLACK, 24);

        LCD_ShowString(30, 0, "MIN_temperature:", BLACK, BLACK, 32, 1);
        LCD_ShowFloatNum1(325, 5, min_temperature, 2, BLACK, BLACK, 24);
        LCD_ShowIntNum(300, 5, min_temperature, 2, BLACK, BLACK, 24);
		rt_thread_mdelay(5);
		SCB_CleanDCache();
    }
}

int thread_sample(void)
{
    /* �����߳� 1�������� thread1������� thread1_entry*/
    tid1 = rt_thread_create("thread1",
                            thread1_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);

    /* �������߳̿��ƿ飬��������߳� */
    if(tid1 != RT_NULL)
        rt_thread_startup(tid1);
}
INIT_DEVICE_EXPORT(thread_sample);
MSH_CMD_EXPORT(thread_sample, thread sample);


/******************************************************************************
      ����˵������ʾ�����ַ�
      ������ݣ�x,y��ʾ����
                num Ҫ��ʾ���ַ�
                fc �ֵ���ɫ
                bc �ֵı���ɫ
                sizey �ֺ�
                mode:  0�ǵ���ģʽ  1����ģʽ
      ����ֵ��  ��
******************************************************************************/
void LCD_ShowChar(uint16_t x, uint16_t y, uint8_t num, uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode)
{
    uint8_t temp, sizex, t, m = 0;
    uint16_t i, TypefaceNum; //һ���ַ���ռ�ֽڴ�С
    uint16_t x0 = x;
    sizex = sizey / 2;
    TypefaceNum = (sizex / 8 + ((sizex % 8) ? 1 : 0)) * sizey;
    num = num - ' '; //�õ�ƫ�ƺ��ֵ
    for(i = 0; i < TypefaceNum; i++)
    {
        if(sizey == 12)temp = ascii_1206[num][i];		   //����6x12����
        else if(sizey == 16)temp = ascii_1608[num][i];		 //����8x16����
        else if(sizey == 24)temp = ascii_2412[num][i];		 //����12x24����
        else if(sizey == 32)temp = ascii_3216[num][i];		 //����16x32����
        else return;
        for(t = 0; t < 8; t++)
        {
            if(!mode)//�ǵ���ģʽ
            {

                m++;
                if(m % sizex == 0)
                {
                    m = 0;
                    break;
                }
            }
            else//����ģʽ
            {
                if(temp & (0x01 << t))lcd_draw_pixel(x, y, fc); //��һ����
                x++;
                if((x - x0) == sizex)
                {
                    x = x0;
                    y++;
                    break;
                }
            }
        }
    }
}
/******************************************************************************
      ����˵������ʾ�ַ���
      ������ݣ�x,y��ʾ����
                *p Ҫ��ʾ���ַ���
                fc �ֵ���ɫ
                bc �ֵı���ɫ
                sizey �ֺ�
                mode:  0�ǵ���ģʽ  1����ģʽ
      ����ֵ��  ��
******************************************************************************/
void LCD_ShowString(uint16_t x, uint16_t y, const uint8_t* p, uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode)
{
    while(*p != '\0')
    {
        LCD_ShowChar(x, y, *p, fc, bc, sizey, mode);
        x += sizey / 2;
        p++;
    }
}
/******************************************************************************
      ����˵������ʾ����
      ������ݣ�m������nָ��
      ����ֵ��  ��
******************************************************************************/
uint32_t mypow(uint8_t m, uint8_t n)
{
    uint32_t result = 1;
    while(n--)result *= m;
    return result;
}

/******************************************************************************
      ����˵������ʾ��λС������
      ������ݣ�x,y��ʾ����
                num Ҫ��ʾС������
                len Ҫ��ʾ��λ��
                fc �ֵ���ɫ
                bc �ֵı���ɫ
                sizey �ֺ�
      ����ֵ��  ��
******************************************************************************/
void LCD_ShowFloatNum1(uint16_t x, uint16_t y, float num, uint8_t len, uint16_t fc, uint16_t bc, uint8_t sizey)
{
    uint8_t t, temp, sizex;
    uint16_t num1;
    sizex = sizey / 2;
    num1 = num * 100;
    for(t = 0; t < len; t++)
    {
        temp = (num1 / mypow(10, len - t - 1)) % 10;
        if(t == (len - 2))
        {
            for(int col = x - 40; col < x + 45; col++)
            {
                for(int row = y + 25; row > y; row--)
                {
                    lcd_draw_pixel(col, row, WHITE);
                }
            }
            LCD_ShowChar(x + (len - 2)*sizex, y, '.', fc, bc, sizey, 1);
            t++;
            len += 1;
        }
        LCD_ShowChar(x + t * sizex, y, temp + 48, fc, bc, sizey, 1);
    }
}

/******************************************************************************
      ����˵������ʾ��������
      ������ݣ�x,y��ʾ����
                num Ҫ��ʾ��������
                len Ҫ��ʾ��λ��
                fc �ֵ���ɫ
                bc �ֵı���ɫ
                sizey �ֺ�
      ����ֵ��  ��
******************************************************************************/
void LCD_ShowIntNum(uint16_t x, uint16_t y, float num, uint8_t len, uint16_t fc, uint16_t bc, uint8_t sizey)
{
    uint8_t t, temp;
    uint8_t enshow = 0;
    uint8_t sizex = sizey / 2;
    uint16_t num1;
    num1 = num * 100 / 100;
    for(t = 0; t < len; t++)
    {
        temp = (num1 / mypow(10, len - t - 1)) % 10;
        if(enshow == 0 && t < (len - 1))
        {
            if(temp == 0)
            {
                LCD_ShowChar(x + t * sizex, y, ' ', fc, bc, sizey, 1);
                continue;
            }
            else enshow = 1;
        }
        LCD_ShowChar(x + t * sizex, y, temp + 48, fc, bc, sizey, 1);
    }
}

