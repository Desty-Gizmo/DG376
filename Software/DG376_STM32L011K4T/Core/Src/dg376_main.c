/**
   @file dg376_main.c
   @brief Actual main program body
   @author LanternD
   @date_created 2020.06.20
   @note The main.c is always subject to modification by the CubeMX code
   generator. I would like to move the user added functions here to keep all my
   code consistent.
 */

#include "dg376_main.h"
#include "main.h"

/* Global variables **********************************************************/
timer_choice_t g_timer_choice_st = {0, 0, 0, 0};
notification_choice_t g_notification_st = {0, 0};
dg376_status_et g_dg376_status = STATE_IDLE;

int16_t remain_timer = 0;
uint8_t led_on_count = 0; // 0 - 5

/* Functions *****************************************************************/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == CNT_START_Pin) {
    g_dg376_status = STATE_TIMER_ON;
    read_timer_choice(&g_timer_choice_st);
    remain_timer += calculate_incremental_step(g_timer_choice_st);
    if (led_on_count < 5){
      led_on_count += 1;
      turn_on_led_based_on_count();
    }

  } else if (GPIO_Pin == CNT_RST_Pin) {
    g_dg376_status = STATE_IDLE;
    // Switch to IDLE no matter what previous status is.
    switch (g_dg376_status) {
    case STATE_NOTIFYING:
      // Placeholder
      break;
    case STATE_TIMER_ON:
      turn_off_all_led();
      led_on_count = 0;
      remain_timer = 0;
      break;
    default:
      // Placeholder
      break;
    }
  }
}

void read_timer_choice(timer_choice_t *g_timer_st) {
  g_timer_st->s_10_enable =
      HAL_GPIO_ReadPin(S_10_SELECT_GPIO_Port, S_10_SELECT_Pin);
  g_timer_st->s_20_enable =
      HAL_GPIO_ReadPin(S_20_SELECT_GPIO_Port, S_20_SELECT_Pin);
  g_timer_st->s_30_enable =
      HAL_GPIO_ReadPin(S_30_SELECT_GPIO_Port, S_30_SELECT_Pin);
  g_timer_st->s_60_enable =
      HAL_GPIO_ReadPin(S_60_SELECT_GPIO_Port, S_60_SELECT_Pin);
}

void read_notification_choice(notification_choice_t *g_notification_st) {
  g_notification_st->rgb_enable =
      HAL_GPIO_ReadPin(RGB_LED_EN_GPIO_Port, RGB_LED_EN_Pin);
  g_notification_st->vibration_enable =
      HAL_GPIO_ReadPin(VIBRATION_EN_GPIO_Port, VIBRATION_EN_Pin);
}

uint8_t calulate_incremental_step(timer_choice_t *g_timer_choice_st) {
  uint8_t res = 0;
  res += 10 * g_timer_choice_st->s_10_enable +
         20 * g_timer_choice_st->s_20_enable +
         30 * g_timer_choice_st->s_30_enable +
         60 * g_timer_choice_st->s_60_enable;
  if (res == 0) {
    return 1;
  } else {
    return res;
  }
}

void turn_off_all_led(void) {
  LED0_OFF;
  LED1_OFF;
  LED2_OFF;
  LED3_OFF;
  LED4_OFF;
}

void turn_on_led_based_on_count(void) {
  turn_off_all_led();
  switch (led_on_count) {
  case 0:
    break;
  case 1:
    LED0_ON;
    break;
  case 2:
    LED0_ON;
    LED1_ON;
    break;
  case 3:
    LED0_ON;
    LED1_ON;
    LED2_ON;
    break;
  case 4:
    LED0_ON;
    LED1_ON;
    LED2_ON;
    LED3_ON;
    break;
  case 5:
    LED0_ON;
    LED1_ON;
    LED2_ON;
    LED3_ON;
    LED4_ON;
    break;
  default:
    break;
  }
}

void start_notification(void) {
  read_notification_choice(&g_notification_st);
  uint8_t notification_cycle_count = 0;
  if (g_notification_st.rgb_enable == 0 &&
      g_notification_st.vibration_enable == 0) {
    // Use LED Blinking to notify the user.
    LED0_ON;
    LED1_OFF;
    LED2_ON;
    LED3_OFF;
    LED4_ON;
    while (g_dg376_status == STATE_NOTIFYING && notification_cycle_count < 40) {
      LED0_TOGGLE;
      LED1_TOGGLE;
      LED2_TOGGLE;
      LED3_TOGGLE;
      LED4_TOGGLE;
      HAL_Delay(250);
      notification_cycle_count += 1;
    }
    turn_off_all_led();
  } else {
    while (g_dg376_status == STATE_NOTIFYING && notification_cycle_count < 10) {
      if (g_notification_st.rgb_enable) {
        LED_R_ON;
        LED_G_OFF;
        LED_B_OFF;
        HAL_Delay(200);
        LED_R_OFF;
        LED_G_ON;
        LED_B_OFF;
        HAL_Delay(200);
        LED_R_OFF;
        LED_G_OFF;
        LED_B_ON;
        HAL_Delay(200);
      }
      if (g_notification_st.vibration_enable) {
        MOTOR_ON;
        HAL_Delay(400);
        MOTOR_OFF;
      }
      notification_cycle_count += 1;
    }
  }
  g_dg376_status = STATE_IDLE;
}

void timer_expired(void){
  g_dg376_status = STATE_NOTIFYING;
  start_notification();
}

void dg376_while_loop(void){
  while (1) {
    if (g_dg376_status == STATE_TIMER_ON){
      HAL_Delay(1000);
      remain_timer -= 1;
      if (remain_timer <= 0){
        timer_expired();
      }
    }
  }
}
