#include "backLight.h"
#include "common.h"
#include "sys/app_controller.h"

extern AppController *app_controller; // APP控制器

void setBackLight(float duty)
{
    duty = constrain(duty, 0, 1);
    ledcWrite(LCD_BL_PWM_CHANNEL, (duty * 255));
}

void setBackLight_fade(float duty)
{
    duty = constrain(duty, 0, 1);
    int target_pwm = duty * 255;
    // duty = 1 - duty;
    uint32_t current_pwm = ledcRead(LCD_BL_PWM_CHANNEL);
    if (current_pwm > target_pwm)
    {
        for (int i = current_pwm; i > target_pwm; i = i - 1)
        {
            ledcWrite(LCD_BL_PWM_CHANNEL, i);
            vTaskDelay(50);
        }
    }
    else
    {
        for (int i = current_pwm; i <= target_pwm; i = i + 1)
        {
            ledcWrite(LCD_BL_PWM_CHANNEL, i);
            vTaskDelay(50);
        }
    }
}

void auto_set_backLight()
{
    if (app_controller->sys_cfg.auto_backLight)
    {
        if (lux < 5) // 暗光环境极低亮度显示
        {
            setBackLight_fade((3 / 100.0));
        }
        else if (lux < 10)
        {
            setBackLight_fade((10 / 100.0));
        }
        else if (lux < 20)
        {
            setBackLight_fade((20 / 100.0));
        }
        else if (lux < 80)
        {
            setBackLight_fade((30 / 100.0));
        }
        else if (lux < 200)
        {
            setBackLight_fade((40 / 100.0));
        }
        else if (lux < 400)
        {
            setBackLight_fade((50 / 100.0));
        }
        else if (lux < 700)
        {
            setBackLight_fade((60 / 100.0));
        }
        else if (lux < 900)
        {
            setBackLight_fade((70 / 100.0));
        }
        else if (lux < 1200)
        {
            setBackLight_fade((90 / 100.0));
        }
        else
        {
            setBackLight_fade(1);
        }
    }
}

void get_BH1750_data()
{
    if (lightMeter.measurementReady(true))
    {
        lux = lightMeter.readLightLevel();
        if (lux < 0)
        {
            // Serial.println(F("Error condition detected"));
        }
        else
        {
            if (lux > 40000.0)
            {
                // reduce measurement time - needed in direct sun light
                if (lightMeter.setMTreg(32))
                {
                    // Serial.println(
                    //  F("Setting MTReg to low value for high light environment"));
                }
                else
                {
                    // Serial.println(
                    // F("Error setting MTReg to low value for high light environment"));
                }
            }
            else
            {
                if (lux > 10.0)
                {
                    // typical light environment
                    if (lightMeter.setMTreg(69))
                    {
                        // Serial.println(F("Setting MTReg to default value for normal light environment"));
                    }
                    else
                    {
                        // Serial.println(F("Error setting MTReg to default value for normal "
                        //                  "light environment"));
                    }
                }
                else
                {
                    if (lux <= 10.0)
                    {
                        // very low light environment
                        if (lightMeter.setMTreg(138))
                        {
                            // Serial.println(
                            //     F("Setting MTReg to high value for low light environment"));
                        }
                        else
                        {
                            // Serial.println(F("Error setting MTReg to high value for low "
                            //                  "light environment"));
                        }
                    }
                }
            }
        }
    }
}