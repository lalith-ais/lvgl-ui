
/*base system to test code from squarline*/
#include "ui.h"
//#include "lvgl.h"

#include <Arduino_GFX_Library.h>

#define GFX_BL 2

// define panel parameters
Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel(
		40 /* DE */, 41 /* VSYNC */, 39 /* HSYNC */, 42 /* PCLK */,
		45 /* R0 */, 48 /* R1 */, 47 /* R2 */, 21 /* R3 */, 14 /* R4 */,
		5 /* G0 */, 6 /* G1 */, 7 /* G2 */, 15 /* G3 */, 16 /* G4 */, 4 /* G5 */,
		8 /* B0 */, 3 /* B1 */, 46 /* B2 */, 9 /* B3 */, 1 /* B4 */,
		0 /* hsync_polarity */, 1 /* hsync_front_porch */, 1 /* hsync_pulse_width */, 43 /* hsync_back_porch */,
		0 /* vsync_polarity */, 3 /* vsync_front_porch */, 1 /* vsync_pulse_width */, 12 /* vsync_back_porch */,
		1 /* pclk_active_neg */, 10000000 /* prefer_speed */);

// framebuffer
Arduino_RGB_Display *gfx = new Arduino_RGB_Display(
		480 /* width */, 272 /* height */, rgbpanel, 0 /* rotation */, true /* auto_flush */);

/*******************************************************************************
 * Please config the touch panel in touch.h
 ******************************************************************************/
#include "touch.h"

/* Change to your screen resolution */
static uint32_t screenWidth;
static uint32_t screenHeight;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *disp_draw_buf;
static lv_disp_drv_t disp_drv;

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
	uint32_t w = (area->x2 - area->x1 + 1);
	uint32_t h = (area->y2 - area->y1 + 1);

#if (LV_COLOR_16_SWAP != 0)
	gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#else
	gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#endif

	lv_disp_flush_ready(disp);
}

void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
	if (touch_has_signal())
	{
		if (touch_touched())
		{
			data->state = LV_INDEV_STATE_PR;

			/*Set the coordinates*/
			data->point.x = touch_last_x;
			data->point.y = touch_last_y;
		}
		else if (touch_released())
		{
			data->state = LV_INDEV_STATE_REL;
		}
	}
	else
	{
		data->state = LV_INDEV_STATE_REL;
	}
}

void setup()
{
	Serial.begin(115200);
	// while (!Serial);
	Serial.println("LVGL Widgets Demo");

	// Init touch device


	// Init Display
	gfx->begin();

	lv_init();
	delay(10);
	touch_init();
	screenWidth = gfx->width();
	screenHeight = gfx->height();
#ifdef ESP32
	disp_draw_buf = (lv_color_t *)heap_caps_malloc(sizeof(lv_color_t) * screenWidth * screenHeight/4 , MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
#else
	disp_draw_buf = (lv_color_t *)malloc(sizeof(lv_color_t) * screenWidth * screenHeight/4);
#endif
	if (!disp_draw_buf)
	{
		Serial.println("LVGL disp_draw_buf allocate failed!");
	}
	else
	{
		lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, NULL, screenWidth * screenHeight/4);

		/* Initialize the display */
		lv_disp_drv_init(&disp_drv);
		/* Change the following line to your display resolution */
		disp_drv.hor_res = screenWidth;
		disp_drv.ver_res = screenHeight;
		disp_drv.flush_cb = my_disp_flush;
		disp_drv.draw_buf = &draw_buf;
		lv_disp_drv_register(&disp_drv);

		/* Initialize the (dummy) input device driver */
		static lv_indev_drv_t indev_drv;
		lv_indev_drv_init(&indev_drv);
		indev_drv.type = LV_INDEV_TYPE_POINTER;
		indev_drv.read_cb = my_touchpad_read;
		lv_indev_drv_register(&indev_drv);


		ui_init();

      pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);

		Serial.println("Setup done");
	}
}

void loop()
{
	lv_timer_handler(); /* let the GUI do its work */
	delay(5);
}
