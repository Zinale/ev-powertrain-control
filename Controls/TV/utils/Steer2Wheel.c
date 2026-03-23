#include <stdio.h>
#include <math.h>

#define STEER_RATIO (30.0f / 180.0f) //rapporto volante-ruota
#define DEAD_ZONE_DEG 2.0f //TV disattivato sotto la soglia
#define DEG_TO_RAD (3.14159265f / 180.0f)

// Parametri sensore sterzo 
#define ADC_MIN_COUNT 0
#define ADC_MAX_COUNT 4095
#define ADC_CENTER_COUNT 2048      // volante dritto 
#define ADC_CENTER_DEAD_COUNT 20   // zona morta attorno al centro ADC

// Taratura corsa volante da centro ai fine corsa
#define ADC_LEFT_LOCK_COUNT 0      // fine corsa sinistra
#define ADC_RIGHT_LOCK_COUNT 4095  // fine corsa destra
#define STEER_MAX_DEG 90.0f        // angolo volante massimo per lato

static float clampf(float value, float min_v, float max_v) {
    if (value < min_v) {
        return min_v;
    }
    if (value > max_v) {
        return max_v;
    }
    return value;
}

float adc_to_steer_deg(int adc_count) {
    float adc = (float)adc_count;
    float center = (float)ADC_CENTER_COUNT;
    float delta = adc - center;

    if (fabsf(delta) <= (float)ADC_CENTER_DEAD_COUNT) {
        return 0.0f;
    }

    if (delta > 0.0f) {
        float right_span = (float)ADC_RIGHT_LOCK_COUNT - center;
        if (right_span <= 0.0f) {
            return 0.0f;
        }
        return clampf((delta / right_span) * STEER_MAX_DEG, 0.0f, STEER_MAX_DEG);
    }

    float left_span = center - (float)ADC_LEFT_LOCK_COUNT;
    if (left_span <= 0.0f) {
        return 0.0f;
    }
    return clampf((delta / left_span) * STEER_MAX_DEG, -STEER_MAX_DEG, 0.0f);
}

void steer2wheel(float steer_angle_deg, float *wheel_angle_deg_out, float *tan_delta_out) {
    float wheel_angle_deg = steer_angle_deg * STEER_RATIO; 
    float abs_wheel_angle_deg = fabsf(wheel_angle_deg);

    if (abs_wheel_angle_deg < DEAD_ZONE_DEG) {
        *wheel_angle_deg_out = 0.0f;
        *tan_delta_out = 0.0f;
        return; // TV disattivato
    }

    // Intervalli di 1 grado: [n, n+1), tangente costante nel centro dell'intervallo.
    float interval_start_deg = floorf(abs_wheel_angle_deg);
    float center_deg = interval_start_deg + 0.5f;
    float tand = tanf(center_deg * DEG_TO_RAD);

    *wheel_angle_deg_out = copysignf(center_deg, wheel_angle_deg);
    *tan_delta_out = copysignf(tand, wheel_angle_deg);
}

int main() {
    printf("ADC range: [%d, %d], center: %d, steer max per side: %.1f deg\n",
           ADC_MIN_COUNT,
           ADC_MAX_COUNT,
           ADC_CENTER_COUNT,
           STEER_MAX_DEG);
    printf("Sensitivity near center: %.5f deg/ADC-count\n\n",
           STEER_MAX_DEG / ((float)ADC_RIGHT_LOCK_COUNT - (float)ADC_CENTER_COUNT));

    for (int adc = ADC_MIN_COUNT; adc <= ADC_MAX_COUNT; adc += 256) {
        float steer_angle = adc_to_steer_deg(adc);
        float wheel_angle_deg;
        float tan_delta;
        steer2wheel(steer_angle, &wheel_angle_deg, &tan_delta);
        printf("ADC: %4d, Steer: %7.2f deg, Wheel: %6.2f deg, tan(delta): % .4f\n",
               adc,
               steer_angle,
               wheel_angle_deg,
               tan_delta);
    }   

    // Esempio puntuale: volante circa dritto
    {
        int adc_mid = ADC_CENTER_COUNT;
        float steer_angle = adc_to_steer_deg(adc_mid);
        float wheel_angle_deg;
        float tan_delta;
        steer2wheel(steer_angle, &wheel_angle_deg, &tan_delta);
        printf("\nCenter check -> ADC: %d, Steer: %.2f deg, Wheel: %.2f deg, tan(delta): %.4f\n",
               adc_mid,
               steer_angle,
               wheel_angle_deg,
               tan_delta);
    }

    return 0;
}
