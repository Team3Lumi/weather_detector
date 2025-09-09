#if 1
    cJSON *times      = cJSON_GetObjectItemCaseSensitive(daily, "time");
    cJSON *temp_max   = cJSON_GetObjectItemCaseSensitive(daily, "temperature_2m_max");
    cJSON *rain_prob  = cJSON_GetObjectItemCaseSensitive(daily, "precipitation_probability_mean");
    cJSON *weathercode= cJSON_GetObjectItemCaseSensitive(daily, "weathercode");
    cJSON *hourly = cJSON_GetObjectItemCaseSensitive(json, "hourly");
    cJSON *humi_hourly = cJSON_GetObjectItemCaseSensitive(hourly, "relative_humidity_2m");
    cJSON *time_hourly = cJSON_GetObjectItemCaseSensitive(hourly, "time");

    if (!cJSON_IsArray(times) || !cJSON_IsArray(temp_max) ||
        !cJSON_IsArray(rain_prob) || !cJSON_IsArray(weathercode)) {
        ESP_LOGW(TAG, "Missing expected arrays in 'daily'");
        cJSON_Delete(json); free(resp);
        return;
    }

    if (!cJSON_IsArray(humi_hourly) || !cJSON_IsArray(time_hourly)) {
        ESP_LOGW(TAG, "Missing hourly humidity/time");
        cJSON_Delete(json); free(resp);
        return;
    }

    int n = cJSON_GetArraySize(times);  // số ngày daily
    if (n > FORECAST_DAYS) n = FORECAST_DAYS;
    int humi_per_day = 24;  // vì hourly=relative_humidity_2m (theo giờ)

    for (int i = 0; i < n; ++i) {
    cJSON *ti = cJSON_GetArrayItem(times, i);
    cJSON *tm = cJSON_GetArrayItem(temp_max, i);
    cJSON *rp = cJSON_GetArrayItem(rain_prob, i);
    cJSON *wc = cJSON_GetArrayItem(weathercode, i);

    const char *date = (cJSON_IsString(ti) && ti->valuestring) ? ti->valuestring : "N/A";
    double tmax = cJSON_IsNumber(tm) ? tm->valuedouble : 0.0;
    double rprob= cJSON_IsNumber(rp) ? rp->valuedouble : 0.0;
    int wcode   = cJSON_IsNumber(wc) ? wc->valueint   : 0;


    int n = cJSON_GetArraySize(times);
    // đảm bảo đồng bộ các mảng
    n = (n < cJSON_GetArraySize(temp_max))    ? n : cJSON_GetArraySize(temp_max);
    n = (n < cJSON_GetArraySize(rain_prob))   ? n : cJSON_GetArraySize(rain_prob);
    n = (n < cJSON_GetArraySize(weathercode)) ? n : cJSON_GetArraySize(weathercode);
    if (n > FORECAST_DAYS) n = FORECAST_DAYS;

        // --- Tính trung bình độ ẩm 24h cho ngày i ---
    double humi_avg = 0;
    int valid = 0;
    for (int h = 0; h < humi_per_day; h++) {
        int idx = i * humi_per_day + h;  // vị trí trong mảng hourly
        cJSON *hv = cJSON_GetArrayItem(humi_hourly, idx);
        if (cJSON_IsNumber(hv)) {
            humi_avg += hv->valuedouble;
            valid++;
        }
    }
    if (valid > 0) humi_avg /= valid;

    ESP_LOGI(TAG, "=== 7-day forecast (up to %d days) ===", n);
    for (int i = 0; i < n; ++i) {
        cJSON *ti = cJSON_GetArrayItem(times, i);
        cJSON *tm = cJSON_GetArrayItem(temp_max, i);
        cJSON *rp = cJSON_GetArrayItem(rain_prob, i);
        cJSON *wc = cJSON_GetArrayItem(weathercode, i);

        const char *date = (cJSON_IsString(ti) && ti->valuestring) ? ti->valuestring : "N/A";
        double tmax = cJSON_IsNumber(tm) ? tm->valuedouble : 0.0;
        double rprob= cJSON_IsNumber(rp) ? rp->valuedouble : 0.0;
        int wcode   = cJSON_IsNumber(wc) ? wc->valueint   : 0;

        ESP_LOGI(TAG, "[%d] %s | Tmax=%.1f C | Rain=%.1f %% | Hum=%.1f %% | code=%d",
                 i, date, tmax, rprob, humi_avg, wcode);
#endif
