
//#include <rtthread.h>

//#include "MLX90640_API.h"

//struct grey_boundary_param
//{
//    float a,b,c,d;
//    int16_t minX100,maxX100;
//    uint16_t w,h;
//    int max_index;
//    int min_index;
//};
//static inline void grey_find_max_min(
//        int16_t *greyX100, int num,
//        int16_t *maxX100, int *maxidx,
//        int16_t *minX100, int *minidx)
//{
//    int max_index = 0;
//    int min_index = 0;
//    int16_t tmp_max = greyX100[0], tmp_min = greyX100[0];
//    for(int i=1; i<num; i++)
//    {
//        if(greyX100[i] < tmp_min)
//        {
//            tmp_min = greyX100[i];
//            min_index = i;
//        }
//        if(greyX100[i] > tmp_max)
//        {
//            tmp_max = greyX100[i];
//            max_index = i;
//        }
//    }
//    if(maxX100)
//    {
//        *maxX100 = tmp_max;
//    }
//    if(minX100)
//    {
//        *minX100 = tmp_min;
//    }
//    if(maxidx)
//    {
//        *maxidx = max_index;
//    }
//    if(minidx)
//    {
//        *minidx = min_index;
//    }
//}
//void grey_update_boundary_param(int16_t *greyX100, const uint16_t w, const uint16_t h, struct grey_boundary_param *bound)
//{
//    int16_t max, min;
//    grey_find_max_min(greyX100, w*h, &max, &bound->max_index, &min, &bound->min_index);
//    bound->a = min + (max - min) * 0.2121f;
//    bound->b = min + (max - min) * 0.3182f;
//    bound->c = min + (max - min) * 0.4242f;
//    bound->d = min + (max - min) * 0.8182f;
//    bound->w = w;
//    bound->h = h;
//    bound->minX100 = min;
//    bound->maxX100 = max;
//}
//#define constrain(v, min, max)      ((v) > (max)?(max):(v)<(min)?(min):(v))
///**
// * @brief grey2rgb
//*/
//void grey2color(int16_t *grey_p, lv_color_t *color_p, int num, struct grey_boundary_param *bound)
//{
//    int16_t minX100 = bound->minX100, maxX100 = bound->maxX100;
//    int16_t v;
//    float a = bound->a, b = bound->b, c = bound->c, d = bound->d;
//    uint8_t cr,cg,cb=0;
//    
//    for(int i=0; i<num; i++)
//    {
//        v = grey_p[i];
//        cr = constrain(255.0f / (c - b) * v - ((b * 255.0f) / (c - b)), 0, 255);
//        if((v > minX100) & (v < a))
//        {
//            cg = constrain(255.0f / (a - minX100) * v - ((minX100 * 255.0f) / (a - minX100)), 0, 255);
//        }
//        else if((v >= a) & (v <= c))
//        {
//            cg = 255;
//        }
//        else if(v > c)
//        {
//            cg = constrain(255.0f / (c - d) * v - ((d * 255.0f) / (c - d)), 0, 255);
//        }
//        else //if((v > d) | (v < a))
//        {
//            cg = 0;
//        }
//        
//        if(v <= b)
//        {
//            cb = constrain(255.0f / (a - b) * v - ((b * 255.0f) / (a - b)), 0, 255);
//        }
//        else if(v <= d) // if((v > b) & (v <= d))
//        {
//            cb = 0;
//        }
//        else //if(v > d)
//        {
//            cg = constrain(240.0f / (maxX100 - d) * v - ((d * 240.0f) / (maxX100 - d)), 0, 240);
//        }
//        color_p[i] = lv_color_hex((cr << 16) | (cg << 8) | cb);
//    }
//}
//void grey_resize(float *grey_p, uint16_t iw, uint16_t ih, int16_t *grey_op, uint16_t ow, uint16_t oh)
//{
//    float w_scale, h_scale, pos_x, pos_y, dx, dy, grey0y, grey1y, greyxy;
//    uint8_t grey00, grey01, grey10, grey11;
//    int pos_xs, pos_xe, pos_ys, pos_ye;
//    
//    w_scale = (float)(iw - 1) / (ow - 1);
//    h_scale = (float)(ih - 1) / (oh - 1);
//    
//    for(int x = 0; x < ow; x++)
//    {
//        for(int y = 0; y < oh; y++)
//        {
//            pos_x = x * w_scale;
//            pos_y = y * h_scale;
//            pos_xs = (int)pos_x;
//            pos_ys = (int)pos_y;
//            if(pos_xs == iw - 1)
//            {
//                pos_xs = iw - 2;
//            }
//            if(pos_ys == ih - 1)
//            {
//                pos_ys = ih - 2;
//            }
//            pos_xe = pos_xs + 1;
//            pos_ye = pos_ys + 1;
//            
//            dx = pos_x - pos_xs;
//            dy = pos_y - pos_ys;
//            
//            grey00 = grey_p[pos_ys * iw + pos_xs];
//            grey01 = grey_p[pos_ys * iw + pos_xe];
//            grey10 = grey_p[pos_ye * iw + pos_xs];
//            grey11 = grey_p[pos_ye * iw + pos_xe];
//            
//            grey0y = grey01 * dx + (1 - dx) * grey00;
//            grey1y = grey11 * dx + (1 - dx) * grey10;
//            greyxy = grey1y * dy + (1 - dy) * grey0y;
//            
//            grey_op[y * ow + x] = greyxy * 100;
//        }
//    }
//}
//void grey2color_resize(float *grey_p, struct grey_boundary_param *bound, lv_color_t *color_p, uint16_t ow, uint16_t oh)
//{
//    float w_scale, h_scale, pos_x, pos_y, dx, dy, grey0y, grey1y;
//    int16_t greyxyX100;
//    uint8_t grey00, grey01, grey10, grey11;
//    int pos_xs, pos_xe, pos_ys, pos_ye;
//    int iw = bound->w;
//    int ih = bound->h;
//    
//    w_scale = (float)(iw - 1) / (ow - 1);
//    h_scale = (float)(ih - 1) / (oh - 1);
//    
//    for(int x = 0; x < ow; x++)
//    {
//        for(int y = 0; y < oh; y++)
//        {
//            pos_x = x * w_scale;
//            pos_y = y * h_scale;
//            pos_xs = (int)pos_x;
//            pos_ys = (int)pos_y;
//            if(pos_xs == iw - 1)
//            {
//                pos_xs = iw - 2;
//            }
//            if(pos_ys == ih - 1)
//            {
//                pos_ys = ih - 2;
//            }
//            pos_xe = pos_xs + 1;
//            pos_ye = pos_ys + 1;
//            
//            dx = pos_x - pos_xs;
//            dy = pos_y - pos_ys;
//            
//            grey00 = grey_p[pos_ys * iw + pos_xs];
//            grey01 = grey_p[pos_ys * iw + pos_xe];
//            grey10 = grey_p[pos_ye * iw + pos_xs];
//            grey11 = grey_p[pos_ye * iw + pos_xe];
//            
//            grey0y = grey01 * dx + (1 - dx) * grey00;
//            grey1y = grey11 * dx + (1 - dx) * grey10;
//            greyxyX100 = (grey1y * dy + (1 - dy) * grey0y) * 100;
//            
//            grey2color(&greyxyX100, &color_p[y * ow + x], 1, bound);
//        }
//    }
//}
//void generateLegendData(lv_color_t *color_p, uint16_t w, uint16_t h, struct grey_boundary_param *bound)
//{
//    int16_t greyX100;
//    int16_t minX100 = bound->minX100;
//    int16_t temp_scale = (bound->maxX100 - minX100);
//    lv_color_t color;
//    temp_scale = temp_scale / w;
//    for(int i=0; i<w; i++)
//    {
//        greyX100 = minX100 + (temp_scale * i);
//        grey2color(&greyX100, &color, 1, bound);
//        for(int j = 0; j < h; j++)
//        {
//            color_p[(j * w) + i] = color;
//        }
//    }
//}
//void filterTemp(float *in, int w, int h, float *out)
//{
//    if (1) // mirror
//    {
//        for (int i = 0; i < w * h; i++)
//        {
//            out[i] = (out[i] + in[i]) / 2;
//        }
//    }
//    else
//    {
//        for (int i = 0; i < h; i++)
//            for (int j = 0; j < w; j++)
//            {
//                out[w * i + 31 - j] = (out[w * i + 31 - j] + in[w * i + j]) / 2;
//            }
//    }
//}

//#define  FPS2HZ   0x02
//#define  FPS4HZ   0x03
//#define  FPS8HZ   0x04
//#define  FPS16HZ  0x05
//#define  FPS32HZ  0x06

//#define MLX90640_ADDR       0x33
//#define RefreshRate         FPS8HZ
//#define EMMISIVITY          0.95f
//#define TA_SHIFT            8 //Default shift for MLX90640 in open air

//#define IRAY_IMG_WIDTH          32
//#define IRAY_IMG_HEIGHT         24
//static int lv_img_w = 320;//160;
//static int lv_img_h = 240;//120;
//static paramsMLX90640 mlx90640;
////static float readValues[IRAY_IMG_WIDTH*IRAY_IMG_HEIGHT];
//static float tempValues[IRAY_IMG_WIDTH*IRAY_IMG_HEIGHT];
////static int16_t *tempValuesBuff = RT_NULL;
////static uint16_t mlx90640Frame[834];
//static struct grey_boundary_param _mlx90640Bound;
///*
//void grey_boundary_param_print(void)
//{
//    int max_index = _mlx90640Bound.max_index;
//    int w = _mlx90640Bound.w;
//    int h = _mlx90640Bound.h;
//    int x = max_index % LV_IMG_WIDTH;
//    int y = max_index / LV_IMG_WIDTH;
//    rt_kprintf("max_index:%d, (%d,%d)/(%d,%d), %dx%d\r\n"
//        , max_index
//        , x, y, lv_obj_get_x(max_point.lv_text), lv_obj_get_y(max_point.lv_text)
//        , w, h);
//}
//*/
//// Read pixel data from MLX90640.
//int readTempValues(int16_t *buff)
//{
//    uint16_t *mlx90640Frame = (uint16_t *)buff;
//    if(mlx90640Frame == RT_NULL)
//    {
//        return -1;
//    }
//    for (uint8_t x = 0 ; x < 2 ; x++) // Read both subpages
//    {
//        //uint16_t mlx90640Frame[834];
//        int status = MLX90640_GetFrameData(MLX90640_ADDR, mlx90640Frame);
//        if (status < 0)
//        {
//            //rt_kprintf("GetFrame Error: %d, %d\r\n", x, status);
//            return -1;
//        }

//        MLX90640_GetVdd(mlx90640Frame, &mlx90640);
//        float Ta = MLX90640_GetTa(mlx90640Frame, &mlx90640);

//        float tr = Ta - TA_SHIFT; //Reflected temperature based on the sensor ambient temperature

//        MLX90640_CalculateTo(mlx90640Frame, &mlx90640, EMMISIVITY, tr, tempValues);
//        MLX90640_BadPixelsCorrection(mlx90640.brokenPixels, tempValues, 1, &mlx90640);
//    }
//    return 0;
//}
//void generatePicture(int16_t *buff, int w, int h)
//{
//    //
//    //filterTemp(readValues, IRAY_IMG_WIDTH, IRAY_IMG_HEIGHT, tempValues);
//    //
//    //grey_update_boundary_param(tempValues, IRAY_IMG_WIDTH, IRAY_IMG_HEIGHT, &_mlx90640Bound);
//    //grey2color_resize(tempValues, &_mlx90640Bound, img_buf, LV_IMG_WIDTH, LV_IMG_HEIGHT);
//    grey_resize(tempValues, IRAY_IMG_WIDTH, IRAY_IMG_HEIGHT, buff, w, h);
//    grey_update_boundary_param(buff, w, h, &_mlx90640Bound);
//    //grey2color(tempValuesBuff, (void *)img_dsc.data, (LV_IMG_WIDTH * LV_IMG_HEIGHT), &_mlx90640Bound);
//}
//void readMaxTempFromBuff(int16_t *buff, struct mlx90640_point_data *obuff)
//{
//    obuff[0].temp = buff[_mlx90640Bound.max_index];
//    obuff[0].x = _mlx90640Bound.max_index % _mlx90640Bound.w;
//    obuff[0].y = _mlx90640Bound.max_index / _mlx90640Bound.w;
//}
//void readMinTempFromBuff(int16_t *buff, struct mlx90640_point_data *obuff)
//{
//    obuff[0].temp = buff[_mlx90640Bound.min_index];
//    obuff[0].x = _mlx90640Bound.min_index % _mlx90640Bound.w;
//    obuff[0].y = _mlx90640Bound.min_index / _mlx90640Bound.w;
//}
//void readMaxMinTempFromBuff(int16_t *buff, struct mlx90640_point_data *obuff)
//{
//    obuff[0].temp = buff[_mlx90640Bound.max_index];
//    obuff[0].x = _mlx90640Bound.max_index % _mlx90640Bound.w;
//    obuff[0].y = _mlx90640Bound.max_index / _mlx90640Bound.w;
//    obuff[1].temp = buff[_mlx90640Bound.min_index];
//    obuff[1].x = _mlx90640Bound.min_index % _mlx90640Bound.w;
//    obuff[1].y = _mlx90640Bound.min_index / _mlx90640Bound.w;
//}
//void readTempFromBuff(int16_t *buff, struct mlx90640_point_data *obuff, uint16_t num)
//{
//    if(num > 5)
//    {
//        num = 5;
//    }
//    int w = _mlx90640Bound.w;
//    for(int i=0; i<num; i++)
//    {
//        obuff[i].temp = buff[obuff[i].y * w + obuff[i].x];
//    }
//}
//void pictureConvert(int16_t *buff, int num, lv_color_t *color_p)
//{
//    grey2color(buff, color_p, num, &_mlx90640Bound);
//}
//void tempSensorInit(int16_t *buff)
//{
//    uint16_t *eeMLX90640 = (uint16_t *)buff;
//    int status = MLX90640_DumpEE(MLX90640_ADDR, eeMLX90640);
//    if (status != 0) rt_kprintf("\r\nload system parameters error with code:%d\r\n",status);
//    status = MLX90640_ExtractParameters(eeMLX90640, &mlx90640);
//    if (status != 0) rt_kprintf("\r\nParameter extraction failed with error code:%d\r\n",status);
//    MLX90640_SetRefreshRate(MLX90640_ADDR, RefreshRate);
//    //MLX90640_SetChessMode(MLX90640_ADDR);
//}
