#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include "cJSON.h"
#include "pso.h"
#include "menu.h"

void exportResultJSON(const char* path){
    if(!uncoveredSolution
                 || !sensorPoints || !sensorRange
                 || !sensorCost){
        exit(0);
    }


    cJSON* root = cJSON_CreateObject();

    cJSON_AddNumberToObject(root, "pso_value", psoValue);
    cJSON_AddNumberToObject(root, "sensor_cost", totalCostSolution);
    cJSON_AddNumberToObject(root, "uncovered_points", uncoveredSolution);

    cJSON* sensorsArr = cJSON_AddArrayToObject(root, "sensors");

    for (uint32_t i = 0; i < sensorPointsCount; i++) {

        int type = psoSolution[i];
        if(type == 0){
            continue;
        }
        cJSON* s = cJSON_CreateObject();


        cJSON_AddNumberToObject(s, "x", sensorPoints[i].x);
        cJSON_AddNumberToObject(s, "y", sensorPoints[i].y);
        cJSON_AddNumberToObject(s, "type", type);

        cJSON_AddNumberToObject(s, "range", sensorRange[type]);
        cJSON_AddNumberToObject(s, "cost", sensorCost[type]);

        cJSON_AddItemToArray(sensorsArr, s);
    }

    cJSON* covArr = cJSON_AddArrayToObject(root, "coverage");
    for (uint32_t i = 0; i < pointsCount; i++) {
        cJSON_AddItemToArray(covArr, cJSON_CreateNumber(coverSolution[i]));
    }

    char* text = cJSON_Print(root);

    FILE* f = fopen(path, "w");
    if (f) {
        fputs(text, f);
        fclose(f);
    }

    cJSON_free(text);
    cJSON_Delete(root);
}